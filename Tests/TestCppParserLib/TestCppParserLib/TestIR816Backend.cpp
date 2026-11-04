#include <string.h>
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "Assembler65Lib/Asm65816Assembler.h"
#include "Assembler65Lib/Asm65816Lockstep.h"
#include "Assembler65Lib/Asm65Linker.h"
#include "Assembler65Lib/Asm65Library.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "CppParserLib/IRParser.h"
#include "CppParserLib/IRVerifier.h"
#include "CppParserLib/IR816Backend.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The W65C816 back end (CIR816Backend), on IR programs: each runs on CFast65C816 (some in lockstep with the
// CW65C816 simulator too) and must give the result expected and the 65C64 back end's.
//
//////////////////////////////////////////////////////////////////////////


#define TEST_IR816_STEPS		50000000		//The most steps a test program may take.
#define TEST_IR816_MEMORY		0x100000
#define TEST_IR816_STACK_TOP	0xEFFF
#define TEST_IR816_MATH_TABLES	".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65" _FS_ "MathTbls"


int64 TestIR65Run(const char* szIR, bool bFloat, CChars* pszAssembly);


//The start up: the reset vector and its jml stub in bank 0, then main's result (A and X, or 8 bytes through the
//hidden pointer) is stored at Result and the processor stops.
static const char* gszTestIR816StartUp =
	"\t\t.section rodata.Vectors, at 0xFFFC\n"
	"\t\t.word ResetStub\n"
	"\t\t.section text.ResetStub, at 0xFF00\n"
	"ResetStub:\n"
	"\t\tjml Start\n"
	"\t\t.section text.Start\n"
	"Start:\n"
	"\t\tclc\n"
	"\t\txce\n"
	"\t\trep (m, x)\n"
	"\t\tlda 0xEFFF\n"
	"\t\ttcs\n"
	"\t\tlda 0\n"
	"\t\ttcd\n"
	"\t\tjsl _main\n"
	"RESULT"
	"\t\tstp\n"
	"\t\t.section bss.Result\n"
	".global Result\n"
	"Result:\n"
	"\t\t.space 8\n";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestIR816Assemble(CAsm65Object* pcObject, const char* szName, const char* szSource)
{
	CAsm65816Assembler	cAssembler;
	bool				bResult;

	pcObject->Init(szName);
	cAssembler.Init();
	bResult = cAssembler.AssembleObject(szSource, pcObject);
	AssertString("", cAssembler.GetErrors());
	cAssembler.Kill();
	return bResult;
}


//The maths tables the runtime uses (MathMult for multiply, MathInvert for divide), assembled once and linked when used.
static CAsm65Library*	gpcTestIR816MathTables = NULL;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static CAsm65Library* TestIR816MathTables(void)
{
	static const char*	aszTables[] = { "MathMult.asm", "MathInvert.asm", NULL };
	CTextFile			cFile;
	CChars				szPath;
	CAsm65816Assembler	cAssembler;
	size				i;

	if (gpcTestIR816MathTables)
	{
		return gpcTestIR816MathTables;
	}
	gpcTestIR816MathTables = NewMalloc<CAsm65Library>();
	gpcTestIR816MathTables->Init("MathTables");
	for (i = 0; aszTables[i]; i++)
	{
		szPath.Init(TEST_IR816_MATH_TABLES _FS_);
		szPath.Append(aszTables[i]);
		cFile.Init();
		AssertTrue(cFile.Read(szPath.Text()));
		cFile.PassifyNewlines();
		cAssembler.Init();
		cAssembler.SetDirectory(TEST_IR816_MATH_TABLES);
		AssertTrue(cAssembler.AssembleObject(cFile.Text(), gpcTestIR816MathTables->AddObject(aszTables[i])));
		AssertString("", cAssembler.GetErrors());
		cAssembler.Kill();
		cFile.Kill();
		szPath.Kill();
	}
	return gpcTestIR816MathTables;
}


//////////////////////////////////////////////////////////////////////////
//
// Parses and verifies szIR, generates it (and the runtime routines it uses) with the W65C816 back end, links it
// with the start up and runs it.  Returns main's result, sign extended from its type.
//
//////////////////////////////////////////////////////////////////////////
int64 TestIR816Run(const char* szIR, bool bLockstep, CChars* pszAssembly = NULL)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRVerifier		cVerifier;
	CIR816Backend	cBackend;
	CChars			szCode;
	CChars			szRuntime;
	CChars			szStartUp;
	int				iLabels;
	CAsm65Object	acObjects[3];
	CAsm65Linker	cLinker;
	CAsm65Memory	cMemory;
	CFast65C816		cFast;
	CAsm65816Lockstep	cLockstep;
	CIRFunction*	pcMain;
	SIRType			sReturn;
	int64			iResult;
	int64			lliResult;
	size			uiSteps;
	bool			bWide;
	int				i;

	cModule.Init();
	cModule.AddSpace("default", 4);
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	cParser.Parse(szIR);
	cParser.Kill();
	cVerifier.Init(&cModule, &cErrors);
	cVerifier.Verify();
	cVerifier.Kill();
	AssertString("", cErrors.Text());

	pcMain = cModule.FindFunction((uint32)cModule.FindSymbol("_main"));
	sReturn = pcMain ? pcMain->msReturnType : IRType(IRTK_I32);
	bWide = sReturn.Is(IRTK_I64);

	szCode.Init();
	szRuntime.Init();
	iLabels = 0;
	cBackend.Init(&cModule, &iLabels);
	AssertTrue(cBackend.GenerateModule(&szCode, &cErrors));
	AssertTrue(CIR816Backend::GenerateRuntime(cBackend.GetRuntime(), &iLabels, &szRuntime, &cErrors));
	AssertString("", cErrors.Text());
	cBackend.Kill();
	if (pszAssembly)
	{
		pszAssembly->Append(szCode);
	}

	szStartUp.Init(gszTestIR816StartUp);
	szStartUp.Replace("\t\tjsl", bWide ? "\t\tpea Result >> 16\n\t\tpea Result & 0xFFFF\n\t\tjsl" : "\t\tjsl");
	szStartUp.Replace("RESULT", bWide ? "" : "\t\tsta [Result]:24\n\t\ttxa\n\t\tsta [Result + 2]:24\n");
	lliResult = -1;
	if (TestIR816Assemble(&acObjects[0], "startup", szStartUp.Text()) &&
		TestIR816Assemble(&acObjects[1], "ir", szCode.Text()) &&
		TestIR816Assemble(&acObjects[2], "runtime", szRuntime.Text()))
	{
		cLinker.Init();
		cLinker.SetBankSize(0x10000);
		cLinker.SetAddress(ASK65_Text, 0x010000);
		for (i = 0; i < 3; i++)
		{
			cLinker.AddObject(&acObjects[i]);
		}
		cLinker.AddLibrary(TestIR816MathTables());
		AssertTrue(cLinker.Link());
		AssertString("", cLinker.GetErrors());
		AssertTrue(cLinker.GetSymbol("Result", &iResult));

		cMemory.Init(TEST_IR816_MEMORY);
		cMemory.Load(cLinker.GetOutput());
		cMemory.InitFast65816(&cFast);
		cFast.Reset();
		if (bLockstep)
		{
			cLockstep.Init(&cMemory, &cFast);
			uiSteps = cLockstep.Run(TEST_IR816_STEPS, 64);
			AssertString("", cLockstep.GetDifferences());
			cLockstep.Kill();
		}
		else
		{
			uiSteps = cFast.Run(TEST_IR816_STEPS);
		}
		AssertTrue(uiSteps < TEST_IR816_STEPS);
		AssertTrue(cFast.IsStopped());
		AssertIntHex(bWide ? TEST_IR816_STACK_TOP - 4 : TEST_IR816_STACK_TOP, cFast.GetS());
		AssertInt(0, (int)cMemory.NumBadAccesses());
		lliResult = (int64)cMemory.GetQuad((uint32)iResult);
		if (!bWide)
		{
			lliResult = IRSignExtend(sReturn, (uint64)lliResult);
		}
		cFast.Kill();
		cMemory.Kill();
		cLinker.Kill();
	}
	for (i = 0; i < 3; i++)
	{
		acObjects[i].Kill();
	}
	szStartUp.Kill();
	szRuntime.Kill();
	szCode.Kill();
	cErrors.Kill();
	cModule.Kill();
	return lliResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The expected result, the W65C816's and the 65C64's.
