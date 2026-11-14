#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "CppCompilerLib/IRParser.h"
#include "CppCompilerLib/IRVerifier.h"
#include "CppCompilerLib/IR65Backend.h"
#include "CppCompilerLib/IR65BackendInternal.h"
#include "CppCompilerLib/IR65Flags.h"
#include "CppCompilerLib/IR65Peephole.h"
#include "CppCompilerLib/IR65Widths.h"
#include "CppCompilerLib/IRSourceMarkers.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


#define TEST_IR65_STEPS		20000000		//The most steps a test program may take.


//The generator's R_DivideSigned and R_DivideSigned64 (CCPPUnit::AppendDivideSigned), for programs that
//are only IR.
const char*	gszTestIR65DivideSigned =
	"\t\t.section text.R_DivideSigned\n"
	"R_DivideSigned:\n"
	"\t\tcpy X, A\n"
	"\t\txor X, B\n"
	"\t\tpsh X\n"
	"\t\tpsh A\n"
	"\t\tabs Y, A\n"
	"\t\tabs A, B\n"
	"\t\tcpy B, 0\n"
	"\t\tcpy W, 0\n"
	"\t\tdiv\n"
	"\t\tpul X\n"
	"\t\tcmp X, 0\n"
	"\t\tbrac n, R_DivideSigned1\n"
	"\t\tneg B, B\n"
	"R_DivideSigned1:\n"
	"\t\tpul X\n"
	"\t\tcmp X, 0\n"
	"\t\tbrac n, R_DivideSigned2\n"
	"\t\tneg Y, Y\n"
	"R_DivideSigned2:\n"
	"\t\tcpy A, Y\n"
	"\t\tret\n"
	"\n";

const char*	gszTestIR65DivideSigned64 =
	"\t\t.section text.R_DivideSigned64\n"
	"R_DivideSigned64:\n"
	"\t\twtth (X, Y), 64\n"
	"\t\tcpy X, A\n"
	"\t\txor X, B\n"
	"\t\tpsh X\n"
	"\t\tpsh A\n"
	"\t\tabs Y, A\n"
	"\t\tabs A, B\n"
	"\t\tcpy B, 0\n"
	"\t\tcpy W, 0\n"
	"\t\tdiv\n"
	"\t\tpul X\n"
	"\t\tcmp X, 0\n"
	"\t\tbrac n, R_DivideSigned641\n"
	"\t\tneg B, B\n"
	"R_DivideSigned641:\n"
	"\t\tpul X\n"
	"\t\tcmp X, 0\n"
	"\t\tbrac n, R_DivideSigned642\n"
	"\t\tneg Y, Y\n"
	"R_DivideSigned642:\n"
	"\t\tcpy A, Y\n"
	"\t\twtth (X, Y), 32\n"
	"\t\tret\n"
	"\n";


int64 TestIR816RunW65C816(const char* szIR, bool bLockstep, CChars* pszAssembly);


//////////////////////////////////////////////////////////////////////////
//
// Parses and verifies szIR, generates it with the 65C64 back end, links it
// with the start up (and the soft float runtime when bFloat) and runs it.
// Returns A when the program stops (main's result).  Only the 65C64:
// TestIR65Run runs it on the W65C816 too.
//
//////////////////////////////////////////////////////////////////////////
int64 TestIR65Run65C64(const char* szIR, bool bFloat, CChars* pszAssembly)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRVerifier		cVerifier;
	CIR65Backend	cBackend;
	CChars			szCode;
	int				iLabels;
	CCPP65Builder	cBuilder;
	CFileUtil		cFileUtil;
	CChars			szDirectory;
	STestCPP65Run	sRun;
	int64			lliResult;
	bool			bResult;

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

	szCode.Init();
	iLabels = 0;
	cBackend.Init(&cModule, &iLabels);
	AssertTrue(cBackend.GenerateModule(&szCode, &cErrors));
	AssertString("", cErrors.Text());
	if (cBackend.GetRuntime()->Contains("R_DivideSigned"))
	{
		szCode.Append(gszTestIR65DivideSigned);
	}
	if (cBackend.GetRuntime()->Contains("R_DivideSigned64"))
	{
		szCode.Append(gszTestIR65DivideSigned64);
	}
	cBackend.Kill();
	if (pszAssembly)
	{
		pszAssembly->Append(szCode);
	}

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	bResult = cBuilder.AddStartUp();
	bResult &= cBuilder.AddAssembly("ir", szCode.Text());
	if (bFloat)
	{
		szDirectory.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
		cFileUtil.FullPath(&szDirectory);
		cBuilder.AddUnit("Libc65", szDirectory.Text(), "softfloat.cpp");
		cBuilder.AddUnit("Libc65", szDirectory.Text(), "softfloat.asm");
		szDirectory.Kill();
	}
	bResult &= cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bResult);

	lliResult = -1;
	if (bResult && TestCPP65BeginBuilt(&sRun, &cBuilder))
	{
		AssertTrue(TestCPP65Run(&sRun, TEST_IR65_STEPS) < TEST_IR65_STEPS);
		AssertIntHex(CPP65_STACK_TOP, TestCPP65MPU(&sRun)->GetS());
		lliResult = (int64)TestCPP65MPU(&sRun)->GetA();
		TestCPP65End(&sRun);
	}
	cBuilder.Kill();
	szCode.Kill();
	cErrors.Kill();
	cModule.Kill();
	return lliResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the two processors' results for szIR are the same: all 64 bits
// when main returns i64, else the low 32 (the 65C64's A is wider than
// main's result; the W65C816's is sign extended from main's type).
//
//////////////////////////////////////////////////////////////////////////
void AssertIRSameResult(int64 lliResult65C64, int64 lliResultW65C816, const char* szIR)
{
	if (strstr(szIR, "function i64 @_main") == NULL)
	{
		lliResult65C64 = (int32)lliResult65C64;
		lliResultW65C816 = (int32)lliResultW65C816;
	}
	AssertLong(lliResult65C64, lliResultW65C816);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs szIR on the 65C64 (TestIR65Run65C64) and, unless it needs the soft
// float runtime (which TestIR816Run doesn't link), on the W65C816 too, which
// must give the same result.  pszAssembly gets the 65C64's assembly.
//
//////////////////////////////////////////////////////////////////////////
int64 TestIR65Run(const char* szIR, bool bFloat = false, CChars* pszAssembly = NULL)
{
	int64	lliResult;

	lliResult = TestIR65Run65C64(szIR, bFloat, pszAssembly);
	if (!bFloat)
	{
		AssertIRSameResult(lliResult, TestIR816RunW65C816(szIR, false, NULL), szIR);
	}
	return lliResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 TestIR65Main(const char* szIR, bool bFloat = false)
{
	return (int32)TestIR65Run(szIR, bFloat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendReturn(void)
{
	CChars	szAssembly;

	szAssembly.Init();
	AssertLong(42, (int32)TestIR65Run("function i32 @_main()\n{\nb:\n\tret i32 42\n}\n", false, &szAssembly));
	AssertString(
		"\t\t.section text._main\n"
		"\t\t.global _main\n"
		"_main:\n"
		"L1:\n"
		"\t\tcpy A, 42\n"
		"L2:\n"
		"\t\tret\n"
		"\n", szAssembly.Text());
	szAssembly.Kill();

	AssertInt(-7, TestIR65Main("function i32 @_main()\n{\nb:\n\t%x = sub i32 3, 10\n\tret i32 %x\n}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Sum of a table, with a loop's phis.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendLoop(void)
{
	AssertInt(150, TestIR65Main(
		"global @_gTable, align 4 { i32 10, i32 20, i32 30, i32 40, i32 50 }\n"
		"\n"
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
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
		"exit:\n"
		"\tret i32 %s\n"
		"}\n"
		"\n"
		"function i32 @_main()\n"
		"{\n"
		"b:\n"
		"\t%r = call i32 @_Sum__FPii(ptr @_gTable, i32 5)\n"
		"\tret i32 %r\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Phis that swap must all read before any is written.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendSwap(void)
{
	AssertInt(1 * 1000 + 2, TestIR65Main(
		"function i32 @_main()\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
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
		"\tret i32 %r\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// i8 and i16 wrap, extend and compare by their own widths.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendNarrow(void)
{
	AssertInt(-56, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%t = trunc i32 200 to i8\n"
		"\t%s = sext i8 %t to i32\n"
		"\tret i32 %s\n}\n"));
	AssertInt(200, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%t = trunc i32 200 to i8\n"
		"\t%s = zext i8 %t to i32\n"
		"\tret i32 %s\n}\n"));
	AssertInt(44, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = add i8 100, 200\n"
		"\t%s = zext i8 %a to i32\n"
		"\tret i32 %s\n}\n"));
	//Signed i8 and i16 comparisons at their own width, of parameters (so not folded): -1 < 1, 1 > -1, -2 <= -2,
	//-32768 < 32767, 100 > -1 (an immediate), 127 >= -128.
	AssertInt(111111, TestIR65Main(
		"function i32 @_C__Fcc(i8 %a, i8 %b, i16 %c, i16 %d), local\n{\nb:\n"
		"\t%1 = slt i8 %a, %b\n\t%2 = sgt i8 %b, %a\n\t%3 = sle i8 %a, %a\n\t%4 = slt i16 %c, %d\n"
		"\t%5 = sgt i8 %b, -1\n\t%6 = sge i8 %b, %a\n"
		"\t%z1 = zext i1 %1 to i32\n\t%z2 = zext i1 %2 to i32\n\t%z3 = zext i1 %3 to i32\n\t%z4 = zext i1 %4 to i32\n\t%z5 = zext i1 %5 to i32\n\t%z6 = zext i1 %6 to i32\n"
		"\t%m1 = mul i32 %z1, 100000\n\t%m2 = mul i32 %z2, 10000\n\t%m3 = mul i32 %z3, 1000\n\t%m4 = mul i32 %z4, 100\n\t%m5 = mul i32 %z5, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %m3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %m5\n\t%s5 = add i32 %s4, %z6\n"
		"\tret i32 %s5\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_C__Fcc(i8 -1, i8 1, i16 -32768, i16 32767)\n\tret i32 %r\n}\n"));
	AssertInt(1, TestIR65Main(
		"function i32 @_D__Fcc(i8 %a, i8 %b), local\n{\nb:\n\t%c = sge i8 %a, %b\n\t%z = zext i1 %c to i32\n\tret i32 %z\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_D__Fcc(i8 127, i8 -128)\n\tret i32 %r\n}\n"));

	//sext of an i1 is 0 or -1, and so compares below 0.
	AssertInt(-1 * 10 + 1, TestIR65Main(
		"function i32 @_Flag__Fi(i32 %x), local\n{\nb:\n\t%c = ne i32 %x, 0\n\t%s = sext i1 %c to i32\n\t%n = slt i1 %c, 0\n\t%z = zext i1 %n to i32\n\t%m = mul i32 %s, 10\n\t%r = add i32 %m, %z\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_Flag__Fi(i32 5)\n\tret i32 %r\n}\n"));
	AssertInt(1, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%c = slt i8 -1, 1\n"
		"\t%s = zext i1 %c to i32\n"
		"\tret i32 %s\n}\n"));
	AssertInt(0, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%c = ult i8 -1, 1\n"
		"\t%s = zext i1 %c to i32\n"
		"\tret i32 %s\n}\n"));
	AssertInt(-2, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = ashr i16 -8, 2\n"
		"\t%s = sext i16 %a to i32\n"
		"\tret i32 %s\n}\n"));
	AssertInt(16382, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = lshr i16 -8, 2\n"
		"\t%s = zext i16 %a to i32\n"
		"\tret i32 %s\n}\n"));
	//x > 0 compares with 1, so it can't branch on n alone: INT_MIN > 0 is false, INT_MIN <= 0 true, 5 > 0 true, -1 > -1 false.
	AssertInt(110, TestIR65Main(
		"function i32 @_P__Fiii(i32 %a, i32 %b, i32 %c), local\n{\nb:\n"
		"\t%1 = sgt i32 %a, 0\n\t%2 = sle i32 %a, 0\n\t%3 = sgt i32 %b, 0\n\t%4 = sgt i32 %c, -1\n"
		"\t%z1 = zext i1 %1 to i32\n\t%z2 = zext i1 %2 to i32\n\t%z3 = zext i1 %3 to i32\n\t%z4 = zext i1 %4 to i32\n"
		"\t%m1 = mul i32 %z1, 1000\n\t%m2 = mul i32 %z2, 100\n\t%m3 = mul i32 %z3, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %m3\n\t%s3 = add i32 %s2, %z4\n\tret i32 %s3\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_P__Fiii(i32 -2147483648, i32 5, i32 -1)\n\tret i32 %r\n}\n"));

	//Narrow x > c and x <= c compare with c + 1: 1 > 0, 0 > 0, 5 <= 5, 6 <= 5, -128 <= -1, 127 > 127 (not adjusted), -1 > -2.
	AssertInt(1010101, TestIR65Main(
		"function i32 @_N__Fssccccc(i16 %a, i16 %b, i8 %c, i8 %d, i8 %e, i8 %f, i8 %g), local\n{\nb:\n"
		"\t%1 = sgt i16 %a, 0\n\t%2 = sgt i16 %b, 0\n\t%3 = sle i8 %c, 5\n\t%4 = sle i8 %d, 5\n\t%5 = sle i8 %e, -1\n\t%6 = sgt i8 %f, 127\n\t%7 = sgt i8 %g, -2\n"
		"\t%z1 = zext i1 %1 to i32\n\t%z2 = zext i1 %2 to i32\n\t%z3 = zext i1 %3 to i32\n\t%z4 = zext i1 %4 to i32\n\t%z5 = zext i1 %5 to i32\n\t%z6 = zext i1 %6 to i32\n\t%z7 = zext i1 %7 to i32\n"
		"\t%m1 = mul i32 %z1, 1000000\n\t%m2 = mul i32 %z2, 100000\n\t%m3 = mul i32 %z3, 10000\n\t%m4 = mul i32 %z4, 1000\n\t%m5 = mul i32 %z5, 100\n\t%m6 = mul i32 %z6, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %m3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %m5\n\t%s5 = add i32 %s4, %m6\n\t%s6 = add i32 %s5, %z7\n"
		"\tret i32 %s6\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_N__Fssccccc(i16 1, i16 0, i8 5, i8 6, i8 -128, i8 127, i8 -1)\n\tret i32 %r\n}\n"));

	//A comparison with 0 of a value just copied branches on the flags the cpy set: a call's result kept in X and a load.
	AssertInt(-3 * 100 + 2 * 10 + 1, TestIR65Main(
		"function i32 @_F__Fi(i32 %a), local\n{\nb:\n\tret i32 %a\n}\n\n"
		"function i32 @_G__FPi(i32 %a, ptr %p), local\n{\n"
		"b:\n\t%r = call i32 @_F__Fi(i32 %a)\n\t%n = slt i32 %r, 0\n\tbrif %n, neg, pos\n"
		"neg:\n\t%m = mul i32 %r, 100\n\tret i32 %m\n"
		"pos:\n\t%v = load i32 %p\n\t%z = eq i32 %v, 0\n\tbrif %z, zero, more\n"
		"zero:\n\tret i32 1\n"
		"more:\n\t%w = mul i32 %v, 10\n\tret i32 %w\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%s = slot ptr 4, 4\n\tstore i32 2, %s\n\t%t = slot ptr 4, 4\n\tstore i32 0, %t\n"
		"\t%1 = call i32 @_G__FPi(i32 -3, ptr %s)\n\t%2 = call i32 @_G__FPi(i32 5, ptr %s)\n\t%3 = call i32 @_G__FPi(i32 0, ptr %t)\n"
		"\t%a = add i32 %1, %2\n\t%b = add i32 %a, %3\n\tret i32 %b\n}\n"));

	//...and in the block only that branch goes to: a call's result is 0, below 0 or above.
	AssertInt(0 * 100 + 1 * 10 + 2, TestIR65Main(
		"function i32 @_F__Fi(i32 %a), local\n{\nb:\n\tret i32 %a\n}\n\n"
		"function i32 @_S__Fi(i32 %a), local\n{\n"
		"b:\n\t%r = call i32 @_F__Fi(i32 %a)\n\t%z = eq i32 %r, 0\n\tbrif %z, zero, nonzero\n"
		"zero:\n\tret i32 0\n"
		"nonzero:\n\t%n = slt i32 %r, 0\n\tbrif %n, neg, pos\n"
		"neg:\n\tret i32 1\n"
		"pos:\n\tret i32 2\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%1 = call i32 @_S__Fi(i32 0)\n\t%2 = call i32 @_S__Fi(i32 -7)\n\t%3 = call i32 @_S__Fi(i32 9)\n"
		"\t%m1 = mul i32 %1, 100\n\t%m2 = mul i32 %2, 10\n\t%a = add i32 %m1, %m2\n\t%b = add i32 %a, %3\n\tret i32 %b\n}\n"));

	//Signed compares with 0 branch on n alone: -5 < 0, 0 >= 0, 7 >= 0, -1 < 0 (i8), 0 < 0 (i8).
	AssertInt(11110, TestIR65Main(
		"function i32 @_Z__Fiic(i32 %a, i32 %b, i32 %c, i8 %d, i8 %e), local\n{\nb:\n"
		"\t%1 = slt i32 %a, 0\n\t%2 = sge i32 %b, 0\n\t%3 = sge i32 %c, 0\n\t%4 = slt i8 %d, 0\n\t%5 = slt i8 %e, 0\n"
		"\t%z1 = zext i1 %1 to i32\n\t%z2 = zext i1 %2 to i32\n\t%z3 = zext i1 %3 to i32\n\t%z4 = zext i1 %4 to i32\n\t%z5 = zext i1 %5 to i32\n"
		"\t%m1 = mul i32 %z1, 10000\n\t%m2 = mul i32 %z2, 1000\n\t%m3 = mul i32 %z3, 100\n\t%m4 = mul i32 %z4, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %m3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %z5\n"
		"\tret i32 %s4\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_Z__Fiic(i32 -5, i32 0, i32 7, i8 -1, i8 0)\n\tret i32 %r\n}\n"));

	//A three way compare: the ne after the slt branches on the slt's flags.  -1 * 100 + 0 * 10 + 1, twice.
	AssertInt(-99 * 2, TestIR65Main(
		"function i32 @_Cmp__Fcc(i8 %a, i8 %b), local\n{\nx:\n\t%l = slt i8 %a, %b\n\tbrif %l, y, z\ny:\n\tret i32 -1\n"
		"z:\n\t%g = ne i8 %a, %b\n\tbrif %g, w, v\nw:\n\tret i32 1\nv:\n\tret i32 0\n}\n\n"
		"function i32 @_Cmp__Fii(i32 %a, i32 %b), local\n{\nx:\n\t%l = slt i32 %a, %b\n\tbrif %l, y, z\ny:\n\tret i32 -1\n"
		"z:\n\t%g = ne i32 %b, %a\n\tbrif %g, w, v\nw:\n\tret i32 1\nv:\n\tret i32 0\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%1 = call i32 @_Cmp__Fcc(i8 -2, i8 2)\n\t%2 = call i32 @_Cmp__Fcc(i8 -2, i8 -2)\n\t%3 = call i32 @_Cmp__Fcc(i8 3, i8 -2)\n"
		"\t%4 = call i32 @_Cmp__Fii(i32 1, i32 2)\n\t%5 = call i32 @_Cmp__Fii(i32 2, i32 2)\n\t%6 = call i32 @_Cmp__Fii(i32 3, i32 2)\n"
		"\t%m1 = mul i32 %1, 100\n\t%m2 = mul i32 %2, 10\n\t%m4 = mul i32 %4, 100\n\t%m5 = mul i32 %5, 10\n"
		"\t%s1 = add i32 %m1, %m2\n\t%s2 = add i32 %s1, %3\n\t%s3 = add i32 %s2, %m4\n\t%s4 = add i32 %s3, %m5\n\t%s5 = add i32 %s4, %6\n"
		"\tret i32 %s5\n}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// How many div instructions szAssembly has.
//
//////////////////////////////////////////////////////////////////////////
static int CountDivs(const char* szAssembly)
{
	int			iDivs;
	const char*	sz;

	iDivs = 0;
	for (sz = strstr(szAssembly, "\t\tdiv\n"); sz; sz = strstr(sz + 1, "\t\tdiv\n"))
	{
		iDivs++;
	}
	return iDivs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendDivide(void)
{
	CChars	szAssembly;

	AssertInt(-3, TestIR65Main("function i32 @_main()\n{\nb:\n\t%a = sdiv i32 -7, 2\n\tret i32 %a\n}\n"));
	AssertInt(-1, TestIR65Main("function i32 @_main()\n{\nb:\n\t%a = srem i32 -7, 2\n\tret i32 %a\n}\n"));
	AssertInt(2147483644, TestIR65Main("function i32 @_main()\n{\nb:\n\t%a = udiv i32 -7, 2\n\tret i32 %a\n}\n"));
	AssertInt(1, TestIR65Main("function i32 @_main()\n{\nb:\n\t%a = urem i32 -7, 2\n\tret i32 %a\n}\n"));
	//Multipliers of a few signed digits are shifts and adds, cheaper than mul (ExpandMultiplies): 13 * 10 + 13 * 7 +
	//13 * 24 + -3 * 3 + 13 * 1536 + 13 * 3 (i8, wraps to 39) + 13 * 11 + 13 * 255 + 13 * -3 (i8, wraps to 217).
	szAssembly.Init();
	AssertInt(13 * 10 + 13 * 7 + 13 * 24 - 3 * 3 + 13 * 1536 + 39 + 13 * 11 + 13 * 255 + 217, (int32)TestIR65Run(
		"function i32 @_M__Fiic(i32 %a, i32 %b, i8 %c), local\n{\nb:\n"
		"\t%1 = mul i32 %a, 10\n\t%2 = mul i32 %a, 7\n\t%3 = mul i32 %a, 24\n\t%4 = mul i32 %b, 3\n\t%5 = mul i32 %a, 1536\n\t%6 = mul i8 %c, 3\n"
		"\t%7 = mul i32 %a, 11\n\t%8 = mul i32 %a, 255\n\t%9 = mul i8 %c, -3\n"
		"\t%z = zext i8 %6 to i32\n\t%y = zext i8 %9 to i32\n"
		"\t%s1 = add i32 %1, %2\n\t%s2 = add i32 %s1, %3\n\t%s3 = add i32 %s2, %4\n\t%s4 = add i32 %s3, %5\n\t%s5 = add i32 %s4, %z\n"
		"\t%s6 = add i32 %s5, %7\n\t%s7 = add i32 %s6, %8\n\t%s8 = add i32 %s7, %y\n"
		"\tret i32 %s8\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_M__Fiic(i32 13, i32 -3, i8 13)\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertFalse(szAssembly.Contains("\t\tmul\n"));
	szAssembly.Kill();

	//Too many signed digits: a multiplier below 256 is one mul step (B 8 bits wide), a bigger one a step a byte:
	//1234567 * 85 + 1234567 * 171 + 13 * 85 (i8, wraps to 81) + 13 * 0x12345.
	szAssembly.Init();
	AssertInt(1234567 * 85 + 1234567 * 171 + 81 + 13 * 0x12345, (int32)TestIR65Run(
		"function i32 @_B__Fic(i32 %a, i8 %c, i32 %d), local\n{\nb:\n"
		"\t%1 = mul i32 %a, 85\n\t%2 = mul i32 %a, 171\n\t%3 = mul i8 %c, 85\n\t%z = zext i8 %3 to i32\n\t%4 = mul i32 %d, 74565\n"
		"\t%s1 = add i32 %1, %2\n\t%s2 = add i32 %s1, %z\n\t%s3 = add i32 %s2, %4\n\tret i32 %s3\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_B__Fic(i32 1234567, i8 13, i32 13)\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("\t\tmul\n") && szAssembly.Contains("wth B, 8"));
	szAssembly.Kill();

	//A narrow load only a sext uses is sign extended as it is read: -2 (i8) and -300 (i16), and an i8 sext to i16 (kept zero extended).
	AssertInt(-2 - 300 + 65534, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%p = slot ptr 4, 4\n\tstore i32 -300, %p\n\t%q = slot ptr 4, 4\n\tstore i32 -2, %q\n"
		"\t%1 = load i8 %q\n\t%2 = sext i8 %1 to i32\n\t%3 = load i16 %p\n\t%4 = sext i16 %3 to i32\n"
		"\t%5 = load i8 %q\n\t%6 = sext i8 %5 to i16\n\t%7 = zext i16 %6 to i32\n"
		"\t%s = add i32 %2, %4\n\t%t = add i32 %s, %7\n\tret i32 %t\n}\n"));

	//udiv and urem of the same operands next to each other (the optimiser puts them there: SetDivideRemainders) share a
	//div, in either order: 1234 / 10 * 1000 + 1234 % 10 and 1234 % 100 * 1000 + 1234 / 100.
	szAssembly.Init();
	AssertInt(123 * 1000 + 4 + 34 * 1000 + 12, (int32)TestIR65Run(
		"function i32 @_D__Fjj(i32 %a, i32 %b, i32 %c), local\n{\nb:\n"
		"\t%q = udiv i32 %a, %b\n\t%r = urem i32 %a, %b\n\t%m = mul i32 %q, 1000\n\t%s = add i32 %m, %r\n"
		"\t%r2 = urem i32 %a, %c\n\t%q2 = udiv i32 %a, %c\n\t%m2 = mul i32 %r2, 1000\n\t%s2 = add i32 %m2, %q2\n"
		"\t%t = add i32 %s, %s2\n\tret i32 %t\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_D__Fjj(i32 1234, i32 10, i32 100)\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertInt(2, CountDivs(szAssembly.Text()));
	szAssembly.Kill();
	AssertInt(-14, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = sdiv i8 -100, 7\n"
		"\t%s = sext i8 %a to i32\n"
		"\tret i32 %s\n}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendWide(void)
{
	AssertLong(1000000000000LL * 3, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%a = mul i64 1000000000000, 3\n"
		"\tret i64 %a\n}\n"));
	AssertLong(-4, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%a = sdiv i64 -9000000000000, 2250000000000\n"
		"\tret i64 %a\n}\n"));
	AssertLong(1, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%c = slt i64 -5000000000, 4000000000\n"
		"\t%r = zext i1 %c to i64\n"
		"\tret i64 %r\n}\n"));
	AssertLong(-1, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%a = ashr i64 -9000000000000, 60\n"
		"\tret i64 %a\n}\n"));
	AssertLong(0xFFFFFFFFLL, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%a = zext i32 -1 to i64\n"
		"\tret i64 %a\n}\n"));
	//A value squared: it is both operands, copied from A to B all 64 bits wide.
	AssertLong(20123405LL * 20123405LL, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%a = add i32 20123404, 1\n"
		"\t%w = sext i32 %a to i64\n"
		"\t%s = mul i64 %w, %w\n"
		"\tret i64 %s\n}\n"));
	AssertLong(-1, TestIR65Run(
		"function i64 @_main()\n{\nb:\n"
		"\t%a = sext i32 -1 to i64\n"
		"\tret i64 %a\n}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Narrow memory, slots, a narrow argument and recursion.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendMemory(void)
{
	AssertInt(-1 + 300 + 70000, TestIR65Main(
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
		"\tret i32 %r\n}\n"));

	AssertInt(55, TestIR65Main(
		"function i32 @_Fibonacci__Fc(i8 %n)\n{\n"
		"entry:\n"
		"\t%c = slt i8 %n, 2\n"
		"\tbrif %c, small, large\n"
		"small:\n"
		"\t%r = zext i8 %n to i32\n"
		"\tret i32 %r\n"
		"large:\n"
		"\t%a = sub i8 %n, 1\n"
		"\t%b = sub i8 %n, 2\n"
		"\t%x = call i32 @_Fibonacci__Fc(i8 %a)\n"
		"\t%y = call i32 @_Fibonacci__Fc(i8 %b)\n"
		"\t%s = add i32 %x, %y\n"
		"\tret i32 %s\n"
		"}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Fibonacci__Fc(i8 10)\n"
		"\tret i32 %r\n}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A variadic function walks its arguments from argaddr; a function pointer
// call; a thiscall.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendCalls(void)
{
	CChars	szAssembly;

	AssertInt(1 + 20 + 300, TestIR65Main(
		"function i32 @_SumAll(i32 %count, ...)\n{\n"
		"entry:\n"
		"\t%first = argaddr ptr 0\n"
		"\t%p0 = ptradd ptr %first, 4\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i32 [entry, 0], [body, %s2]\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%p = phi ptr [entry, %p0], [body, %p2]\n"
		"\t%c = slt i32 %i, %count\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%v = load i32 %p\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%p2 = ptradd ptr %p, 4\n"
		"\tjmp loop\n"
		"exit:\n"
		"\tret i32 %s\n"
		"}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_SumAll(i32 3, ..., i32 1, i32 20, i32 300)\n"
		"\tret i32 %r\n}\n"));

	AssertInt(12, TestIR65Main(
		"function i32 @_Double__Fi(i32 %x)\n{\nb:\n\t%r = add i32 %x, %x\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%f = copy ptr @_Double__Fi\n"
		"\t%r = call i32 %f(i32 6)\n"
		"\tret i32 %r\n}\n"));

	//The pointer is called through where it was pushed, twice: it and the first result are live across calls, and V for
	//one with a kept pseudo register for the other would cost more than the frame (CIR65Backend::PaysV).
	szAssembly.Init();
	AssertInt(6 + 7, (int32)TestIR65Run(
		"function i32 @_Increment__Fi(i32 %x)\n{\nb:\n\t%r = add i32 %x, 1\n\tret i32 %r\n}\n\n"
		"function i32 @_Twice(ptr %f, i32 %x)\n{\nb:\n"
		"\t%a = call i32 %f(i32 %x)\n"
		"\t%b = call i32 %f(i32 %a)\n"
		"\t%r = add i32 %a, %b\n"
		"\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%f = copy ptr @_Increment__Fi\n"
		"\t%r = call i32 @_Twice(ptr %f, i32 5)\n"
		"\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("jsr [13 + V]"));
	AssertFalse(szAssembly.Contains("[128 + W]"));
	AssertFalse(szAssembly.Contains("jsr A"));
	szAssembly.Kill();

	AssertInt(7 + 5, TestIR65Main(
		"global @_gObject, align 4 { i32 7 }\n\n"
		"function i32 @_C__Get__Fi(ptr %this, i32 %x), thiscall\n{\nb:\n"
		"\t%v = load i32 %this\n"
		"\t%r = add i32 %v, %x\n"
		"\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call thiscall i32 @_C__Get__Fi(ptr @_gObject, i32 5)\n"
		"\tret i32 %r\n}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Floats through the soft float runtime.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendFloat(void)
{
	//(1.5 + 2.25) * 4 = 15.
	AssertInt(15, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = fadd f32 1.5, 2.25\n"
		"\t%m = fmul f32 %a, 4\n"
		"\t%i = ftosi f32 %m to i32\n"
		"\tret i32 %i\n}\n", true));
	AssertInt(-3, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = sitof i32 7 to f64\n"
		"\t%d = fdiv f64 %a, -2\n"
		"\t%n = fneg f64 %d\n"
		"\t%s = fsub f64 0.5, %n\n"
		"\t%i = ftosi f64 %s to i32\n"
		"\tret i32 %i\n}\n", true));
	AssertInt(1, TestIR65Main(
		"function i32 @_main()\n{\nb:\n"
		"\t%a = fext f32 0.25 to f64\n"
		"\t%c = flt f64 %a, 0.5\n"
		"\tbrif %c, yes, no\n"
		"yes:\n\tret i32 1\n"
		"no:\n\tret i32 0\n}\n", true));
}


//////////////////////////////////////////////////////////////////////////
//
// Register allocation: a loop's values in X, Y and pseudo registers; phis
// that rotate; values kept across calls (in the pseudo registers a function
// keeps, so recursion must keep them); more values than homes; mul and div
// around values kept in Y; narrow loads into a register through itself.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendAllocation(void)
{
	CChars	szAssembly;
	CChars	szSum;
	CChars	szIR;
	int		i;
	int32	iExpected;

	szAssembly.Init();
	AssertInt(10 + 20 + 30, (int32)TestIR65Run(
		"global @_gTable, align 4 { i32 10, i32 20, i32 30 }\n\n"
		"function i32 @_Sum__FPii(ptr %p, i32 %n), local\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
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
		"exit:\n"
		"\tret i32 %s\n"
		"}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Sum__FPii(ptr @_gTable, i32 3)\n"
		"\tret i32 %r\n}\n", false, &szAssembly));
	szSum.Init();
	szSum.AppendSubString(szAssembly, 0, szAssembly.Find("\t\t.section text._main"));
	AssertString(
		"\t\t.section text._Sum__FPii\n"
		"_Sum__FPii:\n"
		"\t\tpsh V\n"
		"\t\tcpy W, [9 + S]\n"
		"\t\tcpy V, [13 + S]\n"
		"L1:\n"
		"\t\tcpy Y, 0\n"
		"\t\tcpy X, 0\n"
		"\t\tbra L3\n"
		"L2:\n"
		"\t\tcpy A, X\n"
		"\t\tshl A, 2\n"
		"\t\tadd A, W\n"
		"\t\tcpy A, [A]\n"
		"\t\tadd Y, A\n"
		"\t\tadd X, 1\n"
		"L3:\n"
		"\t\tcmp X, V\n"
		"\t\tbraso (n, v), L2\n"
		"L4:\n"
		"\t\tcpy A, Y\n"
		"L5:\n"
		"\t\tpul V\n"
		"\t\tret\n"
		"\n", szSum.Text());
	szSum.Kill();
	szAssembly.Kill();

	//(a, b, c) = (b, c, a) three times leaves them as they started.
	AssertInt(1 * 100 + 2 * 10 + 3, TestIR65Main(
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
		"\tret i32 %r\n}\n"));

	//Sum of Triangle(i) for i < 6: the loop's values are kept across the calls, in pseudo registers the callee keeps.
	AssertInt(0 + 1 + 3 + 6 + 10 + 15, TestIR65Main(
		"function i32 @_Triangle__Fi(i32 %n), local\n{\n"
		"entry:\n"
		"\t%z = eq i32 %n, 0\n"
		"\tbrif %z, zero, more\n"
		"zero:\n\tret i32 0\n"
		"more:\n"
		"\t%m = sub i32 %n, 1\n"
		"\t%t = call i32 @_Triangle__Fi(i32 %m)\n"
		"\t%r = add i32 %t, %n\n"
		"\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i32 [entry, 0], [body, %s2]\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%c = slt i32 %i, 6\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%t = call i32 @_Triangle__Fi(i32 %i)\n"
		"\t%s2 = add i32 %s, %t\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp loop\n"
		"exit:\n\tret i32 %s\n}\n"));

	//Count(n) = 1 + Count(0) + ... + Count(n - 1) = 2^n: its loop's values are in the pseudo registers it keeps, so
	//each call must give the caller's back.
	AssertInt(1 << 6, TestIR65Main(
		"function i32 @_Count__Fi(i32 %n), local\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i32 [entry, 1], [body, %s2]\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%t = call i32 @_Count__Fi(i32 %i)\n"
		"\t%s2 = add i32 %s, %t\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp loop\n"
		"exit:\n\tret i32 %s\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Count__Fi(i32 6)\n"
		"\tret i32 %r\n}\n"));

	//A parameter used once, also written through its argaddr, is read as it was passed.
	AssertInt(5 * 100 + 9, TestIR65Main(
		"function i32 @_Keep__Fi(i32 %x), local\n{\nb:\n"
		"\t%a = argaddr ptr 0\n"
		"\tstore volatile i32 9, %a\n"
		"\t%v = load volatile i32 %a\n"
		"\t%m = mul i32 %x, 100\n"
		"\t%r = add i32 %m, %v\n"
		"\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n\t%r = call i32 @_Keep__Fi(i32 5)\n\tret i32 %r\n}\n"));

	//40 values live at once: more than X, Y and the pseudo registers hold.
	szIR.Init("function i32 @_main()\n{\nb:\n");
	iExpected = 0;
	for (i = 0; i < 40; i++)
	{
		szIR.Append("\t%v")->Append(i)->Append(" = mul i32 ")->Append(i + 1)->Append(", 3\n");
	}
	szIR.Append("\t%s0 = copy i32 %v0\n");
	for (i = 1; i < 40; i++)
	{
		szIR.Append("\t%s")->Append(i)->Append(" = add i32 %s")->Append(i - 1)->Append(", %v")->Append(i)->Append('\n');
	}
	for (i = 0; i < 40; i++)
	{
		iExpected += 3 * (i + 1);
	}
	szIR.Append("\tret i32 %s39\n}\n");
	AssertInt(iExpected, TestIR65Main(szIR.Text()));
	szIR.Kill();

	//mul and div change Y: values live across them aren't kept there.
	AssertInt((7 * 6 + 5) * 1000 + 100 / 7 + 100 % 7 + 11, TestIR65Main(
		"function i32 @_Work__Fiii(i32 %a, i32 %b, i32 %c), local\n{\n"
		"entry:\n"
		"\t%m = mul i32 %a, %b\n"
		"\t%x = add i32 %m, %c\n"
		"\t%k = mul i32 %x, 1000\n"
		"\t%q = udiv i32 100, %a\n"
		"\t%r = urem i32 100, %a\n"
		"\t%t = add i32 %k, %q\n"
		"\t%u = add i32 %t, %r\n"
		"\t%w = add i32 %u, %b\n"
		"\t%y = add i32 %w, %c\n"
		"\tret i32 %y\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Work__Fiii(i32 7, i32 6, i32 5)\n"
		"\tret i32 %r\n}\n"));

	//A narrow load whose result is kept in the register its address was in.
	AssertInt(5, TestIR65Main(
		"global @_gText, align 1 { i8 104, i8 101, i8 108, i8 108, i8 111, i8 0 }\n\n"
		"function i32 @_Length__FPc(ptr %s), local\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%p = phi ptr [entry, %s], [body, %p2]\n"
		"\t%c = load i8 %p\n"
		"\t%z = ne i8 %c, 0\n"
		"\tbrif %z, body, exit\n"
		"body:\n"
		"\t%p2 = ptradd ptr %p, 1\n"
		"\tjmp loop\n"
		"exit:\n"
		"\t%a = ptrtoi ptr %p to i32\n"
		"\t%b = ptrtoi ptr %s to i32\n"
		"\t%l = sub i32 %a, %b\n"
		"\tret i32 %l\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Length__FPc(ptr @_gText)\n"
		"\tret i32 %r\n}\n"));

	//64 bit phis in pseudo register pairs, and kept across a call.
	AssertLong(3000000000LL * 4 + 7, TestIR65Run(
		"function i64 @_Seven__F(), local\n{\nb:\n\tret i64 7\n}\n\n"
		"function i64 @_main()\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i64 [entry, 0], [loop, %s2]\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%s2 = add i64 %s, 3000000000\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%c = ult i32 %i2, 4\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n"
		"\t%v = call i64 @_Seven__F()\n"
		"\t%r = add i64 %s2, %v\n"
		"\tret i64 %r\n}\n"));
	//A string compare's pointers are only addresses and steps, so the bytes get X and Y; one pointer gets W and the other
	//V.  A cpy reads through either ([W]; an add, cmp or cpyx would have W copied to B first): "abc" against "abd" is -1.  The bytes are only compared, so they are read narrow (cpy, not cpyx): X and Y stay 8 bits wide across
	//the loop.
	szAssembly.Init();
	AssertLong(-1, (int32)TestIR65Run(
		"function i32 @_Compare__FPcPc(ptr %a, ptr %b), local\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%p = phi ptr [entry, %a], [next, %p2]\n"
		"\t%q = phi ptr [entry, %b], [next, %q2]\n"
		"\t%p2 = ptradd ptr %p, 1\n\t%x = load i8 %p\n\t%q2 = ptradd ptr %q, 1\n\t%y = load i8 %q\n"
		"\t%z = eq i8 %x, 0\n\tbrif %z, done, next\n"
		"next:\n\t%e = eq i8 %x, %y\n\tbrif %e, loop, done\n"
		"done:\n\t%l = slt i8 %x, %y\n\tbrif %l, less, more\n"
		"less:\n\tret i32 -1\n"
		"more:\n\t%g = sgt i8 %x, %y\n\t%r = zext i1 %g to i32\n\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%s = slot ptr 4, 4\n\tstore i32 6513249, %s\n\t%t = slot ptr 4, 4\n\tstore i32 6578785, %t\n"
		"\t%r = call i32 @_Compare__FPcPc(ptr %s, ptr %t)\n\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("\t\tcpy X, [W]\n\t\tadd W, 1\n\t\tcpy Y, [V]\n\t\tadd V, 1\n") && !szAssembly.Contains("cpyx"));
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CIR65Widths on szCode, as a whole function.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65WidthsOptimise(const char* szExpected, const char* szCode)
{
	CIR65Widths	cWidths;
	CChars		sz;

	sz.Init(szCode);
	cWidths.Init();
	cWidths.Optimise(&sz, 0);
	cWidths.Kill();
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A wthx whose extended bits are never read goes; widths are only set
// before the instructions that need them, once before a loop, and two
// registers going to one width share a wtth.  A runtime routine called
// with other widths leaves them unknown.  A string's length counted
// through a byte only compared keeps its register narrow in the loop.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendWidths(void)
{
	CChars	szAssembly;
	CChars	szCount;

	//The byte is only compared: Y stays 8 bits through the loop and is set back to 32 for the return.
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\tcpy X, 0\n"
		"\t\twth Y, 8\n"
		"L1:\n"
		"\t\tcpy Y, [[0 + W]]\n"
		"\t\tcmp Y, 0\n"
		"\t\tbras z, L2\n"
		"\t\tadd [0], 1\n"
		"\t\tadd X, 1\n"
		"\t\tbra L1\n"
		"L2:\n"
		"\t\tcpy A, X\n"
		"\t\twth Y, 32\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy X, 0\n"
		"L1:\n"
		"\t\twth Y, 8\n"
		"\t\tcpy Y, [[0 + W]]\n"
		"\t\twthx Y, 32\n"
		"\t\twth Y, 8\n"
		"\t\tcmp Y, 0\n"
		"\t\twth Y, 32\n"
		"\t\tbras z, L2\n"
		"\t\tadd [0], 1\n"
		"\t\tadd X, 1\n"
		"\t\tbra L1\n"
		"L2:\n"
		"\t\tcpy A, X\n"
		"\t\tret\n");

	//L3 is a loop's label first gone to with Y's width unknown (the ways into L1 disagree), so it starts with it unknown
	//and the call is made with Y set back to 32 bits (deciding it again as 32 at the label left the call with Y 8 bits).
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L1\n"
		"\t\twth Y, 8\n"
		"\t\tcpy Y, [X]\n"
		"L1:\n"
		"\t\tcmp A, 1\n"
		"\t\tbras z, L3\n"
		"\t\tbra L4\n"
		"L3:\n"
		"\t\tcmp A, 2\n"
		"\t\twth Y, 32\n"
		"\t\tbras z, L5\n"
		"\t\tjsr _G\n"
		"\t\tret\n"
		"L4:\n"
		"\t\tsub A, 1\n"
		"\t\tbra L3\n"
		"L5:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L1\n"
		"\t\twth Y, 8\n"
		"\t\tcpy Y, [X]\n"
		"\t\twth Y, 32\n"
		"L1:\n"
		"\t\tcmp A, 1\n"
		"\t\tbras z, L3\n"
		"\t\tbra L4\n"
		"L3:\n"
		"\t\tcmp A, 2\n"
		"\t\tbras z, L5\n"
		"\t\tjsr _G\n"
		"\t\tret\n"
		"L4:\n"
		"\t\tsub A, 1\n"
		"\t\tbra L3\n"
		"L5:\n"
		"\t\tret\n");

	//The extended bits are returned, so the wthx stays.
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\twth A, 8\n"
		"\t\tcpy A, [X]\n"
		"\t\twthx A, 32:s\n"
		"\t\tret\n",
		"_F:\n"
		"\t\twth A, 8\n"
		"\t\tcpy A, [X]\n"
		"\t\twthx A, 32:s\n"
		"\t\tret\n");

	//B is next needed at 16 bits too, so it is set with A; both ways from the branch return, so both go back to 32
	//bits once before it.
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\twtth (A, B), 16\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy B, [Y]\n"
		"\t\tcmp A, B\n"
		"\t\twtth (A, B), 32\n"
		"\t\tbras z, L1\n"
		"\t\tcpy A, 0\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\twth A, 16\n"
		"\t\tcpy A, [X]\n"
		"\t\twth A, 32\n"
		"\t\twth B, 16\n"
		"\t\tcpy B, [Y]\n"
		"\t\twth B, 32\n"
		"\t\twtth (A, B), 16\n"
		"\t\tcmp A, B\n"
		"\t\twtth (A, B), 32\n"
		"\t\tbras z, L1\n"
		"\t\tcpy A, 0\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n");

	//Both ways into L1 need A at 32 bits: the narrow way sets it before the label.
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\tcmp X, 0\n"
		"\t\tbras z, L1\n"
		"\t\twth A, 8\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy [0 + W], A\n"
		"\t\twth A, 32\n"
		"L1:\n"
		"\t\tcpy A, 5\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp X, 0\n"
		"\t\tbras z, L1\n"
		"\t\twth A, 8\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy [0 + W], A\n"
		"\t\twth A, 32\n"
		"L1:\n"
		"\t\tcpy A, 5\n"
		"\t\tret\n");

	//A runtime routine called with A and B 64 bits wide leaves them so: they are set back before A is stored.
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\twtth (A, B), 64\n"
		"\t\tcpy A, [0 + W]\n"
		"\t\tcpy B, [8 + W]\n"
		"\t\tjsr R_DivideSigned64\n"
		"\t\twtth (A, B), 32\n"
		"\t\tcpy [16 + W], A\n"
		"\t\tret\n",
		"_F:\n"
		"\t\twtth (A, B), 64\n"
		"\t\tcpy A, [0 + W]\n"
		"\t\tcpy B, [8 + W]\n"
		"\t\tjsr R_DivideSigned64\n"
		"\t\twtth (A, B), 32\n"
		"\t\tcpy [16 + W], A\n"
		"\t\tret\n");

	//An instruction the pass doesn't know leaves the code as it was.
	TestIR65WidthsOptimise(
		"_F:\n"
		"\t\twth A, 8\n"
		"\t\tgwth X, A\n"
		"\t\twth A, 32\n"
		"\t\tret\n",
		"_F:\n"
		"\t\twth A, 8\n"
		"\t\tgwth X, A\n"
		"\t\twth A, 32\n"
		"\t\tret\n");

	szAssembly.Init();
	AssertInt(5, (int32)TestIR65Run(
		"global @_gsz, align 1 { i8 104, i8 101, i8 108, i8 108, i8 111, i8 0 }\n\n"
		"function i32 @_Count__FPc(ptr %p), local\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%q = phi ptr [entry, %p], [body, %q2]\n"
		"\t%n = phi i32 [entry, 0], [body, %n2]\n"
		"\t%c = load i8 %q\n"
		"\t%z = eq i8 %c, 0\n"
		"\tbrif %z, exit, body\n"
		"body:\n"
		"\t%q2 = ptradd ptr %q, 1\n"
		"\t%n2 = add i32 %n, 1\n"
		"\tjmp loop\n"
		"exit:\n"
		"\tret i32 %n\n"
		"}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Count__FPc(ptr @_gsz)\n"
		"\tret i32 %r\n}\n", false, &szAssembly));
	szCount.Init();
	szCount.AppendSubString(szAssembly, 0, szAssembly.Find("\t\t.section text._main"));
	AssertFalse(szCount.Contains("wthx"));
	szCount.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CIR65Flags on szCode, as a whole function.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65FlagsOptimise(const char* szExpected, const char* szCode)
{
	CIR65Flags	cFlags;
	CChars		sz;

	sz.Init(szCode);
	cFlags.Init();
	cFlags.Optimise(&sz, 0);
	cFlags.Kill();
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A cmp R, 0 after n and z were set from R goes when only n and z are read
// after it; a cmp the flags already hold goes (across a label both ways
// into agree on); a cmp whose operand changed, or whose c is read, stays.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendFlags(void)
{
	//and set n and z from A; the cpy X, A from both X and A.
	//A cmp V, 0 or cmp W, 0 straight after an instruction that wrote the register (and set n and z from it) goes; one
	//after anything else, or whose c is read, stays.
	TestIR65FlagsOptimise(
		"_F:\n"
		"\t\tcpy V, [V]\n"
		"\t\tbras z, L1\n"
		"\t\tadd W, 1\n"
		"\t\tbras z, L1\n"
		"\t\tcpy A, 1\n"
		"\t\tcmp V, 0\n"
		"\t\tbras z, L1\n"
		"\t\tsub W, 1\n"
		"\t\tcmp W, 0\n"
		"\t\tbras c, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy V, [V]\n"
		"\t\tcmp V, 0\n"
		"\t\tbras z, L1\n"
		"\t\tadd W, 1\n"
		"\t\tcmp W, 0\n"
		"\t\tbras z, L1\n"
		"\t\tcpy A, 1\n"
		"\t\tcmp V, 0\n"
		"\t\tbras z, L1\n"
		"\t\tsub W, 1\n"
		"\t\tcmp W, 0\n"
		"\t\tbras c, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n");

	TestIR65FlagsOptimise(
		"_F:\n"
		"\t\tand A, 255\n"
		"\t\tbras z, L1\n"
		"\t\tcpy X, A\n"
		"\t\tbras n, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tand A, 255\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L1\n"
		"\t\tcpy X, A\n"
		"\t\tcmp A, 0\n"
		"\t\tbras n, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n");

	//The second cmp's flags are the first's (L1 is reached from the branch, and by falling through a psh, which sets
	//no flags).
	TestIR65FlagsOptimise(
		"_F:\n"
		"\t\tcmp X, [4]\n"
		"\t\tbras z, L1\n"
		"\t\tpsh A\n"
		"L1:\n"
		"\t\tbrac c, L2\n"
		"\t\tret\n"
		"L2:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp X, [4]\n"
		"\t\tbras z, L1\n"
		"\t\tpsh A\n"
		"L1:\n"
		"\t\tcmp X, [4]\n"
		"\t\tbrac c, L2\n"
		"\t\tret\n"
		"L2:\n"
		"\t\tret\n");

	//A cmp of [2 + S] after a psh or pul is of another slot (S moved), and one of [2 + V] after pul V of another value:
	//they stay.
	TestIR65FlagsOptimise(
		"_F:\n"
		"\t\tcmp X, [2 + S]\n"
		"\t\tpsh A\n"
		"\t\tcmp X, [2 + S]\n"
		"\t\tpul A\n"
		"\t\tcmp X, [2 + S]\n"
		"\t\tcmp X, [2 + V]\n"
		"\t\tpul V\n"
		"\t\tcmp X, [2 + V]\n"
		"\t\tbras z, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp X, [2 + S]\n"
		"\t\tpsh A\n"
		"\t\tcmp X, [2 + S]\n"
		"\t\tpul A\n"
		"\t\tcmp X, [2 + S]\n"
		"\t\tcmp X, [2 + V]\n"
		"\t\tpul V\n"
		"\t\tcmp X, [2 + V]\n"
		"\t\tbras z, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n");

	//cpy A, 1 sets the flags, X changes, [4 + W] is written, and c is read after a cmp A, 0: all stay.
	TestIR65FlagsOptimise(
		"_F:\n"
		"\t\tcmp X, [4]\n"
		"\t\tadd X, 1\n"
		"\t\tcmp X, [4]\n"
		"\t\tcpy [4 + W], A\n"
		"\t\tcmp X, [4]\n"
		"\t\tbras z, L1\n"
		"\t\tadd A, 1\n"
		"\t\tcmp A, 0\n"
		"\t\tbrac c, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp X, [4]\n"
		"\t\tadd X, 1\n"
		"\t\tcmp X, [4]\n"
		"\t\tcpy [4 + W], A\n"
		"\t\tcmp X, [4]\n"
		"\t\tbras z, L1\n"
		"\t\tadd A, 1\n"
		"\t\tcmp A, 0\n"
		"\t\tbrac c, L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tret\n");

	//The ways into L1 disagree (n and z of A, of X): the cmp stays.
	TestIR65FlagsOptimise(
		"_F:\n"
		"\t\tcpy A, [0 + W]\n"
		"\t\tbras z, L1\n"
		"\t\tcpy X, [4 + W]\n"
		"L1:\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L2\n"
		"\t\tret\n"
		"L2:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy A, [0 + W]\n"
		"\t\tbras z, L1\n"
		"\t\tcpy X, [4 + W]\n"
		"L1:\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L2\n"
		"\t\tret\n"
		"L2:\n"
		"\t\tret\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The soft float routines both back ends call, and the compare results that
// make each comparison true (CIRBackend, shared by the 65C64's and the
// W65C816's).
//
//////////////////////////////////////////////////////////////////////////
static void AssertSoftFloatName(const char* szExpected, uint16 uiOpcode, EIRTypeKind eTo, EIRTypeKind eFrom)
{
	SIRInstruction	sInstruction;
	CChars			sz;

	memset(&sInstruction, 0, sizeof(sInstruction));
	sInstruction.uiOpcode = uiOpcode;
	sInstruction.sType = IRType(eTo);
	sz.Init();
	CIRBackend::AppendSoftFloatName(&sz, &sInstruction, IRType(eFrom));
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendSoftFloatNames(void)
{
	int		aiValues[2];
	bool	bNot;

	AssertSoftFloatName("SoftFloatAdd", IRO_FAdd, IRTK_F32, IRTK_F32);
	AssertSoftFloatName("SoftDoubleDivide", IRO_FDiv, IRTK_F64, IRTK_F64);
	AssertSoftFloatName("SoftFloatToDouble", IRO_FExt, IRTK_F64, IRTK_F32);
	AssertSoftFloatName("SoftDoubleToFloat", IRO_FTrunc, IRTK_F32, IRTK_F64);
	AssertSoftFloatName("SoftDoubleFromInt", IRO_UIToF, IRTK_F64, IRTK_I16);
	AssertSoftFloatName("SoftFloatFromUnsigned", IRO_UIToF, IRTK_F32, IRTK_I32);
	AssertSoftFloatName("SoftFloatFromLongLong", IRO_SIToF, IRTK_F32, IRTK_I64);
	AssertSoftFloatName("SoftDoubleToUnsignedLongLong", IRO_FToUI, IRTK_I64, IRTK_F64);
	AssertSoftFloatName("SoftFloatToInt", IRO_FToSI, IRTK_I32, IRTK_F32);
	AssertSoftFloatName("SoftDoubleCompare", IRO_FLt, IRTK_F64, IRTK_F64);

	AssertInt(1, CIRBackend::FloatCompareValues(IRO_FNe, aiValues, &bNot));
	AssertInt(0, aiValues[0]);
	AssertTrue(bNot);
	AssertInt(2, CIRBackend::FloatCompareValues(IRO_FLe, aiValues, &bNot));
	AssertInt(-1, aiValues[0]);
	AssertInt(0, aiValues[1]);
	AssertFalse(bNot);
	AssertInt(1, CIRBackend::FloatCompareValues(IRO_FOrdered, aiValues, &bNot));
	AssertInt(2, aiValues[0]);
	AssertTrue(bNot);
}


//////////////////////////////////////////////////////////////////////////
//
// Two comparisons of the same operands, in either order (FlagsKnown, both
// back ends).
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendSameOperands(void)
{
	SIRInstruction	sFirst;
	SIRInstruction	sSecond;

	memset(&sFirst, 0, sizeof(sFirst));
	memset(&sSecond, 0, sizeof(sSecond));
	sFirst.asArguments[0] = IRReference(IRRK_Temporary, 1);
	sFirst.asArguments[1] = IRReference(IRRK_Constant, 2);
	sSecond.asArguments[0] = IRReference(IRRK_Temporary, 1);
	sSecond.asArguments[1] = IRReference(IRRK_Constant, 2);
	AssertTrue(CIRBackend::SameOperands(&sFirst, &sSecond));

	sSecond.asArguments[0] = IRReference(IRRK_Constant, 2);
	sSecond.asArguments[1] = IRReference(IRRK_Temporary, 1);
	AssertTrue(CIRBackend::SameOperands(&sFirst, &sSecond));

	sSecond.asArguments[1] = IRReference(IRRK_Temporary, 3);
	AssertFalse(CIRBackend::SameOperands(&sFirst, &sSecond));

	sSecond.asArguments[0] = IRReference(IRRK_Temporary, 1);
	sSecond.asArguments[1] = IRReference(IRRK_Temporary, 1);
	AssertFalse(CIRBackend::SameOperands(&sFirst, &sSecond));
}


//////////////////////////////////////////////////////////////////////////
//
// A multiplier's signed digits (ExpandMultiplies, both back ends): its
// non-adjacent form, lowest first, none past the top bit.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendSignedDigits(void)
{
	int		aiPlaces[IR_MAX_MULTIPLY_DIGITS];
	int		aiSigns[IR_MAX_MULTIPLY_DIGITS];

	//3 is 4 - 1.
	AssertInt(2, CIRBackend::SignedDigits(3, 32, aiPlaces, aiSigns));
	AssertInt(0, aiPlaces[0]);
	AssertInt(-1, aiSigns[0]);
	AssertInt(2, aiPlaces[1]);
	AssertInt(1, aiSigns[1]);

	//10 is 8 + 2.
	AssertInt(2, CIRBackend::SignedDigits(10, 32, aiPlaces, aiSigns));
	AssertInt(1, aiPlaces[0]);
	AssertInt(1, aiSigns[0]);
	AssertInt(3, aiPlaces[1]);
	AssertInt(1, aiSigns[1]);

	//11 is 16 - 4 - 1.
	AssertInt(3, CIRBackend::SignedDigits(11, 32, aiPlaces, aiSigns));
	AssertInt(0, aiPlaces[0]);
	AssertInt(-1, aiSigns[0]);
	AssertInt(2, aiPlaces[1]);
	AssertInt(-1, aiSigns[1]);
	AssertInt(4, aiPlaces[2]);
	AssertInt(1, aiSigns[2]);

	//0xFFFF on 16 bits is -1: the 2^16 digit is past the top.
	AssertInt(1, CIRBackend::SignedDigits(0xFFFF, 16, aiPlaces, aiSigns));
	AssertInt(0, aiPlaces[0]);
	AssertInt(-1, aiSigns[0]);

	//0x5555 (8 digits) fits; 0x15555 (9) doesn't.
	AssertInt(8, CIRBackend::SignedDigits(0x5555, 32, aiPlaces, aiSigns));
	AssertInt(14, aiPlaces[7]);
	AssertInt(-1, CIRBackend::SignedDigits(0x15555, 32, aiPlaces, aiSigns));
}


//////////////////////////////////////////////////////////////////////////
//
// The back end's peephole: a store to a pseudo register read straight back
// is a cpy when both registers are known to be 32 bits.  They are at the
// function's label and a block's; at a label in an instruction's code (not
// a block's) they aren't known, so the load stays.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendPeepholeWidths(void)
{
	CArrayTemplate<int32>	aiBlockLabels;
	CChars					sz;
	int32					iLabel;

	aiBlockLabels.Init();
	iLabel = 3;
	aiBlockLabels.Add(&iLabel);

	sz.Init("_F:\n\t\tcpy [4 + W], A\n\t\tcpy X, [4 + W]\nL3:\n\t\tcpy [4 + W], A\n\t\tcpy X, [4 + W]\nL7:\n\t\tcpy [4 + W], A\n\t\tcpy X, [4 + W]\n\t\tret\n");
	CIR65Backend::PeepholeCode(&sz, 0, &aiBlockLabels, false);
	AssertString("_F:\n\t\tcpy [4 + W], A\n\t\tcpy X, A\nL3:\n\t\tcpy [4 + W], A\n\t\tcpy X, A\nL7:\n\t\tcpy [4 + W], A\n\t\tcpy X, [4 + W]\n\t\tret\n", sz.Text());
	sz.Kill();
	aiBlockLabels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CIR65Peephole on szCode, as a whole function; bHomeV: V is a value's
// home in it.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65PeepholeOptimise(const char* szExpected, const char* szCode, bool bHomeV = false)
{
	CIR65Peephole	cPeephole;
	CChars			sz;

	sz.Init(szCode);
	cPeephole.Init();
	cPeephole.Optimise(&sz, 0, bHomeV);
	cPeephole.Kill();
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// V as a third register (CIR65Backend::CanUseV) in a function that needs
// no frame: it holds 32 bit values and is kept by calls.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendHomeV(void)
{
	CArrayTemplate<int32>	aiBlockLabels;
	CChars					szAssembly;
	CChars					sz;

	//A list walked with a call for each node: the node is live across the calls, in V.
	szAssembly.Init();
	AssertInt(5 + 7 + 9, (int32)TestIR65Run(
		"global @_gThird, align 4 { i32 0, i32 9 }\n"
		"global @_gSecond, align 4 { ptr @_gThird, i32 7 }\n"
		"global @_gFirst, align 4 { ptr @_gSecond, i32 5 }\n\n"
		"function i32 @_Value__FP5SNode(ptr %n)\n{\nb:\n\t%a = ptradd ptr %n, 4\n\t%v = load i32 %a\n\tret i32 %v\n}\n\n"
		"function i32 @_Sum__FP5SNode(ptr %first)\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%n = phi ptr [entry, %first], [loop, %next]\n"
		"\t%s = phi i32 [entry, 0], [loop, %s2]\n"
		"\t%v = call i32 @_Value__FP5SNode(ptr %n)\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%next = load ptr %n\n"
		"\t%c = ne ptr %next, 0\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n\tret i32 %s2\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Sum__FP5SNode(ptr @_gFirst)\n"
		"\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("\t\tpsh V\n"));
	AssertTrue(szAssembly.Contains("\t\tcpy V, [V]\n"));
	AssertTrue(szAssembly.Contains("\t\tpul V\n\t\tret\n"));
	szAssembly.Kill();

	//An i32 in V: stored as a byte, pushed as a byte and made from a sign extended byte, all through A (V has no other
	//width).  200 as a byte is -56; the sum's low byte is stored and read back.
	szAssembly.Init();
	AssertInt(-56 + 3 + ((-56 + 3) & 0xFF) + 144, (int32)TestIR65Run(
		"global @_gBytes, align 1 { i8 200, i8 0 }\n\n"
		"function i32 @_Three__F()\n{\nb:\n\tret i32 3\n}\n\n"
		"function i32 @_Byte__Fc(i8 %b)\n{\nb:\n\t%r = zext i8 %b to i32\n\tret i32 %r\n}\n\n"
		"function i32 @_Work__FPc(ptr %p)\n{\nb:\n"
		"\t%1 = load i8 %p\n\t%x = sext i8 %1 to i32\n"
		"\t%t = call i32 @_Three__F()\n"
		"\t%y = add i32 %x, %t\n"
		"\t%q = ptradd ptr %p, 1\n"
		"\t%yb = trunc i32 %y to i8\n\tstore i8 %yb, %q\n"
		"\t%u = call i32 @_Byte__Fc(i8 %yb)\n"
		"\t%2 = load i8 %q\n\t%z = zext i8 %2 to i32\n"
		"\t%s = add i32 %y, %z\n\t%r = add i32 %s, %u\n"
		"\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Work__FPc(ptr @_gBytes)\n"
		"\t%d = sub i32 %r, 59\n"
		"\tret i32 %d\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("\t\tpsh V\n"));
	AssertFalse(szAssembly.Contains("wth V") || szAssembly.Contains("wthx V") || szAssembly.Contains("cpyx V") || szAssembly.Contains(", V\n\t\twth"));
	szAssembly.Kill();

	//A function with a slot has a frame: V is its register, and the pointer live across the calls is in a kept pseudo
	//register.
	szAssembly.Init();
	AssertInt(8 + 9, (int32)TestIR65Run(
		"function i32 @_Next__Fi(i32 %x)\n{\nb:\n\t%r = add i32 %x, 1\n\tret i32 %r\n}\n\n"
		"function i32 @_Twice__FPFiiE(ptr %f)\n{\nb:\n"
		"\t%s = slot ptr 4, 4\n\tstore i32 7, %s\n"
		"\t%v = load i32 %s\n"
		"\t%a = call i32 %f(i32 %v)\n"
		"\tstore i32 %a, %s\n"
		"\t%b = call i32 %f(i32 %a)\n"
		"\t%w = load i32 %s\n"
		"\t%r = add i32 %w, %b\n"
		"\tret i32 %r\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%f = copy ptr @_Next__Fi\n"
		"\t%r = call i32 @_Twice__FPFiiE(ptr %f)\n"
		"\tret i32 %r\n}\n", false, &szAssembly));
	AssertTrue(szAssembly.Contains("\t\tcpy V, S\n"));
	AssertFalse(szAssembly.Contains("jsr V"));
	szAssembly.Kill();

	//W as a fourth register between calls, multiplies and divides, which change it: the loop's four values have X, Y, W
	//and V; after it comes a multiply, which counts in W from 0 (W is set first), and a call.  The pseudo registers are
	//written [n], not [n + W].
	szAssembly.Init();
	AssertInt((1 + 2 + 3 + 4) * (1 ^ 2 ^ 3 ^ 4) + 1, (int32)TestIR65Run(
		"function i32 @_Next__Fi(i32 %x)\n{\nb:\n\t%r = add i32 %x, 1\n\tret i32 %r\n}\n\n"
		"function i32 @_Work__Fi(i32 %d)\n{\n"
		"entry:\n\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%s = phi i32 [entry, 0], [loop, %s2]\n"
		"\t%x = phi i32 [entry, 0], [loop, %x2]\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%s2 = add i32 %s, %i2\n"
		"\t%x2 = xor i32 %x, %i2\n"
		"\t%c1 = slt i32 %i2, %d\n"
		"\tbrif %c1, loop, exit\n"
		"exit:\n"
		"\t%m = mul i32 %s2, %x2\n"
		"\t%n = call i32 @_Next__Fi(i32 %m)\n"
		"\tret i32 %n\n}\n\n"
		"function i32 @_main()\n{\nb:\n"
		"\t%r = call i32 @_Work__Fi(i32 4)\n"
		"\tret i32 %r\n}\n", false, &szAssembly));
	sz.Init();
	sz.AppendSubString(szAssembly, szAssembly.Find("_Work__Fi:"), szAssembly.Find("\t\t.section text._main"));
	AssertTrue(sz.Contains("\t\txor W, X\n"));
	AssertTrue(sz.Contains("\t\tcpy B, W\n\t\tcpy W, 0\n\t\tmul\n\t\tcpy X, Y\n"));
	AssertFalse(sz.Contains("\t\tcpy W, 0\n\t\tjsr") || sz.Contains("\t\tcpy W, 0\n\t\tpul V\n"));
	AssertFalse(sz.Contains(" + W]"));
	sz.Kill();
	szAssembly.Kill();

	//A cpy W, 0 where W is 0 already goes (after another, with nothing that writes W between); one at the function's
	//start, after a call, after W is written, after a mul, or where a way in leaves W unknown stays.
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy W, 0\n"
		"\t\tadd A, 1\n"
		"\t\tmul\n"
		"\t\tcpy W, 0\n"
		"\t\tjsr _G\n"
		"\t\tcpy W, 0\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L1\n"
		"\t\tcpy W, A\n"
		"L1:\n"
		"\t\tcpy W, 0\n"
		"\t\tdiv\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy W, 0\n"
		"\t\tadd A, 1\n"
		"\t\tcpy W, 0\n"
		"\t\tmul\n"
		"\t\tcpy W, 0\n"
		"\t\tcpy W, 0\n"
		"\t\tjsr _G\n"
		"\t\tcpy W, 0\n"
		"\t\tcmp A, 0\n"
		"\t\tbras z, L1\n"
		"\t\tcpy W, A\n"
		"L1:\n"
		"\t\tcpy W, 0\n"
		"\t\tdiv\n"
		"\t\tret\n");

	//Where V is a value's home, [n + V] is anyone's memory (a device's register, say): a store to it read straight
	//back stays a read, and two reads of it stay two.
	aiBlockLabels.Init();
	sz.Init("_F:\n\t\tcpy [4 + V], A\n\t\tcpy X, [4 + V]\n\t\tret\n");
	CIR65Backend::PeepholeCode(&sz, 0, &aiBlockLabels, true);
	AssertString("_F:\n\t\tcpy [4 + V], A\n\t\tcpy X, [4 + V]\n\t\tret\n", sz.Text());
	sz.Kill();
	sz.Init("_F:\n\t\tcpy [4 + V], A\n\t\tcpy X, [4 + V]\n\t\tret\n");
	CIR65Backend::PeepholeCode(&sz, 0, &aiBlockLabels, false);
	AssertString("_F:\n\t\tcpy [4 + V], A\n\t\tcpy X, A\n\t\tret\n", sz.Text());
	sz.Kill();
	aiBlockLabels.Kill();

	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tcpy X, A\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tadd A, X\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tcpy X, A\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tadd A, X\n"
		"\t\tret\n", true);
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tcpy X, A\n"
		"\t\tadd A, X\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tcpy X, A\n"
		"\t\tcpy A, [4 + V]\n"
		"\t\tadd A, X\n"
		"\t\tret\n", false);
}


//////////////////////////////////////////////////////////////////////////
//
// A value already held stays known among others, and one whose name starts
// with another's is its own (CIR65Peephole::AddValue).
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendPeepholeValues(void)
{
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy A, _Long\n"
		"\t\tcpy B, _LongName\n"
		"\t\tcpy X, A\n"
		"\t\tcpy Y, _Other\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy A, _Long\n"
		"\t\tcpy B, _LongName\n"
		"\t\tcpy X, A\n"
		"\t\tcpy Y, _Other\n"
		"\t\tcpy A, _Long\n"
		"\t\tcpy B, _LongName\n"
		"\t\tcpy X, _Long\n"
		"\t\tret\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A branch or bra to the label after it goes (only a label or nothing
// between); one with an instruction between stays.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendPeepholeBranchesToNext(void)
{
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcmp A, B\n"
		"L1:\n"
		"\t\tadd A, 1\n"
		"L2:\n"
		"\t\tbras z, L3\n"
		"\t\tadd A, 2\n"
		"L3:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp A, B\n"
		"\t\tbras z, L1\n"
		"L1:\n"
		"\t\tadd A, 1\n"
		"\t\tbra L2\n"
		"L2:\n"
		"\t\tbras z, L3\n"
		"\t\tadd A, 2\n"
		"L3:\n"
		"\t\tret\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A cpy of what the register already holds goes: a register, a constant
// or the function's own memory, unchanged since (across a label the ways
// in agree on), when the flags it sets aren't read.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendPeepholeCopies(void)
{
	//B still holds [9 + V]; A holds [12 + W] after storing it; X holds what A did.
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy B, [9 + V]\n"
		"\t\tcpy X, [6 + B]\n"
		"\t\tcpy A, [4 + B]\n"
		"\t\tcpy [12 + W], A\n"
		"\t\tcpy X, A\n"
		"\t\tadd X, [12]\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy B, [9 + V]\n"
		"\t\tcpy X, [6 + B]\n"
		"\t\tcpy B, [9 + V]\n"
		"\t\tcpy A, [4 + B]\n"
		"\t\tcpy [12 + W], A\n"
		"\t\tcpy A, [12 + W]\n"
		"\t\tcpy X, A\n"
		"\t\tcpy A, X\n"
		"\t\tadd X, [12]\n"
		"\t\tret\n");

	//Kept: z of the second cpy is read; [8 + V] may be written through X; [X] may be volatile; a call changes
	//everything; A is 16 bits wide; B changed.
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\tcpy X, A\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\tbras z, L1\n"
		"\t\tcpy A, [8 + V]\n"
		"\t\tcpy [X], 0\n"
		"\t\tcpy A, [8 + V]\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy Y, 5\n"
		"\t\tjsr _G\n"
		"\t\tcpy Y, 5\n"
		"\t\twth A, 16\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\twth A, 32\n"
		"\t\tcpy A, B\n"
		"\t\tadd B, 1\n"
		"\t\tcpy A, B\n"
		"L1:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\tcpy X, A\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\tbras z, L1\n"
		"\t\tcpy A, [8 + V]\n"
		"\t\tcpy [X], 0\n"
		"\t\tcpy A, [8 + V]\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy A, [X]\n"
		"\t\tcpy Y, 5\n"
		"\t\tjsr _G\n"
		"\t\tcpy Y, 5\n"
		"\t\twth A, 16\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\tcpy A, [4 + W]\n"
		"\t\twth A, 32\n"
		"\t\tcpy A, B\n"
		"\t\tadd B, 1\n"
		"\t\tcpy A, B\n"
		"L1:\n"
		"\t\tret\n");

	//[8 + W] can't be written through a pointer; both ways into L1 have X = 1, only one has Y = 2.
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcpy A, [8 + W]\n"
		"\t\tcpy [X], B\n"
		"\t\tcpy X, 1\n"
		"\t\tcmp A, B\n"
		"\t\tbras z, L1\n"
		"\t\tcpy Y, 2\n"
		"L1:\n"
		"\t\tcpy Y, 2\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcpy A, [8 + W]\n"
		"\t\tcpy [X], B\n"
		"\t\tcpy A, [8 + W]\n"
		"\t\tcpy X, 1\n"
		"\t\tcmp A, B\n"
		"\t\tbras z, L1\n"
		"\t\tcpy Y, 2\n"
		"L1:\n"
		"\t\tcpy X, 1\n"
		"\t\tcpy Y, 2\n"
		"\t\tret\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A branch round a bra becomes the opposite branch; a jump to a jump goes
// straight on; a bra to a ret is a ret; then the code nothing reaches goes.
//
//////////////////////////////////////////////////////////////////////////
//
// A listing's source markers are taken out before the text passes and put
// back after: before the line they were before, a line a pass rewrote
// (paired in order between lines that stayed), or the next line still
// there; of two that go before one line the last stays.  A line repeated
// further on doesn't draw a marker to it.
//
//////////////////////////////////////////////////////////////////////////
void TestIRSourceMarkers(void)
{
	CIRSourceMarkers	cMarkers;
	CChars				sz;

	sz.Init("; before\n");
	sz.Append(
		"_F:\n"
		"\t\t;@5\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0 + W], A\n"
		"\t\t;@7\n"
		"\t\tcpy A, 1\n"
		"\t\t;@9\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [4 + W], A\n"
		"\t\t;@11\n"
		"\t\tadd A, 2\n"
		"\t\tret\n");
	cMarkers.Init();
	cMarkers.Take(&sz, 9);
	AssertString(
		"; before\n"
		"_F:\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0 + W], A\n"
		"\t\tcpy A, 1\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [4 + W], A\n"
		"\t\tadd A, 2\n"
		"\t\tret\n", sz.Text());

	//The passes: the + W goes, the first cpy A, 1 goes.
	sz.Kill();
	sz.Init(
		"; before\n"
		"_F:\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0], A\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [4], A\n"
		"\t\tadd A, 2\n"
		"\t\tret\n");
	cMarkers.Put(&sz, 9);
	AssertString(
		"; before\n"
		"_F:\n"
		"\t\t;@5\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0], A\n"
		"\t\t;@9\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [4], A\n"
		"\t\t;@11\n"
		"\t\tadd A, 2\n"
		"\t\tret\n", sz.Text());
	cMarkers.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A block a pass moves keeps its marker: line 6's code moved to the end
// is still line 6's, not lost to line 7's.
//
//////////////////////////////////////////////////////////////////////////
void TestIRSourceMarkersMovedBlock(void)
{
	CIRSourceMarkers	cMarkers;
	CChars				sz;

	sz.Init(
		"_F:\n"
		"\t\t;@5\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0 + W], A\n"
		"\t\t;@6\n"
		"L1:\n"
		"\t\tcpy A, 5\n"
		"\t\tadd A, 6\n"
		"\t\tcpy [2], A\n"
		"\t\t;@7\n"
		"\t\tcpy A, 1\n"
		"\t\tcpy [4], A\n"
		"\t\tret\n");
	cMarkers.Init();
	cMarkers.Take(&sz, 0);

	//The passes: the + W goes and L1's block goes to the end.  The block is longer than the code left in place, so
	//it's the code left in place that's found out of order.
	sz.Kill();
	sz.Init(
		"_F:\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0], A\n"
		"\t\tbra L1\n"
		"L2:\n"
		"\t\tcpy A, 1\n"
		"\t\tcpy [4], A\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tcpy A, 5\n"
		"\t\tadd A, 6\n"
		"\t\tcpy [2], A\n"
		"\t\tbra L2\n");
	cMarkers.Put(&sz, 0);
	AssertString(
		"_F:\n"
		"\t\t;@5\n"
		"\t\tcpy A, 0\n"
		"\t\tcpy [0], A\n"
		"\t\tbra L1\n"
		"L2:\n"
		"\t\t;@7\n"
		"\t\tcpy A, 1\n"
		"\t\tcpy [4], A\n"
		"\t\tret\n"
		"\t\t;@6\n"
		"L1:\n"
		"\t\tcpy A, 5\n"
		"\t\tadd A, 6\n"
		"\t\tcpy [2], A\n"
		"\t\tbra L2\n", sz.Text());
	cMarkers.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
void TestIR65BackendPeepholeJumps(void)
{
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcmp A, 1\n"
		"\t\tbras z, L4\n"
		"L1:\n"
		"\t\tcpy X, 0\n"
		"\t\tbrac z, L2\n"
		"\t\tcpy A, 2\n"
		"\t\tret\n"
		"L2:\n"
		"\t\tcpy A, 3\n"
		"\t\tret\n"
		"L4:\n"
		"\t\tcpy Y, A\n"
		"L5:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp A, 1\n"
		"\t\tbrac z, L1\n"
		"\t\tbra L3\n"
		"L1:\n"
		"\t\tcpy X, 0\n"
		"\t\tbrac z, L2\n"
		"\t\tcpy A, 2\n"
		"\t\tbra L5\n"
		"L2:\n"
		"\t\tcpy A, 3\n"
		"\t\tbra L5\n"
		"L3:\n"
		"\t\tbra L4\n"
		"L4:\n"
		"\t\tcpy Y, A\n"
		"L5:\n"
		"\t\tret\n");

	//braso has no opposite (its bra to a ret is a ret); a branch to the next line goes.
	TestIR65PeepholeOptimise(
		"_F:\n"
		"\t\tcmp A, 1\n"
		"\t\tbraso (n, v), L1\n"
		"\t\tret\n"
		"L1:\n"
		"\t\tcpy A, 2\n"
		"L2:\n"
		"\t\tret\n",
		"_F:\n"
		"\t\tcmp A, 1\n"
		"\t\tbraso (n, v), L1\n"
		"\t\tbra L2\n"
		"L1:\n"
		"\t\tcpy A, 2\n"
		"\t\tbras z, L2\n"
		"L2:\n"
		"\t\tret\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The callee saved pseudo registers start at [$80 + W], four bytes apart, and
// a register's bit is its index in the order A, B, X, Y.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendUnits(void)
{
	AssertInt(128, IR65_SAVED_UNIT(0));
	AssertInt(132, IR65_SAVED_UNIT(1));
	AssertInt(252, IR65_SAVED_UNIT(IR65_PSEUDO_UNITS - 1));
	AssertInt(1, IR65_REGISTER_BIT(IR65CR_A));
	AssertInt(2, IR65_REGISTER_BIT(IR65CR_B));
	AssertInt(4, IR65_REGISTER_BIT(IR65CR_X));
	AssertInt(8, IR65_REGISTER_BIT(IR65CR_Y));
}


//////////////////////////////////////////////////////////////////////////
//
// The mnemonic of each integer opcode.
//
//////////////////////////////////////////////////////////////////////////
void TestIR65BackendMnemonics(void)
{
	AssertString("add", IR65IntegerMnemonic(IRO_Add));
	AssertString("add", IR65IntegerMnemonic(IRO_PtrAdd));
	AssertString("sub", IR65IntegerMnemonic(IRO_Sub));
	AssertString("and", IR65IntegerMnemonic(IRO_And));
	AssertString("or", IR65IntegerMnemonic(IRO_Or));
	AssertString("xor", IR65IntegerMnemonic(IRO_Xor));
	AssertString("xor", IR65IntegerMnemonic(IRO_Not));
	AssertString("shl", IR65IntegerMnemonic(IRO_Shl));
	AssertString("shr", IR65IntegerMnemonic(IRO_LShr));
	AssertString("neg", IR65IntegerMnemonic(IRO_Neg));
	AssertTrue(IR65IntegerMnemonic(IRO_AShr) == NULL);
	AssertTrue(IR65IntegerMnemonic(IRO_Mul) == NULL);
	AssertTrue(IR65IntegerMnemonic(IRO_Call) == NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR65Backend(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();
	C65C64InstructionFactory::GetInstance()->Init();

	TestIR65BackendMnemonics();
	TestIR65BackendUnits();
	TestIR65BackendReturn();
	TestIR65BackendLoop();
	TestIR65BackendSwap();
	TestIR65BackendNarrow();
	TestIR65BackendDivide();
	TestIR65BackendWide();
	TestIR65BackendMemory();
	TestIR65BackendCalls();
	TestIR65BackendFloat();
	TestIR65BackendAllocation();
	TestIR65BackendWidths();
	TestIR65BackendFlags();
	TestIR65BackendPeepholeWidths();
	TestIR65BackendPeepholeBranchesToNext();
	TestIR65BackendPeepholeValues();
	TestIR65BackendSoftFloatNames();
	TestIR65BackendSameOperands();
	TestIR65BackendSignedDigits();
	TestIR65BackendPeepholeCopies();
	TestIR65BackendPeepholeJumps();
	TestIR65BackendHomeV();
	TestIRSourceMarkers();
	TestIRSourceMarkersMovedBlock();

	C65C64InstructionFactory::GetInstance()->Kill();
	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}