//
//////////////////////////////////////////////////////////////////////////
static void AssertIR816(int64 lliExpected, const char* szIR, bool bLockstep = false)
{
	int64	lliResult65;

	AssertLong(lliExpected, TestIR816Run(szIR, bLockstep));
	lliResult65 = TestIR65Run(szIR, false, NULL);
	if (strstr(szIR, "function i64 @_main") == NULL)
	{
		lliResult65 = (int32)lliResult65;
	}
	AssertLong(lliExpected, lliResult65);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendBasics(void)
{
	CChars	szAssembly;

	szAssembly.Init();
	AssertLong(42, TestIR816Run("function i32 @_main()\n{\nb:\n\tret i32 42\n}\n", true, &szAssembly));
	AssertString(
		"\t\t.section text._main\n"
		"\t\t.global _main\n"
		"_main:\n"
		"\t\t.width (A, X), 16\n"
		"\t\t.relax\n"
		"\t\tldx 0\n"
		"\t\tlda 42\n"
		"\t\trtl\n"
		"\n", szAssembly.Text());
	szAssembly.Kill();

	AssertIR816(-7, "function i32 @_main()\n{\nb:\n\t%x = sub i32 3, 10\n\tret i32 %x\n}\n", true);
	AssertIR816(0x12345678 ^ 0x0F0F0F0F, "function i32 @_main()\n{\nb:\n\t%x = xor i32 305419896, 252645135\n\tret i32 %x\n}\n");
	AssertIR816(-305419897, "function i32 @_main()\n{\nb:\n\t%x = not i32 305419896\n\tret i32 %x\n}\n");
	AssertIR816(-100000, "function i32 @_main()\n{\nb:\n\t%x = neg i32 100000\n\tret i32 %x\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendLoops(void)
{
	AssertIR816(150,
		"global @_gTable, align 4 { i32 10, i32 20, i32 30, i32 40, i32 50 }\n"
		"\n"
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i32 [entry, 0], [body, %s2]\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%o = shl i32 %i, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp loop\n"
		"exit:\n\tret i32 %s\n"
		"}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Sum__FPii(ptr @_gTable, i32 5)\n"
		"\tret i32 %r\n}\n", true);

	//Phis that swap (and rotate) must all be read before any is written.
	AssertIR816(1 * 1000 + 2,
		"function i32 @_main()\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%a = phi i32 [entry, 1], [loop, %b]\n"
		"\t%b = phi i32 [entry, 2], [loop, %a]\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%c = ult i32 %i2, 5\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n"
		"\t%m = mul i32 %a, 1000\n"
		"\t%r = add i32 %m, %b\n"
		"\tret i32 %r\n}\n");
	AssertIR816(1 * 100 + 2 * 10 + 3,
		"function i32 @_main()\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%a = phi i32 [entry, 1], [loop, %b]\n"
		"\t%b = phi i32 [entry, 2], [loop, %c]\n"
		"\t%c = phi i32 [entry, 3], [loop, %a]\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%k = ult i32 %i2, 4\n"
		"\tbrif %k, loop, exit\n"
		"exit:\n"
		"\t%a1 = mul i32 %a, 100\n"
		"\t%b1 = mul i32 %b, 10\n"
		"\t%s = add i32 %a1, %b1\n"
		"\t%r = add i32 %s, %c\n"
		"\tret i32 %r\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// i1, i8 and i16 wrap, extend and compare by their own widths.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendNarrow(void)
{
	AssertIR816(-56, "function i32 @_main()\n{\nb:\n\t%t = trunc i32 200 to i8\n\t%s = sext i8 %t to i32\n\tret i32 %s\n}\n");
	AssertIR816(200, "function i32 @_main()\n{\nb:\n\t%t = trunc i32 200 to i8\n\t%s = zext i8 %t to i32\n\tret i32 %s\n}\n");
	AssertIR816(44, "function i32 @_main()\n{\nb:\n\t%a = add i8 100, 200\n\t%s = zext i8 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(-2, "function i32 @_main()\n{\nb:\n\t%a = ashr i16 -8, 2\n\t%s = sext i16 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(16382, "function i32 @_main()\n{\nb:\n\t%a = lshr i16 -8, 2\n\t%s = zext i16 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(-4, "function i32 @_main()\n{\nb:\n\t%a = ashr i8 -16, 2\n\t%s = sext i8 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(1, "function i32 @_main()\n{\nb:\n\t%c = slt i8 -1, 1\n\t%s = zext i1 %c to i32\n\tret i32 %s\n}\n");
	AssertIR816(0, "function i32 @_main()\n{\nb:\n\t%c = ult i8 -1, 1\n\t%s = zext i1 %c to i32\n\tret i32 %s\n}\n");

	//Signed i8 and i16 comparisons of parameters: -1 < 1, 1 > -1, -2 <= -2, -32768 < 32767, 100 > -1, 127 >= -128.
	AssertIR816(111111,
		"function i32 @_C__Fcc(i8 %a, i8 %b, i16 %c, i16 %d), local\n{\nb:\n"
		"\t%1 = slt i8 %a, %b\n\t%2 = sgt i8 %b, %a\n\t%3 = sle i8 %a, %a\n\t%4 = slt i16 %c, %d\n"
		"\t%5 = sgt i8 %b, -1\n\t%6 = sge i8 %b, %a\n"
		"\t%z1 = zext i1 %1 to i32\n\t%z2 = zext i1 %2 to i32\n\t%z3 = zext i1 %3 to i32\n\t%z4 = zext i1 %4 to i32\n\t%z5 = zext i1 %5 to i32\n\t%z6 = zext i1 %6 to i32\n"
		"\t%m1 = mul i32 %z1, 100000\n\t%m2 = mul i32 %z2, 10000\n\t%m3 = mul i32 %z3, 1000\n\t%m4 = mul i32 %z4, 100\n\t%m5 = mul i32 %z5, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %m3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %m5\n\t%s5 = add i32 %s4, %z6\n"
		"\tret i32 %s5\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_C__Fcc(i8 -1, i8 1, i16 -32768, i16 32767)\n\tret i32 %r\n}\n", true);

	//sext of an i1 is 0 or -1, and so compares below 0.
	AssertIR816(-1 * 10 + 1,
		"function i32 @_Flag__Fi(i32 %x), local\n{\nb:\n\t%c = ne i32 %x, 0\n\t%s = sext i1 %c to i32\n\t%n = slt i1 %c, 0\n\t%z = zext i1 %n to i32\n\t%m = mul i32 %s, 10\n\t%r = add i32 %m, %z\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_Flag__Fi(i32 5)\n\tret i32 %r\n}\n");

	//32 bit compares: INT_MIN > 0 is false, INT_MIN <= 0 true, 5 > 0 true, -1 > -1 false; unsigned too.
	AssertIR816(1101,
		"function i32 @_P__Fiii(i32 %a, i32 %b, i32 %c), local\n{\nb:\n"
		"\t%1 = sgt i32 %a, 0\n\t%2 = sle i32 %a, 0\n\t%3 = sgt i32 %b, 0\n\t%4 = sgt i32 %c, -1\n\t%5 = ugt i32 %a, %b\n"
		"\t%z1 = zext i1 %1 to i32\n\t%z2 = zext i1 %2 to i32\n\t%z3 = zext i1 %3 to i32\n\t%z4 = zext i1 %4 to i32\n\t%z5 = zext i1 %5 to i32\n"
		"\t%m1 = mul i32 %z1, 10000\n\t%m2 = mul i32 %z2, 1000\n\t%m3 = mul i32 %z3, 100\n\t%m4 = mul i32 %z4, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %m3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %z5\n\tret i32 %s4\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_P__Fiii(i32 -2147483648, i32 5, i32 -1)\n\tret i32 %r\n}\n");

	//A three way compare, twice.
	AssertIR816(-99 * 2,
		"function i32 @_Cmp__Fcc(i8 %a, i8 %b), local\n{\nx:\n\t%l = slt i8 %a, %b\n\tbrif %l, y, z\ny:\n\tret i32 -1\n"
		"z:\n\t%g = ne i8 %a, %b\n\tbrif %g, w, v\nw:\n\tret i32 1\nv:\n\tret i32 0\n}\n\n"
		"function i32 @_Cmp__Fii(i32 %a, i32 %b), local\n{\nx:\n\t%l = slt i32 %a, %b\n\tbrif %l, y, z\ny:\n\tret i32 -1\n"
		"z:\n\t%g = ne i32 %b, %a\n\tbrif %g, w, v\nw:\n\tret i32 1\nv:\n\tret i32 0\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%1 = call i32 @_Cmp__Fcc(i8 -2, i8 2)\n\t%2 = call i32 @_Cmp__Fcc(i8 -2, i8 -2)\n\t%3 = call i32 @_Cmp__Fcc(i8 3, i8 -2)\n"
		"\t%4 = call i32 @_Cmp__Fii(i32 1, i32 2)\n\t%5 = call i32 @_Cmp__Fii(i32 2, i32 2)\n\t%6 = call i32 @_Cmp__Fii(i32 3, i32 2)\n"
		"\t%m1 = mul i32 %1, 100\n\t%m2 = mul i32 %2, 10\n\t%m4 = mul i32 %4, 100\n\t%m5 = mul i32 %5, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %m5\n\t%s5 = add i32 %s4, %6\n"
		"\tret i32 %s5\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime routines: multiply, divide and remainder at 8, 16, 32 and 64 bits.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendDivide(void)
{
	AssertIR816(-3, "function i32 @_main()\n{\nb:\n\t%a = sdiv i32 -7, 2\n\tret i32 %a\n}\n", true);
	AssertIR816(-1, "function i32 @_main()\n{\nb:\n\t%a = srem i32 -7, 2\n\tret i32 %a\n}\n");
	AssertIR816(2147483644, "function i32 @_main()\n{\nb:\n\t%a = udiv i32 -7, 2\n\tret i32 %a\n}\n");
	AssertIR816(1, "function i32 @_main()\n{\nb:\n\t%a = urem i32 -7, 2\n\tret i32 %a\n}\n");
	AssertIR816(-14, "function i32 @_main()\n{\nb:\n\t%a = sdiv i8 -100, 7\n\t%s = sext i8 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(-2, "function i32 @_main()\n{\nb:\n\t%a = srem i8 -100, 7\n\t%s = sext i8 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(-4681, "function i32 @_main()\n{\nb:\n\t%a = sdiv i16 -32768, 7\n\t%s = sext i16 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(65535 / 3, "function i32 @_main()\n{\nb:\n\t%a = udiv i16 -1, 3\n\t%s = zext i16 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(4000000000U % 3000000000U, "function i32 @_main()\n{\nb:\n\t%a = urem i32 4000000000, 3000000000\n\tret i32 %a\n}\n");
	AssertIR816(39, "function i32 @_main()\n{\nb:\n\t%a = mul i8 13, 3\n\t%s = zext i8 %a to i32\n\tret i32 %s\n}\n");
	AssertIR816(1234567 * 11 + 1234567 * 255 + 217,
		"function i32 @_B__Fic(i32 %a, i8 %c), local\n{\nb:\n"
		"\t%1 = mul i32 %a, 11\n\t%2 = mul i32 %a, 255\n\t%3 = mul i8 %c, -3\n\t%z = zext i8 %3 to i32\n"
		"\t%s1 = add i32 %1, %2\n\t%s2 = add i32 %s1, %z\n\tret i32 %s2\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_B__Fic(i32 1234567, i8 13)\n\tret i32 %r\n}\n");
	AssertIR816(123 * 1000 + 4 + 34 * 1000 + 12,
		"function i32 @_D__Fjj(i32 %a, i32 %b, i32 %c), local\n{\nb:\n"
		"\t%q = udiv i32 %a, %b\n\t%m = mul i32 %q, 1000\n\t%r = urem i32 %a, %b\n\t%s = add i32 %m, %r\n"
		"\t%r2 = urem i32 %a, %c\n\t%m2 = mul i32 %r2, 1000\n\t%q2 = udiv i32 %a, %c\n\t%s2 = add i32 %m2, %q2\n"
		"\t%t = add i32 %s, %s2\n\tret i32 %t\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_D__Fjj(i32 1234, i32 10, i32 100)\n\tret i32 %r\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime divide's paths (R816_UDiv32 and R816_UDiv64 in IR816Backend.cpp), with values passed in so they aren't
// worked out when compiled: both under 65536, a divisor under 65536 with a dividend's high word below it and not
// (and a divisor of 65535, which carries out of A), a dividend below the divisor, a divisor of 1, a dividend under 256,
// a wide divisor with a narrow dividend and with a wide one; 64
// bits with zero words at the top of the dividend skipped and not.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendDividePaths(void)
{
	static const uint32	auiPairs32[][2] = { { 60000, 7 }, { 0x0003FFFF, 10 }, { 4000000000U, 7 }, { 0xFFFFFFFF, 0xFFFF }, { 0xFFFFFFFF, 0x8001 },
											{ 1234, 100000 }, { 4000000000U, 70000 }, { 0xFFFFFFFF, 0x10000 }, { 65535, 65535 }, { 0x0000FFFF, 0x0000FFFF },
											{ 5, 9 }, { 200, 7 }, { 255, 16 }, { 40000, 1 }, { 0x12345678, 1 }, { 0x12345678, 0x1234 } };
	static const uint64	auiPairs64[][2] = { { 0xFFFFFFFFULL, 7 }, { 0x8000000000000005ULL, 3 }, { 12345, 0x100000000ULL }, { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFULL },
											{ 0x123456789ABCDEFULL, 0x10001ULL }, { 5, 5 } };
	char				sz[1024];
	size				i;
	CChars				szAssembly;

	//The values reach the routines (they aren't worked out by inlining).
	szAssembly.Init();
	snprintf(sz, sizeof(sz), "function i32 @_D__Fjj(i32 %%a, i32 %%b), local\n{\nb:\n\t%%q = udiv i32 %%a, %%b\n\tret i32 %%q\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_D__Fjj(i32 %u, i32 %u)\n\tret i32 %%r\n}\n", auiPairs32[0][0], auiPairs32[0][1]);
	AssertLong(auiPairs32[0][0] / auiPairs32[0][1], TestIR816Run(sz, false, &szAssembly));
	AssertTrue(szAssembly.Contains("jsl R816_UDiv32"));
	szAssembly.Kill();

	for (i = 0; i < sizeof(auiPairs32) / sizeof(auiPairs32[0]); i++)
	{
		snprintf(sz, sizeof(sz), "function i32 @_D__Fjj(i32 %%a, i32 %%b), local\n{\nb:\n\t%%q = udiv i32 %%a, %%b\n\tret i32 %%q\n}\n\n"
			"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_D__Fjj(i32 %u, i32 %u)\n\tret i32 %%r\n}\n", auiPairs32[i][0], auiPairs32[i][1]);
		AssertIR816((int32)(auiPairs32[i][0] / auiPairs32[i][1]), sz);
		snprintf(sz, sizeof(sz), "function i32 @_D__Fjj(i32 %%a, i32 %%b), local\n{\nb:\n\t%%q = urem i32 %%a, %%b\n\tret i32 %%q\n}\n\n"
			"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_D__Fjj(i32 %u, i32 %u)\n\tret i32 %%r\n}\n", auiPairs32[i][0], auiPairs32[i][1]);
		AssertIR816((int32)(auiPairs32[i][0] % auiPairs32[i][1]), sz);
	}
	for (i = 0; i < sizeof(auiPairs64) / sizeof(auiPairs64[0]); i++)
	{
		snprintf(sz, sizeof(sz), "function i64 @_D__Fyy(i64 %%a, i64 %%b), local\n{\nb:\n\t%%q = udiv i64 %%a, %%b\n\tret i64 %%q\n}\n\n"
			"function i64 @_main()\n{\nb:\n\t%%r = call i64 @_D__Fyy(i64 %llu, i64 %llu)\n\tret i64 %%r\n}\n", (unsigned long long)auiPairs64[i][0], (unsigned long long)auiPairs64[i][1]);
		AssertIR816((int64)(auiPairs64[i][0] / auiPairs64[i][1]), sz);
		snprintf(sz, sizeof(sz), "function i64 @_D__Fyy(i64 %%a, i64 %%b), local\n{\nb:\n\t%%q = urem i64 %%a, %%b\n\tret i64 %%q\n}\n\n"
			"function i64 @_main()\n{\nb:\n\t%%r = call i64 @_D__Fyy(i64 %llu, i64 %llu)\n\tret i64 %%r\n}\n", (unsigned long long)auiPairs64[i][0], (unsigned long long)auiPairs64[i][1]);
		AssertIR816((int64)(auiPairs64[i][0] % auiPairs64[i][1]), sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime multiply (R816_Mul16 and R816_Mul32, byte products from MathMult): 1024 products of two pseudo
// random values shifted right by every pair of amounts (so every half and byte is 0 in some), hashed.
//
//////////////////////////////////////////////////////////////////////////
static const char* gszTestIR816Multiply =
	"function $T @_M__F$M($T %a, $T %b), local\n{\nb:\n\t%m = mul $T %a, %b\n\tret $T %m\n}\n\n"
	"function i32 @_main()\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%i = phi $T [entry, 0], [body, %i2]\n\t%x = phi $T [entry, 12345], [body, %x2]\n\t%y = phi $T [entry, 6789], [body, %y2]\n\t%h = phi $T [entry, 0], [body, %h2]\n"
	"\t%done = eq $T %i, 1024\n\tbrif %done, exit, body\n"
	"body:\n"
	"\t%xm = mul $T %x, 25173\n\t%x2 = add $T %xm, 13849\n\t%ym = mul $T %y, 1103\n\t%y2 = add $T %ym, 7919\n"
	"\t%sa = and $T %i, $S\n\t%ti = lshr $T %i, 5\n\t%sb = and $T %ti, $S\n"
	"\t%a = lshr $T %x2, %sa\n\t%b = lshr $T %y2, %sb\n"
	"\t%p = call $T @_M__F$M($T %a, $T %b)\n"
	"\t%hs = shl $T %h, 5\n\t%ha = add $T %hs, %h\n\t%h2 = xor $T %ha, %p\n"
	"\t%i2 = add $T %i, 1\n\tjmp loop\n"
	"exit:\n$RET}\n";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendMultiply(void)
{
	uint32	i;
	uint32	x32;
	uint32	y32;
	uint32	h32;
	uint16	x16;
	uint16	y16;
	uint16	h16;
	CChars	szIR;
	CChars	szAssembly;

	x32 = 12345;
	y32 = 6789;
	h32 = 0;
	x16 = 12345;
	y16 = 6789;
	h16 = 0;
	for (i = 0; i < 1024; i++)
	{
		x32 = x32 * 25173 + 13849;
		y32 = y32 * 1103 + 7919;
		h32 = (h32 * 33) ^ ((x32 >> (i & 31)) * (y32 >> ((i >> 5) & 31)));
		x16 = (uint16)(x16 * 25173 + 13849);
		y16 = (uint16)(y16 * 1103 + 7919);
		h16 = (uint16)((h16 * 33) ^ (uint16)((uint16)(x16 >> (i & 15)) * (uint16)(y16 >> ((i >> 5) & 15))));
	}

	szIR.Init(gszTestIR816Multiply);
	szIR.Replace("$T", "i32");
	szIR.Replace("$M", "jj");
	szIR.Replace("$S", "31");
	szIR.Replace("$RET", "\tret i32 %h\n");
	szAssembly.Init();
	AssertLong((int32)h32, TestIR816Run(szIR.Text(), false, &szAssembly));
	AssertTrue(szAssembly.Contains("jsl R816_Mul32"));
	szAssembly.Kill();
	AssertIR816((int32)h32, szIR.Text());
	szIR.Kill();

	szIR.Init(gszTestIR816Multiply);
	szIR.Replace("$T", "i16");
	szIR.Replace("$M", "tt");
	szIR.Replace("$S", "15");
	szIR.Replace("$RET", "\t%r = zext i16 %h to i32\n\tret i32 %r\n");
	szAssembly.Init();
	AssertLong(h16, TestIR816Run(szIR.Text(), false, &szAssembly));
	AssertTrue(szAssembly.Contains("jsl R816_Mul16"));
	szAssembly.Kill();
	AssertIR816(h16, szIR.Text());
	szIR.Kill();

	AssertIR816(0x7FFF8001, "function i32 @_M__Fjj(i32 %a, i32 %b), local\n{\nb:\n\t%m = mul i32 %a, %b\n\tret i32 %m\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_M__Fjj(i32 65535, i32 -32769)\n\tret i32 %r\n}\n", true);
	AssertIR816(1, "function i32 @_M__Fjj(i32 %a, i32 %b), local\n{\nb:\n\t%m = mul i32 %a, %b\n\tret i32 %m\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_M__Fjj(i32 -1, i32 -1)\n\tret i32 %r\n}\n", true);
	AssertIR816(1, "function i16 @_M__Ftt(i16 %a, i16 %b), local\n{\nb:\n\t%m = mul i16 %a, %b\n\tret i16 %m\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i16 @_M__Ftt(i16 -1, i16 -1)\n\t%z = zext i16 %r to i32\n\tret i32 %z\n}\n", true);
}


//////////////////////////////////////////////////////////////////////////
//
// The 16 bit runtime divide (R816_UDiv16 and R816_URem16, reciprocals from MathInvert): 2048 pseudo random dividends
// and divisors shifted right by every pair of amounts (divisors of every size, powers of two, 0 and 1, dividends below
// them), the quotients and remainders hashed.
//
//////////////////////////////////////////////////////////////////////////
static const char* gszTestIR816Divide16 =
	"function i16 @_Q__Ftt(i16 %a, i16 %b), local\n{\nb:\n\t%q = udiv i16 %a, %b\n\tret i16 %q\n}\n\n"
	"function i16 @_R__Ftt(i16 %a, i16 %b), local\n{\nb:\n\t%r = urem i16 %a, %b\n\tret i16 %r\n}\n\n"
	"function i32 @_main()\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%i = phi i16 [entry, 0], [body, %i2]\n\t%x = phi i16 [entry, 12345], [body, %x2]\n\t%y = phi i16 [entry, 6789], [body, %y2]\n\t%h = phi i16 [entry, 0], [body, %h3]\n"
	"\t%done = eq i16 %i, 2048\n\tbrif %done, exit, body\n"
	"body:\n"
	"\t%xm = mul i16 %x, 25173\n\t%x2 = add i16 %xm, 13849\n\t%ym = mul i16 %y, 1103\n\t%y2 = add i16 %ym, 7919\n"
	"\t%sa = and i16 %i, 15\n\t%ti = lshr i16 %i, 4\n\t%sb = and i16 %ti, 15\n"
	"\t%a = lshr i16 %x2, %sa\n\t%b0 = lshr i16 %y2, %sb\n\t%bz = eq i16 %b0, 0\n\t%bw = zext i1 %bz to i16\n\t%b = or i16 %b0, %bw\n"
	"\t%q = call i16 @_Q__Ftt(i16 %a, i16 %b)\n\t%r = call i16 @_R__Ftt(i16 %a, i16 %b)\n"
	"\t%hs = shl i16 %h, 5\n\t%ha = add i16 %hs, %h\n\t%h2 = xor i16 %ha, %q\n\t%h2s = shl i16 %h2, 3\n\t%h3 = xor i16 %h2s, %r\n"
	"\t%i2 = add i16 %i, 1\n\tjmp loop\n"
	"exit:\n\t%z = zext i16 %h to i32\n\tret i32 %z\n}\n";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendDivide16(void)
{
	uint16	i;
	uint16	x;
	uint16	y;
	uint16	h;
	uint16	a;
	uint16	b;
	CChars	szAssembly;

	x = 12345;
	y = 6789;
	h = 0;
	for (i = 0; i < 2048; i++)
	{
		x = (uint16)(x * 25173 + 13849);
		y = (uint16)(y * 1103 + 7919);
		a = (uint16)(x >> (i & 15));
		b = (uint16)(y >> ((i >> 4) & 15));
		if (b == 0)
		{
			b = 1;
		}
		h = (uint16)((h * 33) ^ (a / b));
		h = (uint16)((h << 3) ^ (a % b));
	}

	szAssembly.Init();
	AssertLong(h, TestIR816Run(gszTestIR816Divide16, false, &szAssembly));
	AssertTrue(szAssembly.Contains("jsl R816_UDiv16"));
	AssertTrue(szAssembly.Contains("jsl R816_URem16"));
	szAssembly.Kill();
	AssertIR816(h, gszTestIR816Divide16);

	AssertIR816(0xFFFF, "function i16 @_Q__Ftt(i16 %a, i16 %b), local\n{\nb:\n\t%q = udiv i16 %a, %b\n\tret i16 %q\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i16 @_Q__Ftt(i16 1234, i16 0)\n\t%z = zext i16 %r to i32\n\tret i32 %z\n}\n", true);
	AssertIR816(65535 / 300, "function i16 @_Q__Ftt(i16 %a, i16 %b), local\n{\nb:\n\t%q = udiv i16 %a, %b\n\tret i16 %q\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i16 @_Q__Ftt(i16 -1, i16 300)\n\t%z = zext i16 %r to i32\n\tret i32 %z\n}\n", true);
}


//////////////////////////////////////////////////////////////////////////
//
// A mul by a constant is shifts and adds when they cost less than the runtime call (ExpandMultiplies): constants with
// a few signed digits, negative ones and ones whose digits carry past the top, on 16, 32 and 64 bits, against the
// expected product and the 65C64's.  The value is a parameter so the product isn't worked out when compiled.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendMultiplyConstants(void)
{
	static const int64	alliConstants[] = { 3, 5, 6, 7, 10, 12, 15, 31, 100, 255, 320, 1000, 65537, -3, -5, -10, -320, 0x7FFFFFFF };
	static const int64	alliValues[] = { 1234, -77, 0x12345 };
	static const char*	aszTypes[] = { "i16", "i32", "i64" };
	static const int	aiBits[] = { 16, 32, 64 };
	char				sz[512];
	size				i;
	size				j;
	size				k;
	int64				lliExpected;
	uint64				ulliMask;
	CChars				szAssembly;

	for (k = 0; k < 3; k++)
	{
		ulliMask = (aiBits[k] == 64) ? (uint64)-1 : (((uint64)1 << aiBits[k]) - 1);
		for (i = 0; i < sizeof(alliConstants) / sizeof(alliConstants[0]); i++)
		{
			for (j = 0; j < sizeof(alliValues) / sizeof(alliValues[0]); j++)
			{
				lliExpected = (int64)(((uint64)alliValues[j] * (uint64)alliConstants[i]) & ulliMask);
				if ((aiBits[k] < 64) && (lliExpected & ((int64)1 << (aiBits[k] - 1))))
				{
					lliExpected -= (int64)1 << aiBits[k];
				}
				if (aiBits[k] >= 32)
				{
					snprintf(sz, sizeof(sz), "function %s @_M__F(%s %%a), local\n{\nb:\n\t%%m = mul %s %%a, %lld\n\tret %s %%m\n}\n\n"
						"function %s @_main()\n{\nb:\n\t%%r = call %s @_M__F(%s %lld)\n\tret %s %%r\n}\n", aszTypes[k], aszTypes[k], aszTypes[k], (long long)(alliConstants[i] & (int64)ulliMask),
						aszTypes[k], aszTypes[k], aszTypes[k], aszTypes[k], (long long)(alliValues[j] & (int64)ulliMask), aszTypes[k]);
				}
				else
				{
					snprintf(sz, sizeof(sz), "function %s @_M__F(%s %%a), local\n{\nb:\n\t%%m = mul %s %%a, %lld\n\tret %s %%m\n}\n\n"
						"function i32 @_main()\n{\nb:\n\t%%r = call %s @_M__F(%s %lld)\n\t%%s = sext %s %%r to i32\n\tret i32 %%s\n}\n",
						aszTypes[k], aszTypes[k], aszTypes[k], (long long)(alliConstants[i] & (int64)ulliMask), aszTypes[k], aszTypes[k], aszTypes[k], (long long)(alliValues[j] & (int64)ulliMask), aszTypes[k]);
					if (aiBits[k] == 16)
					{
						lliExpected = (int16)lliExpected;
					}
				}
				AssertIR816(lliExpected, sz);
			}
		}
	}

	//An i8 product's shift by a constant mustn't count in X, where the trunc it shifts may be kept (AllocateX).
	AssertIR816(0x01080F16, "function i32 @_main()\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%h = phi i32 [entry, 0], [loop, %h2]\n"
		"\t%t = trunc i32 %i to i8\n\t%m = mul i8 %t, 7\n\t%a = add i8 %m, 1\n\t%z = zext i8 %a to i32\n"
		"\t%hs = shl i32 %h, 8\n\t%h2 = add i32 %hs, %z\n\t%i2 = add i32 %i, 1\n\t%c = slt i32 %i2, 4\n\tbrif %c, loop, exit\n"
		"exit:\n\tret i32 %h2\n}\n", true);

	//The products' code: 320 has two digits (shifts and an add), 0x55555555 too many (the call).
	szAssembly.Init();
	AssertLong(1234 * 320, TestIR816Run("function i32 @_M__Fl(i32 %a), local\n{\nb:\n\t%m = mul i32 %a, 320\n\tret i32 %m\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_M__Fl(i32 1234)\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertFalse(szAssembly.Contains("R816_Mul32"));
	szAssembly.Kill();
	szAssembly.Init();
	AssertLong((int32)(1234 * 0x55555555U), TestIR816Run("function i32 @_M__Fl(i32 %a), local\n{\nb:\n\t%m = mul i32 %a, 1431655765\n\tret i32 %m\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_M__Fl(i32 1234)\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("jsl R816_Mul32"));
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Shifts by a constant (ShiftConstant): whole words, then a byte moved a word at a time, then bits; the counts around
// each byte and word of shl, lshr and ashr on 16, 32 and 64 bits.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendShiftConstants(void)
{
	static const char*	aszOpcodes[] = { "shl", "lshr", "ashr" };
	static const char*	aszTypes[] = { "i16", "i32", "i64" };
	static const int	aiBits[] = { 16, 32, 64 };
	static const uint64	aulliValues[] = { 0x8123456789ABCDEFULL, 0x0123456789ABCDEFULL };
	static const int	aiCounts[] = { 1, 3, 7, 8, 9, 15, 16, 17, 23, 24, 25, 31, 32, 33, 39, 40, 47, 48, 55, 56, 57, 63 };
	char				sz[512];
	int					iOpcode;
	int					iType;
	int					iCount;
	size				i;
	size				iValue;
	uint64				ulliValue;
	uint64				ulliMask;
	uint64				ulliExpected;
	int64				lliSigned;

	for (iType = 0; iType < 3; iType++)
	{
		ulliMask = (aiBits[iType] == 64) ? (uint64)-1 : (((uint64)1 << aiBits[iType]) - 1);
		for (iValue = 0; iValue < 2; iValue++)
		{
			//The value's top bit set, and not.
			ulliValue = aulliValues[iValue] >> (64 - aiBits[iType]);
			lliSigned = (int64)(ulliValue << (64 - aiBits[iType])) >> (64 - aiBits[iType]);
			for (iOpcode = 0; iOpcode < 3; iOpcode++)
			{
				for (i = 0; (i < sizeof(aiCounts) / sizeof(aiCounts[0])) && (aiCounts[i] < aiBits[iType]); i++)
				{
					iCount = aiCounts[i];
					if (iOpcode == 0)
					{
						ulliExpected = (ulliValue << iCount) & ulliMask;
					}
					else if (iOpcode == 1)
					{
						ulliExpected = ulliValue >> iCount;
					}
					else
					{
						ulliExpected = (uint64)(lliSigned >> iCount) & ulliMask;
					}
					if (aiBits[iType] >= 32)
					{
						snprintf(sz, sizeof(sz), "function %s @_S__F(%s %%a), local\n{\nb:\n\t%%s = %s %s %%a, %d\n\tret %s %%s\n}\n\n"
							"function %s @_main()\n{\nb:\n\t%%r = call %s @_S__F(%s %llu)\n\tret %s %%r\n}\n", aszTypes[iType], aszTypes[iType], aszOpcodes[iOpcode], aszTypes[iType], iCount, aszTypes[iType],
							aszTypes[iType], aszTypes[iType], aszTypes[iType], (unsigned long long)ulliValue, aszTypes[iType]);
						AssertIR816((aiBits[iType] == 64) ? (int64)ulliExpected : (int64)(int32)(uint32)ulliExpected, sz);
					}
					else
					{
						snprintf(sz, sizeof(sz), "function %s @_S__F(%s %%a), local\n{\nb:\n\t%%s = %s %s %%a, %d\n\tret %s %%s\n}\n\n"
							"function i32 @_main()\n{\nb:\n\t%%r = call %s @_S__F(%s %llu)\n\t%%z = zext %s %%r to i32\n\tret i32 %%z\n}\n",
							aszTypes[iType], aszTypes[iType], aszOpcodes[iOpcode], aszTypes[iType], iCount, aszTypes[iType], aszTypes[iType], aszTypes[iType], (unsigned long long)ulliValue, aszTypes[iType]);
						AssertIR816((int64)ulliExpected, sz);
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Unsigned 32 bit compares branch on the high words' Z, then on the low words' C: the peephole mustn't take the
// compare of a high word with 0 out (its carry is read after the branch, a label and a .width away).
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendCompareCarry(void)
{
	static const uint32	auiPairs[][2] = { { 0x10000, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 0, 0x10000 }, { 0x20000, 0x10000 }, { 0x10003, 0x10004 } };
	char				sz[512];
	size				i;

	for (i = 0; i < sizeof(auiPairs) / sizeof(auiPairs[0]); i++)
	{
		snprintf(sz, sizeof(sz), "function i32 @_C__Fjj(i32 %%a, i32 %%b), local\n{\nb:\n\t%%c = ult i32 %%a, %%b\n\tbrif %%c, y, n\n"
			"y:\n\tret i32 1\nn:\n\t%%d = uge i32 %%a, 4\n\tbrif %%d, g, l\ng:\n\tret i32 2\nl:\n\tret i32 3\n}\n\n"
			"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_C__Fjj(i32 %u, i32 %u)\n\tret i32 %%r\n}\n", auiPairs[i][0], auiPairs[i][1]);
		AssertIR816((auiPairs[i][0] < auiPairs[i][1]) ? 1 : ((auiPairs[i][0] >= 4) ? 2 : 3), sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A result shares the place of an argument that dies where it is defined (and a loop's next value its phi's place),
// but not of one still live after it: a call's result that is never used is only live where it is written, and a
// shift's count is read after its value is copied to the result.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendSharedPlaces(void)
{
	AssertIR816(42,
		"function i32 @_Seven__Fi(i32 %x), local\n{\nb:\n\tret i32 7\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%a = add i32 40, 2\n\t%r = call i32 @_Seven__Fi(i32 %a)\n\tret i32 %a\n}\n");

	AssertIR816(126,
		"function i32 @_Seven__Fi(i32 %x), local\n{\nb:\n\tret i32 7\n}\n\n"
		"function i32 @_main()\n{\n"
		"entry:\n\t%p = add i32 40, 2\n\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%r = call i32 @_Seven__Fi(i32 %p)\n"
		"\t%i2 = add i32 %i, %p\n"
		"\t%c = ult i32 %i2, 100\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n\tret i32 %i2\n}\n", true);

	AssertIR816((((1000 - 42) ^ 255) << 2 << 2) + (2 << 2),
		"function i32 @_main()\n{\nb:\n"
		"\t%a = add i32 40, 2\n"
		"\t%b = sub i32 1000, %a\n"
		"\t%c = xor i32 %b, 255\n"
		"\t%d = shl i32 %c, 2\n"
		"\t%k = add i32 1, 1\n"
		"\t%e = shl i32 %d, %k\n"
		"\t%j = add i32 1, 1\n"
		"\t%f = shl i32 %j, %j\n"
		"\t%g = add i32 %e, %f\n"
		"\tret i32 %g\n}\n", true);
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit values used once by the next instruction, read into A a word at a
// time as it goes (CanFuse): a load or'd, an xor stored, a load added as the
// second operand, a sub stored, a load and'd as the second operand, a load
// added then stored, and chains of them.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendFused(void)
{
	AssertIR816(0x123C5670,
		"function i32 @_main()\n{\nb:\n"
		"\t%s = slot ptr 8, 4\n"
		"\t%t = ptradd ptr %s, 4\n"
		"\tstore i32 305419896, %s\n"
		"\tstore i32 -16777216, %t\n"
		"\t%a = load i32 %s\n"
		"\t%o = or i32 %a, 15\n"
		"\t%x = xor i32 %o, 65535\n"
		"\tstore i32 %x, %t\n"
		"\t%b = load i32 %t\n"
		"\t%k = load i32 %s\n"
		"\t%c = add i32 %b, %k\n"
		"\t%d = sub i32 %c, 7\n"
		"\tstore i32 %d, %s\n"
		"\t%e = load i32 %s\n"
		"\t%m = and i32 1048575, %e\n"
		"\t%f = load i32 %t\n"
		"\t%g = add i32 %f, 1\n"
		"\tstore i32 %g, %t\n"
		"\t%h = load i32 %t\n"
		"\t%r = xor i32 %m, %h\n"
		"\tret i32 %r\n}\n", true);

	//Chains: a load, and and xor stored back to the same place; an add, or and store; an add then an and of a constant
	//(a word the and doesn't read would leave the add's carry out, so it isn't fused).
	AssertIR816(545128460,
		"function i32 @_main()\n{\nb:\n"
		"\t%s = slot ptr 8, 4\n"
		"\t%t = ptradd ptr %s, 4\n"
		"\tstore i32 305419896, %s\n"
		"\tstore i32 252645135, %t\n"
		"\t%a = load i32 %t\n"
		"\t%b = and i32 %a, -65281\n"
		"\t%c = load i32 %s\n"
		"\t%d = xor i32 %b, %c\n"
		"\tstore i32 %d, %t\n"
		"\t%e = load i32 %s\n"
		"\t%f = add i32 %e, 65536\n"
		"\t%g = or i32 %f, 3\n"
		"\tstore i32 %g, %s\n"
		"\t%h = load i32 %t\n"
		"\t%i = load i32 %s\n"
		"\t%j = add i32 %h, %i\n"
		"\t%k = and i32 %j, -65536\n"
		"\t%l = load i32 %t\n"
		"\t%m = load i32 %s\n"
		"\t%n = xor i32 %l, %m\n"
		"\t%r = xor i32 %n, %k\n"
		"\tret i32 %r\n}\n", true);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the assembly has a lone szStep ("\t\tinc [D + " or "\t\tdec [D + "):
// a whole line after an instruction other than a bne and before no bne (a
// 16 bit step; a 32 bit one carries or borrows across its words with a bne
// round the high word's step, and its low word's dec follows a label).
//
//////////////////////////////////////////////////////////////////////////
static bool TestIR816LoneStep(const char* szAssembly, const char* szStep)
{
	const char*	szAt;
	const char*	szEnd;
	const char*	szPrevious;

	for (szAt = strstr(szAssembly, szStep); szAt; szAt = strstr(szAt + 1, szStep))
	{
		if ((szAt == szAssembly) || (szAt[-1] != '\n'))
		{
			continue;
		}
		for (szPrevious = szAt - 1; (szPrevious > szAssembly) && (szPrevious[-1] != '\n'); szPrevious--)
		{
		}
		szEnd = strchr(szAt, '\n');
		if (szEnd && (strncmp(szEnd + 1, "\t\tbne", 5) != 0) && (strncmp(szPrevious, "\t\t", 2) == 0) && (strncmp(szPrevious, "\t\tbne", 5) != 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit values stepped by 1 and -1 in their own places (StepInPlace): inc
// and dec on the direct page, carrying across the words; and 16 bit ones,
// a lone inc or dec.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendStepInPlace(void)
{
	CChars	szAssembly;

	szAssembly.Init();
	AssertLong(0x10001 ^ 0xFFFE, TestIR816Run(
		"function i32 @_main()\n{\n"
		"e:\n\tjmp l\n"
		"l:\n"
		"\t%i = phi i32 [e, 65534], [l, %i2]\n"
		"\t%d = phi i32 [e, 65537], [l, %d2]\n"
		"\t%n = phi i32 [e, 0], [l, %n2]\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d2 = sub i32 %d, 1\n"
		"\t%n2 = add i32 %n, 1\n"
		"\t%c = ult i32 %n2, 3\n"
		"\tbrif %c, l, x\n"
		"x:\n"
		"\t%r = xor i32 %i2, %d2\n"
		"\tret i32 %r\n}\n", true, &szAssembly));
	AssertTrue(strstr(szAssembly.Text(), "\t\tinc [D + ") != NULL);
	AssertTrue(strstr(szAssembly.Text(), "\t\tdec [D + ") != NULL);
	AssertFalse(TestIR816LoneStep(szAssembly.Text(), "\t\tinc [D + "));
	AssertFalse(TestIR816LoneStep(szAssembly.Text(), "\t\tdec [D + "));
	szAssembly.Kill();

	//16 bits: a lone inc and dec, with no carry into a high word (the loop counter, in X, steps by inx).
	szAssembly.Init();
	AssertLong(400 - 700, TestIR816Run(
		"function i32 @_main()\n{\n"
		"e:\n\tjmp l\n"
		"l:\n"
		"\t%i = phi i16 [e, 0], [l, %i2]\n"
		"\t%j = phi i16 [e, 100], [l, %j2]\n"
		"\t%k = phi i16 [e, 1000], [l, %k2]\n"
		"\t%i2 = add i16 %i, 1\n"
		"\t%j2 = add i16 %j, 1\n"
		"\t%k2 = sub i16 %k, 1\n"
		"\t%c = ult i16 %i2, 300\n"
		"\tbrif %c, l, x\n"
		"x:\n"
		"\t%a = zext i16 %j2 to i32\n"
		"\t%b = zext i16 %k2 to i32\n"
		"\t%r = sub i32 %a, %b\n"
		"\tret i32 %r\n}\n", true, &szAssembly));
	AssertTrue(TestIR816LoneStep(szAssembly.Text(), "\t\tinc [D + "));
	AssertTrue(TestIR816LoneStep(szAssembly.Text(), "\t\tdec [D + "));
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Shifts by a count only known at run time (in X, a word in A, a whole word
// moved first for 16 or more) and by constants, of 16 and 32 bit values.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendShifts(void)
{
	static const int	aiCounts[] = { 0, 1, 5, 15, 16, 17, 31 };
	static const char*	aszOpcodes[] = { "shl", "lshr", "ashr" };
	char				sz[512];
	size				i;
	size				j;
	int64				lliExpected;
	uint32				uiValue;
	uint16				uiShort;

	uiValue = 0x9234ABCD;
	uiShort = 0x9A5C;
	for (i = 0; i < sizeof(aiCounts) / sizeof(aiCounts[0]); i++)
	{
		for (j = 0; j < 3; j++)
		{
			lliExpected = (j == 0) ? (int32)(uiValue << aiCounts[i]) : ((j == 1) ? (int32)(uiValue >> aiCounts[i]) : ((int32)uiValue >> aiCounts[i]));
			snprintf(sz, sizeof(sz), "function i32 @_S__Fl(i32 %%n), local\n{\nb:\n\t%%r = %s i32 %u, %%n\n\tret i32 %%r\n}\n\n"
				"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_S__Fl(i32 %d)\n\tret i32 %%r\n}\n", aszOpcodes[j], uiValue, aiCounts[i]);
			AssertIR816(lliExpected, sz);
			snprintf(sz, sizeof(sz), "function i32 @_main()\n{\nb:\n\t%%r = %s i32 %u, %d\n\tret i32 %%r\n}\n", aszOpcodes[j], uiValue, aiCounts[i]);
			AssertIR816(lliExpected, sz);

			if (aiCounts[i] < 16)
			{
				lliExpected = (j == 0) ? (int16)(uint16)(uiShort << aiCounts[i]) : ((j == 1) ? (int16)(uiShort >> aiCounts[i]) : ((int16)uiShort >> aiCounts[i]));
				snprintf(sz, sizeof(sz), "function i16 @_S__Fs(i16 %%n), local\n{\nb:\n\t%%r = %s i16 %u, %%n\n\tret i16 %%r\n}\n\n"
					"function i32 @_main()\n{\nb:\n\t%%r = call i16 @_S__Fs(i16 %d)\n\t%%w = sext i16 %%r to i32\n\tret i32 %%w\n}\n", aszOpcodes[j], uiShort, aiCounts[i]);
				AssertIR816(lliExpected, sz);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Values kept in X (AllocateX): a loop's counter and sum (phis, the counter
// stepped with inx), the counter as the second operand of a sub (through
// the scratch word), and signed compares of a 16 bit value with 0 that
// branch on N and Z.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendX(void)
{
	static const int16	aiValues[] = { -32768, -5, -1, 0, 1, 7, 32767 };
	static const char*	aszCompares[] = { "slt", "sle", "sgt", "sge" };
	char				sz[512];
	CChars				szAssembly;
	size				i;
	size				j;
	int64				lliExpected;

	szAssembly.Init();
	AssertLong(4950, TestIR816Run(
		"function i32 @_main()\n{\n"
		"e:\n\tjmp l\n"
		"l:\n"
		"\t%i = phi i16 [e, 0], [l, %i2]\n"
		"\t%s = phi i16 [e, 0], [l, %s2]\n"
		"\t%s2 = add i16 %s, %i\n"
		"\t%i2 = add i16 %i, 1\n"
		"\t%c = slt i16 %i2, 100\n"
		"\tbrif %c, l, x\n"
		"x:\n\t%r = sext i16 %s2 to i32\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("inx"));
	szAssembly.Kill();

	AssertIR816(1000 * 100 - 4950,
		"function i32 @_main()\n{\n"
		"e:\n\tjmp l\n"
		"l:\n"
		"\t%i = phi i16 [e, 0], [l, %i2]\n"
		"\t%s = phi i32 [e, 0], [l, %s2]\n"
		"\t%d = sub i16 1000, %i\n"
		"\t%w = sext i16 %d to i32\n"
		"\t%s2 = add i32 %s, %w\n"
		"\t%i2 = add i16 %i, 1\n"
		"\t%c = ult i16 %i2, 100\n"
		"\tbrif %c, l, x\n"
		"x:\n\tret i32 %s2\n}\n");

	//trunc, zext, sext and a shift by a constant worked out in A and kept in X.
	AssertIR816(4 * 4950 + 1468,
		"function i32 @_main()\n{\n"
		"e:\n\tjmp l\n"
		"l:\n"
		"\t%i = phi i16 [e, 0], [l, %i2]\n"
		"\t%s = phi i16 [e, 0], [l, %s3]\n"
		"\t%b = trunc i16 %i to i8\n"
		"\t%w = zext i8 %b to i16\n"
		"\t%t = shl i16 %w, 2\n"
		"\t%m = ult i16 %i, 100\n"
		"\t%z = zext i1 %m to i16\n"
		"\t%n = sub i16 0, %z\n"
		"\t%u = and i16 %t, %n\n"
		"\t%e = sext i8 %b to i16\n"
		"\t%s2 = add i16 %s, %u\n"
		"\t%s3 = add i16 %s2, %e\n"
		"\t%i2 = add i16 %i, 1\n"
		"\t%c = slt i16 %i2, 200\n"
		"\tbrif %c, l, x\n"
		"x:\n\t%r = sext i16 %s3 to i32\n\tret i32 %r\n}\n");

	for (i = 0; i < sizeof(aiValues) / sizeof(aiValues[0]); i++)
	{
		for (j = 0; j < 4; j++)
		{
			lliExpected = (j == 0) ? (aiValues[i] < 0) : ((j == 1) ? (aiValues[i] <= 0) : ((j == 2) ? (aiValues[i] > 0) : (aiValues[i] >= 0)));
			snprintf(sz, sizeof(sz), "function i32 @_C__Fs(i16 %%x), local\n{\nb:\n\t%%c = %s i16 %%x, 0\n\tbrif %%c, y, n\ny:\n\tret i32 1\nn:\n\tret i32 0\n}\n\n"
				"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_C__Fs(i16 %d)\n\tret i32 %%r\n}\n", aszCompares[j], aiValues[i]);
			AssertIR816(lliExpected, sz);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit values: four words, returned through the hidden pointer.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendWide(void)
{
	AssertIR816(1000000000000LL * 3, "function i64 @_main()\n{\nb:\n\t%a = mul i64 1000000000000, 3\n\tret i64 %a\n}\n", true);
	AssertIR816(-4, "function i64 @_main()\n{\nb:\n\t%a = sdiv i64 -9000000000000, 2250000000000\n\tret i64 %a\n}\n");
	AssertIR816(1, "function i64 @_main()\n{\nb:\n\t%c = slt i64 -5000000000, 4000000000\n\t%r = zext i1 %c to i64\n\tret i64 %r\n}\n");
	AssertIR816(-1, "function i64 @_main()\n{\nb:\n\t%a = ashr i64 -9000000000000, 60\n\tret i64 %a\n}\n");
	AssertIR816(0xFFFFFFFFLL, "function i64 @_main()\n{\nb:\n\t%a = zext i32 -1 to i64\n\tret i64 %a\n}\n");
	AssertIR816(-1, "function i64 @_main()\n{\nb:\n\t%a = sext i32 -1 to i64\n\tret i64 %a\n}\n");
	AssertIR816(20123405LL * 20123405LL,
		"function i64 @_main()\n{\nb:\n\t%a = add i32 20123404, 1\n\t%w = sext i32 %a to i64\n\t%s = mul i64 %w, %w\n\tret i64 %s\n}\n");
	AssertIR816(0x123456789ABCDEFLL % 1000000007LL,
		"function i64 @_main()\n{\nb:\n\t%a = urem i64 81985529216486895, 1000000007\n\tret i64 %a\n}\n");

	//A function returning i64 called from one that returns i32.
	AssertIR816(7,
		"function i64 @_Big__Fi(i32 %x), local\n{\nb:\n\t%w = sext i32 %x to i64\n\t%s = shl i64 %w, 40\n\tret i64 %s\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i64 @_Big__Fi(i32 7)\n\t%t = lshr i64 %r, 40\n\t%n = trunc i64 %t to i32\n\tret i32 %n\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The shift runtime routines (R816_Shl64, R816_LShr64, R816_AShr64 on the
// caller's frame at D + X; R816_Shl32, R816_LShr32 and R816_AShr32 in the
// scratch bytes): every count, from a function taking the count so it is
// only known when it runs.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendShiftRoutines(void)
{
	static const char*	aszOpcodes[] = { "shl", "lshr", "ashr" };
	static const uint64	aulliValues[] = { 0x923456789ABCDEF1ULL, 0x5A5A00FF8001C3E7ULL };
	char				sz[768];
	CChars				szAssembly;
	size				i;
	size				j;
	size				k;
	int64				lliExpected;
	uint64				ulliValue;
	uint32				uiValue;

	for (k = 0; k < 2; k++)
	{
		ulliValue = aulliValues[k];
		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 64; i++)
			{
				lliExpected = (j == 0) ? (int64)(ulliValue << i) : ((j == 1) ? (int64)(ulliValue >> i) : ((int64)ulliValue >> i));
				snprintf(sz, sizeof(sz), "function i64 @_S__Fy(i64 %%v, i32 %%n), local\n{\nb:\n\t%%m = zext i32 %%n to i64\n\t%%r = %s i64 %%v, %%m\n\tret i64 %%r\n}\n\n"
					"function i64 @_main()\n{\nb:\n\t%%r = call i64 @_S__Fy(i64 %lld, i32 %d)\n\tret i64 %%r\n}\n", aszOpcodes[j], (int64)ulliValue, (int)i);
				szAssembly.Init();
				AssertLong(lliExpected, TestIR816Run(sz, (i % 9) == 0, &szAssembly));
				AssertTrue(szAssembly.Contains("jsl R816_"));
				szAssembly.Kill();
			}

			uiValue = (uint32)(ulliValue >> 13);
			for (i = 0; i < 32; i++)
			{
				lliExpected = (j == 0) ? (int32)(uiValue << i) : ((j == 1) ? (int32)(uiValue >> i) : ((int32)uiValue >> i));
				snprintf(sz, sizeof(sz), "function i32 @_S__Fl(i32 %%v, i32 %%n), local\n{\nb:\n\t%%r = %s i32 %%v, %%n\n\tret i32 %%r\n}\n\n"
					"function i32 @_main()\n{\nb:\n\t%%r = call i32 @_S__Fl(i32 %d, i32 %d)\n\tret i32 %%r\n}\n", aszOpcodes[j], (int32)uiValue, (int)i);
				szAssembly.Init();
				AssertLong(lliExpected, TestIR816Run(sz, (i % 7) == 0, &szAssembly));
				AssertTrue(szAssembly.Contains("jsl R816_"));
				szAssembly.Kill();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit multiplies: of two values whose high halves are 0 (an and with
// 0xFFFFFFFF, an lshr by 32, a zext: R816_UMulWide32) and of any two
// (R816_Mul64), pseudo random values hashed.
//
//////////////////////////////////////////////////////////////////////////
static const char* gszTestIR816Multiply64 =
	"function i64 @_M__Fyy(i64 %a, i64 %b), local\n{\nb:\n$BODY\tret i64 %m\n}\n\n"
	"function i64 @_main()\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%i = phi i16 [entry, 0], [body, %i2]\n\t%x = phi i64 [entry, 1234567], [body, %x2]\n\t%y = phi i64 [entry, 89101112], [body, %y2]\n\t%h = phi i64 [entry, 0], [body, %h3]\n"
	"\t%done = eq i16 %i, 300\n\tbrif %done, exit, body\n"
	"body:\n"
	"\t%xm = mul i64 %x, 6364136223846793005\n\t%x2 = add i64 %xm, 1442695040888963407\n\t%ym = mul i64 %y, 2862933555777941757\n\t%y2 = add i64 %ym, 3037000493\n"
	"\t%m = call i64 @_M__Fyy(i64 %x2, i64 %y2)\n"
	"\t%hs = shl i64 %h, 5\n\t%ha = add i64 %hs, %h\n\t%h3 = xor i64 %ha, %m\n"
	"\t%i2 = add i16 %i, 1\n\tjmp loop\n"
	"exit:\n\tret i64 %h\n}\n";

void TestIR816BackendMultiply64(void)
{
	uint64		x;
	uint64		y;
	uint64		h;
	uint64		m;
	int			i;
	int			iKind;
	CChars		szIR;
	CChars		szAssembly;
	const char*	aszBodies[] = {
		"\t%m = mul i64 %a, %b\n",
		"\t%la = and i64 %a, 4294967295\n\t%lb = and i64 %b, 4294967295\n\t%m = mul i64 %la, %lb\n",
		"\t%ha = lshr i64 %a, 32\n\t%lb = and i64 %b, 4294967295\n\t%m = mul i64 %ha, %lb\n",
		"\t%ta = trunc i64 %a to i32\n\t%za = zext i32 %ta to i64\n\t%hb = lshr i64 %b, 40\n\t%m = mul i64 %za, %hb\n" };

	for (iKind = 0; iKind < 4; iKind++)
	{
		x = 1234567;
		y = 89101112;
		h = 0;
		for (i = 0; i < 300; i++)
		{
			x = x * 6364136223846793005ULL + 1442695040888963407ULL;
			y = y * 2862933555777941757ULL + 3037000493ULL;
			switch (iKind)
			{
			case 0:		m = x * y;									break;
			case 1:		m = (x & 0xFFFFFFFF) * (y & 0xFFFFFFFF);	break;
			case 2:		m = (x >> 32) * (y & 0xFFFFFFFF);			break;
			default:	m = (uint64)(uint32)x * (y >> 40);			break;
			}
			h = ((h << 5) + h) ^ m;
		}
		szIR.Init(gszTestIR816Multiply64);
		szIR.Replace("$BODY", aszBodies[iKind]);
		szAssembly.Init();
		AssertLong((int64)h, TestIR816Run(szIR.Text(), false, &szAssembly));
		AssertTrue(szAssembly.Contains((iKind == 0) ? "jsl R816_Mul64" : "jsl R816_UMulWide32"));
		szAssembly.Kill();
		AssertLong((int64)h, TestIR816Run(szIR.Text(), true));
		szIR.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Narrow memory, slots, recursion, a variadic function, a function pointer and a thiscall.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendMemoryAndCalls(void)
{
	AssertIR816(-1 + 300 + 70000,
		"function i32 @_main()\n{\nb:\n"
		"\t%p = slot ptr 8, 4\n"
		"\tstore i32 -1, %p\n"
		"\t%q = ptradd ptr %p, 4\n"
		"\tstore i16 300, %q\n"
		"\t%c = load i8 %p\n"
		"\t%cs = sext i8 %c to i32\n"
		"\t%h = load volatile i16 %q\n"
		"\t%hs = zext i16 %h to i32\n"
		"\t%t = add i32 %cs, %hs\n"
		"\t%r = add i32 %t, 70000\n"
		"\tret i32 %r\n}\n", true);

	//A byte stored through a pointer leaves the bytes either side alone.
	AssertIR816(0x11AA3344,
		"global @_gWord, align 4 { i32 287454020 }\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%p = ptradd ptr @_gWord, 2\n"
		"\tstore i8 -86, %p\n"
		"\t%v = load i32 @_gWord\n"
		"\tret i32 %v\n}\n");

	AssertIR816(55,
		"function i32 @_Fibonacci__Fc(i8 %n)\n{\n"
		"entry:\n\t%c = slt i8 %n, 2\n\tbrif %c, small, large\n"
		"small:\n\t%r = zext i8 %n to i32\n\tret i32 %r\n"
		"large:\n\t%a = sub i8 %n, 1\n\t%b = sub i8 %n, 2\n"
		"\t%x = call i32 @_Fibonacci__Fc(i8 %a)\n\t%y = call i32 @_Fibonacci__Fc(i8 %b)\n"
		"\t%s = add i32 %x, %y\n\tret i32 %s\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_Fibonacci__Fc(i8 10)\n\tret i32 %r\n}\n", true);

	AssertIR816(1 + 20 + 300,
		"function i32 @_SumAll(i32 %count, ...)\n{\n"
		"entry:\n\t%first = argaddr ptr 0\n\t%p0 = ptradd ptr %first, 4\n\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i32 [entry, 0], [body, %s2]\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%p = phi ptr [entry, %p0], [body, %p2]\n"
		"\t%c = slt i32 %i, %count\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%v = load i32 %p\n\t%s2 = add i32 %s, %v\n\t%i2 = add i32 %i, 1\n\t%p2 = ptradd ptr %p, 4\n\tjmp loop\n"
		"exit:\n\tret i32 %s\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_SumAll(i32 3, ..., i32 1, i32 20, i32 300)\n\tret i32 %r\n}\n");

	AssertIR816(12,
		"function i32 @_Double__Fi(i32 %x)\n{\nb:\n\t%r = add i32 %x, %x\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%f = copy ptr @_Double__Fi\n\t%r = call i32 %f(i32 6)\n\tret i32 %r\n}\n", true);

	AssertIR816(7 + 5,
		"global @_gObject, align 4 { i32 7 }\n\n"
		"function i32 @_C__Get__Fi(ptr %this, i32 %x), thiscall\n{\nb:\n\t%v = load i32 %this\n\t%r = add i32 %v, %x\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call thiscall i32 @_C__Get__Fi(ptr @_gObject, i32 5)\n\tret i32 %r\n}\n");

	//Sum of Triangle(i) for i < 6, and Count(n) = 1 + Count(0) + ... + Count(n - 1) = 2^n.
	AssertIR816(0 + 1 + 3 + 6 + 10 + 15,
		"function i32 @_Triangle__Fi(i32 %n), local\n{\n"
		"entry:\n\t%z = eq i32 %n, 0\n\tbrif %z, zero, more\n"
		"zero:\n\tret i32 0\n"
		"more:\n\t%m = sub i32 %n, 1\n\t%t = call i32 @_Triangle__Fi(i32 %m)\n\t%r = add i32 %t, %n\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n\t%s = phi i32 [entry, 0], [body, %s2]\n\t%i = phi i32 [entry, 0], [body, %i2]\n\t%c = slt i32 %i, 6\n\tbrif %c, body, exit\n"
		"body:\n\t%t = call i32 @_Triangle__Fi(i32 %i)\n\t%s2 = add i32 %s, %t\n\t%i2 = add i32 %i, 1\n\tjmp loop\n"
		"exit:\n\tret i32 %s\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer plus a zero extended i16 used only by 1 word loads and stores
// puts the i16 in Y ([[D + p]:24 + Y]), the zext not worked out.  Y carries
// into the bank: the bytes go from $07FFF0 over $080000, and the word at
// $07FFFF is read across it.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendIndexed(void)
{
	static const char*	szIR =
		"function i32 @_main()\n{\nentry:\n\t%p = itoptr i32 524272 to ptr\n\tjmp fill\n"
		"fill:\n\t%i = phi i16 [entry, 0], [fill, %i2]\n\t%w = zext i16 %i to i32\n\t%a = ptradd ptr %p, %w\n"
		"\t%b = trunc i16 %i to i8\n\tstore i8 %b, %a\n\t%i2 = add i16 %i, 1\n\t%c = ult i16 %i2, 32\n\tbrif %c, fill, sum\n"
		"sum:\n\t%j = phi i16 [fill, 0], [sum, %j2]\n\t%s = phi i16 [fill, 0], [sum, %s2]\n\t%x = zext i16 %j to i32\n"
		"\t%q = ptradd ptr %p, %x\n\t%v = load i8 %q\n\t%vw = zext i8 %v to i16\n\t%s2 = add i16 %s, %vw\n"
		"\t%j2 = add i16 %j, 1\n\t%d = ult i16 %j2, 32\n\tbrif %d, sum, done\n"
		"done:\n\t%k = copy i16 15\n\t%y = zext i16 %k to i32\n\t%r = ptradd ptr %p, %y\n\t%h = load i16 %r\n"
		"\t%t = add i16 %s2, %h\n\t%u = zext i16 %t to i32\n\tret i32 %u\n}\n";
	static const char*	szCopyIR =
		"function i32 @_main()\n{\nentry:\n\t%p = itoptr i32 524272 to ptr\n\t%o = itoptr i32 458736 to ptr\n\tjmp fill\n"
		"fill:\n\t%i = phi i16 [entry, 0], [fill, %i2]\n\t%w = zext i16 %i to i32\n\t%a = ptradd ptr %p, %w\n"
		"\t%b = trunc i16 %i to i8\n\tstore i8 %b, %a\n\t%i2 = add i16 %i, 1\n\t%c = ult i16 %i2, 32\n\tbrif %c, fill, copy\n"
		"copy:\n\t%j = phi i16 [fill, 0], [copy, %j2]\n\t%x = zext i16 %j to i32\n\t%f = ptradd ptr %p, %x\n\t%g = ptradd ptr %o, %x\n"
		"\t%v = load i8 %f\n\tstore i8 %v, %g\n\t%j2 = add i16 %j, 1\n\t%d = ult i16 %j2, 32\n\tbrif %d, copy, sum\n"
		"sum:\n\t%k = phi i16 [copy, 0], [sum, %k2]\n\t%s = phi i16 [copy, 0], [sum, %s2]\n\t%y = zext i16 %k to i32\n"
		"\t%q = ptradd ptr %o, %y\n\t%e = load i8 %q\n\t%ew = zext i8 %e to i16\n\t%s2 = add i16 %s, %ew\n"
		"\t%k2 = add i16 %k, 1\n\t%m = ult i16 %k2, 32\n\tbrif %m, sum, done\n"
		"done:\n\t%u = zext i16 %s2 to i32\n\tret i32 %u\n}\n";
	static const char*	szWordsIR =
		"function i32 @_main()\n{\nentry:\n\t%p = itoptr i32 524286 to ptr\n\tstore i32 305419896, %p\n"
		"\t%k = copy i32 2\n\t%q = ptradd ptr %p, %k\n\t%h = load i16 %q\n\t%v = load i32 %p\n"
		"\t%w = zext i16 %h to i32\n\t%r = add i32 %v, %w\n\tret i32 %r\n}\n";
	CChars				szAssembly;

	szAssembly.Init();
	AssertLong(496 + 0x100F, TestIR816Run(szIR, true, &szAssembly));
	AssertTrue(strstr(szAssembly.Text(), "\t\ttxy\n") || strstr(szAssembly.Text(), "\t\tldy [D + "));
	szAssembly.Kill();

	//A byte loaded and only stored by the next instruction is copied through A under the store's sep m, from
	//$07FFF0 + i to $06FFF0 + i (both over a bank).
	szAssembly.Init();
	AssertLong(496, TestIR816Run(szCopyIR, true, &szAssembly));
	AssertTrue(strstr(szAssembly.Text(), "\t\tsep m\n\t\ttxy\n\t\tlda [[D + ") != NULL);
	szAssembly.Kill();

	//Word 0 through a pointer needs no Y ([[D + p]:24]); word 1 at $07FFFE + 2 is over the bank.
	szAssembly.Init();
	AssertLong(0x12345678 + 0x1234, TestIR816Run(szWordsIR, true, &szAssembly));
	AssertTrue(strstr(szAssembly.Text(), "]:24]\n") != NULL);
	AssertTrue(strstr(szAssembly.Text(), "\t\tldy 0\n") == NULL);
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the line before szLine is an lda of a direct page word.
//
//////////////////////////////////////////////////////////////////////////
static bool TestIR816PreviousIsDirectLoad(const char* szAssembly, const char* szLine)
{
	const char*	szPrevious;

	szPrevious = szLine - 1;
	while ((szPrevious > szAssembly) && (szPrevious[-1] != '\n'))
	{
		szPrevious--;
	}
	return strncmp(szPrevious, "\t\tlda [D + ", 11) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the code has a bra to the label after it, a branch to a label
// whose first instruction is a bra, a branch over a bra to the label after
// it, an lda before a bra to code that starts with lda or txa, tax and txa
// together, or a loop's mark (PeepholeBranches leaves none).
//
//////////////////////////////////////////////////////////////////////////
static bool TestIR816UntidyBranch(const char* szAssembly)
{
	const char*	szLine;
	const char*	szEnd;
	const char*	szTarget;
	char		szLabel[40];
	int			iLength;

	if (strstr(szAssembly, "\t\ttax\n\t\ttxa\n") || strstr(szAssembly, "\t\ttxa\n\t\ttax\n") || strstr(szAssembly, "\t\ttax\n\t\tclc\n\t\ttxa\n") || strstr(szAssembly, " ;R\n"))
	{
		return true;
	}
	for (szLine = szAssembly; *szLine; szLine = *szEnd ? szEnd + 1 : szEnd)
	{
		szEnd = strchr(szLine, '\n');
		if (!szEnd)
		{
			szEnd = szLine + strlen(szLine);
		}
		if ((szLine[0] != '\t') || (szLine[2] != 'b') || (szLine[5] != ' ') || (szLine[6] != 'L'))
		{
			continue;
		}
		iLength = (int)(szEnd - (szLine + 6));
		if ((iLength <= 0) || (iLength > 30))
		{
			continue;
		}
		snprintf(szLabel, sizeof(szLabel), "\n%.*s:\n", iLength, szLine + 6);
		if ((strncmp(szLine + 2, "bra", 3) == 0) && (strncmp(szEnd, szLabel, strlen(szLabel)) == 0))
		{
			return true;
		}
		szTarget = strstr(szAssembly, szLabel);
		if (szTarget && (strncmp(szTarget + strlen(szLabel), "\t\tbra ", 6) == 0))
		{
			return true;
		}
		if ((strncmp(szLine + 2, "bra", 3) != 0) && (strncmp(szEnd, "\n\t\tbra L", 8) == 0) && strchr(szEnd + 1, '\n') &&
			(strncmp(strchr(szEnd + 1, '\n'), szLabel, strlen(szLabel)) == 0))
		{
			return true;
		}
		if ((strncmp(szLine + 2, "bra", 3) == 0) && szTarget && ((strncmp(szTarget + strlen(szLabel), "\t\tlda ", 6) == 0) || (strncmp(szTarget + strlen(szLabel), "\t\ttxa", 5) == 0)) &&
			(szLine >= szAssembly + 2) && TestIR816PreviousIsDirectLoad(szAssembly, szLine))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel loop clipped at both sides (like DrawSprite's): its branches
// are tidied (PeepholeBranches) and it still works out the same.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendBranches(void)
{
	static const char*	szIR =
		"function i32 @_main()\n{\nentry:\n\t%p = itoptr i32 524272 to ptr\n\t%o = itoptr i32 458736 to ptr\n\tjmp fill\n"
		"fill:\n\t%i = phi i16 [entry, 0], [fill, %i2]\n\t%w = zext i16 %i to i32\n\t%a = ptradd ptr %p, %w\n"
		"\t%b = trunc i16 %i to i8\n\tstore i8 %b, %a\n\t%i2 = add i16 %i, 1\n\t%c = ult i16 %i2, 32\n\tbrif %c, fill, start\n"
		"start:\n\tjmp loop\n"
		"loop:\n\t%x = phi i16 [start, 65531], [next, %x2]\n\t%s = phi ptr [start, %p], [next, %s2]\n"
		"\t%n = slt i16 %x, 0\n\tbrif %n, next, right\n"
		"right:\n\t%r = ugt i16 %x, 19\n\tbrif %r, next, draw\n"
		"draw:\n\t%v = load i8 %s\n\t%xw = zext i16 %x to i32\n\t%d = ptradd ptr %o, %xw\n\tstore i8 %v, %d\n\tjmp next\n"
		"next:\n\t%s2 = ptradd ptr %s, 1\n\t%x2 = add i16 %x, 1\n\t%e = ne i16 %x2, 27\n\tbrif %e, loop, sum\n"
		"sum:\n\t%k = phi i16 [next, 0], [sum, %k2]\n\t%t = phi i16 [next, 0], [sum, %t2]\n\t%y = zext i16 %k to i32\n"
		"\t%q = ptradd ptr %o, %y\n\t%f = load i8 %q\n\t%fw = zext i8 %f to i16\n\t%t2 = add i16 %t, %fw\n"
		"\t%k2 = add i16 %k, 1\n\t%m = ult i16 %k2, 20\n\tbrif %m, sum, done\n"
		"done:\n\t%u = zext i16 %t2 to i32\n\tret i32 %u\n}\n";
	CChars				szAssembly;

	//x from -5 to 26 draws x = 0..19 from source bytes 5..24.
	szAssembly.Init();
	AssertLong(290, TestIR816Run(szIR, true, &szAssembly));
	AssertFalse(TestIR816UntidyBranch(szAssembly.Text()));
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The code PeepholeRegisters makes of szCode.
//
//////////////////////////////////////////////////////////////////////////
static void AssertPeepholeRegisters(const char* szExpected, const char* szCode)
{
	CChars	sz;

	sz.Init(szCode);
	CIR816Backend::PeepholeRegisters(&sz, 0);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A load of what the register already holds goes: after a load and a store
// (A holds both words), after transfers and of constants.  It stays when
// the N and Z it sets are read, the word may have been written (an
// overlapping store, a store through a pointer, a step of memory), or a
// label, call or width change came between.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendPeepholeRegisters(void)
{
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tadc 1\n\t\tsta [D + 11]\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tlda [D + 5]\n\t\tadc 1\n\t\tsta [D + 11]\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tclc\n\t\tadc 1\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tlda [D + 9]\n\t\tclc\n\t\tadc 1\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tldx 4\n\t\ttxa\n\t\ttay\n\t\tsta [D + 3]\n\t\trtl\n",
		"\t\tldx 4\n\t\ttxa\n\t\ttay\n\t\tldy 4\n\t\tlda 4\n\t\tsta [D + 3]\n\t\trtl\n");

	//The N and Z it sets are read: kept.
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tldx 3\n\t\tlda [D + 9]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tldx 3\n\t\tlda [D + 9]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n");

	//The word may have changed: kept.
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tsta [D + 6]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tsta [D + 6]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tldy 2\n\t\tsta [[D + 9]:24 + Y]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tldy 2\n\t\tsta [[D + 9]:24 + Y]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tinc [D + 5]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tinc [D + 5]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tsta [D + X]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tsta [D + X]\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n");

	//A label, a call or a width change between: kept.
	AssertPeepholeRegisters("\t\tlda 1\nL1:\n\t\tlda 1\n\t\tsta [D + 3]\n\t\trtl\n", "\t\tlda 1\nL1:\n\t\tlda 1\n\t\tsta [D + 3]\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tlda 1\n\t\tjsl _F\n\t\tlda 1\n\t\tsta [D + 3]\n\t\trtl\n", "\t\tlda 1\n\t\tjsl _F\n\t\tlda 1\n\t\tsta [D + 3]\n\t\trtl\n");
	AssertPeepholeRegisters("\t\tlda 1\n\t\tsep m\n\t\tlda 1\n\t\tsta [D + 3]\n\t\trtl\n", "\t\tlda 1\n\t\tsep m\n\t\tlda 1\n\t\tsta [D + 3]\n\t\trtl\n");

	//A changed: kept.
	AssertPeepholeRegisters("\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tadc 1\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n",
		"\t\tlda [D + 5]\n\t\tsta [D + 9]\n\t\tadc 1\n\t\tlda [D + 5]\n\t\tsta [D + 3]\n\t\trtl\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The code PeepholeBranches makes of szCode.
//
//////////////////////////////////////////////////////////////////////////
static void AssertPeepholeBranches(const char* szExpected, const char* szCode)
{
	CChars	sz;

	sz.Init(szCode);
	CIR816Backend::PeepholeBranches(&sz, 0);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each of PeepholeBranches's rules on code written for it, and code each
// must leave alone.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendPeepholeBranches(void)
{
	//A bra to the label after it.
	AssertPeepholeBranches("\t\tlda 1\n\t\trtl\n", "\t\tlda 1\n\t\tbra L2\nL2:\n\t\trtl\n");

	//A branch to a bra goes where it goes (and the bra, reached no other way, goes).
	AssertPeepholeBranches("\t\tbeq L3\n\t\tlda 2\n\t\trtl\nL3:\n\t\trtl\n",
		"\t\tbeq L1\n\t\tlda 2\n\t\trtl\nL1:\n\t\tbra L3\nL3:\n\t\trtl\n");

	//Bcc X, bra Y, X: is B!cc Y, X:.
	AssertPeepholeBranches("\t\tcmp 5\n\t\tbcs L2\n\t\tlda 1\nL2:\n\t\trtl\n",
		"\t\tcmp 5\n\t\tbcc L1\n\t\tbra L2\nL1:\n\t\tlda 1\nL2:\n\t\trtl\n");

	//bcc X, beq X, bra Y, X: is beq X, bcs Y, X:.
	AssertPeepholeBranches("\t\tcmp 5\n\t\tbeq L1\n\t\tbcs L2\nL1:\n\t\tlda 1\nL2:\n\t\trtl\n",
		"\t\tcmp 5\n\t\tbcc L1\n\t\tbeq L1\n\t\tbra L2\nL1:\n\t\tlda 1\nL2:\n\t\trtl\n");

	//Round a loop (marked ;R) is the branch taken; the mark goes.  Then L3's code, after the bra out, can't be
	//reached, and the bra goes to the label after it.
	AssertPeepholeBranches("L1:\n\t\tinx\n\t\tcpx 5\n\t\tbne L1\n\t\trtl\n",
		"L1:\n\t\tinx\n\t\tcpx 5\n\t\tbeq L2\n\t\tbra L1 ;R\nL3:\n\t\tlda 1\nL2:\n\t\trtl\n");

	//An lda for nothing: before a bra to code that sets A first (past clc), and falling into code that does (past a
	//label and ldx).
	AssertPeepholeBranches("\t\tbra L1\nL2:\n\t\trtl\nL1:\n\t\tclc\n\t\tlda 4\n\t\tbra L2\n",
		"\t\tlda [D + 3]\n\t\tbra L1\nL2:\n\t\trtl\nL1:\n\t\tclc\n\t\tlda 4\n\t\tbra L2\n");
	AssertPeepholeBranches("\t\tldx 2\n\t\ttxa\n\t\trtl\n", "\t\tlda [D + 3]\nL1:\n\t\tldx 2\n\t\ttxa\n\t\trtl\n");
	//...and past steps and shifts of memory, pushes and index compares (IR816LeavesA, as IR816DeadLoad).
	AssertPeepholeBranches("\t\tinc [D + 5]\n\t\tpei [D + 7]\n\t\tphx\n\t\tcpx 3\n\t\tlda 1\n\t\trtl\n",
		"\t\tlda [D + 3]\nL1:\n\t\tinc [D + 5]\n\t\tpei [D + 7]\n\t\tphx\n\t\tcpx 3\n\t\tlda 1\n\t\trtl\n");
	AssertPeepholeBranches("\t\tlda [D + 3]\n\t\tinc A\n\t\tlda 1\n\t\trtl\n", "\t\tlda [D + 3]\nL1:\n\t\tinc A\n\t\tlda 1\n\t\trtl\n");
	AssertPeepholeBranches("\t\tlda [D + 3]\n\t\tpha\n\t\tlda 1\n\t\trtl\n", "\t\tlda [D + 3]\nL1:\n\t\tpha\n\t\tlda 1\n\t\trtl\n");

	//Kept: a branch reads its N and Z, the code after reads A, or it reads memory that isn't the direct page's.
	AssertPeepholeBranches("\t\tlda [D + 3]\n\t\tbeq L2\n\t\trtl\nL2:\n\t\trtl\n", "\t\tlda [D + 3]\nL1:\n\t\tbeq L2\n\t\trtl\nL2:\n\t\trtl\n");
	AssertPeepholeBranches("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\trtl\n", "\t\tlda [D + 3]\nL1:\n\t\tclc\n\t\tadc 4\n\t\trtl\n");
	AssertPeepholeBranches("\t\tlda [_gX + 0]:24\n\t\tlda 1\n\t\trtl\n", "\t\tlda [_gX + 0]:24\n\t\tlda 1\n\t\trtl\n");

	//txa after tax with clc between.
	AssertPeepholeBranches("\t\ttax\n\t\tclc\n\t\tadc 1\n\t\trtl\n", "\t\ttax\n\t\tclc\n\t\ttxa\n\t\tadc 1\n\t\trtl\n");

	//A tax for nothing: X written again before it is read.  Kept when [D + X], a label or a call comes first.
	AssertPeepholeBranches("\t\tclc\n\t\tadc [D + 3]\n\t\ttax\n\t\tstx [D + 5]\n\t\trtl\n", "\t\ttax\n\t\tclc\n\t\tadc [D + 3]\n\t\ttax\n\t\tstx [D + 5]\n\t\trtl\n");
	AssertPeepholeBranches("\t\ttax\n\t\tlda [D + X]\n\t\ttax\n\t\trtl\n", "\t\ttax\n\t\tlda [D + X]\n\t\ttax\n\t\trtl\n");
	AssertPeepholeBranches("\t\ttax\nL1:\n\t\tldx 3\n\t\tbne L1\n\t\trtl\n", "\t\ttax\nL1:\n\t\tldx 3\n\t\tbne L1\n\t\trtl\n");
	AssertPeepholeBranches("\t\ttax\n\t\tjsl _F\n\t\tldx 3\n\t\trtl\n", "\t\ttax\n\t\tjsl _F\n\t\tldx 3\n\t\trtl\n");

	//A second txy with nothing between that changes X or Y.
	AssertPeepholeBranches("\t\ttxy\n\t\tlda [[D + 3]:24 + Y]\n\t\tsta [[D + 7]:24 + Y]\n\t\trtl\n",
		"\t\ttxy\n\t\tlda [[D + 3]:24 + Y]\n\t\ttxy\n\t\tsta [[D + 7]:24 + Y]\n\t\trtl\n");

	//Code after rtl up to a label that is used.
	AssertPeepholeBranches("\t\tbeq L6\n\t\trtl\nL6:\n\t\tlda 3\n\t\trtl\n", "\t\tbeq L6\n\t\trtl\n\t\tlda 1\nL5:\n\t\tlda 2\nL6:\n\t\tlda 3\n\t\trtl\n");

	//A label nothing names goes, so a branch over a bra round a loop is one branch: beq L1, bcs L2, L3:, bra L1, L2: is beq L1,
	//bcc L1, L2:.  One a jump table names stays.
	AssertPeepholeBranches("L1:\n\t\tinx\n\t\tcpx [D + 25]\n\t\tbeq L1\n\t\tbcc L1\n\t\tlda 0\n\t\trtl\n",
		"L1:\n\t\tinx\n\t\tcpx [D + 25]\nL4:\n\t\tbeq L1\n\t\tbcs L2\nL3:\n\t\tbra L1\nL2:\n\t\tlda 0\n\t\trtl\n");
	AssertPeepholeBranches("\t\tjmp [L7 + X]\nL7:\n\t\t.word L8 & 0xFFFF\nL8:\n\t\trtl\n", "\t\tjmp [L7 + X]\nL7:\n\t\t.word L8 & 0xFFFF\nL8:\n\t\trtl\n");
}


typedef void (*TestIR816TextPass)(CChars* pszDest, size uiStart);


//////////////////////////////////////////////////////////////////////////
//
// The code a text pass makes of szCode: from the start, and from after a
// function's label that it must leave alone.
//
//////////////////////////////////////////////////////////////////////////
static void AssertIR816Pass(const char* szExpected, const char* szCode, TestIR816TextPass fPass)
{
	CChars	sz;
	CChars	szAll;

	sz.Init(szCode);
	fPass(&sz, 0);
	AssertString(szExpected, sz.Text());
	sz.Kill();

	sz.Init("_F:\n");
	sz.Append(szCode);
	fPass(&sz, 4);
	szAll.Init("_F:\n");
	szAll.Append(szExpected);
	AssertString(szAll.Text(), sz.Text());
	szAll.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A store to a direct page word nothing in the function reads goes
// (RemoveUnreadStores), even past a frame's exit (tdc to tcs).  Kept when
// any byte of it is read (an overlapping word, a pointer through it), when
// a runtime routine may read it (the scratch bytes), and in a function
// that takes the frame's address or indexes the direct page.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendRemoveUnreadStores(void)
{
	TestIR816TextPass	fPass;

	fPass = CIR816Backend::RemoveUnreadStores;
	AssertIR816Pass("\t\tlda 1\n\t\tsta [D + 7]\n\t\tlda [D + 7]\n\t\ttay\n\t\ttdc\n\t\tclc\n\t\tadc 20\n\t\ttcs\n\t\ttya\n\t\tpld\n\t\trtl\n",
		"\t\tlda 1\n\t\tsta [D + 35]\n\t\tsta [D + 7]\n\t\tlda [D + 7]\n\t\ttay\n\t\ttdc\n\t\tclc\n\t\tadc 20\n\t\ttcs\n\t\ttya\n\t\tpld\n\t\trtl\n", fPass);

	//Read: an overlapping word, a pointer through it, a runtime routine's scratch bytes.
	AssertIR816Pass("\t\tlda 1\n\t\tsta [D + 35]\n\t\tlda [D + 36]\n\t\trtl\n", "\t\tlda 1\n\t\tsta [D + 35]\n\t\tlda [D + 36]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda 1\n\t\tsta [D + 36]\n\t\tlda [[D + 34]:24]\n\t\trtl\n", "\t\tlda 1\n\t\tsta [D + 36]\n\t\tlda [[D + 34]:24]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda 1\n\t\tsta [D + 19]\n\t\tjsl R816_Mul16\n\t\trtl\n", "\t\tlda 1\n\t\tsta [D + 19]\n\t\tjsl R816_Mul16\n\t\trtl\n", fPass);

	//The frame's address taken, or the direct page indexed: nothing goes.
	AssertIR816Pass("\t\tlda 1\n\t\tsta [D + 35]\n\t\tpea 0\n\t\ttdc\n\t\tclc\n\t\tadc 35\n\t\tpha\n\t\tjsl _F\n\t\trtl\n",
		"\t\tlda 1\n\t\tsta [D + 35]\n\t\tpea 0\n\t\ttdc\n\t\tclc\n\t\tadc 35\n\t\tpha\n\t\tjsl _F\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda 1\n\t\tsta [D + 35]\n\t\tldx 35\n\t\tlda [D + X]\n\t\trtl\n", "\t\tlda 1\n\t\tsta [D + 35]\n\t\tldx 35\n\t\tlda [D + X]\n\t\trtl\n", fPass);
}


//////////////////////////////////////////////////////////////////////////
//
// Peephole: in straight line code, a value stored by a store marked " ;A"
// stays in A for its one lda or pei (IR816KeepsInA: not when N or Z of the
// lda is read) or as the first operand of a commutative instruction after
// an lda of something else (IR816SwapsInA); pei of the word A holds is pha;
// an lda of the word A holds goes (not before a branch on Z or N, nor after
// the word changes); an ldy of the constant Y holds goes; adc 0 after clc
// (and an lda) goes; an sta of A to the word it holds goes; and cmp 0 after
// an instruction that set N and Z from A goes when a branch on Z or N
// follows and no carry is read after the branch or where it goes.  It ends
// with RemoveDeadStores.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendPeephole(void)
{
	TestIR816TextPass	fPass;

	fPass = CIR816Backend::Peephole;

	//A marked store and its lda both go, the value kept in A.  Not when a branch reads the lda's Z (the mark goes).
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 5]\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23] ;A\n\t\tlda [D + 23]\n\t\tsta [D + 5]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23]\n\t\tlda [D + 23]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23] ;A\n\t\tlda [D + 23]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n", fPass);

	//A marked store pushed by pei: pha.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tpha\n\t\tjsl _F\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23] ;A\n\t\tpei [D + 23]\n\t\tjsl _F\n\t\trtl\n", fPass);

	//A marked store kept in A as adc's first operand, the lda's operand its second.  Not for sbc.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tclc\n\t\tadc [D + 7]\n\t\tsta [D + 9]\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23] ;A\n\t\tclc\n\t\tlda [D + 7]\n\t\tadc [D + 23]\n\t\tsta [D + 9]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23]\n\t\tsec\n\t\tlda [D + 7]\n\t\tsbc [D + 23]\n\t\tsta [D + 9]\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tclc\n\t\tadc 4\n\t\tsta [D + 23] ;A\n\t\tsec\n\t\tlda [D + 7]\n\t\tsbc [D + 23]\n\t\tsta [D + 9]\n\t\trtl\n", fPass);

	//pei of the word A holds is pha; of another word it is kept.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tpha\n\t\tjsl _F\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tpei [D + 3]\n\t\tjsl _F\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tpei [D + 5]\n\t\tjsl _F\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tpei [D + 5]\n\t\tjsl _F\n\t\trtl\n", fPass);

	//An lda of the word A holds goes.  Kept before bne; and after an inc of the word (then the first lda is dead).
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tsta [D + 5]\n\t\tsta [D + 7]\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tsta [D + 5]\n\t\tlda [D + 5]\n\t\tsta [D + 7]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tsta [D + 5]\n\t\tlda [D + 5]\n\t\tbne L1\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tsta [D + 5]\n\t\tlda [D + 5]\n\t\tbne L1\n\t\trtl\nL1:\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tinc [D + 3]\n\t\tlda [D + 3]\n\t\tsta [D + 7]\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tinc [D + 3]\n\t\tlda [D + 3]\n\t\tsta [D + 7]\n\t\trtl\n", fPass);

	//An ldy of the constant Y holds goes.  Not after an iny.
	AssertIR816Pass("\t\tldy 2\n\t\tlda [[D + 3]:24 + Y]\n\t\tsta [[D + 7]:24 + Y]\n\t\trtl\n",
		"\t\tldy 2\n\t\tlda [[D + 3]:24 + Y]\n\t\tldy 2\n\t\tsta [[D + 7]:24 + Y]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tldy 2\n\t\tlda [[D + 3]:24 + Y]\n\t\tiny\n\t\tldy 2\n\t\tsta [[D + 7]:24 + Y]\n\t\trtl\n",
		"\t\tldy 2\n\t\tlda [[D + 3]:24 + Y]\n\t\tiny\n\t\tldy 2\n\t\tsta [[D + 7]:24 + Y]\n\t\trtl\n", fPass);

	//adc 0 after clc and an lda goes.  Kept after an adc (the high word of an add, the carry from the low).
	AssertIR816Pass("\t\tclc\n\t\tlda [D + 3]\n\t\tsta [D + 5]\n\t\trtl\n", "\t\tclc\n\t\tlda [D + 3]\n\t\tadc 0\n\t\tsta [D + 5]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tclc\n\t\tlda [D + 3]\n\t\tadc [D + 7]\n\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tadc 0\n\t\tsta [D + 11]\n\t\trtl\n",
		"\t\tclc\n\t\tlda [D + 3]\n\t\tadc [D + 7]\n\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tadc 0\n\t\tsta [D + 11]\n\t\trtl\n", fPass);

	//An sta of A back to the word it was loaded from goes.  Kept when the word changed between.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tldx 2\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tldx 2\n\t\tsta [D + 3]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tinc [D + 3]\n\t\tsta [D + 3]\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tinc [D + 3]\n\t\tsta [D + 3]\n\t\trtl\n", fPass);

	//cmp 0 after and (N and Z from A) before beq goes.  Kept when the carry is read where beq goes, after beq, or
	//when ldx set N and Z last.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand [D + 5]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\tlda 1\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tand [D + 5]\n\t\tcmp 0\n\t\tbeq L1\n\t\trtl\nL1:\n\t\tlda 1\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand [D + 5]\n\t\tcmp 0\n\t\tbeq L1\n\t\trtl\nL1:\n\t\tbcs L2\n\t\trtl\nL2:\n\t\tlda 1\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tand [D + 5]\n\t\tcmp 0\n\t\tbeq L1\n\t\trtl\nL1:\n\t\tbcs L2\n\t\trtl\nL2:\n\t\tlda 1\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand [D + 5]\n\t\tcmp 0\n\t\tbeq L1\n\t\tbcs L2\n\t\trtl\nL1:\n\t\trtl\nL2:\n\t\tlda 1\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tand [D + 5]\n\t\tcmp 0\n\t\tbeq L1\n\t\tbcs L2\n\t\trtl\nL1:\n\t\trtl\nL2:\n\t\tlda 1\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tldx 2\n\t\tcmp 0\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tldx 2\n\t\tcmp 0\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n", fPass);
}


//////////////////////////////////////////////////////////////////////////
//
// RemoveDeadStores: a store to a direct page word goes when, in straight
// line code, the next instruction naming a byte near it (other than a store
// to another direct page word; a word [D + n] is near when it shares a
// byte) stores to it (IR816DeadStore); an lda of a
// direct page word or a constant goes when A is loaded again before A or
// its N and Z are read (IR816DeadLoad).
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendRemoveDeadStores(void)
{
	TestIR816TextPass	fPass;

	fPass = CIR816Backend::RemoveDeadStores;

	//The first store is overwritten unread, past a store to the word next to it.
	AssertIR816Pass("\t\tsta [D + 25]\n\t\tstz [D + 23]\n\t\trtl\n", "\t\tsta [D + 23]\n\t\tsta [D + 25]\n\t\tstz [D + 23]\n\t\trtl\n", fPass);

	//A word read two bytes away ([D + 25] is bytes 25 and 26) doesn't read it: the copy of a value's low word, and the
	//lda feeding it, go.  Kept when the word read shares a byte with it ([D + 26]).
	AssertIR816Pass("\t\tlda [D + 25]\n\t\tsta [D + 27]\n\t\trtl\n", "\t\tlda [D + 23]\n\t\tsta [D + 27]\n\t\tlda [D + 25]\n\t\tsta [D + 27]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 23]\n\t\tsta [D + 27]\n\t\tlda [D + 26]\n\t\tsta [D + 27]\n\t\trtl\n", "\t\tlda [D + 23]\n\t\tsta [D + 27]\n\t\tlda [D + 26]\n\t\tsta [D + 27]\n\t\trtl\n", fPass);

	//Kept: a pointer at [D + 22] read or stored through, a branch or a label before the next store.
	AssertIR816Pass("\t\tsta [D + 23]\n\t\tlda [[D + 22]:24]\n\t\tstz [D + 23]\n\t\trtl\n", "\t\tsta [D + 23]\n\t\tlda [[D + 22]:24]\n\t\tstz [D + 23]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 23]\n\t\tsta [[D + 22]:24]\n\t\tstz [D + 23]\n\t\trtl\n", "\t\tsta [D + 23]\n\t\tsta [[D + 22]:24]\n\t\tstz [D + 23]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 23]\n\t\tbne L1\n\t\tstz [D + 23]\nL1:\n\t\trtl\n", "\t\tsta [D + 23]\n\t\tbne L1\n\t\tstz [D + 23]\nL1:\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 23]\nL1:\n\t\tstz [D + 23]\n\t\trtl\n", "\t\tsta [D + 23]\nL1:\n\t\tstz [D + 23]\n\t\trtl\n", fPass);

	//An lda loaded over unread goes; kept when tax reads it.
	AssertIR816Pass("\t\tldx 2\n\t\tlda 5\n\t\tsta [D + 7]\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tldx 2\n\t\tlda 5\n\t\tsta [D + 7]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\ttax\n\t\tlda 5\n\t\tsta [D + 7]\n\t\trtl\n", "\t\tlda [D + 3]\n\t\ttax\n\t\tlda 5\n\t\tsta [D + 7]\n\t\trtl\n", fPass);
}


//////////////////////////////////////////////////////////////////////////
//
// RemoveFlowDeadStores (as a TestIR816TextPass).
//
//////////////////////////////////////////////////////////////////////////
static void TestIR816RemoveFlowDeadStores(CChars* pszDest, size uiStart)
{
	CIR816Backend::RemoveFlowDeadStores(pszDest, uiStart);
}


//////////////////////////////////////////////////////////////////////////
//
// RemoveFlowDeadStores in a function whose frame is reached other than as
// [D + n]: a pointer into the frame (tdc, clc, adc n) keeps the stores from
// n on but not those below; [D + X] after ldx m is [D + m] (or past the
// direct page); a 64 bit shift routine after ldx m reads the eight bytes at
// m.  An X not loaded just before still makes it give up.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendRemoveFlowDeadStores(void)
{
	TestIR816TextPass	fPass;
	CChars				szCode;
	CChars				szExpected;
	const char*			szEntry;
	const char*			szExit;

	fPass = TestIR816RemoveFlowDeadStores;
	szEntry = "\t\tphd\n\t\ttsc\n\t\tsec\n\t\tsbc 40\n\t\ttcs\n\t\ttcd\n";
	szExit = "\t\ttdc\n\t\tclc\n\t\tadc 40\n\t\ttcs\n\t\tpld\n\t\trtl\n";

	//A pointer to D + 30 is pushed for _F: the store to 23, below it, is dead; the one to 31 might be read through it.
	szCode.Init(szEntry);
	szCode.Append("\t\ttdc\n\t\tclc\n\t\tadc 30\n\t\tsta [D + 3]\n\t\tstz [D + 5]\n\t\tlda 7\n\t\tsta [D + 23]\n\t\tsta [D + 31]\n\t\tpei [D + 5]\n\t\tpei [D + 3]\n\t\tjsl _F\n");
	szCode.Append(szExit);
	szExpected.Init(szEntry);
	szExpected.Append("\t\ttdc\n\t\tclc\n\t\tadc 30\n\t\tsta [D + 3]\n\t\tstz [D + 5]\n\t\tlda 7\n\t\tsta [D + 31]\n\t\tpei [D + 5]\n\t\tpei [D + 3]\n\t\tjsl _F\n");
	szExpected.Append(szExit);
	AssertIR816Pass(szExpected.Text(), szCode.Text(), fPass);
	szExpected.Kill();
	szCode.Kill();

	//[D + X] after ldx 300 is past the direct page, after ldx 23 it reads [D + 23]: the stores to 25 and 27 go.
	szCode.Init(szEntry);
	szCode.Append("\t\tlda 7\n\t\tsta [D + 23]\n\t\tsta [D + 25]\n\t\tldx 300\n\t\tlda [D + X]\n\t\tldx 23\n\t\tadc [D + X]\n\t\tsta [D + 27]\n");
	szCode.Append(szExit);
	szExpected.Init(szEntry);
	szExpected.Append("\t\tlda 7\n\t\tsta [D + 23]\n\t\tldx 300\n\t\tlda [D + X]\n\t\tldx 23\n\t\tadc [D + X]\n");
	szExpected.Append(szExit);
	AssertIR816Pass(szExpected.Text(), szCode.Text(), fPass);
	szExpected.Kill();
	szCode.Kill();

	//R816_LShr64 after ldx 31 reads bytes 31 to 38: the store to 33 stays, the one to 27 goes.
	szCode.Init(szEntry);
	szCode.Append("\t\tlda 7\n\t\tsta [D + 33]\n\t\tsta [D + 27]\n\t\tlda 3\n\t\tldx 31\n\t\tjsl R816_LShr64\n");
	szCode.Append(szExit);
	szExpected.Init(szEntry);
	szExpected.Append("\t\tlda 7\n\t\tsta [D + 33]\n\t\tlda 3\n\t\tldx 31\n\t\tjsl R816_LShr64\n");
	szExpected.Append(szExit);
	AssertIR816Pass(szExpected.Text(), szCode.Text(), fPass);
	szExpected.Kill();
	szCode.Kill();

	//An X loaded before a branch to L1 or before a call, not just before [D + X]: nothing goes.
	szCode.Init(szEntry);
	szCode.Append("\t\tlda 7\n\t\tsta [D + 23]\n\t\tsta [D + 27]\n\t\tldx 23\n\t\tbne L1\n\t\tldx 25\nL1:\n\t\tlda [D + X]\n");
	szCode.Append(szExit);
	AssertIR816Pass(szCode.Text(), szCode.Text(), fPass);
	szCode.Kill();
	szCode.Init(szEntry);
	szCode.Append("\t\tlda 7\n\t\tsta [D + 27]\n\t\tldx 23\n\t\tjsl _F\n\t\tlda [D + X]\n");
	szCode.Append(szExit);
	AssertIR816Pass(szCode.Text(), szCode.Text(), fPass);
	szCode.Kill();

	//[D + X] with X not loaded just before (tax), and tdc that isn't clc, adc n: nothing goes.
	szCode.Init(szEntry);
	szCode.Append("\t\tlda 7\n\t\tsta [D + 27]\n\t\ttax\n\t\tlda [D + X]\n");
	szCode.Append(szExit);
	AssertIR816Pass(szCode.Text(), szCode.Text(), fPass);
	szCode.Kill();
	szCode.Init(szEntry);
	szCode.Append("\t\tlda 7\n\t\tsta [D + 27]\n\t\ttdc\n\t\tpha\n\t\tjsl _F\n");
	szCode.Append(szExit);
	AssertIR816Pass(szCode.Text(), szCode.Text(), fPass);
	szCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// PeepholeMasks: an "and 0xFF" goes when only clc, sec, adc, sbc, ora, eor,
// and or asl A come before the next "and 0xFF" (A's low byte doesn't depend
// on the high byte through them), and no carry they leave is read after it
// (IR816MaskRedone).
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendPeepholeMasks(void)
{
	TestIR816TextPass	fPass;

	fPass = CIR816Backend::PeepholeMasks;

	//The first mask goes: the clc after the second sets the carry before it is read.  With ora between, no carry.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tclc\n\t\tadc 3\n\t\tand 0xFF\n\t\tsta [D + 5]\n\t\tclc\n\t\tadc [D + 7]\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tclc\n\t\tadc 3\n\t\tand 0xFF\n\t\tsta [D + 5]\n\t\tclc\n\t\tadc [D + 7]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tora 4\n\t\tand 0xFF\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tora 4\n\t\tand 0xFF\n\t\trtl\n", fPass);

	//Kept: the adc's carry is read after the second mask (bcs; and rtl might), or xba, lsr A or sta comes between.
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tclc\n\t\tadc 3\n\t\tand 0xFF\n\t\tbcs L1\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tclc\n\t\tadc 3\n\t\tand 0xFF\n\t\tbcs L1\n\t\trtl\nL1:\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tclc\n\t\tadc 3\n\t\tand 0xFF\n\t\trtl\n",
		"\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tclc\n\t\tadc 3\n\t\tand 0xFF\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand 0xFF\n\t\txba\n\t\tand 0xFF\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tand 0xFF\n\t\txba\n\t\tand 0xFF\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tlsr A\n\t\tand 0xFF\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tlsr A\n\t\tand 0xFF\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tsta [D + 5]\n\t\tand 0xFF\n\t\trtl\n", "\t\tlda [D + 3]\n\t\tand 0xFF\n\t\tsta [D + 5]\n\t\tand 0xFF\n\t\trtl\n", fPass);
}


//////////////////////////////////////////////////////////////////////////
//
// PeepholeStores: sta w / dec w / lda w is dec A / sta w (inc the same);
// sta w / ldy w is sta w / tay (ldx: tax); sta w ... ldy w with neither Y
// nor a byte near w used between (IR816LoadYLater) and no N or Z read after
// either is sta w / tay ...; and the sta goes too when w is written again
// before it is read (IR816DeadStore).
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendPeepholeStores(void)
{
	TestIR816TextPass	fPass;

	fPass = CIR816Backend::PeepholeStores;

	//dec and inc in A; the sta goes too when stz overwrites it.
	AssertIR816Pass("\t\tdec A\n\t\tsta [D + 5]\n\t\tsta [D + 7]\n\t\trtl\n", "\t\tsta [D + 5]\n\t\tdec [D + 5]\n\t\tlda [D + 5]\n\t\tsta [D + 7]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tinc A\n\t\tsta [D + 7]\n\t\tstz [D + 5]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tinc [D + 5]\n\t\tlda [D + 5]\n\t\tsta [D + 7]\n\t\tstz [D + 5]\n\t\trtl\n", fPass);

	//Kept: the lda after the dec is of another word.
	AssertIR816Pass("\t\tsta [D + 5]\n\t\tdec [D + 5]\n\t\tlda [D + 9]\n\t\trtl\n", "\t\tsta [D + 5]\n\t\tdec [D + 5]\n\t\tlda [D + 9]\n\t\trtl\n", fPass);

	//ldy and ldx of the word just stored: tay and tax; the sta goes too when the word is stored again unread.
	AssertIR816Pass("\t\tsta [D + 5]\n\t\ttay\n\t\tlda [[D + 9]:24 + Y]\n\t\trtl\n", "\t\tsta [D + 5]\n\t\tldy [D + 5]\n\t\tlda [[D + 9]:24 + Y]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\ttay\n\t\tlda [[D + 9]:24 + Y]\n\t\tsta [D + 5]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tldy [D + 5]\n\t\tlda [[D + 9]:24 + Y]\n\t\tsta [D + 5]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 5]\n\t\ttax\n\t\trtl\n", "\t\tsta [D + 5]\n\t\tldx [D + 5]\n\t\trtl\n", fPass);

	//sta w ... ldy w with only A's work between: tay at once.
	AssertIR816Pass("\t\tsta [D + 5]\n\t\ttay\n\t\tlda [D + 9]\n\t\tclc\n\t\tadc 2\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tclc\n\t\tadc 2\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n", fPass);

	//Kept: iny, a Y indexed load, or a mention of [D + 4] or [D + 6] between; or beq reads the ldy's Z.
	AssertIR816Pass("\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tiny\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tiny\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 5]\n\t\tlda [[D + 9]:24 + Y]\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tlda [[D + 9]:24 + Y]\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 5]\n\t\tlda [D + 4]\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tlda [D + 4]\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 5]\n\t\tlda [D + 6]\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tlda [D + 6]\n\t\tldy [D + 5]\n\t\tsta [[D + 11]:24 + Y]\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tldy [D + 5]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\tsta [D + 5]\n\t\tlda [D + 9]\n\t\tldy [D + 5]\n\t\tbeq L1\n\t\trtl\nL1:\n\t\trtl\n", fPass);
}


//////////////////////////////////////////////////////////////////////////
//
// PeepholeTransfers: tax ... txy is tay ... when nothing between uses X or
// Y or branches, X is written before it is read after the txy
// (IR816XDeadAfter) and nothing reads the N and Z the txy set.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendPeepholeTransfers(void)
{
	TestIR816TextPass	fPass;

	fPass = CIR816Backend::PeepholeTransfers;

	AssertIR816Pass("\t\ttay\n\t\tlda [D + 3]\n\t\tlda [[D + 5]:24 + Y]\n\t\tldx 2\n\t\trtl\n",
		"\t\ttax\n\t\tlda [D + 3]\n\t\ttxy\n\t\tlda [[D + 5]:24 + Y]\n\t\tldx 2\n\t\trtl\n", fPass);

	//Kept: bne after the txy, an X indexed load between, or stx reading X after.
	AssertIR816Pass("\t\ttax\n\t\tlda [D + 3]\n\t\ttxy\n\t\tbne L1\n\t\tldx 2\n\t\trtl\nL1:\n\t\trtl\n",
		"\t\ttax\n\t\tlda [D + 3]\n\t\ttxy\n\t\tbne L1\n\t\tldx 2\n\t\trtl\nL1:\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\ttax\n\t\tlda [D + X]\n\t\ttxy\n\t\tlda [[D + 5]:24 + Y]\n\t\tldx 2\n\t\trtl\n",
		"\t\ttax\n\t\tlda [D + X]\n\t\ttxy\n\t\tlda [[D + 5]:24 + Y]\n\t\tldx 2\n\t\trtl\n", fPass);
	AssertIR816Pass("\t\ttax\n\t\tlda [D + 3]\n\t\ttxy\n\t\tlda [[D + 5]:24 + Y]\n\t\tstx [D + 7]\n\t\trtl\n",
		"\t\ttax\n\t\tlda [D + 3]\n\t\ttxy\n\t\tlda [[D + 5]:24 + Y]\n\t\tstx [D + 7]\n\t\trtl\n", fPass);
}


//////////////////////////////////////////////////////////////////////////
//
// A frame bigger than the direct page's 256 bytes: values past it are reached through X, and an argument past it is
// copied near before it is followed as a pointer.
//
//////////////////////////////////////////////////////////////////////////
void TestIR816BackendBigFrame(void)
{
	CChars	szIR;
	int		i;
	int64	lliExpected;

	szIR.Init("global @_gValue, align 4 { i32 3 }\n\n");
	szIR.Append("function i32 @_Many__FPi(ptr %p), local\n{\nb:\n\t%x = load i32 %p\n");
	lliExpected = 0;
	for (i = 1; i <= 80; i++)
	{
		szIR.Append("\t%v");
		szIR.Append(i);
		szIR.Append(" = mul i32 %x, ");
		szIR.Append(i);
		szIR.AppendNewLine();
		lliExpected += 3 * i;
	}
	szIR.Append("\t%s1 = copy i32 %v1\n");
	for (i = 2; i <= 80; i++)
	{
		szIR.Append("\t%s");
		szIR.Append(i);
		szIR.Append(" = add i32 %s");
		szIR.Append(i - 1);
		szIR.Append(", %v");
		szIR.Append(i);
		szIR.AppendNewLine();
	}
	szIR.Append("\t%y = load i32 %p\n\t%r = add i32 %s80, %y\n\tret i32 %r\n}\n\n");
	szIR.Append("function i32 @_main()\n{\nb:\n\t%r = call i32 @_Many__FPi(ptr @_gValue)\n\tret i32 %r\n}\n");
	AssertIR816(lliExpected + 3, szIR.Text(), true);
	szIR.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR816Backend(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();
	C65C64InstructionFactory::GetInstance()->Init();
	CW65C816InstructionFactory::GetInstance()->Init();

	TestIR816BackendBasics();
	TestIR816BackendLoops();
	TestIR816BackendNarrow();
	TestIR816BackendDivide();
	TestIR816BackendDividePaths();
	TestIR816BackendMultiply();
	TestIR816BackendDivide16();
	TestIR816BackendMultiplyConstants();
	TestIR816BackendShiftConstants();
	TestIR816BackendCompareCarry();
	TestIR816BackendSharedPlaces();
	TestIR816BackendFused();
	TestIR816BackendStepInPlace();
	TestIR816BackendShifts();
	TestIR816BackendX();
	TestIR816BackendWide();
	TestIR816BackendShiftRoutines();
	TestIR816BackendMultiply64();
	TestIR816BackendMemoryAndCalls();
	TestIR816BackendIndexed();
	TestIR816BackendBranches();
	TestIR816BackendPeepholeBranches();
	TestIR816BackendPeepholeRegisters();
	TestIR816BackendRemoveUnreadStores();
	TestIR816BackendPeephole();
	TestIR816BackendRemoveDeadStores();
	TestIR816BackendRemoveFlowDeadStores();
	TestIR816BackendPeepholeMasks();
	TestIR816BackendPeepholeStores();
	TestIR816BackendPeepholeTransfers();
	TestIR816BackendBigFrame();

	SafeKill(gpcTestIR816MathTables);
	CW65C816InstructionFactory::GetInstance()->Kill();
	C65C64InstructionFactory::GetInstance()->Kill();
	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

