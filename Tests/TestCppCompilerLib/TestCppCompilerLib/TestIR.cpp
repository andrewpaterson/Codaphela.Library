#include "CppCompilerLib/IRParser.h"
#include "CppCompilerLib/IRPrinter.h"
#include "CppCompilerLib/IRVerifier.h"
#include "CppCompilerLib/IRThrows.h"
#include "CppCompilerLib/IROptimiserInternal.h"
#include "CppCompilerLib/IROptimiser.h"
#include "CppCompilerLib/IRLoops.h"
#include "CppCompilerLib/IRImports.h"
#include "CppCompilerLib/IRInliner.h"
#include "CppCompilerLib/IRSSAUpdater.h"
#include "CppCompilerLib/IRLiveness.h"
#include "CppCompilerLib/IRBackendPasses.h"
#include "CppCompilerLib/IRW65C816Costs.h"
#include "CppCompilerLib/CPPTarget.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Parses szText, then prints the module into pszPrinted.  The parser's and
// verifier's errors are in pcErrors.
//
//////////////////////////////////////////////////////////////////////////
bool TestIRParseVerifyPrint(const char* szText, CIRModule* pcModule, CCPPErrors* pcErrors, CChars* pszPrinted)
{
	CIRParser	cParser;
	CIRVerifier	cVerifier;
	CIRPrinter	cPrinter;
	bool		bParsed;
	bool		bVerified;

	cParser.Init(pcModule, pcErrors);
	bParsed = cParser.Parse(szText);
	cParser.Kill();
	if (!bParsed)
	{
		return false;
	}

	cVerifier.Init(pcModule, pcErrors);
	bVerified = cVerifier.Verify();
	cVerifier.Kill();

	cPrinter.Init(pcModule, pszPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	return bVerified;
}


//////////////////////////////////////////////////////////////////////////
//
// szText must give exactly the errors szExpected (parsing, then verifying if
// it parses).  With no errors it must print back as szText.
//
//////////////////////////////////////////////////////////////////////////
void AssertIRText(const char* szExpected, const char* szText)
{
	CIRModule	cModule;
	CCPPErrors	cErrors;
	CChars		szPrinted;
	bool		bResult;

	cModule.Init();
	cErrors.Init();
	szPrinted.Init();
	bResult = TestIRParseVerifyPrint(szText, &cModule, &cErrors, &szPrinted);
	AssertString(szExpected, cErrors.Text());
	AssertBool(szExpected[0] == '\0', bResult);
	if (bResult)
	{
		AssertString(szText, szPrinted.Text());
	}
	szPrinted.Kill();
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIRRoundTrip(const char* szText)
{
	AssertIRText("", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIRErrors(const char* szExpected, const char* szText)
{
	AssertIRText(szExpected, szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripLoop(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
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
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol marked for register arguments (CIRModule::SetRegisterCall, the
// W65C816's) is printed and parsed as regcall, after the restricts.
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripRegisterCall(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
		"restrict @M\n"
		"regcall @_F__Fii\n"
		"regcall @_memcpy\n"
		"\n"
		"function i32 @_F__Fii(i32 %a, i32 %b)\n"
		"{\n"
		"entry:\n"
		"\t%m = load ptr @M\n"
		"\t%p = call ptr @_memcpy(ptr %m, ptr %m, i32 %b)\n"
		"\t%s = add i32 %a, %b\n"
		"\tret i32 %s\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol marked as writing no memory its callers can see (CIRModule::
// SetNoWrites) is printed and parsed as nowrites, after the regcalls.
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripNoWrites(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
		"regcall @_F__Fi\n"
		"nowrites @_F__Fi\n"
		"nowrites @_G__Fv\n"
		"\n"
		"function i32 @_F__Fi(i32 %a)\n"
		"{\n"
		"entry:\n"
		"\t%s = add i32 %a, 1\n"
		"\tret i32 %s\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripEverything(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
		"\n"
		"global @gTable, align 4, local { i32 1, i16 -2, i8 -1, ptr @Str3+4, ptr @gTable-8, i64 -9223372036854775808, zero 52 }\n"
		"\n"
		"global @Str3, align 1, local, constant { bytes \"hello \\22world\\22\\0A\\00\" }\n"
		"\n"
		"global @gFloats, align 8 { f32 0x3F800000, f64 0x400921FB54442D18 }\n"
		"\n"
		"global @gEmpty, align 1, weak { }\n"
		"\n"
		"function void @_Everything__Fv(), noreturn, inline\n"
		"{\n"
		"start:\n"
		"\t%slot = slot ptr 8, 4\n"
		"\tstore volatile i32 7, %slot\n"
		"\t%x = load volatile i32 %slot\n"
		"\t%y = sub i32 0, %x\n"
		"\t%z = mul i32 %y, -1\n"
		"\t%d = sdiv i32 %z, 3\n"
		"\t%e = udiv i32 %d, 3\n"
		"\t%f = srem i32 %e, 3\n"
		"\t%g = urem i32 %f, 3\n"
		"\t%h = and i32 %g, 255\n"
		"\t%k = or i32 %h, 256\n"
		"\t%l = xor i32 %k, -1\n"
		"\t%m = lshr i32 %l, 1\n"
		"\t%n = ashr i32 %m, 1\n"
		"\t%q = neg i32 %n\n"
		"\t%r = not i32 %q\n"
		"\t%s = copy i32 %r\n"
		"\t%t = trunc i32 %s to i8\n"
		"\t%u = sext i8 %t to i64\n"
		"\t%w = zext i8 %t to i16\n"
		"\t%fl = sitof i64 %u to f32\n"
		"\t%fu = uitof i16 %w to f64\n"
		"\t%fa = fadd f32 %fl, 0x40000000\n"
		"\t%fb = fsub f32 %fa, %fl\n"
		"\t%fc = fmul f32 %fb, %fb\n"
		"\t%fd = fdiv f32 %fc, %fa\n"
		"\t%fe = fneg f32 %fd\n"
		"\t%ff = fext f32 %fe to f64\n"
		"\t%fg = ftrunc f64 %fu to f32\n"
		"\t%fh = ftosi f64 %ff to i32\n"
		"\t%fi = ftoui f32 %fg to i16\n"
		"\t%c1 = feq f64 %ff, %fu\n"
		"\t%c2 = funordered f32 %fe, %fe\n"
		"\t%c3 = ule i16 %fi, 3\n"
		"\t%c4 = eq ptr %slot, 0\n"
		"\t%pi = ptrtoi ptr %slot to i32\n"
		"\t%ip = itoptr i32 %pi to ptr\n"
		"\t%pa = ptradd ptr @gTable, 8\n"
		"\t%r1 = call i32 @_Callee__Fii(i32 %fh, ptr %ip)\n"
		"\tcall void @printf(ptr @Str3, ..., i32 %r1, f64 %ff)\n"
		"\tcall void @_Exit(...)\n"
		"\tcall void %pa()\n"
		"\t%r2 = call i8 @vf(i8 %t, ...)\n"
		"\tunreachable\n"
		"}\n"
		"\n"
		"function i32 @printf(ptr %format, ...), weak\n"
		"{\n"
		"b0:\n"
		"\tret i32 0\n"
		"}\n"
		"\n"
		"function void @_Nothing__Fv(), local\n"
		"{\n"
		"b0:\n"
		"\tret\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// W65C816 spaces: far by default, near and zero.  A pointer constant in
// another space keeps its type as a load's or store's address and a callee.
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripSpaces(void)
{
	AssertIRRoundTrip(
		"space far 4\n"
		"space near 2\n"
		"space zero 2\n"
		"\n"
		"function ptr @_Spaces__FPNcs(ptr.near %p, i16 %o)\n"
		"{\n"
		"entry:\n"
		"\t%q = ptradd inbank ptr.near %p, %o\n"
		"\t%r = ptradd ptr.near %q, -1\n"
		"\t%l = slot ptr.zero 2, 2\n"
		"\tstore ptr.near %r, %l\n"
		"\t%f = spacecast ptr.near %r to ptr\n"
		"\t%g = spacecast ptr.zero %l to ptr\n"
		"\t%c = ult ptr %f, %g\n"
		"\tstore i16 7, ptr.near 16\n"
		"\t%v = load i16 ptr.zero 2\n"
		"\tstore i16 %v, 32\n"
		"\tcall void ptr.near 64()\n"
		"\tbrif %c, yes, no\n"
		"yes:\n"
		"\tret ptr %f\n"
		"no:\n"
		"\tret ptr %g\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Builds the loop with the API and prints it.
//
//////////////////////////////////////////////////////////////////////////
void TestIRBuild(void)
{
	CIRModule		cModule;
	CIRFunction*	pcFunction;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcBody;
	CIRBlock*		pcExit;
	SIRReference	sP;
	SIRReference	sN;
	SIRReference	sS;
	SIRReference	sI;
	SIRReference	sC;
	SIRReference	sO;
	SIRReference	sA;
	SIRReference	sV;
	SIRReference	sS2;
	SIRReference	sI2;
	SIRReference	sZero;
	SIRType			sInt;
	SIRType			sPtr;
	CCPPErrors		cErrors;
	CIRVerifier		cVerifier;
	CIRPrinter		cPrinter;
	CChars			szPrinted;

	cModule.Init();
	cModule.AddSpace("default", 4);
	sInt = IRType(IRTK_I32);
	sPtr = IRType(IRTK_Pointer);
	pcFunction = cModule.AddFunction("_Sum__FPii", sInt);
	sP = pcFunction->AddParameter(sPtr, "p");
	sN = pcFunction->AddParameter(sInt, "n");
	pcEntry = pcFunction->AddBlock("entry");
	pcLoop = pcFunction->AddBlock("loop");
	pcBody = pcFunction->AddBlock("body");
	pcExit = pcFunction->AddBlock("exit");

	sZero = pcFunction->AddInteger(sInt, 0);
	pcEntry->Jump(pcLoop);

	sS = pcFunction->AddTemporary(sInt, "s");
	sI = pcFunction->AddTemporary(sInt);
	pcLoop->AddPhi(sInt, sS);
	pcLoop->AddPhi(sInt, sI);
	pcLoop->AddPhiArgument(pcLoop->masPhis.Get(0), pcEntry, sZero);
	pcLoop->AddPhiArgument(pcLoop->masPhis.Get(1), pcEntry, sZero);
	sC = pcFunction->Add(pcLoop, IRO_SLt, sInt, sI, sN);
	pcLoop->Branch(sC, pcBody, pcExit);

	sO = pcFunction->Add(pcBody, IRO_Shl, sInt, sI, pcFunction->AddInteger(sInt, 2));
	sA = pcFunction->Add(pcBody, IRO_PtrAdd, sPtr, sP, sO);
	sV = pcFunction->Load(pcBody, sInt, sA);
	sS2 = pcFunction->Add(pcBody, IRO_Add, sInt, sS, sV, "s");
	sI2 = pcFunction->Add(pcBody, IRO_Add, sInt, sI, pcFunction->AddInteger(sInt, 1));
	pcBody->Jump(pcLoop);
	pcLoop->AddPhiArgument(pcLoop->masPhis.Get(0), pcBody, sS2);
	pcLoop->AddPhiArgument(pcLoop->masPhis.Get(1), pcBody, sI2);

	pcExit->Return(sInt, sS);
	pcFunction->ComputePredecessors();

	cErrors.Init();
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	AssertString("", cErrors.Text());
	cVerifier.Kill();
	cErrors.Kill();

	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
	AssertString(
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%s = phi i32 [entry, 0], [body, %s.1]\n"
		"\t%0 = phi i32 [entry, 0], [body, %5]\n"
		"\t%1 = slt i32 %0, %n\n"
		"\tbrif %1, body, exit\n"
		"body:\n"
		"\t%2 = shl i32 %0, 2\n"
		"\t%3 = ptradd ptr %p, %2\n"
		"\t%4 = load i32 %3\n"
		"\t%s.1 = add i32 %s, %4\n"
		"\t%5 = add i32 %0, 1\n"
		"\tjmp loop\n"
		"exit:\n"
		"\tret i32 %s\n"
		"}\n", szPrinted.Text());
	szPrinted.Kill();

	AssertLong(-1, pcFunction->GetSigned(pcFunction->AddInteger(IRType(IRTK_I8), 255)));
	AssertLong(255, (int64)pcFunction->GetConstant(pcFunction->AddInteger(IRType(IRTK_I8), -1))->ulliBits);
	AssertInt(pcFunction->AddInteger(sInt, 2).uiValue, pcFunction->AddInteger(sInt, 2).uiValue);
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks are ordered by their labels however they were first named.
//
//////////////////////////////////////////////////////////////////////////
void TestIRParseForwardReferences(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
		"\n"
		"function i32 @_F__Fi(i32 %x)\n"
		"{\n"
		"a:\n"
		"\t%c = eq i32 %x, 0\n"
		"\tbrif %c, d, b\n"
		"b:\n"
		"\t%y = add i32 %x, 1\n"
		"\tjmp c\n"
		"c:\n"
		"\tjmp d\n"
		"d:\n"
		"\t%r = phi i32 [a, %x], [c, %y]\n"
		"\tret i32 %r\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Unnamed temporaries and blocks print as numbers (in definition and block
// order) and parse back as unnamed.
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripNumbered(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
		"\n"
		"function i32 @f(i32 %0, i32 %x)\n"
		"{\n"
		"0:\n"
		"\t%1 = add i32 %0, %x\n"
		"\tjmp 1\n"
		"1:\n"
		"\t%2 = phi i32 [0, %1], [named, %3]\n"
		"\t%c = eq i32 %2, 0\n"
		"\tbrif %c, named, 2\n"
		"named:\n"
		"\t%3 = sub i32 %2, 1\n"
		"\tjmp 1\n"
		"2:\n"
		"\tret i32 %2\n"
		"}\n");
}


SIROpcode	gasTestIRTargetOpcodes[2] =
{
	{ "cpya", IRSH_Operands, IRTM_Integer, 0, { IROK_Same, IROK_None }, IRRS_Same, 0, 0 },
	{ "pshb", IRSH_Operands, IRTM_Integer, 0, { IROK_Same, IROK_None }, IRRS_None, IRF_Volatile, 0 },
};


//////////////////////////////////////////////////////////////////////////
//
// A target's opcodes parse, verify and print like the generic ones.
//
//////////////////////////////////////////////////////////////////////////
void TestIRTargetOpcodes(void)
{
	CIRModule	cModule;
	CCPPErrors	cErrors;
	CChars		szPrinted;
	const char*	szText;

	szText =
		"function i32 @f(i32 %x)\n"
		"{\n"
		"b:\n"
		"\t%y = cpya i32 %x\n"
		"\tpshb volatile i32 %y\n"
		"\t%z = add i32 %y, 1\n"
		"\tret i32 %z\n"
		"}\n";
	cModule.Init();
	cModule.SetTargetOpcodes(gasTestIRTargetOpcodes, 2);
	AssertInt(IRO_NumGeneric + 1, cModule.FindOpcode("pshb"));
	AssertString("cpya", cModule.GetOpcode(IRO_NumGeneric)->szName);
	AssertNull(cModule.GetOpcode(IRO_NumGeneric + 2));
	cErrors.Init();
	szPrinted.Init();
	AssertTrue(TestIRParseVerifyPrint(szText, &cModule, &cErrors, &szPrinted));
	AssertString("", cErrors.Text());
	AssertString(szText, szPrinted.Text());
	szPrinted.Kill();
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A long chain of blocks, a block with many predecessors and phis and many
// constants, so a quadratic path would show as a slow test.
//
//////////////////////////////////////////////////////////////////////////
void TestIRLarge(void)
{
	CChars	szText;
	int		i;
	int		j;

	szText.Init("space default 4\n\nfunction i32 @f(i32 %x)\n{\n");
	for (i = 0; i < 5000; i++)
	{
		szText.Append("b");
		szText.Append(i);
		szText.Append(":\n\t%v");
		szText.Append(i);
		szText.Append(" = add i32 %x, ");
		szText.Append(i * 7 + 100000);
		szText.Append("\n\t%c");
		szText.Append(i);
		szText.Append(" = eq i32 %v");
		szText.Append(i);
		szText.Append(", 0\n\tbrif %c");
		szText.Append(i);
		szText.Append(", merge, b");
		szText.Append(i + 1);
		szText.Append("\n");
	}
	szText.Append("b5000:\n\tjmp merge\nmerge:\n");
	for (j = 0; j < 20; j++)
	{
		szText.Append("\t%p");
		szText.Append(j);
		szText.Append(" = phi i32");
		for (i = 0; i <= 5000; i++)
		{
			szText.Append(i == 0 ? " [b" : ", [b");
			szText.Append(i);
			szText.Append(", ");
			if (i < 5000)
			{
				szText.Append("%v");
				szText.Append(i);
			}
			else
			{
				szText.Append(j);
			}
			szText.Append("]");
		}
		szText.Append("\n");
	}
	szText.Append("\tret i32 %p19\n}\n");

	AssertIRRoundTrip(szText.Text());
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRParserErrors(void)
{
	AssertIRErrors("(1, 1): Expected space, global, function, restrict, regcall, nowrites or nothrow. [fun]\n", "fun");
	AssertIRErrors("(4, 7): Expected an opcode with a result. [plus]\n",
		"function void @f()\n{\nb:\n\t%x = plus i32 1, 2\n\tret\n}\n");
	AssertIRErrors("(4, 2): Expected a block label. [%x]\n",
		"function void @f()\n{\n\n\t%x = add i32 1, 2\n}\n");
	AssertIRErrors("(5, 2): Instruction after the block's jump. [ret]\n",
		"function void @f()\n{\nb:\n\tret\n\tret\n}\n");
	AssertIRErrors("(4, 15): Temporary is never defined. [%y]\n",
		"function void @f()\n{\nb:\n\t%x = add i32 %y, 2\n\tret\n}\n");
	AssertIRErrors("(5, 2): Temporary defined twice. [%x]\n",
		"function void @f()\n{\nb:\n\t%x = add i32 1, 2\n\t%x = add i32 1, 2\n\tret\n}\n");
	AssertIRErrors("(4, 6): Block has no label. [c]\n",
		"function void @f()\n{\nb:\n\tjmp c\n}\n");
	AssertIRErrors("(3, 10): Unknown address space. [ptr.far]\n",
		"space default 4\n\nfunction ptr.far @f()\n{\nb:\n\tret\n}\n");
	AssertIRErrors("(1, 30): Expected two hexadecimal digits after \\. [\\q\"]\n",
		"global @s, align 1 { bytes \"a\\q\" }\n");

	//Each of the parser's other errors, but two that no text raises.  "Too many instructions in the block." is what
	//a block's array says when it can hold no more, which on a 64 bit host no text is long enough for.  "Expected a
	//value." is for an operand of a kind the parser has no type for, and the opcode table has none outside the
	//conversions, which are read another way.
	AssertIRErrors("(1, 26): Expected a number. [q]\n",
		"global @g, align 4 { f32 q }\n");
	AssertIRErrors("(1, 32): Unterminated string.\n",
		"global @s, align 1 { bytes \"abc\n }\n");
	AssertIRErrors("(1, 21): Expected an attribute. [bogus]\n",
		"function void @f(), bogus\n{\nb:\n\tret\n}\n");
	AssertIRErrors("(7, 15): Symbol defined twice. [@f]\n",
		"function void @f()\n{\nb:\n\tret\n}\n\nfunction void @f()\n{\nb:\n\tret\n}\n");
	AssertIRErrors("(2, 7): Address space defined twice. [default]\n",
		"space default 4\nspace default 4\n");
	AssertIRErrors("(5, 1): Expected '}'.\n",
		"function void @f()\n{\nb:\n\tret\n");
	AssertIRErrors("(7, 1): Block defined twice. [b]\n",
		"function void @f(i1 %c)\n{\na:\n\tbrif %c, b, a2\nb:\n\tret\nb:\n\tret\na2:\n\tret\n}\n");
	AssertIRErrors("(4, 7): Expected an instruction. [(]\n",
		"function void @f()\n{\nb:\n\t%x = (\n\tret\n}\n");
	AssertIRErrors("(6, 7): Expected an opcode with a result. [store]\n",
		"space default 4\n\nfunction void @f(ptr %p)\n{\nb:\n\t%x = store i32 1, %p\n\tret\n}\n");
	AssertIRErrors("(4, 2): The opcode needs a result. [add]\n",
		"function void @f()\n{\nb:\n\tadd i32 1, 2\n\tret\n}\n");
	AssertIRErrors("(7, 2): Phi after the block's instructions. [%y]\n",
		"function void @f()\n{\na:\n\tjmp b\nb:\n\t%x = add i32 1, 2\n\t%y = phi i32 [a, 1]\n\tret\n}\n");
	AssertIRErrors("(4, 2): A void call has no result. [%x]\n",
		"function void @f()\n{\nb:\n\t%x = call void @g()\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRVerifierErrors(void)
{
	AssertIRErrors("@f b: Block has no jump.\n",
		"function void @f()\n{\nb:\n\t%x = add i32 1, 2\n}\n");
	AssertIRErrors("@f b: Operand has the wrong type. [x]\n",
		"function void @f()\n{\nb:\n\t%x = add i16 1, 2\n\t%y = add i32 %x, 2\n\tret\n}\n");
	AssertIRErrors("@f b: ret's type is not the function's.\n",
		"function i32 @f()\n{\nb:\n\tret\n}\n");
	AssertIRErrors("@f b: Type not allowed for the opcode. [store]\n",
		"space default 4\n\nfunction void @f(ptr %p)\n{\nb:\n\t%c = eq i32 1, 2\n\tstore i1 %c, %p\n\tret\n}\n");
	AssertIRErrors("@f b: arg not followed by a call.\n",
		"function void @f()\n{\nb:\n\targ i32 1\n\tret\n}\n");
	AssertIRErrors("@f c: Phi needs one argument for each predecessor. [x]\n",
		"function void @f(i1 %c)\n{\na:\n\tbrif %c, b, c\nb:\n\tjmp c\nc:\n\t%x = phi i32 [a, 1]\n\tret\n}\n");
	AssertIRErrors("@f c: Definition doesn't dominate the use. [y]\n",
		"function i32 @f(i1 %c)\n{\na:\n\tbrif %c, b, c\nb:\n\t%y = add i32 1, 2\n\tjmp c\nc:\n\tret i32 %y\n}\n");
	AssertIRErrors("@f b: Temporary used before it is defined. [y]\n",
		"function i32 @f()\n{\nb:\n\t%x = add i32 %y, 1\n\t%y = add i32 1, 2\n\tret i32 %x\n}\n");
	AssertIRErrors("@f b: Conversion between these types is not allowed. [sext]\n",
		"function void @f()\n{\nb:\n\t%x = sext i32 1 to i16\n\tret\n}\n");
	AssertIRErrors("@f a: brif to the same block twice.\n",
		"function void @f(i1 %c)\n{\na:\n\tbrif %c, b, b\nb:\n\tret\n}\n");
	AssertIRErrors("@f b: The entry block has predecessors.\n",
		"function void @f()\n{\nb:\n\tjmp b\n}\n");
	AssertIRErrors("@f b: Condition has the wrong type. [x]\n",
		"function void @f(i32 %x)\n{\nb:\n\tbrif %x, c, d\nc:\n\tret\nd:\n\tret\n}\n");
	AssertIRErrors("@f: Function has no blocks.\n",
		"function void @f()\n{\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Parses szText, which is right, into pcModule, for a test to put wrong by
// hand what no text can say.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRParseRight(CIRModule* pcModule, const char* szText)
{
	CIRParser	cParser;
	CCPPErrors	cErrors;

	pcModule->Init();
	cErrors.Init();
	cParser.Init(pcModule, &cErrors);
	AssertTrue(cParser.Parse(szText));
	AssertString("", cErrors.Text());
	cParser.Kill();
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The verifier must refuse pcModule and say exactly szExpected of it.  The
// module is killed.
//
//////////////////////////////////////////////////////////////////////////
static void AssertIRVerifierSays(const char* szExpected, CIRModule* pcModule)
{
	CCPPErrors	cErrors;
	CIRVerifier	cVerifier;

	cErrors.Init();
	cVerifier.Init(pcModule, &cErrors);
	AssertFalse(cVerifier.Verify());
	cVerifier.Kill();
	AssertString(szExpected, cErrors.Text());
	cErrors.Kill();
	pcModule->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What the verifier says of IR that no text gives, as the parser refuses
// it first or cannot write it: a function a pass has left wrong.  Each is
// a right function put wrong by hand in one place.
//
//////////////////////////////////////////////////////////////////////////
void TestIRVerifierBroken(void)
{
	static const char*	szNeg = "function void @f(i32 %a)\n{\nb:\n\t%x = neg i32 %a\n\tret\n}\n";
	static const char*	szUsed = "function i32 @f(i32 %a)\n{\nb:\n\t%x = neg i32 %a\n\tret i32 %x\n}\n";
	static const char*	szStore = "space default 4\n\nfunction void @f(ptr %p)\n{\nb:\n\tstore i32 1, %p\n\tret\n}\n";
	static const char*	szPhi = "function void @f(i1 %c)\n{\na:\n\tbrif %c, b, c\nb:\n\tjmp c\nc:\n\t%x = phi i32 [a, 1], [b, 2]\n\tret\n}\n";
	static const char*	szTwo = "function void @f()\n{\nb:\n\tjmp c\nc:\n\tret\n}\n\nfunction void @g()\n{\nd:\n\tret\n}\n";
	static const char*	szCall = "space default 4\n\nfunction void @f(ptr %p)\n{\nb:\n\tcall void @g(ptr %p, ..., i32 1)\n\tret\n}\n";
	CIRModule			cModule;
	CIRFunction*		pcFunction;
	SIRInstruction*		psInstruction;
	SIRPhi*				psPhi;
	SIRType				sPointer;

	//A temporary with no type, and one that points into a space the module hasn't.
	TestIRParseRight(&cModule, szNeg);
	cModule.GetFunction(0)->AddTemporary(IRType(IRTK_None), "t");
	AssertIRVerifierSays("@f: Missing type.\n@f: Temporary has no type. [t]\n", &cModule);

	TestIRParseRight(&cModule, szNeg);
	sPointer = IRType(IRTK_Pointer);
	sPointer.uiSpace = 9;
	cModule.GetFunction(0)->AddTemporary(sPointer, "t");
	AssertIRVerifierSays("@f: Unknown address space.\n@f: Temporary has no type. [t]\n", &cModule);

	//A jump to another function's block.
	TestIRParseRight(&cModule, szTwo);
	cModule.GetFunction(0)->GetBlock(0)->msJump.pcThen = cModule.GetFunction(1)->GetBlock(0);
	AssertIRVerifierSays("@f b: Jump to a block not in the function.\n", &cModule);

	//Predecessors that aren't the blocks that jump there: one too many, and one too few.
	TestIRParseRight(&cModule, szTwo);
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(1)->mapcPredecessors.Add(pcFunction->GetBlock(1));
	AssertIRVerifierSays("@f c: Predecessor doesn't jump to the block once.\n", &cModule);

	TestIRParseRight(&cModule, szTwo);
	cModule.GetFunction(0)->GetBlock(1)->mapcPredecessors.ReInit();
	AssertIRVerifierSays("@f c: A block that jumps to the block is missing from its predecessors.\n", &cModule);

	//A temporary two instructions define, and one none does.
	TestIRParseRight(&cModule, "function void @f(i32 %a)\n{\nb:\n\t%x = neg i32 %a\n\t%y = neg i32 %a\n\tret\n}\n");
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(0)->masInstructions.Get(1)->sResult = pcFunction->GetBlock(0)->masInstructions.Get(0)->sResult;
	AssertIRVerifierSays("@f b: Temporary defined twice. [x]\n", &cModule);

	TestIRParseRight(&cModule, szUsed);
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(0)->masInstructions.Get(0)->sResult = pcFunction->AddTemporary(IRType(IRTK_I32), "other");
	AssertIRVerifierSays("@f b: Temporary is never defined. [x]\n", &cModule);

	//A result an opcode must have, one it can't have, and one of another type than the instruction's.
	TestIRParseRight(&cModule, szNeg);
	cModule.GetFunction(0)->GetBlock(0)->masInstructions.Get(0)->sResult = IRNone();
	AssertIRVerifierSays("@f b: Instruction has no result. [neg]\n", &cModule);

	TestIRParseRight(&cModule, szStore);
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(0)->masInstructions.Get(0)->sResult = pcFunction->AddTemporary(IRType(IRTK_I32), "r");
	AssertIRVerifierSays("@f b: Instruction can't have a result. [store]\n", &cModule);

	TestIRParseRight(&cModule, szNeg);
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(0)->masInstructions.Get(0)->sResult = pcFunction->AddTemporary(IRType(IRTK_I16), "narrow");
	AssertIRVerifierSays("@f b: Result has the wrong type. [narrow]\n", &cModule);

	//Phis: one in the entry block, one whose result is of another type, one with an argument from a block that
	//doesn't jump there, and one with two from the same block.
	TestIRParseRight(&cModule, szNeg);
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(0)->AddPhi(IRType(IRTK_I32), pcFunction->AddTemporary(IRType(IRTK_I32), "p"));
	AssertIRVerifierSays("@f b: The entry block has phis.\n", &cModule);

	TestIRParseRight(&cModule, szPhi);
	pcFunction = cModule.GetFunction(0);
	pcFunction->GetBlock(2)->masPhis.Get(0)->sResult = pcFunction->AddTemporary(IRType(IRTK_I16), "narrow");
	AssertIRVerifierSays("@f c: Result has the wrong type. [narrow]\n", &cModule);

	TestIRParseRight(&cModule, szPhi);
	pcFunction = cModule.GetFunction(0);
	psPhi = pcFunction->GetBlock(2)->masPhis.Get(0);
	psPhi->asArguments.Get(1)->pcBlock = pcFunction->GetBlock(2);
	AssertIRVerifierSays("@f c: Phi argument from a block that is not a predecessor. [x]\n", &cModule);

	TestIRParseRight(&cModule, szPhi);
	pcFunction = cModule.GetFunction(0);
	psPhi = pcFunction->GetBlock(2)->masPhis.Get(0);
	psPhi->asArguments.Get(1)->pcBlock = psPhi->asArguments.Get(0)->pcBlock;
	AssertIRVerifierSays("@f c: Phi has two arguments from one block. [x]\n", &cModule);

	//An opcode the module hasn't, and a variadic call with more named arguments than it has arguments.
	TestIRParseRight(&cModule, szNeg);
	cModule.GetFunction(0)->GetBlock(0)->masInstructions.Get(0)->uiOpcode = 60000;
	AssertIRVerifierSays("@f b: Unknown opcode.\n", &cModule);

	TestIRParseRight(&cModule, szCall);
	pcFunction = cModule.GetFunction(0);
	psInstruction = pcFunction->GetBlock(0)->masInstructions.Get(pcFunction->GetBlock(0)->masInstructions.NumElements() - 1);
	psInstruction->uiNamed = 5;
	AssertIRVerifierSays("@f b: More named arguments than arguments.\n", &cModule);
}


//////////////////////////////////////////////////////////////////////////
//
// An operand slot the opcode doesn't use is empty: a neg left with a
// second operand (by a pass that rewrote it) is an error, not printed and
// then lost.
//
//////////////////////////////////////////////////////////////////////////
void TestIRVerifierUnusedOperand(void)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRVerifier		cVerifier;
	CIRFunction*	pcFunction;
	SIRInstruction*	psNeg;

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse("function i32 @f(i32 %a)\n{\nb:\n\t%x = neg i32 %a\n\tret i32 %x\n}\n"));
	cParser.Kill();
	pcFunction = cModule.GetFunction(0);
	psNeg = pcFunction->GetBlock(0)->masInstructions.Get(0);
	psNeg->asArguments[1] = pcFunction->AddInteger(IRType(IRTK_I32), 3);

	cVerifier.Init(&cModule, &cErrors);
	AssertFalse(cVerifier.Verify());
	cVerifier.Kill();
	AssertString("@f b: Operand the opcode doesn't take. [neg]\n", cErrors.Text());
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// a -> b, c; b -> d; c -> d; d -> e, a (a loop); x is unreachable.
//
//////////////////////////////////////////////////////////////////////////
void TestIRDominators(void)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRDominators	cDominators;
	CIRFunction*	pcFunction;
	CIRBlock*		pcA;
	CIRBlock*		pcB;
	CIRBlock*		pcC;
	CIRBlock*		pcD;
	CIRBlock*		pcE;
	CIRBlock*		pcX;

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"function void @f(i1 %c)\n"
		"{\n"
		"s:\n\tjmp a\n"
		"a:\n\tbrif %c, b, c\n"
		"b:\n\tjmp d\n"
		"c:\n\tjmp d\n"
		"x:\n\tjmp d\n"
		"d:\n\tbrif %c, e, a\n"
		"e:\n\tret\n"
		"}\n"));
	cParser.Kill();
	AssertString("", cErrors.Text());

	pcFunction = cModule.GetFunction(0);
	pcA = pcFunction->FindBlock("a");
	pcB = pcFunction->FindBlock("b");
	pcC = pcFunction->FindBlock("c");
	pcD = pcFunction->FindBlock("d");
	pcE = pcFunction->FindBlock("e");
	pcX = pcFunction->FindBlock("x");

	cDominators.Init();
	cDominators.Compute(pcFunction);
	AssertSize(6, cDominators.NumReachable());
	AssertPointer(pcFunction->GetEntry(), cDominators.GetReversePostOrder(0));
	AssertPointer(pcFunction->GetEntry(), cDominators.GetImmediate(pcA));
	AssertPointer(pcA, cDominators.GetImmediate(pcB));
	AssertPointer(pcA, cDominators.GetImmediate(pcC));
	AssertPointer(pcA, cDominators.GetImmediate(pcD));
	AssertPointer(pcD, cDominators.GetImmediate(pcE));
	AssertNull(cDominators.GetImmediate(pcFunction->GetEntry()));
	AssertFalse(cDominators.IsReachable(pcX));
	AssertTrue(cDominators.Dominates(pcA, pcE));
	AssertTrue(cDominators.Dominates(pcD, pcD));
	AssertFalse(cDominators.Dominates(pcB, pcD));
	AssertFalse(cDominators.Dominates(pcA, pcX));
	cDominators.Kill();

	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// a heads a loop (latch d) with c a loop of its own inside it; x is
// unreachable.  %x is worked out before both loops, %y in the outer one.
//
//////////////////////////////////////////////////////////////////////////
void TestIRLoops(void)
{
	CIRModule			cModule;
	CCPPErrors			cErrors;
	CIRParser			cParser;
	CIRDominators		cDominators;
	CIRDefinitions		cDefinitions;
	CIRLoops			cLoops;
	CArrayIRLoopExits	asExits;
	CIRFunction*		pcFunction;
	CIRBlock*			pcS;
	CIRBlock*			pcA;
	CIRBlock*			pcB;
	CIRBlock*			pcC;
	CIRBlock*			pcD;
	CIRBlock*			pcE;
	CIRBlock*			pcX;
	SIRLoop*			psLoop;
	SIRReference		sX;
	SIRReference		sY;

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"function void @f(i1 %c, i32 %p)\n"
		"{\n"
		"s:\n\t%x = add i32 %p, 1\n\tjmp a\n"
		"a:\n\tbrif %c, b, e\n"
		"b:\n\t%y = add i32 %x, 1\n\tjmp c\n"
		"c:\n\t%z = add i32 %y, %x\n\tbrif %c, c, d\n"
		"x:\n\tjmp c\n"
		"d:\n\tbrif %c, a, e\n"
		"e:\n\tret\n"
		"}\n"));
	cParser.Kill();
	AssertString("", cErrors.Text());

	pcFunction = cModule.GetFunction(0);
	pcS = pcFunction->FindBlock("s");
	pcA = pcFunction->FindBlock("a");
	pcB = pcFunction->FindBlock("b");
	pcC = pcFunction->FindBlock("c");
	pcD = pcFunction->FindBlock("d");
	pcE = pcFunction->FindBlock("e");
	pcX = pcFunction->FindBlock("x");
	sX = pcFunction->FindTemporary("x");
	sY = pcFunction->FindTemporary("y");

	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	cDominators.Init();
	cDominators.Compute(pcFunction);
	cDefinitions.Init();
	cDefinitions.Compute(pcFunction);
	cLoops.Init();
	cLoops.Compute(pcFunction, &cDominators);

	AssertSize(2, cLoops.NumLoops());
	psLoop = cLoops.GetLoop(0);
	AssertPointer(pcA, psLoop->pcHead);
	AssertPointer(pcS, psLoop->pcEntering);
	AssertPointer(pcS, psLoop->pcPreheader);
	AssertInt(-1, psLoop->iParent);
	AssertInt(1, psLoop->iDepth);
	AssertSize(4, cLoops.NumBlocks(0));
	AssertPointer(pcA, cLoops.GetBlock(0, 0));
	AssertSize(1, cLoops.NumLatches(0));
	AssertPointer(pcD, cLoops.GetLatch(0, 0));

	psLoop = cLoops.GetLoop(1);
	AssertPointer(pcC, psLoop->pcHead);
	AssertPointer(pcB, psLoop->pcEntering);
	AssertPointer(pcB, psLoop->pcPreheader);
	AssertInt(0, psLoop->iParent);
	AssertInt(2, psLoop->iDepth);
	AssertSize(1, cLoops.NumBlocks(1));
	AssertPointer(pcC, cLoops.GetLatch(1, 0));

	AssertTrue(cLoops.Contains(0, pcC));
	AssertTrue(cLoops.Contains(0, pcD));
	AssertFalse(cLoops.Contains(1, pcB));
	AssertFalse(cLoops.Contains(0, pcE));
	AssertFalse(cLoops.Contains(0, pcX));
	AssertInt(2, cLoops.Depth(pcC));
	AssertInt(1, cLoops.Depth(pcB));
	AssertInt(0, cLoops.Depth(pcE));
	AssertInt(-1, cLoops.Innermost(pcX));
	AssertTrue(cLoops.IsHead(pcA));
	AssertFalse(cLoops.IsHead(pcB));

	asExits.Init();
	cLoops.GetExits(0, &asExits);
	AssertSize(2, asExits.NumElements());
	AssertPointer(pcA, asExits.Get(0)->pcFrom);
	AssertPointer(pcE, asExits.Get(0)->pcTo);
	AssertPointer(pcD, asExits.Get(1)->pcFrom);
	AssertPointer(pcE, asExits.Get(1)->pcTo);
	cLoops.GetExits(1, &asExits);
	AssertSize(1, asExits.NumElements());
	AssertPointer(pcD, asExits.Get(0)->pcTo);
	asExits.Kill();

	AssertTrue(cLoops.IsInvariant(0, sX, &cDefinitions));
	AssertFalse(cLoops.IsInvariant(0, sY, &cDefinitions));
	AssertTrue(cLoops.IsInvariant(1, sY, &cDefinitions));
	AssertTrue(cLoops.IsInvariant(1, *pcFunction->masParameters.Get(1), &cDefinitions));
	AssertPointer(pcB, cDefinitions.Block(sY));
	AssertTrue(cDefinitions.IsDefinedBy(sY, IRO_Add));
	AssertNotNull(cDefinitions.DefinedBy(sY, IRO_Add));
	AssertNull(cDefinitions.DefinedBy(sY, IRO_Sub));
	AssertFalse(cDefinitions.IsDefinedBy(sY, IRO_Sub));
	AssertInt(2, cDefinitions.NumUses(sX));
	AssertInt(1, cDefinitions.NumUses(sY));

	cLoops.Kill();
	cDefinitions.Kill();
	cDominators.Kill();
	cErrors.Kill();
	cModule.Kill();
}


#define IR_TEST_INDEXED_ADDRESSES	0x01	//SetIndexedAddresses (the 65C64's [R1 + R2]).
#define IR_TEST_LIBRARY				0x02	//SetLibraryInt(i16): loops made memset and memmove calls.
#define IR_TEST_NARROW_MULTIPLIES	0x04	//NarrowMultiplies is in the pipeline (the W65C816's).
#define IR_TEST_NARROW_COUNTERS		0x08	//NarrowCounters is (the W65C816's).
#define IR_TEST_BIASED_INDICES		0x10	//MergeAddressPhis and BiasSignedIndices are (the W65C816's [p],Y).
#define IR_TEST_CONSTANT_DIVISIONS	0x20	//ConstantDivisions is (the W65C816's, whose divides are calls).
#define IR_TEST_DIVIDE_REMAINDERS	0x40	//SetDivideRemainders of the 65C64's divides (unsigned, 8 to 32 bits).
#define IR_TEST_COUNTER_DIVISIONS	0x80	//ReduceDivisions is (the W65C816's).
#define IR_TEST_DIVISION_COSTS		0x100	//SetDivisionCosts of the W65C816's: a reciprocal only where it costs less than the divide.


//////////////////////////////////////////////////////////////////////////
//
// szText, optimised with the IR_TEST_ options in uiOptions, must verify and
// print as szExpected.
//
//////////////////////////////////////////////////////////////////////////
void AssertIROptimisedOptions(const char* szExpected, const char* szText, uint32 uiOptions)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRVerifier		cVerifier;
	CIROptimiser	cOptimiser;
	CIRPrinter		cPrinter;
	CChars			szPrinted;
	size			ui;
	size			uj;
	size			uk;
	CIRBlock*		pcBlock;
	bool			bSources;
	CIRW65C816DivisionCosts	cDivisionCosts;

	//Every instruction parsed is from line 7: the ones optimising makes must have a line too (7, or 0 for none).
	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	cModule.muiSource = 7;
	AssertTrue(cParser.Parse(szText));
	cModule.muiSource = 0;
	cParser.Kill();
	cOptimiser.Init(&cModule);
	cOptimiser.SetIndexedAddresses((uiOptions & IR_TEST_INDEXED_ADDRESSES) != 0);

	//The standard pipeline, without the passes a target chooses that the test doesn't ask for.
	if (!(uiOptions & IR_TEST_NARROW_MULTIPLIES))
	{
		cOptimiser.GetPipeline()->Remove(IRP_NarrowMultiplies);
	}
	if (!(uiOptions & IR_TEST_NARROW_COUNTERS))
	{
		cOptimiser.GetPipeline()->Remove(IRP_NarrowCounters);
	}
	if (!(uiOptions & IR_TEST_COUNTER_DIVISIONS))
	{
		cOptimiser.GetPipeline()->Remove(IRP_ReduceDivisions);
	}
	if (!(uiOptions & IR_TEST_CONSTANT_DIVISIONS))
	{
		cOptimiser.GetPipeline()->Remove(IRP_ConstantDivisions);
	}
	if (!(uiOptions & IR_TEST_BIASED_INDICES))
	{
		cOptimiser.GetPipeline()->Remove(IRP_MergeAddressPhis);
		cOptimiser.GetPipeline()->Remove(IRP_BiasSignedIndices);
	}
	if (!(uiOptions & IR_TEST_CONSTANT_DIVISIONS))
	{
		cOptimiser.GetPipeline()->Remove(IRP_DivisibilityTests);
		cOptimiser.GetPipeline()->Remove(IRP_ConstantDivisions);
		if (!(uiOptions & IR_TEST_DIVIDE_REMAINDERS))
		{
			cOptimiser.GetPipeline()->Remove(IRP_ShareDivisions);
		}
	}
	if (uiOptions & IR_TEST_DIVIDE_REMAINDERS)
	{
		cOptimiser.SetDivideRemainders(IR_DIVIDE_REMAINDERS_8 | IR_DIVIDE_REMAINDERS_16 | IR_DIVIDE_REMAINDERS_32);
	}
	if (uiOptions & IR_TEST_LIBRARY)
	{
		cOptimiser.SetLibraryInt(IRType(IRTK_I16));
	}
	if (uiOptions & IR_TEST_DIVISION_COSTS)
	{
		cOptimiser.SetDivisionCosts(&cDivisionCosts);
	}
	for (ui = 0; ui < cModule.NumFunctions(); ui++)
	{
		cOptimiser.Optimise(cModule.GetFunction(ui));
	}
	cOptimiser.Kill();
	bSources = true;
	for (ui = 0; ui < cModule.NumFunctions(); ui++)
	{
		for (uj = 0; uj < cModule.GetFunction(ui)->NumBlocks(); uj++)
		{
			pcBlock = cModule.GetFunction(ui)->GetBlock(uj);
			for (uk = 0; uk < pcBlock->masInstructions.NumElements(); uk++)
			{
				bSources &= (pcBlock->masInstructions.Get(uk)->uiSource == 0) || (pcBlock->masInstructions.Get(uk)->uiSource == 7);
			}
		}
	}
	AssertTrue(bSources);
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	cVerifier.Kill();
	AssertString("", cErrors.Text());
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	AssertString(szExpected, szPrinted.Text());
	szPrinted.Kill();
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szText, optimised (for a target whose loads and stores add an index for
// nothing, unless bIndexedAddresses is false; with loops made memset and
// memmove calls, a 16 bit int, if bLibrary; with multiplies made narrow if
// bNarrowMultiplies), must verify and print as szExpected.
//
//////////////////////////////////////////////////////////////////////////
void AssertIROptimised(const char* szExpected, const char* szText, bool bIndexedAddresses = true, bool bLibrary = false, bool bNarrowMultiplies = false)
{
	uint32	uiOptions;

	uiOptions = 0;
	uiOptions |= bIndexedAddresses ? IR_TEST_INDEXED_ADDRESSES : 0;
	uiOptions |= bLibrary ? IR_TEST_LIBRARY : 0;
	uiOptions |= bNarrowMultiplies ? IR_TEST_NARROW_MULTIPLIES : 0;
	AssertIROptimisedOptions(szExpected, szText, uiOptions);
}


//////////////////////////////////////////////////////////////////////////
//
// The default space is what a text with no spaces gets, the size the target
// says when a front end adds it, and both targets' pointers are its size.
//
//////////////////////////////////////////////////////////////////////////
void TestIRDefaultSpace(void)
{
	CIRModule	cModule;

	cModule.Init();
	AssertInt(0, cModule.AddDefaultSpace());
	AssertInt(IR_DEFAULT_POINTER_BYTES, cModule.GetPointerSize(0));
	AssertInt(0, cModule.FindSpace("default"));
	cModule.Kill();

	cModule.Init();
	AssertInt(0, cModule.AddDefaultSpace(8));
	AssertInt(8, cModule.GetPointerSize(0));
	cModule.Kill();

	AssertInt(IR_DEFAULT_POINTER_BYTES, GetCPPTarget(CPPT_65C64)->GetPointerBytes());
	AssertInt(IR_DEFAULT_POINTER_BYTES, GetCPPTarget(CPPT_W65C816)->GetPointerBytes());
}


//////////////////////////////////////////////////////////////////////////
//
// Each integer comparison has an inverse and a swapped form that are
// comparisons again (inverted twice is itself); anything else has neither.
//
//////////////////////////////////////////////////////////////////////////
void TestIRComparisonOpcodes(void)
{
	uint16	uiOpcode;

	for (uiOpcode = IRO_Eq; uiOpcode <= IRO_UGe; uiOpcode++)
	{
		AssertTrue(IsIRIntegerComparison(IRInvertedComparison(uiOpcode)));
		AssertInt(uiOpcode, IRInvertedComparison(IRInvertedComparison(uiOpcode)));
		AssertTrue(IsIRIntegerComparison(IRSwappedComparison(uiOpcode)));
		AssertInt(uiOpcode, IRSwappedComparison(IRSwappedComparison(uiOpcode)));
	}
	AssertInt(IRO_ULt, IRInvertedComparison(IRO_UGe));
	AssertInt(IRO_SGe, IRInvertedComparison(IRO_SLt));
	AssertInt(IRO_ULe, IRSwappedComparison(IRO_UGe));
	AssertInt(IRO_None, IRInvertedComparison(IRO_Xor));
	AssertInt(IRO_None, IRInvertedComparison(IRO_And));
	AssertInt(IRO_None, IRInvertedComparison(IRO_None));
	AssertInt(IRO_None, IRInvertedComparison(IRO_NumGeneric));
	AssertInt(IRO_FLt, IRSwappedComparison(IRO_FLt));
	AssertInt(IRO_NumGeneric, IRSwappedComparison(IRO_NumGeneric));
}


//////////////////////////////////////////////////////////////////////////
//
// The comparisons' columns agree with each other and say what each opcode
// does: only eq to uge have them, the inverse holds exactly when the
// comparison doesn't and the swapped form holds of the swapped operands.
//
//////////////////////////////////////////////////////////////////////////
void TestIRComparisonColumns(void)
{
	uint16		uiOpcode;
	SIRType		sType;
	size		uiLeft;
	size		uiRight;
	uint64		aulliValues[] = { 0, 1, 2, 0x7F, 0x80, 0xFF };
	size		uiNumValues;
	bool		bHolds;

	uiNumValues = sizeof(aulliValues) / sizeof(aulliValues[0]);
	sType = IRType(IRTK_I8);
	for (uiOpcode = IRO_None; uiOpcode < IRO_NumGeneric; uiOpcode++)
	{
		AssertBool((uiOpcode >= IRO_Eq) && (uiOpcode <= IRO_UGe), IsIRIntegerComparison(uiOpcode));
		AssertBool((uiOpcode >= IRO_SLt) && (uiOpcode <= IRO_SGe), IsIRSignedComparison(uiOpcode));
		AssertBool((uiOpcode == IRO_SLe) || (uiOpcode == IRO_SGt) || (uiOpcode == IRO_ULe) || (uiOpcode == IRO_UGt), IsIRSwappedComparison(uiOpcode));
		if (IsIRIntegerComparison(uiOpcode))
		{
			for (uiLeft = 0; uiLeft < uiNumValues; uiLeft++)
			{
				for (uiRight = 0; uiRight < uiNumValues; uiRight++)
				{
					bHolds = IRCompareIntegers(uiOpcode, sType, aulliValues[uiLeft], aulliValues[uiRight]);
					AssertBool(!bHolds, IRCompareIntegers(IRInvertedComparison(uiOpcode), sType, aulliValues[uiLeft], aulliValues[uiRight]));
					AssertBool(bHolds, IRCompareIntegers(IRSwappedComparison(uiOpcode), sType, aulliValues[uiRight], aulliValues[uiLeft]));
				}
			}
		}
	}

	//0x80 is -128 signed and 128 unsigned, and bits past the type's width don't count.
	AssertTrue(IRCompareIntegers(IRO_SLt, sType, 0x80, 0x01));
	AssertFalse(IRCompareIntegers(IRO_ULt, sType, 0x80, 0x01));
	AssertTrue(IRCompareIntegers(IRO_Eq, sType, 0x101, 0x01));
	AssertTrue(IRCompareIntegers(IRO_Ne, sType, 0x80, 0x7F));
	AssertTrue(IRCompareIntegers(IRO_SGe, sType, 0x7F, 0x7F));
	AssertFalse(IRCompareIntegers(IRO_UGt, sType, 0x7F, 0x7F));
	AssertTrue(IRCompareIntegers(IRO_ULe, sType, 0x7F, 0xFF));
	AssertTrue(IRCompareIntegers(IRO_SGt, sType, 0x7F, 0xFF));

	//Only eq and ne of them work either way round.
	AssertTrue(IsIRCommutative(IRO_Eq));
	AssertFalse(IsIRCommutative(IRO_SLt));
	AssertFalse(IsIRPure(IRO_NumGeneric));
	AssertFalse(IsIRMayTrap(IRO_NumGeneric));
	AssertFalse(IsIRCommutative(IRO_NumGeneric));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether uiOpcode is one of pauiOpcodes, which IRO_None ends.
//
//////////////////////////////////////////////////////////////////////////
static bool TestIROpcodeIn(uint16 uiOpcode, const uint16* pauiOpcodes)
{
	size	ui;

	for (ui = 0; pauiOpcodes[ui] != IRO_None; ui++)
	{
		if (pauiOpcodes[ui] == uiOpcode)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The passes' one table (IR_PASSES): each pass's name is its enumerator's,
// each says what it does, and the optimiser has a pass of that name for
// each.  A pipeline says which of them are run: the standard one runs
// ConstantDivisions, which is how ShareDivisions knows the target's divides
// are calls, until a target takes it out; and a pass that only follows
// another is not one the pipeline has.
//
//////////////////////////////////////////////////////////////////////////
void TestIRPassTable(void)
{
	CIRPipeline	cPipeline;
	int			iPass;
	size		uiLength;

	AssertString("SimplifyCFG", CIRPipeline::GetPassName(IRP_SimplifyCFG));
	AssertString("ConstantDivisions", CIRPipeline::GetPassName(IRP_ConstantDivisions));
	AssertString("ReduceDivisions", CIRPipeline::GetPassName(IRP_ReduceDivisions));
	AssertInt(IRP_NumPasses - 1, IRP_ReduceDivisions);
	for (iPass = 0; iPass < IRP_NumPasses; iPass++)
	{
		AssertTrue(CIRPipeline::GetPassName((EIRPass)iPass)[0] != '\0');
		uiLength = strlen(CIRPipeline::GetPassDescription((EIRPass)iPass));
		AssertTrue(uiLength > 20);
		AssertTrue(CIRPipeline::GetPassDescription((EIRPass)iPass)[uiLength - 1] == '.');
	}

	cPipeline.Init();
	cPipeline.SetStandard();
	for (iPass = 0; iPass < IRP_NumPasses; iPass++)
	{
		AssertTrue(cPipeline.Has((EIRPass)iPass));
	}
	cPipeline.Remove(IRP_ConstantDivisions);
	AssertFalse(cPipeline.Has(IRP_ConstantDivisions));
	AssertTrue(cPipeline.Has(IRP_ShareDivisions));

	//NarrowMultiplies runs after the rounds, and FoldConstants follows it: without the rounds' FoldConstants the
	//pipeline no longer has that pass, though it is still a step.
	cPipeline.Init();
	cPipeline.AddAfter(IRP_NarrowMultiplies);
	cPipeline.AddFollowing(IRP_FoldConstants);
	AssertTrue(cPipeline.Has(IRP_NarrowMultiplies));
	AssertFalse(cPipeline.Has(IRP_FoldConstants));
	AssertInt(2, cPipeline.NumSteps());
}


//////////////////////////////////////////////////////////////////////////
//
// What a branch says on each of its ways out (SIREdgeCondition): the
// comparison as written on the way taken when it is true and inverted on
// the other; written with a given value on the left, swapped if it has to
// be; and unswapped, so that a > b and b < a are the same.  Nothing is
// said by a jump that is no branch, on a way the branch doesn't go, or by
// a branch on what is no integer comparison.
//
//////////////////////////////////////////////////////////////////////////
void TestIREdgeCondition(void)
{
	CIRModule			cModule;
	CIRFunction*		pcFunction;
	CIRBlock*			pcEntry;
	CIRBlock*			pcThen;
	CIRBlock*			pcElse;
	SIRInstruction*		psCompare;
	SIRInstruction*		psAdd;
	SIREdgeCondition	sCondition;
	SIRReference		sA;
	SIRReference		sB;

	TestIRParseRight(&cModule, "function i32 @f(i32 %a, i32 %b)\n{\ne:\n\t%s = add i32 %a, %b\n\t%c = sgt i32 %a, %b\n\tbrif %c, t, u\nt:\n\tret i32 %s\nu:\n\tret i32 %a\n}\n");
	pcFunction = cModule.GetFunction(0);
	pcEntry = pcFunction->GetBlock(0);
	pcThen = pcFunction->GetBlock(1);
	pcElse = pcFunction->GetBlock(2);
	psAdd = pcEntry->masInstructions.Get(0);
	psCompare = pcEntry->masInstructions.Get(1);
	sA = psCompare->asArguments[0];
	sB = psCompare->asArguments[1];

	//Going to t, a > b; going to u, a <= b.
	AssertTrue(sCondition.Init(&pcEntry->msJump, pcThen, psCompare));
	AssertInt(IRO_SGt, sCondition.uiOpcode);
	AssertTrue(sCondition.sLeft.Equals(sA));
	AssertTrue(sCondition.sRight.Equals(sB));
	AssertTrue(sCondition.psCompare == psCompare);
	AssertTrue(sCondition.Init(&pcEntry->msJump, pcElse, psCompare));
	AssertInt(IRO_SLe, sCondition.uiOpcode);
	AssertTrue(sCondition.sLeft.Equals(sA));

	//Of b it says b >= a there, and of what it doesn't compare, nothing.
	AssertTrue(sCondition.PutLeft(sA));
	AssertInt(IRO_SLe, sCondition.uiOpcode);
	AssertTrue(sCondition.PutLeft(sB));
	AssertInt(IRO_SGe, sCondition.uiOpcode);
	AssertTrue(sCondition.sLeft.Equals(sB));
	AssertTrue(sCondition.sRight.Equals(sA));
	AssertFalse(sCondition.PutLeft(psAdd->sResult));

	//a > b is written b < a; b >= a is left as it is.
	sCondition.Unswap();
	AssertInt(IRO_SGe, sCondition.uiOpcode);
	AssertTrue(sCondition.sLeft.Equals(sB));
	AssertTrue(sCondition.Init(&pcEntry->msJump, pcThen, psCompare));
	sCondition.Unswap();
	AssertInt(IRO_SLt, sCondition.uiOpcode);
	AssertTrue(sCondition.sLeft.Equals(sB));
	AssertTrue(sCondition.sRight.Equals(sA));

	//Not a way the branch goes; not a branch; no instruction; not a comparison.
	AssertFalse(sCondition.Init(&pcEntry->msJump, pcEntry, psCompare));
	AssertFalse(sCondition.Init(&pcThen->msJump, pcElse, psCompare));
	AssertFalse(sCondition.Init(&pcEntry->msJump, pcThen, NULL));
	AssertFalse(sCondition.Init(&pcEntry->msJump, pcThen, psAdd));
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol's attributes are one set (IRSA_): each is given, asked and taken
// away on its own, the restrict symbols are counted for HasRestrict, the
// written ones are printed a line each with all of one attribute before
// the next, and what goes with a symbol into another module is
// IRSA_Carried and no more (not local, and not nowrites).
//
//////////////////////////////////////////////////////////////////////////
void TestIRSymbolAttributes(void)
{
	CIRModule	cModule;
	CIRModule	cOther;
	CIRPrinter	cPrinter;
	CChars		szPrinted;
	uint32		uiA;
	uint32		uiB;
	uint32		uiC;
	uint32		uiOther;

	cModule.Init();
	uiA = cModule.AddSymbol("a");
	uiB = cModule.AddSymbol("b");
	uiC = cModule.AddSymbol("c");
	AssertInt(0, cModule.GetSymbolAttributes(uiA));
	AssertInt(0, cModule.GetSymbolAttributes(uiC + 9));
	AssertFalse(cModule.HasRestrict());

	//Given twice and taken away once it is gone: HasRestrict counts symbols, not calls.
	cModule.SetRestrict(uiA);
	cModule.SetRestrict(uiA);
	AssertTrue(cModule.IsRestrict(uiA));
	AssertTrue(cModule.HasRestrict());
	cModule.SetSymbolAttributes(uiA, IRSA_Restrict, false);
	AssertFalse(cModule.IsRestrict(uiA));
	AssertFalse(cModule.HasRestrict());

	cModule.SetRestrict(uiA);
	cModule.SetLocal(uiA);
	cModule.SetRegisterCall(uiA);
	cModule.SetNoWrites(uiA, true);
	cModule.SetNoThrow(uiA, true);
	AssertInt(IRSA_Restrict | IRSA_Local | IRSA_RegisterCall | IRSA_NoWrites | IRSA_NoThrow, cModule.GetSymbolAttributes(uiA));
	AssertTrue(cModule.IsLocal(uiA));
	AssertTrue(cModule.IsRegisterCall(uiA));
	AssertTrue(cModule.IsNoWrites(uiA));
	AssertTrue(cModule.IsNoThrow(uiA));
	AssertFalse(cModule.IsNoThrow(uiB));
	cModule.SetNoWrites(uiA, false);
	AssertFalse(cModule.IsNoWrites(uiA));
	AssertTrue(cModule.IsNoThrow(uiA));
	cModule.SetNoWrites(uiA, true);

	//Into another module go the three that are true of the symbol wherever it is named.
	cOther.Init();
	uiOther = cOther.AddSymbol("a");
	cOther.CopySymbolAttributes(uiOther, &cModule, uiA, IRSA_Carried);
	AssertInt(IRSA_Restrict | IRSA_RegisterCall | IRSA_NoThrow, cOther.GetSymbolAttributes(uiOther));
	AssertTrue(cOther.HasRestrict());
	cOther.CopySymbolAttributes(uiOther, &cModule, uiB, IRSA_Carried);
	AssertInt(IRSA_Restrict | IRSA_RegisterCall | IRSA_NoThrow, cOther.GetSymbolAttributes(uiOther));
	cOther.Kill();

	//Printed in the table's order (restrict, regcall, nowrites, nothrow), every symbol that has one before the next.
	cModule.SetNoThrow(uiB, true);
	cModule.SetRegisterCall(uiB);
	cModule.SetRestrict(uiC);
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	AssertString("restrict @a\nrestrict @c\nregcall @a\nregcall @b\nnowrites @a\nnothrow @a\nnothrow @b\n", szPrinted.Text());
	szPrinted.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The opcode table's classes and twins, which the passes ask where each
// kept a list of its own: every class is the opcodes written out here and
// no other, and a target's opcodes (from IRO_NumGeneric) are in none.
//
//////////////////////////////////////////////////////////////////////////
void TestIROpcodeClasses(void)
{
	static const uint16	auiArithmetic[] = { IRO_Add, IRO_Sub, IRO_Mul, IRO_And, IRO_Or, IRO_Xor, IRO_Shl, IRO_LShr, IRO_AShr, IRO_Neg, IRO_Not, IRO_None };
	static const uint16	auiLowBits[] = { IRO_Add, IRO_Sub, IRO_Mul, IRO_And, IRO_Or, IRO_Xor, IRO_None };
	static const uint16	auiWrapping[] = { IRO_Add, IRO_Sub, IRO_Mul, IRO_Neg, IRO_Shl, IRO_None };
	static const uint16	auiConversions[] = { IRO_SExt, IRO_ZExt, IRO_Trunc, IRO_None };
	static const uint16	auiSigned[] = { IRO_SLt, IRO_SLe, IRO_SGt, IRO_SGe, IRO_SDiv, IRO_SRem, IRO_None };
	static const uint16	auiUnsigned[] = { IRO_ULt, IRO_ULe, IRO_UGt, IRO_UGe, IRO_UDiv, IRO_URem, IRO_None };
	uint16				uiOpcode;
	uint8				uiHolds;
	SIRType				sType;

	for (uiOpcode = IRO_None; uiOpcode <= IRO_NumGeneric; uiOpcode++)
	{
		AssertBool(TestIROpcodeIn(uiOpcode, auiArithmetic), IsIRIntegerArithmetic(uiOpcode));
		AssertBool(TestIROpcodeIn(uiOpcode, auiLowBits), IsIRLowBits(uiOpcode));
		AssertBool(TestIROpcodeIn(uiOpcode, auiWrapping), IsIRSignedWrapping(uiOpcode));
		AssertBool(TestIROpcodeIn(uiOpcode, auiConversions), IsIRIntegerConversion(uiOpcode));
		AssertBool((uiOpcode >= IRO_SDiv) && (uiOpcode <= IRO_URem), IsIRDivision(uiOpcode));
		AssertBool((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_SRem), IsIRSignedDivision(uiOpcode));
		AssertBool((uiOpcode == IRO_UDiv) || (uiOpcode == IRO_URem), IsIRUnsignedDivision(uiOpcode));
		AssertBool((uiOpcode == IRO_SRem) || (uiOpcode == IRO_URem), IsIRRemainder(uiOpcode));
		AssertBool((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv), IsIRQuotient(uiOpcode));
		AssertBool(((uiOpcode >= IRO_Eq) && (uiOpcode <= IRO_UGe)) || ((uiOpcode >= IRO_FEq) && (uiOpcode <= IRO_FUnordered)), IRResultIsI1(uiOpcode));
		AssertInt(TestIROpcodeIn(uiOpcode, auiSigned) ? IROP_Signed : (TestIROpcodeIn(uiOpcode, auiUnsigned) ? IROP_Unsigned : 0), IRSignedness(uiOpcode));

		//Only a signed comparison and a signed division have an unsigned twin, and only a division a partner.
		if (!TestIROpcodeIn(uiOpcode, auiSigned))
		{
			AssertInt(uiOpcode, IRUnsignedOpcode(uiOpcode));
		}
		if (!IsIRDivision(uiOpcode))
		{
			AssertInt(IRO_None, IRDivisionPartner(uiOpcode));
		}
		if (!IsIRIntegerComparison(uiOpcode))
		{
			AssertInt(0, IRComparisonHolds(uiOpcode));
		}
	}

	AssertInt(IRO_ULt, IRUnsignedOpcode(IRO_SLt));
	AssertInt(IRO_ULe, IRUnsignedOpcode(IRO_SLe));
	AssertInt(IRO_UGt, IRUnsignedOpcode(IRO_SGt));
	AssertInt(IRO_UGe, IRUnsignedOpcode(IRO_SGe));
	AssertInt(IRO_UDiv, IRUnsignedOpcode(IRO_SDiv));
	AssertInt(IRO_URem, IRUnsignedOpcode(IRO_SRem));

	AssertInt(IRO_SRem, IRDivisionPartner(IRO_SDiv));
	AssertInt(IRO_SDiv, IRDivisionPartner(IRO_SRem));
	AssertInt(IRO_URem, IRDivisionPartner(IRO_UDiv));
	AssertInt(IRO_UDiv, IRDivisionPartner(IRO_URem));
	AssertInt(IRO_SDiv, IRDivisionOpcode(true, false));
	AssertInt(IRO_SRem, IRDivisionOpcode(true, true));
	AssertInt(IRO_UDiv, IRDivisionOpcode(false, false));
	AssertInt(IRO_URem, IRDivisionOpcode(false, true));

	//What a comparison holds for is what it says of 1 against 2, 1 against 1 and 2 against 1, and swapping its
	//operands swaps less and greater.
	sType = IRType(IRTK_I8);
	for (uiOpcode = IRO_Eq; uiOpcode <= IRO_UGe; uiOpcode++)
	{
		uiHolds = IRComparisonHolds(uiOpcode);
		AssertBool(IRCompareIntegers(uiOpcode, sType, 1, 2), (uiHolds & IRCH_Less) != 0);
		AssertBool(IRCompareIntegers(uiOpcode, sType, 1, 1), (uiHolds & IRCH_Equal) != 0);
		AssertBool(IRCompareIntegers(uiOpcode, sType, 2, 1), (uiHolds & IRCH_Greater) != 0);
		AssertBool((uiHolds & IRCH_Less) != 0, (IRComparisonHolds(IRSwappedComparison(uiOpcode)) & IRCH_Greater) != 0);
		AssertBool((uiHolds & IRCH_Equal) != 0, (IRComparisonHolds(IRSwappedComparison(uiOpcode)) & IRCH_Equal) != 0);
		AssertInt(IRCH_All & ~uiHolds, IRComparisonHolds(IRInvertedComparison(uiOpcode)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A division is hoisted out of a loop only by a constant that can't trap:
// not 0, and not -1 when it is signed (an unsigned divide by all ones is
// fine).  The divides are the opcodes that may trap (IsIRMayTrap).
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserHoistableDivisions(void)
{
	static const char*	aszDivides[] = { "udiv i32 %a, 4294967295", "urem i32 %a, 4294967295", "udiv i32 %a, 7", "sdiv i32 %a, 7",
										 "sdiv i32 %a, -1", "srem i32 %a, -1", "udiv i32 %a, 0", "udiv i32 %a, %a", NULL };
	static const bool	abHoistable[] = { true, true, true, true, false, false, false, false };
	CIRModule			cModule;
	CCPPErrors			cErrors;
	CIRParser			cParser;
	CChars				szText;
	CIRFunction*		pcFunction;
	size				i;

	for (i = 0; aszDivides[i]; i++)
	{
		cModule.Init();
		cErrors.Init();
		cParser.Init(&cModule, &cErrors);
		szText.Init("function i32 @f(i32 %a)\n{\nb:\n\t%d = ");
		szText.Append(aszDivides[i]);
		szText.Append("\n\tret i32 %d\n}\n");
		AssertTrue(cParser.Parse(szText.Text()));
		cParser.Kill();
		pcFunction = cModule.GetFunction(0);
		AssertBool(abHoistable[i], IROptimiserHoistable(pcFunction, pcFunction->GetBlock(0)->masInstructions.Get(0)));
		szText.Kill();
		cErrors.Kill();
		cModule.Kill();
	}
	AssertTrue(IsIRMayTrap(IRO_UDiv));
	AssertTrue(IsIRMayTrap(IRO_SRem));
	AssertFalse(IsIRMayTrap(IRO_Mul));
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer constant folded from an integer keeps only its space's width:
// 0x10000 as a 2 byte pointer is 0, so it equals one made from 0.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserPointerConstantWidth(void)
{
	AssertIROptimised(
		"space near 2\n\nfunction i1 @_Z__Fv()\n{\nentry:\n\tret i1 1\n}\n",
		"space near 2\n\n"
		"function i1 @_Z__Fv()\n{\nentry:\n\t%p = itoptr i32 65536 to ptr.near\n\t%q = itoptr i32 0 to ptr.near\n"
		"\t%c = eq ptr.near %p, %q\n\tret i1 %c\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A constant goes to the right of a commutative operation (256 * a is a shift like a * 256); a mul of a value the
// branches before it keep from 0 to 199 by 320 fits 16 bits (NarrowMultiplies, the W65C816's), and one that may be
// negative or too big doesn't; a sext of a value the branches keep 0 or more is a zext.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserConstantsRight(void)
{
	static const char*	szNarrow =
		"function i32 @_P__Fs(i16 %y)\n{\nentry:\n\t%n = slt i16 %y, 0\n\tbrif %n, out, check\n"
		"check:\n\t%b = sge i16 %y, LIMIT\n\tbrif %b, out, in\n"
		"in:\n\t%w = sext i16 %y to i32\n\t%m = mul i32 %w, 320\n\tret i32 %m\n"
		"out:\n\tret i32 0\n}\n";
	CChars				szText;

	AssertIROptimised(
		"function i32 @_M__Fi(i32 %a)\n{\nb:\n\t%m = shl i32 %a, 8\n\t%s = add i32 %m, 3\n\tret i32 %s\n}\n",
		"function i32 @_M__Fi(i32 %a)\n{\nb:\n\t%m = mul i32 256, %a\n\t%s = add i32 3, %m\n\tret i32 %s\n}\n");

	//0 to 204: 204 * 320 = 65280 fits.
	szText.Init(szNarrow);
	szText.Replace("LIMIT", "205");
	AssertIROptimised(
		"function i32 @_P__Fs(i16 %y)\n{\nentry:\n\t%n = slt i16 %y, 0\n\tbrif %n, out, check\n"
		"check:\n\t%b = sge i16 %y, 205\n\tbrif %b, out, in\n"
		"in:\n\t%0 = mul i16 %y, 320\n\t%m = zext i16 %0 to i32\n\tret i32 %m\n"
		"out:\n\tret i32 0\n}\n", szText.Text(), true, false, true);
	szText.Kill();

	//0 to 205: 205 * 320 = 65600 doesn't (the sext is a zext, the value not negative).
	szText.Init(szNarrow);
	szText.Replace("LIMIT", "206");
	AssertIROptimised(
		"function i32 @_P__Fs(i16 %y)\n{\nentry:\n\t%n = slt i16 %y, 0\n\tbrif %n, out, check\n"
		"check:\n\t%b = sge i16 %y, 206\n\tbrif %b, out, in\n"
		"in:\n\t%w = zext i16 %y to i32\n\t%m = mul i32 %w, 320\n\tret i32 %m\n"
		"out:\n\tret i32 0\n}\n", szText.Text(), true, false, true);
	szText.Kill();

	//May be negative.
	AssertIROptimised(
		"function i32 @_P__Fs(i16 %y)\n{\nentry:\n\t%b = sge i16 %y, 200\n\tbrif %b, out, in\n"
		"in:\n\t%w = sext i16 %y to i32\n\t%m = mul i32 %w, 320\n\tret i32 %m\n"
		"out:\n\tret i32 0\n}\n",
		"function i32 @_P__Fs(i16 %y)\n{\nentry:\n\t%b = sge i16 %y, 200\n\tbrif %b, out, in\n"
		"in:\n\t%w = sext i16 %y to i32\n\t%m = mul i32 %w, 320\n\tret i32 %m\n"
		"out:\n\tret i32 0\n}\n", true, false, true);

}


//////////////////////////////////////////////////////////////////////////
//
// CIRModule::CopyFunction: the copy prints as the function does but for its
// name, and is its own: blocks, phis and sources are the copy's, it stays
// when the function goes, and it verifies.  It is local and takes register
// arguments as the function does.  A second copy of that name is the first.
//
//////////////////////////////////////////////////////////////////////////
void TestIRCopyFunction(void)
{
	static const char*	szText =
		"space default 4\n"
		"\n"
		"function i32 @_Sum(ptr restrict %p, i32 %n), inline\n"
		"{\n"
		"entry:\n"
		"\t%s = slot ptr 4, 4\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [body, %next]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, body, done\n"
		"body:\n"
		"\tstore i32 %i, %s\n"
		"\t%next = add i32 %i, 1\n"
		"\tjmp loop\n"
		"done:\n"
		"\t%r = load i32 %p\n"
		"\tret i32 %r\n"
		"}\n";
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRPrinter		cPrinter;
	CIRVerifier		cVerifier;
	CChars			szExpected;
	CChars			szPrinted;
	CIRFunction*	pcFunction;
	CIRFunction*	pcCopy;
	CIRBlock*		pcLoop;
	CIRBlock*		pcCopied;
	size			ui;

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	cModule.muiSource = 7;
	AssertTrue(cParser.Parse(szText));
	cModule.muiSource = 0;
	cParser.Kill();
	pcFunction = cModule.GetFunction(0);
	cModule.SetRegisterCall(pcFunction->muiSymbol);

	pcCopy = cModule.CopyFunction(pcFunction, "_Sum__K1_3");
	AssertTrue(pcCopy != NULL);
	AssertTrue(pcCopy != pcFunction);
	AssertTrue(cModule.IsLocal(pcCopy->muiSymbol));
	AssertTrue(cModule.IsRegisterCall(pcCopy->muiSymbol));
	AssertFalse(cModule.IsLocal(pcFunction->muiSymbol));
	AssertTrue(pcCopy->IsRestrictParameter(0));
	AssertTrue(pcCopy == cModule.CopyFunction(pcFunction, "_Sum__K1_3"));
	AssertSize(2, cModule.NumFunctions());

	//Its blocks are its own, and so what they name.
	pcLoop = pcFunction->FindBlock("loop");
	pcCopied = pcCopy->FindBlock("loop");
	AssertTrue(pcCopied != pcLoop);
	AssertTrue(pcCopied == pcCopy->FindBlock("entry")->msJump.pcThen);
	AssertTrue(pcCopy->FindBlock("body") == pcCopied->masPhis.Get(0)->asArguments.Get(1)->pcBlock);
	AssertTrue(pcCopied->HasPredecessor(pcCopy->FindBlock("body")));
	for (ui = 0; ui < pcCopied->masInstructions.NumElements(); ui++)
	{
		AssertInt(7, pcCopied->masInstructions.Get(ui)->uiSource);
	}
	AssertInt(7, pcCopied->msJump.uiSource);

	//It prints the same with the function gone, and verifies.
	szExpected.Init();
	cPrinter.Init(&cModule, &szExpected);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
	szExpected.Replace("@_Sum(", "@_Sum__K1_3(");
	cModule.RemoveFunction(pcFunction);
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintFunction(pcCopy);
	cPrinter.Kill();
	AssertString(szExpected.Text(), szPrinted.Text());
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	cVerifier.Kill();
	szPrinted.Kill();
	szExpected.Kill();
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two inlined calls with a constant argument (SignBit and MaxExponent of a
// specialised DefaultNaN, softfloat): each is a branch on a constant to a
// phi, the second's blocks after the first's join, which is a phi of one
// argument.  It all folds to the constant, nothing left using a phi that's
// gone.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserFoldedInlinedPhis(void)
{
	static const char*	szText =
		"function i64 @_DefaultNaN__Fs__K0_52(i16 %0)\n{\n"
		"0:\n\tjmp 2\n"
		"1:\n\t%1 = phi i64 [4, %4]\n\tjmp 6\n"
		"2:\n\t%2 = eq i16 52, 23\n\tbrif %2, 4, 3\n"
		"3:\n\tjmp 4\n"
		"4:\n\t%3 = phi i64 [3, 63], [2, 31]\n\t%4 = shl i64 1, %3\n\tjmp 1\n"
		"5:\n\t%5 = phi i16 [7, 2047], [8, 255]\n\t%6 = sext i16 %5 to i64\n\t%7 = shl i64 %6, 52\n\t%8 = or i64 %1, %7\n\t%9 = or i64 %8, 2251799813685248\n\tret i64 %9\n"
		"6:\n\t%10 = eq i16 52, 23\n\tbrif %10, 8, 7\n"
		"7:\n\tjmp 5\n"
		"8:\n\tjmp 5\n"
		"}\n";
	CChars				szFloat;

	AssertIROptimised(
		"function i64 @_DefaultNaN__Fs__K0_52(i16 %0)\n{\n0:\n\tret i64 -2251799813685248\n}\n", szText);

	szFloat.Init(szText);
	szFloat.Replace("2251799813685248", "4194304");
	szFloat.Replace("52", "23");
	AssertIROptimised(
		"function i64 @_DefaultNaN__Fs__K0_23(i16 %0)\n{\n0:\n\tret i64 4290772992\n}\n", szFloat.Text());
	szFloat.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// A loop with a branch on a value that is the same each time round is two loops, the preheader choosing
// (UnswitchLoops), the sum after them a phi of both; an address p + (a + i) in a loop with p and a the same each
// time round is (p + a) + i, p + a before the loop (SplitLoopAddresses).
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserLoopInvariants(void)
{
	AssertIROptimised(
		"function i32 @_U__Fii(i32 %n, i32 %f)\n{\nentry:\n\t%c = ne i32 %f, 0\n\tbrif %c, loop, 0\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%s = phi i32 [entry, 0], [loop, %a]\n\t%a = add i32 %s, %i\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\t%0 = phi i32 [loop, %a], [0, %3]\n\tret i32 %0\n"
		"0:\n\t%1 = phi i32 [entry, 0], [0, %4]\n\t%2 = phi i32 [entry, 0], [0, %3]\n\t%3 = sub i32 %2, %1\n"
		"\t%4 = add i32 %1, 1\n\t%5 = slt i32 %4, %n\n\tbrif %5, 0, exit\n}\n",
		"function i32 @_U__Fii(i32 %n, i32 %f)\n{\nentry:\n\t%c = ne i32 %f, 0\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [next, %i2]\n\t%s = phi i32 [entry, 0], [next, %s2]\n\tbrif %c, plus, minus\n"
		"plus:\n\t%a = add i32 %s, %i\n\tjmp next\n"
		"minus:\n\t%b = sub i32 %s, %i\n\tjmp next\n"
		"next:\n\t%s2 = phi i32 [plus, %a], [minus, %b]\n\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret i32 %s2\n}\n", false);
	//The branch's other way is an inner cycle that also leaves the loop: unswitched, the loop's copy of it can't be
	//reached, each of its two blocks the other's only predecessor, and the loop's counter is used after the loop.  The
	//blocks go before the counter's uses are rewritten (CIRSSAUpdater would go round them for ever).
	AssertIROptimised(
		"function i32 @_C__Fiii(i32 %n, i32 %f, i32 %g)\n{\nentry:\n\t%c = ne i32 %f, 0\n\tbrif %c, loop, 0\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\t%0 = phi i32 [loop, %i], [2, %1]\n\tret i32 %0\n"
		"0:\n\t%1 = phi i32 [entry, 0], [1, %2]\n\tjmp 2\n"
		"1:\n\t%2 = add i32 %4, 2\n\t%3 = slt i32 %2, 100\n\tbrif %3, 2, 0\n"
		"2:\n\t%4 = phi i32 [0, %1], [1, %2]\n\t%5 = slt i32 %4, %g\n\tbrif %5, 1, exit\n}\n",
		"function i32 @_C__Fiii(i32 %n, i32 %f, i32 %g)\n{\nentry:\n\t%c = ne i32 %f, 0\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [plus, %i2], [inner2, %k2]\n\tbrif %c, plus, inner\n"
		"plus:\n\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"inner:\n\t%k = phi i32 [loop, %i], [inner2, %k2]\n\t%e = slt i32 %k, %g\n\tbrif %e, inner2, exit\n"
		"inner2:\n\t%k2 = add i32 %k, 2\n\t%x = slt i32 %k2, 100\n\tbrif %x, inner, loop\n"
		"exit:\n\tret i32 %i\n}\n", false);

	AssertIROptimised(
		"space default 4\n\nfunction void @_F__FPci(ptr %p, i32 %a, i32 %n)\n{\nentry:\n\t%0 = ptradd ptr %p, %a\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%q = ptradd ptr %0, %i\n\tstore i8 7, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n",
		"function void @_F__FPci(ptr %p, i32 %a, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%o = add i32 %a, %i\n\t%q = ptradd ptr %p, %o\n\tstore i8 7, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A store through a pointer loaded from a restrict symbol doesn't alias
// other symbols, so their loads leave the loop; through any other pointer
// it might.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserRestrict(void)
{
	AssertIROptimised(
		"space default 4\nrestrict @M\n\nfunction void @_F__Fii(i32 %y, i32 %n)\n{\nentry:\n\t%w = load i32 @W\n\t%r = mul i32 %y, %w\n"
		"\t%m = load ptr @M\n\t%a = ptradd ptr %m, %r\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%q = ptradd ptr %a, %i\n\tstore i8 7, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n",
		"space default 4\nrestrict @M\n\nfunction void @_F__Fii(i32 %y, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%w = load i32 @W\n\t%r = mul i32 %y, %w\n\t%m = load ptr @M\n"
		"\t%a = ptradd ptr %m, %r\n\t%q = ptradd ptr %a, %i\n\tstore i8 7, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	AssertIROptimised(
		"space default 4\n\nfunction void @_F__Fii(i32 %y, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%w = load i32 @W\n\t%r = mul i32 %y, %w\n\t%m = load ptr @M\n"
		"\t%a = ptradd ptr %m, %r\n\t%q = ptradd ptr %a, %i\n\tstore i8 7, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n",
		"space default 4\n\nfunction void @_F__Fii(i32 %y, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%w = load i32 @W\n\t%r = mul i32 %y, %w\n\t%m = load ptr @M\n"
		"\t%a = ptradd ptr %m, %r\n\t%q = ptradd ptr %a, %i\n\tstore i8 7, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	AssertIROptimised(
		"space default 4\nrestrict @M\n\nfunction void @_F__FPcii(ptr %p, i32 %y, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%w = load i32 @W\n\t%m = load ptr @M\n"
		"\t%a = ptradd ptr %m, %w\n\tstore i8 7, %a\n\t%q = ptradd ptr %p, %i\n\tstore i8 8, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n",
		"space default 4\nrestrict @M\n\nfunction void @_F__FPcii(ptr %p, i32 %y, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%w = load i32 @W\n\t%m = load ptr @M\n"
		"\t%a = ptradd ptr %m, %w\n\tstore i8 7, %a\n\t%q = ptradd ptr %p, %i\n\tstore i8 8, %q\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A restrict parameter (and pointers stepped from it through phis) doesn't
// alias symbols, plain parameters or other restrict pointers.  A restrict
// symbol and a plain parameter may.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserRestrictParameters(void)
{
	//Stores through a restrict p stepped in the loop: @W's load leaves it.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPci(ptr restrict %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%w = load i8 @W\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%q = phi ptr [entry, %p], [loop, %q2]\n"
		"\tstore i8 %w, %q\n"
		"\t%q2 = ptradd ptr %q, 1\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPci(ptr restrict %p, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%q = phi ptr [entry, %p], [loop, %q2]\n\t%w = load i8 @W\n\tstore i8 %w, %q\n"
		"\t%q2 = ptradd ptr %q, 1\n\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	//Stores through a plain q: a load through the restrict p leaves the loop.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcPci(ptr restrict %p, ptr %q, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%v = load i8 %p\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%a = ptradd ptr %q, %i\n"
		"\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcPci(ptr restrict %p, ptr %q, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%v = load i8 %p\n\t%a = ptradd ptr %q, %i\n\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	//Neither restrict: it stays.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcPci(ptr %p, ptr %q, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%v = load i8 %p\n"
		"\t%a = ptradd ptr %q, %i\n"
		"\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcPci(ptr %p, ptr %q, i32 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%v = load i8 %p\n\t%a = ptradd ptr %q, %i\n\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	//A restrict symbol's pointer and a plain parameter (which may be the symbol's value): it stays.
	AssertIROptimised(
		"space default 4\n"
		"restrict @M\n"
		"\n"
		"function void @_F__FPci(ptr %q, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%m = load ptr @M\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%v = load i8 %m\n"
		"\t%a = ptradd ptr %q, %i\n"
		"\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\nrestrict @M\n\nfunction void @_F__FPci(ptr %q, i32 %n)\n{\nentry:\n\t%m = load ptr @M\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%v = load i8 %m\n\t%a = ptradd ptr %q, %i\n\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	//The pointer stored through is p in one way and q in the other: no single base, so it stays.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcPcii(ptr restrict %p, ptr %q, i32 %f, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%c = ne i32 %f, 0\n"
		"\tbrif %c, loop, b\n"
		"b:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [b, 0], [loop, %i2], [entry, 0]\n"
		"\t%r = phi ptr [b, %q], [loop, %r], [entry, %p]\n"
		"\t%w = load i8 @W\n"
		"\t%s = ptradd ptr %r, %i\n"
		"\tstore i8 %w, %s\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcPcii(ptr restrict %p, ptr %q, i32 %f, i32 %n)\n{\nentry:\n\t%c = ne i32 %f, 0\n\tbrif %c, a, b\n"
		"a:\n\tjmp loop\nb:\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [a, 0], [b, 0], [loop, %i2]\n\t%r = phi ptr [a, %p], [b, %q], [loop, %r]\n\t%w = load i8 @W\n\t%s = ptradd ptr %r, %i\n\tstore i8 %w, %s\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Strength reduction of an unsigned i16 counter (the W65C816's size) zero
// extended in an address: p + (zext i << 2) becomes a pointer stepping by
// 4 when the loop's tests keep i from wrapping, and stays when they don't.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserZeroExtendedCounters(void)
{
	//i from 0 while i < n: i is at most 65534, so i + 1 never wraps.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p, i16 %n)\n"
		"{\n"
		"b:\n"
		"\t%0 = ult i16 0, %n\n"
		"\tbrif %0, l, x\n"
		"l:\n"
		"\t%1 = phi i16 [l, %j], [b, 0]\n"
		"\t%2 = phi i32 [l, %s2], [b, 0]\n"
		"\t%3 = phi ptr [l, %4], [b, %p]\n"
		"\t%v = load i32 %3\n"
		"\t%s2 = add i32 %2, %v\n"
		"\t%j = add i16 %1, 1\n"
		"\t%4 = ptradd ptr %3, 4\n"
		"\t%5 = ult i16 %j, %n\n"
		"\tbrif %5, l, x\n"
		"x:\n"
		"\t%6 = phi i32 [b, 0], [l, %s2]\n"
		"\tret i32 %6\n"
		"}\n",
		"function i32 @_S(ptr %p, i16 %n)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, 0], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = ult i16 %i, %n\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);

	//i from k while i < n: the loop is only entered when k < n, so k doesn't wrap either.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p, i16 %k, i16 %n)\n"
		"{\n"
		"b:\n"
		"\t%0 = ult i16 %k, %n\n"
		"\tbrif %0, 0, x\n"
		"l:\n"
		"\t%1 = phi i16 [0, %k], [l, %j]\n"
		"\t%2 = phi i32 [0, 0], [l, %s2]\n"
		"\t%3 = phi ptr [0, %9], [l, %4]\n"
		"\t%v = load i32 %3\n"
		"\t%s2 = add i32 %2, %v\n"
		"\t%j = add i16 %1, 1\n"
		"\t%4 = ptradd ptr %3, 4\n"
		"\t%5 = ult i16 %j, %n\n"
		"\tbrif %5, l, x\n"
		"x:\n"
		"\t%6 = phi i32 [b, 0], [l, %s2]\n"
		"\tret i32 %6\n"
		"0:\n"
		"\t%7 = zext i16 %k to i32\n"
		"\t%8 = shl i32 %7, 2\n"
		"\t%9 = ptradd ptr %p, %8\n"
		"\tjmp l\n"
		"}\n",
		"function i32 @_S(ptr %p, i16 %k, i16 %n)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, %k], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = ult i16 %i, %n\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);

	//i stepping by 2 while i < n wraps from 65534 when n is 65535: it stays.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p, i16 %n)\n"
		"{\n"
		"b:\n"
		"\t%0 = ult i16 0, %n\n"
		"\tbrif %0, l, x\n"
		"l:\n"
		"\t%1 = phi i16 [b, 0], [l, %j]\n"
		"\t%2 = phi i32 [b, 0], [l, %s2]\n"
		"\t%e = zext i16 %1 to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %2, %v\n"
		"\t%j = add i16 %1, 2\n"
		"\t%3 = ult i16 %j, %n\n"
		"\tbrif %3, l, x\n"
		"x:\n"
		"\t%4 = phi i32 [b, 0], [l, %s2]\n"
		"\tret i32 %4\n"
		"}\n",
		"function i32 @_S(ptr %p, i16 %n)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, 0], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = ult i16 %i, %n\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 2\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);

	//i stepping by 2 while i < 1000 doesn't wrap.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p)\n"
		"{\n"
		"b:\n"
		"\tjmp l\n"
		"l:\n"
		"\t%0 = phi i16 [l, %j], [b, 0]\n"
		"\t%1 = phi i32 [l, %s2], [b, 0]\n"
		"\t%2 = phi ptr [l, %3], [b, %p]\n"
		"\t%v = load i32 %2\n"
		"\t%s2 = add i32 %1, %v\n"
		"\t%j = add i16 %0, 2\n"
		"\t%3 = ptradd ptr %2, 8\n"
		"\t%4 = ult i16 %j, 1000\n"
		"\tbrif %4, l, x\n"
		"x:\n"
		"\tret i32 %s2\n"
		"}\n",
		"function i32 @_S(ptr %p)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, 0], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = ult i16 %i, 1000\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 2\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);

	//i stepping by 2 while i <= 1000 doesn't wrap either (UnsignedMaximum missed ule).
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p)\n"
		"{\n"
		"b:\n"
		"\tjmp l\n"
		"l:\n"
		"\t%0 = phi i16 [l, %j], [b, 0]\n"
		"\t%1 = phi i32 [l, %s2], [b, 0]\n"
		"\t%2 = phi ptr [l, %3], [b, %p]\n"
		"\t%v = load i32 %2\n"
		"\t%s2 = add i32 %1, %v\n"
		"\t%j = add i16 %0, 2\n"
		"\t%3 = ptradd ptr %2, 8\n"
		"\t%4 = ule i16 %j, 1000\n"
		"\tbrif %4, l, x\n"
		"x:\n"
		"\tret i32 %s2\n"
		"}\n",
		"function i32 @_S(ptr %p)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, 0], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = ule i16 %i, 1000\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 2\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);

	//i stepping by 2 while 1000 >= i: the same, the operands the other way round.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p)\n"
		"{\n"
		"b:\n"
		"\tjmp l\n"
		"l:\n"
		"\t%0 = phi i16 [l, %j], [b, 0]\n"
		"\t%1 = phi i32 [l, %s2], [b, 0]\n"
		"\t%2 = phi ptr [l, %3], [b, %p]\n"
		"\t%v = load i32 %2\n"
		"\t%s2 = add i32 %1, %v\n"
		"\t%j = add i16 %0, 2\n"
		"\t%3 = ptradd ptr %2, 8\n"
		"\t%4 = uge i16 1000, %j\n"
		"\tbrif %4, l, x\n"
		"x:\n"
		"\tret i32 %s2\n"
		"}\n",
		"function i32 @_S(ptr %p)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, 0], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = uge i16 1000, %i\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 2\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);

	//i from k while i != n wraps when k > n: it stays.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(ptr %p, i16 %k, i16 %n)\n"
		"{\n"
		"b:\n"
		"\t%0 = ne i16 %k, %n\n"
		"\tbrif %0, l, x\n"
		"l:\n"
		"\t%1 = phi i16 [b, %k], [l, %j]\n"
		"\t%2 = phi i32 [b, 0], [l, %s2]\n"
		"\t%e = zext i16 %1 to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %2, %v\n"
		"\t%j = add i16 %1, 1\n"
		"\t%3 = ne i16 %j, %n\n"
		"\tbrif %3, l, x\n"
		"x:\n"
		"\t%4 = phi i32 [b, 0], [l, %s2]\n"
		"\tret i32 %4\n"
		"}\n",
		"function i32 @_S(ptr %p, i16 %k, i16 %n)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i16 [b, %k], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = ne i16 %i, %n\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%e = zext i16 %i to i32\n"
		"\t%o = shl i32 %e, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i16 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n", false, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A loop counting c from 0 up to n beside x going up by 1 too tests x
// against x0 + n instead, and c goes, when the loop is only entered when
// 0 < n.  Unguarded (n may be 0: once round, where != would go round
// 65535 times) it stays.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserLoopCounters(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcss(ptr %p, i16 %x0, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%g = ult i16 0, %n\n"
		"\tbrif %g, 0, exit\n"
		"loop:\n"
		"\t%x = phi i16 [0, %x0], [loop, %x2]\n"
		"\t%w = zext i16 %x to i32\n"
		"\t%a = ptradd ptr %p, %w\n"
		"\tstore i8 7, %a\n"
		"\t%x2 = add i16 %x, 1\n"
		"\t%0 = ne i16 %x2, %1\n"
		"\tbrif %0, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"0:\n"
		"\t%1 = add i16 %x0, %n\n"
		"\tjmp loop\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcss(ptr %p, i16 %x0, i16 %n)\n{\nentry:\n\t%g = ult i16 0, %n\n\tbrif %g, loop, exit\n"
		"loop:\n\t%c = phi i16 [entry, 0], [loop, %c2]\n\t%x = phi i16 [entry, %x0], [loop, %x2]\n"
		"\t%w = zext i16 %x to i32\n\t%a = ptradd ptr %p, %w\n\tstore i8 7, %a\n"
		"\t%x2 = add i16 %x, 1\n\t%c2 = add i16 %c, 1\n\t%d = ult i16 %c2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcss(ptr %p, i16 %x0, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%c = phi i16 [entry, 0], [loop, %c2]\n"
		"\t%x = phi i16 [entry, %x0], [loop, %x2]\n"
		"\t%w = zext i16 %x to i32\n"
		"\t%a = ptradd ptr %p, %w\n"
		"\tstore i8 7, %a\n"
		"\t%x2 = add i16 %x, 1\n"
		"\t%c2 = add i16 %c, 1\n"
		"\t%d = ult i16 %c2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcss(ptr %p, i16 %x0, i16 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%c = phi i16 [entry, 0], [loop, %c2]\n\t%x = phi i16 [entry, %x0], [loop, %x2]\n"
		"\t%w = zext i16 %x to i32\n\t%a = ptradd ptr %p, %w\n\tstore i8 7, %a\n"
		"\t%x2 = add i16 %x, 1\n\t%c2 = add i16 %c, 1\n\t%d = ult i16 %c2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A load after a call to a function that writes nothing its callers can
// see (nowrites) is what was loaded before it; after a call to any other
// function it is loaded again.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserNoWritesCalls(void)
{
	static const char*	szText =
		"space default 4\n"
		"NOWRITES"
		"\n"
		"function i32 @_F__Fi(i32 %a)\n{\nentry:\n"
		"\t%x = load i32 @G\n"
		"\t%c = call i32 @_Callee__Fi(i32 %a)\n"
		"\t%y = load i32 @G\n"
		"\t%s = add i32 %x, %y\n"
		"\t%t = add i32 %s, %c\n"
		"\tret i32 %t\n}\n";
	CChars				szNoWrites;
	CChars				szWrites;

	szNoWrites.Init(szText);
	szNoWrites.Replace("NOWRITES", "nowrites @_Callee__Fi\n");
	AssertIROptimised(
		"space default 4\n"
		"nowrites @_Callee__Fi\n"
		"\n"
		"function i32 @_F__Fi(i32 %a)\n{\nentry:\n"
		"\t%x = load i32 @G\n"
		"\t%c = call i32 @_Callee__Fi(i32 %a)\n"
		"\t%s = add i32 %x, %x\n"
		"\t%t = add i32 %s, %c\n"
		"\tret i32 %t\n}\n", szNoWrites.Text());
	szNoWrites.Kill();

	szWrites.Init(szText);
	szWrites.Replace("NOWRITES", "");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_F__Fi(i32 %a)\n{\nentry:\n"
		"\t%x = load i32 @G\n"
		"\t%c = call i32 @_Callee__Fi(i32 %a)\n"
		"\t%y = load i32 @G\n"
		"\t%s = add i32 %x, %y\n"
		"\t%t = add i32 %s, %c\n"
		"\tret i32 %t\n}\n", szWrites.Text());
	szWrites.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A load used only in one later block moves down to its use when nothing
// on the way may write it; a store through another pointer, or a use in a
// loop the load isn't in, keeps it where it is.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserSinkLoads(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcPci(ptr %p, ptr %q, i32 %f)\n"
		"{\n"
		"entry:\n"
		"\t%c = ne i32 %f, 0\n"
		"\tbrif %c, use, exit\n"
		"use:\n"
		"\t%v = load i8 %p\n"
		"\tstore i8 %v, %q\n"
		"\tret\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcPci(ptr %p, ptr %q, i32 %f)\n{\nentry:\n\t%v = load i8 %p\n\t%c = ne i32 %f, 0\n\tbrif %c, use, exit\n"
		"use:\n\tstore i8 %v, %q\n\tret\n"
		"exit:\n\tret\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcPcPci(ptr %p, ptr %q, ptr %r, i32 %f)\n"
		"{\n"
		"entry:\n"
		"\t%v = load i8 %p\n"
		"\tstore i8 0, %r\n"
		"\t%c = ne i32 %f, 0\n"
		"\tbrif %c, use, exit\n"
		"use:\n"
		"\tstore i8 %v, %q\n"
		"\tret\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcPcPci(ptr %p, ptr %q, ptr %r, i32 %f)\n{\nentry:\n\t%v = load i8 %p\n\tstore i8 0, %r\n\t%c = ne i32 %f, 0\n\tbrif %c, use, exit\n"
		"use:\n\tstore i8 %v, %q\n\tret\n"
		"exit:\n\tret\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_F__FPcPci(ptr %p, ptr %q, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%v = load i8 %p\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%a = ptradd ptr %q, %i\n"
		"\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_F__FPcPci(ptr %p, ptr %q, i32 %n)\n{\nentry:\n\t%v = load i8 %p\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [loop, %i2]\n\t%a = ptradd ptr %q, %i\n\tstore i8 %v, %a\n"
		"\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's slot is named for a debugger (Release too, Debugger65C64Plan
// A3).  Split into a slot per field, it forgets its name, so the debugger
// says the variable isn't kept rather than read a slot nothing stores to;
// one whose address goes to a call stays its variable's, named.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserSplitSlotNames(void)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIROptimiser	cOptimiser;
	CIRFunction*	pcFunction;

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"space default 4\n"
		"\n"
		"function i32 @_F__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%dbg0 = slot ptr 8, 4\n"
		"\tstore i32 2, %dbg0\n"
		"\t%y = ptradd ptr %dbg0, 4\n"
		"\tstore i32 3, %y\n"
		"\t%dbg1 = slot ptr 4, 4\n"
		"\tstore i32 5, %dbg1\n"
		"\tcall void @_Use__FPi(ptr %dbg1)\n"
		"\t%x = load i32 %dbg0\n"
		"\t%y2 = load i32 %y\n"
		"\t%r = add i32 %x, %y2\n"
		"\tret i32 %r\n"
		"}\n"));
	cParser.Kill();
	pcFunction = cModule.GetFunction(0);
	AssertFalse(pcFunction->FindTemporary("dbg0").IsNone());
	cOptimiser.Init(&cModule);
	cOptimiser.Optimise(pcFunction);
	cOptimiser.Kill();
	AssertTrue(pcFunction->FindTemporary("dbg0").IsNone());
	AssertFalse(pcFunction->FindTemporary("dbg1").IsNone());
	AssertString("", cErrors.Text());
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A loop stepping p and x by 1 until x + 1 is x0 + d, entered only when
// 0 < d, goes round d times: after it p is p0 + d, and the loop, doing
// nothing else, goes.  Unguarded (d may be 0: 65536 times round) or with
// a store in it, it stays.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserExitValues(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_F__FPcss(ptr %p0, i16 %x0, i16 %d)\n"
		"{\n"
		"entry:\n"
		"\t%g = ult i16 0, %d\n"
		"\tbrif %g, 0, none\n"
		"none:\n"
		"\tret ptr %p0\n"
		"0:\n"
		"\t%0 = zext i16 %d to i32\n"
		"\t%1 = ptradd ptr %p0, %0\n"
		"\tret ptr %1\n"
		"}\n",
		"space default 4\n\nfunction ptr @_F__FPcss(ptr %p0, i16 %x0, i16 %d)\n{\nentry:\n\t%n = add i16 %x0, %d\n\t%g = ult i16 0, %d\n\tbrif %g, loop, none\n"
		"loop:\n\t%x = phi i16 [entry, %x0], [loop, %x2]\n\t%p = phi ptr [entry, %p0], [loop, %p2]\n"
		"\t%p2 = ptradd ptr %p, 1\n\t%x2 = add i16 %x, 1\n\t%e = ne i16 %x2, %n\n\tbrif %e, loop, exit\n"
		"exit:\n\tret ptr %p2\n"
		"none:\n\tret ptr %p0\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_F__FPcss(ptr %p0, i16 %x0, i16 %d)\n"
		"{\n"
		"entry:\n"
		"\t%n = add i16 %x0, %d\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%x = phi i16 [entry, %x0], [loop, %x2]\n"
		"\t%p = phi ptr [entry, %p0], [loop, %p2]\n"
		"\t%p2 = ptradd ptr %p, 1\n"
		"\t%x2 = add i16 %x, 1\n"
		"\t%e = ne i16 %x2, %n\n"
		"\tbrif %e, loop, exit\n"
		"exit:\n"
		"\tret ptr %p2\n"
		"}\n",
		"space default 4\n\nfunction ptr @_F__FPcss(ptr %p0, i16 %x0, i16 %d)\n{\nentry:\n\t%n = add i16 %x0, %d\n\tjmp loop\n"
		"loop:\n\t%x = phi i16 [entry, %x0], [loop, %x2]\n\t%p = phi ptr [entry, %p0], [loop, %p2]\n"
		"\t%p2 = ptradd ptr %p, 1\n\t%x2 = add i16 %x, 1\n\t%e = ne i16 %x2, %n\n\tbrif %e, loop, exit\n"
		"exit:\n\tret ptr %p2\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_F__FPcss(ptr %p0, i16 %x0, i16 %d)\n"
		"{\n"
		"entry:\n"
		"\t%n = add i16 %x0, %d\n"
		"\t%g = ult i16 0, %d\n"
		"\tbrif %g, 0, none\n"
		"loop:\n"
		"\t%x = phi i16 [0, %x0], [loop, %x2]\n"
		"\t%p = phi ptr [0, %p0], [loop, %p2]\n"
		"\tstore i8 0, %p\n"
		"\t%p2 = ptradd ptr %p, 1\n"
		"\t%x2 = add i16 %x, 1\n"
		"\t%e = ne i16 %x2, %n\n"
		"\tbrif %e, loop, exit\n"
		"exit:\n"
		"\tret ptr %1\n"
		"none:\n"
		"\tret ptr %p0\n"
		"0:\n"
		"\t%0 = zext i16 %d to i32\n"
		"\t%1 = ptradd ptr %p0, %0\n"
		"\tjmp loop\n"
		"}\n",
		"space default 4\n\nfunction ptr @_F__FPcss(ptr %p0, i16 %x0, i16 %d)\n{\nentry:\n\t%n = add i16 %x0, %d\n\t%g = ult i16 0, %d\n\tbrif %g, loop, none\n"
		"loop:\n\t%x = phi i16 [entry, %x0], [loop, %x2]\n\t%p = phi ptr [entry, %p0], [loop, %p2]\n\tstore i8 0, %p\n"
		"\t%p2 = ptradd ptr %p, 1\n\t%x2 = add i16 %x, 1\n\t%e = ne i16 %x2, %n\n\tbrif %e, loop, exit\n"
		"exit:\n\tret ptr %p2\n"
		"none:\n\tret ptr %p0\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A comparison only a brif in a later block reads moves to that block (not
// into a loop it wasn't in), and an instruction both ways of a brif work
// out is worked out once before it.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserSinkAndHoist(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_F__Fiii(i32 %a, i32 %b, i32 %f)\n"
		"{\n"
		"entry:\n"
		"\t%g = ne i32 %f, 0\n"
		"\tbrif %g, test, other\n"
		"test:\n"
		"\t%c = slt i32 %a, 0\n"
		"\tbrif %c, neg, pos\n"
		"neg:\n"
		"\tret i32 1\n"
		"pos:\n"
		"\tret i32 2\n"
		"other:\n"
		"\tret i32 3\n"
		"}\n",
		"space default 4\n\nfunction i32 @_F__Fiii(i32 %a, i32 %b, i32 %f)\n{\nentry:\n\t%c = slt i32 %a, 0\n\t%g = ne i32 %f, 0\n\tbrif %g, test, other\n"
		"test:\n\tbrif %c, neg, pos\n"
		"neg:\n\tret i32 1\npos:\n\tret i32 2\nother:\n\tret i32 3\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_F__Fiii(i32 %a, i32 %b, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%c = slt i32 %a, %b\n"
		"\tbrif %c, loop, 0\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, out\n"
		"out:\n"
		"\tret i32 %i2\n"
		"0:\n"
		"\tret i32 0\n"
		"}\n",
		"space default 4\n\nfunction i32 @_F__Fiii(i32 %a, i32 %b, i32 %n)\n{\nentry:\n\t%c = slt i32 %a, %b\n\tjmp loop\n"
		"loop:\n\t%i = phi i32 [entry, 0], [next, %i2]\n\tbrif %c, next, out\n"
		"next:\n\t%i2 = add i32 %i, 1\n\t%d = slt i32 %i2, %n\n\tbrif %d, loop, out\n"
		"out:\n\t%r = phi i32 [loop, %i], [next, %i2]\n\tret i32 %r\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_F__Fiii(i32 %a, i32 %b, i32 %f)\n"
		"{\n"
		"entry:\n"
		"\t%x = mul i32 %a, %b\n"
		"\t%g = ne i32 %f, 0\n"
		"\tbrif %g, one, two\n"
		"one:\n"
		"\t%y = add i32 %x, 1\n"
		"\tret i32 %y\n"
		"two:\n"
		"\t%w = sub i32 %x, 1\n"
		"\tret i32 %w\n"
		"}\n",
		"space default 4\n\nfunction i32 @_F__Fiii(i32 %a, i32 %b, i32 %f)\n{\nentry:\n\t%g = ne i32 %f, 0\n\tbrif %g, one, two\n"
		"one:\n\t%x = mul i32 %a, %b\n\t%y = add i32 %x, 1\n\tret i32 %y\n"
		"two:\n\t%z = mul i32 %a, %b\n\t%w = sub i32 %z, 1\n\tret i32 %w\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A counter times a value the loop doesn't change is a phi of its own,
// stepped by an add (also when it is stepped on some ways round only);
// times a value the loop changes it stays a mul.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserReduceMultiplies(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i16 @_F__Fsss(i16 %y0, i16 %w, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = mul i16 %y0, %w\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%y = phi i16 [entry, %y0], [loop, %y2]\n"
		"\t%s = phi i16 [entry, 0], [loop, %s2]\n"
		"\t%1 = phi i16 [entry, %0], [loop, %2]\n"
		"\t%s2 = xor i16 %s, %1\n"
		"\t%y2 = add i16 %y, 1\n"
		"\t%2 = add i16 %1, %w\n"
		"\t%d = slt i16 %y2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret i16 %s2\n"
		"}\n",
		"space default 4\n\nfunction i16 @_F__Fsss(i16 %y0, i16 %w, i16 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%y = phi i16 [entry, %y0], [loop, %y2]\n\t%s = phi i16 [entry, 0], [loop, %s2]\n"
		"\t%m = mul i16 %y, %w\n\t%s2 = xor i16 %s, %m\n\t%y2 = add i16 %y, 1\n\t%d = slt i16 %y2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret i16 %s2\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i16 @_F__Fsss(i16 %y0, i16 %w, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%y = phi i16 [entry, %y0], [loop, %y2]\n"
		"\t%s = phi i16 [entry, 0], [loop, %s2]\n"
		"\t%m = mul i16 %y, %s\n"
		"\t%s2 = xor i16 %s, %m\n"
		"\t%y2 = add i16 %y, 1\n"
		"\t%d = slt i16 %y2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret i16 %s2\n"
		"}\n",
		"space default 4\n\nfunction i16 @_F__Fsss(i16 %y0, i16 %w, i16 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%y = phi i16 [entry, %y0], [loop, %y2]\n\t%s = phi i16 [entry, 0], [loop, %s2]\n"
		"\t%m = mul i16 %y, %s\n\t%s2 = xor i16 %s, %m\n\t%y2 = add i16 %y, 1\n\t%d = slt i16 %y2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret i16 %s2\n}\n");
	//Two muls of the counter in one block, each by its own stride (BaseLib's Float32Vec2TransformCoords; whether the
	//adds put in the block move its instructions depends on its size: TestCPP816CompileBaseLibUnits compiles that).
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_T__FPcsPcss(ptr %out, i16 %so, ptr %in, i16 %si, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%g = slt i16 0, %n\n"
		"\tbrif %g, loop, exit\n"
		"loop:\n"
		"\t%i = phi i16 [loop, %i2], [entry, 0]\n"
		"\t%0 = phi i16 [loop, %3], [entry, 0]\n"
		"\t%1 = phi i16 [loop, %2], [entry, 0]\n"
		"\t%ax = sext i16 %0 to i32\n"
		"\t%p = call ptr @_Remap(ptr %out, i32 %ax)\n"
		"\t%bx = sext i16 %1 to i32\n"
		"\t%q = call ptr @_Remap(ptr %in, i32 %bx)\n"
		"\tcall void @_Use(ptr %p, ptr %q)\n"
		"\t%i2 = add i16 %i, 1\n"
		"\t%2 = add i16 %1, %si\n"
		"\t%3 = add i16 %0, %so\n"
		"\t%d = slt i16 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"space default 4\n\nfunction void @_T__FPcsPcss(ptr %out, i16 %so, ptr %in, i16 %si, i16 %n)\n{\nentry:\n\t%g = slt i16 0, %n\n\tbrif %g, loop, exit\n"
		"loop:\n\t%i = phi i16 [entry, 0], [loop, %i2]\n\t%a = mul i16 %i, %so\n\t%ax = sext i16 %a to i32\n\t%p = call ptr @_Remap(ptr %out, i32 %ax)\n"
		"\t%b = mul i16 %i, %si\n\t%bx = sext i16 %b to i32\n\t%q = call ptr @_Remap(ptr %in, i32 %bx)\n\tcall void @_Use(ptr %p, ptr %q)\n"
		"\t%i2 = add i16 %i, 1\n\t%d = slt i16 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret\n}\n");
	//y stepped on one way round only: its web (the head's phi, y + 1, the phi joining them) has a product web.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i16 @_F__Fssss(i16 %y0, i16 %w, i16 %f, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = mul i16 %y0, %w\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i16 [entry, 0], [join, %i2]\n"
		"\t%s = phi i16 [entry, 0], [join, %s2]\n"
		"\t%1 = phi i16 [entry, %0], [join, %3]\n"
		"\t%a = and i16 %i, %f\n"
		"\t%c = eq i16 %a, 0\n"
		"\tbrif %c, step, join\n"
		"step:\n"
		"\t%2 = add i16 %1, %w\n"
		"\tjmp join\n"
		"join:\n"
		"\t%3 = phi i16 [loop, %1], [step, %2]\n"
		"\t%s2 = xor i16 %s, %3\n"
		"\t%i2 = add i16 %i, 1\n"
		"\t%d = slt i16 %i2, %n\n"
		"\tbrif %d, loop, exit\n"
		"exit:\n"
		"\tret i16 %s2\n"
		"}\n",
		"space default 4\n\nfunction i16 @_F__Fssss(i16 %y0, i16 %w, i16 %f, i16 %n)\n{\nentry:\n\tjmp loop\n"
		"loop:\n\t%i = phi i16 [entry, 0], [join, %i2]\n\t%y = phi i16 [entry, %y0], [join, %y3]\n\t%s = phi i16 [entry, 0], [join, %s2]\n"
		"\t%a = and i16 %i, %f\n\t%c = eq i16 %a, 0\n\tbrif %c, step, join\n"
		"step:\n\t%y2 = add i16 %y, 1\n\tjmp join\n"
		"join:\n\t%y3 = phi i16 [loop, %y], [step, %y2]\n\t%m = mul i16 %y3, %w\n\t%s2 = xor i16 %s, %m\n"
		"\t%i2 = add i16 %i, 1\n\t%d = slt i16 %i2, %n\n\tbrif %d, loop, exit\n"
		"exit:\n\tret i16 %s2\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// x * 0 and x & 0 are 0.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserTimesZero(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i16 @_F__Fs(i16 %x)\n"
		"{\n"
		"entry:\n"
		"\tret i16 0\n"
		"}\n",
		"space default 4\n\nfunction i16 @_F__Fs(i16 %x)\n{\nentry:\n\t%a = mul i16 %x, 0\n\t%b = and i16 %x, 0\n\t%c = mul i16 0, %x\n\t%d = add i16 %a, %b\n\t%e = add i16 %d, %c\n\tret i16 %e\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer's constant offsets are added together: a member's member.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserPointerOffsets(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_F__Fp(ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%b = ptradd ptr %p, 148\n"
		"\tret ptr %b\n"
		"}\n",
		"space default 4\n\nfunction ptr @_F__Fp(ptr %p)\n{\nentry:\n\t%a = ptradd ptr %p, 128\n\t%b = ptradd ptr %a, 20\n\tret ptr %b\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A restrict symbol stays restrict when a function naming it is exported
// to CIRImports and inlined into another unit.
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsRestrict(void)
{
	CIRModule		cUnit;
	CIRModule		cCaller;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRImports		cImports;
	CIRInliner		cInliner;
	CArrayChars		aszMissing;
	CIRPrinter		cPrinter;
	CChars			szPrinted;
	int				iSymbol;

	cErrors.Init();
	cUnit.Init();
	cParser.Init(&cUnit, &cErrors);
	AssertTrue(cParser.Parse("space default 4\nrestrict @M\n\nfunction void @_Put__Fic(i32 %i, i8 %c)\n{\nentry:\n"
		"\t%m = load ptr @M\n\t%a = ptradd ptr %m, %i\n\tstore i8 %c, %a\n\tret\n}\n"));
	cParser.Kill();
	cImports.Init(16);
	AssertTrue(cImports.Export(&cUnit, cUnit.GetFunction(0)));
	iSymbol = cImports.GetModule()->FindSymbol("M");
	AssertTrue(iSymbol >= 0);
	AssertTrue(cImports.GetModule()->IsRestrict((uint32)iSymbol));

	cCaller.Init();
	cParser.Init(&cCaller, &cErrors);
	AssertTrue(cParser.Parse("space default 4\n\nfunction void @_F__Fi(i32 %i)\n{\nentry:\n\tcall void @_Put__Fic(i32 %i, i8 7)\n\tret\n}\n"));
	cParser.Kill();
	AssertFalse(cCaller.HasRestrict());
	aszMissing.Init();
	cInliner.Init(&cCaller, 16, 16, 4);
	cInliner.SetImports(&cImports, &aszMissing);
	AssertTrue(cInliner.InlineCalls(cCaller.GetFunction(0)));
	cInliner.Kill();
	aszMissing.Kill();

	szPrinted.Init();
	cPrinter.Init(&cCaller, &szPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	AssertString("space default 4\nrestrict @M\n\nfunction void @_F__Fi(i32 %i)\n{\nentry:\n\tjmp 1\n0:\n\tret\n"
		"1:\n\t%0 = load ptr @M\n\t%1 = ptradd ptr %0, %i\n\tstore i8 7, %1\n\tjmp 0\n}\n", szPrinted.Text());
	szPrinted.Kill();

	cCaller.Kill();
	cImports.Kill();
	cUnit.Kill();
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Inlines pcCaller's calls with pcImports, then prints its module.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRImportsInlineAndPrint(CIRModule* pcCaller, CIRImports* pcImports, CChars* pszPrinted)
{
	CIRInliner		cInliner;
	CArrayChars		aszMissing;
	CIRPrinter		cPrinter;

	aszMissing.Init();
	cInliner.Init(pcCaller, 16, 16, 4);
	cInliner.SetImports(pcImports, &aszMissing);
	AssertTrue(cInliner.InlineCalls(pcCaller->GetFunction(0)));
	cInliner.Kill();
	aszMissing.Kill();

	pszPrinted->Init();
	cPrinter.Init(pcCaller, pszPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A function that calls another by name is exported (a forwarder: its
// callers in other units call what it forwards to), the callee called in
// registers there as here.  One that calls a function local to its unit
// isn't.  When the callee is exported too, a caller inlines both.
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsCalls(void)
{
	CIRModule		cUnit;
	CIRModule		cWorkUnit;
	CIRModule		cCaller;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRImports		cImports;
	CChars			szPrinted;
	int				iSymbol;
	const char*		szCaller;

	cErrors.Init();
	cUnit.Init();
	cParser.Init(&cUnit, &cErrors);
	AssertTrue(cParser.Parse("space default 4\nregcall @_Work__Fs\n\n"
		"function i16 @_Fwd__Fs(i16 %x)\n{\nentry:\n\t%a = add i16 %x, 1\n\t%r = call i16 @_Work__Fs(i16 %a)\n\tret i16 %r\n}\n\n"
		"function i16 @_Helper__Fs(i16 %x), local\n{\nentry:\n\t%a = mul i16 %x, 5\n\tret i16 %a\n}\n\n"
		"function i16 @_UsesLocal__Fs(i16 %x)\n{\nentry:\n\t%r = call i16 @_Helper__Fs(i16 %x)\n\tret i16 %r\n}\n"));
	cParser.Kill();
	cImports.Init(16);
	AssertTrue(cImports.Export(&cUnit, cUnit.GetFunction(0)));
	AssertFalse(cImports.Export(&cUnit, cUnit.GetFunction(2)));
	AssertNull(cImports.Find("_UsesLocal__Fs"));
	iSymbol = cImports.GetModule()->FindSymbol("_Work__Fs");
	AssertTrue(iSymbol >= 0);
	AssertTrue(cImports.GetModule()->IsRegisterCall((uint32)iSymbol));

	//Only the forwarder is exported: the caller calls what it forwards to, in registers.
	szCaller = "space default 4\n\nfunction i16 @_F__Fs(i16 %n)\n{\nentry:\n\t%r = call i16 @_Fwd__Fs(i16 %n)\n\tret i16 %r\n}\n";
	cCaller.Init();
	cParser.Init(&cCaller, &cErrors);
	AssertTrue(cParser.Parse(szCaller));
	cParser.Kill();
	TestIRImportsInlineAndPrint(&cCaller, &cImports, &szPrinted);
	AssertString("space default 4\nregcall @_Work__Fs\n\nfunction i16 @_F__Fs(i16 %n)\n{\nentry:\n\tjmp 1\n0:\n\t%r = phi i16 [1, %1]\n\tret i16 %r\n"
		"1:\n\t%0 = add i16 %n, 1\n\t%1 = call i16 @_Work__Fs(i16 %0)\n\tjmp 0\n}\n", szPrinted.Text());
	szPrinted.Kill();
	cCaller.Kill();

	//With the worker exported too, the copy's call of it is inlined as well.
	cWorkUnit.Init();
	cParser.Init(&cWorkUnit, &cErrors);
	AssertTrue(cParser.Parse("space default 4\n\nfunction i16 @_Work__Fs(i16 %x)\n{\nentry:\n\t%a = mul i16 %x, 3\n\tret i16 %a\n}\n"));
	cParser.Kill();
	AssertTrue(cImports.Export(&cWorkUnit, cWorkUnit.GetFunction(0)));
	cCaller.Init();
	cParser.Init(&cCaller, &cErrors);
	AssertTrue(cParser.Parse(szCaller));
	cParser.Kill();
	TestIRImportsInlineAndPrint(&cCaller, &cImports, &szPrinted);
	AssertTrue(strstr(szPrinted.Text(), "= call") == NULL);
	AssertTrue(strstr(szPrinted.Text(), "mul i16") != NULL);
	szPrinted.Kill();
	cCaller.Kill();

	cWorkUnit.Kill();
	cImports.Kill();
	cUnit.Kill();
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where a forwarder is a jump (SetForwardersJump), a function that only
// passes a call on (a parameter moved, or not) takes a callee no bigger
// than itself, and not a bigger one: that would save only the jump, and
// copy the work.  The thunk's own call stays a call (FwdChain).  Any other caller takes either, and so does a forwarder
// where they aren't jumps.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInlineForwarders(void)
{
	static const char*	szModule =
		"space default 4\n\n"
		"function i16 @_Work__Fs(i16 %x)\n{\nentry:\n\t%a = mul i16 %x, 5\n\t%b = xor i16 %a, 9\n\t%c = add i16 %b, %x\n\tret i16 %c\n}\n\n"
		"function i16 @_Thunk__Fs(i16 %x)\n{\nentry:\n\t%a = add i16 %x, 1\n\tret i16 %a\n}\n\n"
		"function i16 @_FwdWork__Fs(i16 %x)\n{\nentry:\n\t%r = call i16 @_Work__Fs(i16 %x)\n\tret i16 %r\n}\n\n"
		"function i16 @_FwdMoved__Fs(i16 %x)\n{\nentry:\n\t%m = add i16 %x, 4\n\t%r = call i16 @_Work__Fs(i16 %m)\n\tret i16 %r\n}\n\n"
		"function i16 @_FwdThunk__Fs(i16 %x)\n{\nentry:\n\t%r = call i16 @_Thunk__Fs(i16 %x)\n\tret i16 %r\n}\n\n"
		"function i16 @_Uses__Fs(i16 %x)\n{\nentry:\n\t%r = call i16 @_Work__Fs(i16 %x)\n\t%s = add i16 %r, 1\n\tret i16 %s\n}\n\n"
		"function i16 @_ThunkToWork__Fs(i16 %x)\n{\nentry:\n\t%m = add i16 %x, 2\n\t%r = call i16 @_Work__Fs(i16 %m)\n\tret i16 %r\n}\n\n"
		"function i16 @_FwdChain__Fs(i16 %x)\n{\nentry:\n\t%r = call i16 @_ThunkToWork__Fs(i16 %x)\n\tret i16 %r\n}\n";
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRInliner		cInliner;
	CIRPrinter		cPrinter;
	CChars			szPrinted;

	cErrors.Init();
	cModule.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(szModule));
	cParser.Kill();

	AssertTrue(CIRInliner::IsForwarder(cModule.GetFunction(2)));
	AssertTrue(CIRInliner::IsForwarder(cModule.GetFunction(3)));
	AssertFalse(CIRInliner::IsForwarder(cModule.GetFunction(5)));
	AssertFalse(CIRInliner::IsThunk(cModule.GetFunction(0)));
	AssertTrue(CIRInliner::IsThunk(cModule.GetFunction(1)));

	cInliner.Init(&cModule, 16, 16, 4);
	cInliner.SetForwardersJump(true);
	AssertFalse(cInliner.InlineCalls(cModule.GetFunction(2)));
	AssertFalse(cInliner.InlineCalls(cModule.GetFunction(3)));
	AssertTrue(cInliner.InlineCalls(cModule.GetFunction(4)));
	AssertTrue(cInliner.InlineCalls(cModule.GetFunction(5)));

	//A thunk's copy in a forwarder leaves it a forwarder: the thunk's call of the work stays a call.
	AssertTrue(cInliner.InlineCalls(cModule.GetFunction(7)));
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintFunction(cModule.GetFunction(7));
	cPrinter.Kill();
	AssertTrue(strstr(szPrinted.Text(), "call i16 @_Work__Fs") != NULL);
	szPrinted.Kill();
	cInliner.Kill();

	cInliner.Init(&cModule, 16, 16, 4);
	AssertTrue(cInliner.InlineCalls(cModule.GetFunction(2)));
	cInliner.Kill();

	cModule.Kill();
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A function that calls itself is inlined once into a caller: its copy's
// calls of itself aren't (they would be inlined again each round, doubling
// the caller: a W65C816 function that grew past 64KB ran off its bank).
//
//////////////////////////////////////////////////////////////////////////
void TestIRInlineRecursive(void)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRInliner		cInliner;
	CIRPrinter		cPrinter;
	CChars			szPrinted;
	CIRFunction*	pcCaller;
	size			ui;

	cErrors.Init();
	cModule.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse("space default 4\n\n"
		"function i16 @_R__Fs(i16 %n)\n{\nentry:\n\t%z = eq i16 %n, 0\n\tbrif %z, done, more\n"
		"done:\n\tret i16 0\nmore:\n\t%m = sub i16 %n, 1\n\t%r = call i16 @_R__Fs(i16 %m)\n\t%s = add i16 %r, 2\n\tret i16 %s\n}\n\n"
		"function i16 @_P__Fs(i16 %n)\n{\nentry:\n\t%d = add i16 %n, %n\n\tret i16 %d\n}\n\n"
		"function i16 @_F__Fs(i16 %n)\n{\nentry:\n\t%a = call i16 @_R__Fs(i16 %n)\n\t%b = call i16 @_P__Fs(i16 %a)\n\tret i16 %b\n}\n"));
	cParser.Kill();
	pcCaller = NULL;
	for (ui = 0; ui < cModule.NumFunctions(); ui++)
	{
		if (cModule.GetFunction(ui)->muiSymbol == (uint32)cModule.FindSymbol("_F__Fs"))
		{
			pcCaller = cModule.GetFunction(ui);
		}
	}
	AssertNotNull(pcCaller);
	cInliner.Init(&cModule, 64, 64, 8);
	AssertTrue(cInliner.InlineCalls(pcCaller));
	cInliner.InlineCalls(pcCaller);
	cInliner.InlineCalls(pcCaller);
	cInliner.Kill();

	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintFunction(pcCaller);
	cPrinter.Kill();
	//_R inlined once (its body's sub and add are in _F), its copy's call of itself left; _P inlined.
	AssertTrue(strstr(szPrinted.Text(), "call i16 @_R__Fs") != NULL);
	AssertTrue(strstr(strstr(szPrinted.Text(), "call i16 @_R__Fs") + 1, "call i16 @_R__Fs") == NULL);
	AssertTrue(strstr(szPrinted.Text(), "sub i16") != NULL);
	AssertTrue(strstr(szPrinted.Text(), "call i16 @_P__Fs") == NULL);
	szPrinted.Kill();

	cModule.Kill();
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Locals in slots and parameters read through argaddr become temporaries
// and phis; constants fold; empty blocks go.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiser(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = slt i32 0, %n\n"
		"\tbrif %0, body, exit\n"
		"body:\n"
		"\t%1 = phi i32 [entry, 0], [body, %sum]\n"
		"\t%2 = phi i32 [entry, 0], [body, %next]\n"
		"\t%a = ptradd ptr %p, %2\n"
		"\t%v = load i32 %a\n"
		"\t%sum = add i32 %1, %v\n"
		"\t%next = add i32 %2, 1\n"
		"\t%3 = slt i32 %next, %n\n"
		"\tbrif %3, body, exit\n"
		"exit:\n"
		"\t%4 = phi i32 [entry, 0], [body, %sum]\n"
		"\tret i32 %4\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%pp = argaddr ptr 0\n"
		"\t%np = argaddr ptr 1\n"
		"\t%s = slot ptr 4, 4\n"
		"\t%i = slot ptr 4, 4\n"
		"\tstore i32 0, %s\n"
		"\tstore i32 0, %i\n"
		"\tjmp test\n"
		"test:\n"
		"\t%iv = load i32 %i\n"
		"\t%nv = load i32 %np\n"
		"\t%c = slt i32 %iv, %nv\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%pv = load ptr %pp\n"
		"\t%iv2 = load i32 %i\n"
		"\t%a = ptradd ptr %pv, %iv2\n"
		"\t%v = load i32 %a\n"
		"\t%sv = load i32 %s\n"
		"\t%sum = add i32 %sv, %v\n"
		"\tstore i32 %sum, %s\n"
		"\t%next = add i32 %iv2, 1\n"
		"\tstore i32 %next, %i\n"
		"\tjmp empty\n"
		"empty:\n"
		"\tjmp test\n"
		"exit:\n"
		"\t%r = load i32 %s\n"
		"\tret i32 %r\n"
		"}\n");

	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @f(i32 %x)\n"
		"{\n"
		"a:\n"
		"\t%m = shl i32 %x, 3\n"
		"\t%r = add i32 %m, %m\n"
		"\tret i32 %r\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @f(i32 %x)\n"
		"{\n"
		"a:\n"
		"\t%k = add i32 2, 3\n"
		"\t%t = mul i32 %k, 0\n"
		"\t%c = eq i32 %t, 0\n"
		"\tbrif %c, b, dead\n"
		"b:\n"
		"\t%m = mul i32 %x, 8\n"
		"\t%m2 = mul i32 %x, 8\n"
		"\t%unused = sub i32 %x, 1\n"
		"\t%r = add i32 %m, %m2\n"
		"\tret i32 %r\n"
		"dead:\n"
		"\tret i32 %k\n"
		"}\n");

	//Loads of what is known to be in memory: a field loaded twice (the second in the only successor), a field
	//just stored, and one stored to through a pointer that may alias it (so loaded again).  A call forgets all.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @g(ptr %this, ptr %q, i32 %v)\n"
		"{\n"
		"a:\n"
		"\t%f = ptradd ptr %this, 4\n"
		"\t%x = load i32 %f\n"
		"\t%c = slt i32 %x, 10\n"
		"\tbrif %c, b, c\n"
		"b:\n"
		"\t%h = ptradd ptr %this, 8\n"
		"\tstore i32 %v, %h\n"
		"\tstore i32 7, %q\n"
		"\t%z = load i32 %h\n"
		"\t%s = add i32 %x, %v\n"
		"\t%t = add i32 %s, %z\n"
		"\t%u = call i32 @g(ptr %this, ptr %q, i32 %t)\n"
		"\t%w = load i32 %f\n"
		"\t%r = add i32 %u, %w\n"
		"\tret i32 %r\n"
		"c:\n"
		"\tret i32 0\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @g(ptr %this, ptr %q, i32 %v)\n"
		"{\n"
		"a:\n"
		"\t%f = ptradd ptr %this, 4\n"
		"\t%x = load i32 %f\n"
		"\t%c = slt i32 %x, 10\n"
		"\tbrif %c, b, c\n"
		"b:\n"
		"\t%f2 = ptradd ptr %this, 4\n"
		"\t%y = load i32 %f2\n"
		"\t%h = ptradd ptr %this, 8\n"
		"\tstore i32 %v, %h\n"
		"\t%y2 = load i32 %h\n"
		"\tstore i32 7, %q\n"
		"\t%z = load i32 %h\n"
		"\t%s = add i32 %y, %y2\n"
		"\t%t = add i32 %s, %z\n"
		"\t%u = call i32 @g(ptr %this, ptr %q, i32 %t)\n"
		"\t%w = load i32 %f\n"
		"\t%r = add i32 %u, %w\n"
		"\tret i32 %r\n"
		"c:\n"
		"\tret i32 0\n"
		"}\n");

	//An object in a slot whose fields are only loaded and stored is split into its fields, which become values.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @s(i32 %a, i32 %b)\n"
		"{\n"
		"e:\n"
		"\t%r = add i32 %a, %b\n"
		"\tret i32 %r\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @s(i32 %a, i32 %b)\n"
		"{\n"
		"e:\n"
		"\t%o = slot ptr 8, 4\n"
		"\tstore i32 %a, %o\n"
		"\t%f = ptradd ptr %o, 4\n"
		"\tstore i32 %b, %f\n"
		"\t%x = load i32 %o\n"
		"\t%g = ptradd ptr %o, 4\n"
		"\t%y = load i32 %g\n"
		"\t%r = add i32 %x, %y\n"
		"\tret i32 %r\n"
		"}\n");

	//A field the loop doesn't write is loaded once, before it.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @l(ptr %this, i32 %n)\n"
		"{\n"
		"e:\n"
		"\t%0 = slt i32 0, %n\n"
		"\tbrif %0, 0, x\n"
		"b:\n"
		"\t%1 = phi i32 [0, 0], [b, %s2]\n"
		"\t%2 = phi i32 [0, 0], [b, %i2]\n"
		"\t%s2 = add i32 %1, %v\n"
		"\t%i2 = add i32 %2, 1\n"
		"\t%3 = slt i32 %i2, %n\n"
		"\tbrif %3, b, x\n"
		"x:\n"
		"\t%4 = phi i32 [e, 0], [b, %s2]\n"
		"\tret i32 %4\n"
		"0:\n"
		"\t%f = ptradd ptr %this, 4\n"
		"\t%v = load i32 %f\n"
		"\tjmp b\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @l(ptr %this, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp h\n"
		"h:\n"
		"\t%s = phi i32 [e, 0], [b, %s2]\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, b, x\n"
		"b:\n"
		"\t%f = ptradd ptr %this, 4\n"
		"\t%v = load i32 %f\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp h\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n");

	//Two argaddrs of one parameter are the same memory: the store through one changes what the other reads.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @h(i32 %p)\n"
		"{\n"
		"a:\n"
		"\t%a1 = argaddr ptr 0\n"
		"\t%x = load volatile i32 %a1\n"
		"\tstore volatile i32 5, %a1\n"
		"\t%y = load i32 %a1\n"
		"\t%r = add i32 %x, %y\n"
		"\tret i32 %r\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @h(i32 %p)\n"
		"{\n"
		"a:\n"
		"\t%a1 = argaddr ptr 0\n"
		"\t%x = load volatile i32 %a1\n"
		"\t%a2 = argaddr ptr 0\n"
		"\tstore volatile i32 5, %a2\n"
		"\t%y = load i32 %a1\n"
		"\t%r = add i32 %x, %y\n"
		"\tret i32 %r\n"
		"}\n");

	//(p == 0) && (q == 0) then p == 0 again: the second test of %c is known on both ways in.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @t(ptr %p, ptr %q)\n"
		"{\n"
		"a:\n"
		"\t%c = eq ptr %p, 0\n"
		"\tbrif %c, b, d\n"
		"b:\n"
		"\t%e = eq ptr %q, 0\n"
		"\tbrif %e, c, f\n"
		"c:\n"
		"\tret i32 0\n"
		"d:\n"
		"\tret i32 2\n"
		"f:\n"
		"\tret i32 1\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @t(ptr %p, ptr %q)\n"
		"{\n"
		"a:\n"
		"\t%c = eq ptr %p, 0\n"
		"\tbrif %c, b, again\n"
		"b:\n"
		"\t%e = eq ptr %q, 0\n"
		"\tbrif %e, c, again\n"
		"c:\n"
		"\tret i32 0\n"
		"again:\n"
		"\tbrif %c, f, d\n"
		"d:\n"
		"\tret i32 2\n"
		"f:\n"
		"\tret i32 1\n"
		"}\n");
	//A zext of a phi of constants is a phi of the wider constants, and a block of only phis and a ret is copied into the
	//blocks that jump to it.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @f(i32 %a, i32 %b)\n"
		"{\n"
		"x:\n"
		"\t%l = slt i32 %a, %b\n"
		"\tbrif %l, done, other\n"
		"other:\n"
		"\t%e = eq i32 %a, %b\n"
		"\tbrif %e, join, more\n"
		"more:\n"
		"\tret i32 1\n"
		"join:\n"
		"\tret i32 0\n"
		"done:\n"
		"\tret i32 -1\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @f(i32 %a, i32 %b)\n"
		"{\n"
		"x:\n"
		"\t%l = slt i32 %a, %b\n"
		"\tbrif %l, less, other\n"
		"less:\n"
		"\tjmp done\n"
		"other:\n"
		"\t%e = eq i32 %a, %b\n"
		"\tbrif %e, same, more\n"
		"same:\n"
		"\tjmp join\n"
		"more:\n"
		"\tjmp join\n"
		"join:\n"
		"\t%p = phi i8 [same, 0], [more, 1]\n"
		"\t%z = zext i8 %p to i32\n"
		"\tjmp done\n"
		"done:\n"
		"\t%r = phi i32 [less, -1], [join, %z]\n"
		"\tret i32 %r\n"
		"}\n");

	//An extension undone: trunc (sext x) is x, and sext (trunc p) is p when p is already a sign extended i8 (a phi of sexts
	//and a constant); zext (trunc q) isn't q when q may be wider.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @e(i8 %a, i8 %b, i1 %c, i32 %q)\n"
		"{\n"
		"x:\n"
		"\tbrif %c, y, z\n"
		"y:\n"
		"\t%sa = sext i8 %a to i32\n"
		"\tjmp z\n"
		"z:\n"
		"\t%p = phi i32 [x, 5], [y, %sa]\n"
		"\t%u = trunc i32 %q to i8\n"
		"\t%v = zext i8 %u to i32\n"
		"\t%r = add i32 %p, %v\n"
		"\t%sb = sext i8 %b to i32\n"
		"\t%s = add i32 %r, %sb\n"
		"\tret i32 %s\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @e(i8 %a, i8 %b, i1 %c, i32 %q)\n"
		"{\n"
		"x:\n"
		"\tbrif %c, y, z\n"
		"y:\n"
		"\t%sa = sext i8 %a to i32\n"
		"\tjmp z\n"
		"z:\n"
		"\t%p = phi i32 [x, 5], [y, %sa]\n"
		"\t%t = trunc i32 %p to i8\n"
		"\t%e = sext i8 %t to i32\n"
		"\t%u = trunc i32 %q to i8\n"
		"\t%v = zext i8 %u to i32\n"
		"\t%r = add i32 %e, %v\n"
		"\t%sb = sext i8 %b to i32\n"
		"\t%tb = trunc i32 %sb to i8\n"
		"\t%eb = sext i8 %tb to i32\n"
		"\t%s = add i32 %r, %eb\n"
		"\tret i32 %s\n"
		"}\n");

	//A loop entered by a brif gets a block on the way in for what it hoists: *p is read once.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @w(ptr %p, i32 %n)\n"
		"{\n"
		"a:\n"
		"\t%c = slt i32 %n, 1\n"
		"\tbrif %c, out, 0\n"
		"loop:\n"
		"\t%i = phi i32 [0, 0], [loop, %i2]\n"
		"\t%s = phi i32 [0, 0], [loop, %s2]\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, out\n"
		"out:\n"
		"\t%r = phi i32 [a, 0], [loop, %s2]\n"
		"\tret i32 %r\n"
		"0:\n"
		"\t%v = load i32 %p\n"
		"\tjmp loop\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @w(ptr %p, i32 %n)\n"
		"{\n"
		"a:\n"
		"\t%c = slt i32 %n, 1\n"
		"\tbrif %c, out, loop\n"
		"loop:\n"
		"\t%i = phi i32 [a, 0], [loop, %i2]\n"
		"\t%s = phi i32 [a, 0], [loop, %s2]\n"
		"\t%v = load i32 %p\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%d = slt i32 %i2, %n\n"
		"\tbrif %d, loop, out\n"
		"out:\n"
		"\t%r = phi i32 [a, 0], [loop, %s2]\n"
		"\tret i32 %r\n"
		"}\n");

	//Unsigned / and % by a power of two are a shift and an and; signed / is a shift after 7 is added to a negative
	//value (so it rounds towards 0).
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @v(i32 %a)\n"
		"{\n"
		"x:\n"
		"\t%q = lshr i32 %a, 3\n"
		"\t%r = and i32 %a, 7\n"
		"\t%0 = ashr i32 %a, 31\n"
		"\t%1 = lshr i32 %0, 29\n"
		"\t%2 = add i32 %a, %1\n"
		"\t%s = ashr i32 %2, 3\n"
		"\t%t = add i32 %q, %r\n"
		"\t%u = add i32 %t, %s\n"
		"\tret i32 %u\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @v(i32 %a)\n"
		"{\n"
		"x:\n"
		"\t%q = udiv i32 %a, 8\n"
		"\t%r = urem i32 %a, 8\n"
		"\t%s = sdiv i32 %a, 8\n"
		"\t%t = add i32 %q, %r\n"
		"\t%u = add i32 %t, %s\n"
		"\tret i32 %u\n"
		"}\n");

	//Without indexed addresses (the W65C816), p + i in a loop counting i up by 1 is a pointer stepping by 1.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = slt i32 0, %n\n"
		"\tbrif %0, body, exit\n"
		"body:\n"
		"\t%1 = phi i32 [body, %sum], [entry, 0]\n"
		"\t%2 = phi i32 [body, %next], [entry, 0]\n"
		"\t%3 = phi ptr [body, %4], [entry, %p]\n"
		"\t%v = load i8 %3\n"
		"\t%w = zext i8 %v to i32\n"
		"\t%sum = add i32 %1, %w\n"
		"\t%next = add i32 %2, 1\n"
		"\t%4 = ptradd ptr %3, 1\n"
		"\t%5 = slt i32 %next, %n\n"
		"\tbrif %5, body, exit\n"
		"exit:\n"
		"\t%6 = phi i32 [entry, 0], [body, %sum]\n"
		"\tret i32 %6\n"
		"}\n",
		"function i32 @_Sum__FPii(ptr %p, i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp test\n"
		"test:\n"
		"\t%s = phi i32 [entry, 0], [body, %sum]\n"
		"\t%i = phi i32 [entry, 0], [body, %next]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%a = ptradd ptr %p, %i\n"
		"\t%v = load i8 %a\n"
		"\t%w = zext i8 %v to i32\n"
		"\t%sum = add i32 %s, %w\n"
		"\t%next = add i32 %i, 1\n"
		"\tjmp test\n"
		"exit:\n"
		"\tret i32 %s\n"
		"}\n", false);

	//A loop storing 7 to p[i] for i from 0 to n is memset(p, 7, n).
	AssertIROptimised(
		"space default 4\n"
		"regcall @_memset\n"
		"\n"
		"function void @_F__FPci(ptr %p, i32 %n)\n"
		"{\n"
		"e:\n"
		"\t%0 = slt i32 0, %n\n"
		"\tbrif %0, 0, x\n"
		"x:\n"
		"\tret\n"
		"0:\n"
		"\t%1 = call ptr @_memset(ptr %p, i16 7, i32 %n)\n"
		"\tret\n"
		"}\n",
		"function void @_F__FPci(ptr %p, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, b, x\n"
		"b:\n"
		"\t%a = ptradd ptr %p, %i\n"
		"\tstore i8 7, %a\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret\n"
		"}\n", true, true);

	//A loop copying q[i] to p[i], 16 bits each, is memmove when p is at or below q or past the end of what it reads.
	AssertIROptimised(
		"space default 4\n"
		"regcall @_memmove\n"
		"\n"
		"function void @_C__FPsPsj(ptr %p, ptr %q, i32 %n)\n"
		"{\n"
		"e:\n"
		"\t%0 = ult i32 0, %n\n"
		"\tbrif %0, 0, x\n"
		"b:\n"
		"\t%1 = phi i32 [0, 0], [b, %i2]\n"
		"\t%2 = phi ptr [0, %q], [b, %4]\n"
		"\t%3 = phi ptr [0, %p], [b, %5]\n"
		"\t%v = load i16 %2\n"
		"\tstore i16 %v, %3\n"
		"\t%i2 = add i32 %1, 1\n"
		"\t%4 = ptradd ptr %2, 2\n"
		"\t%5 = ptradd ptr %3, 2\n"
		"\t%6 = ult i32 %i2, %n\n"
		"\tbrif %6, b, x\n"
		"x:\n"
		"\tret\n"
		"0:\n"
		"\t%7 = shl i32 %n, 1\n"
		"\t%8 = ule ptr %p, %q\n"
		"\t%9 = ptradd ptr %q, %7\n"
		"\t%10 = uge ptr %p, %9\n"
		"\t%11 = or i1 %8, %10\n"
		"\tbrif %11, 1, b\n"
		"1:\n"
		"\t%12 = call ptr @_memmove(ptr %p, ptr %q, i32 %7)\n"
		"\tret\n"
		"}\n",
		"function void @_C__FPsPsj(ptr %p, ptr %q, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%c = ult i32 %i, %n\n"
		"\tbrif %c, b, x\n"
		"b:\n"
		"\t%o = shl i32 %i, 1\n"
		"\t%s = ptradd ptr %q, %o\n"
		"\t%v = load i16 %s\n"
		"\t%d = ptradd ptr %p, %o\n"
		"\tstore i16 %v, %d\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret\n"
		"}\n", true, true);

	//Counting bytes until a 0 is strlen: the index it stops at is the length.
	AssertIROptimised(
		"space default 4\n"
		"regcall @_strlen\n"
		"\n"
		"function i32 @_L__FPc(ptr %s)\n"
		"{\n"
		"e:\n"
		"\t%0 = load i8 %s\n"
		"\t%1 = ne i8 %0, 0\n"
		"\tbrif %1, 0, x\n"
		"x:\n"
		"\tret i32 0\n"
		"0:\n"
		"\t%2 = ptradd ptr %s, 1\n"
		"\t%3 = call i32 @_strlen(ptr %2)\n"
		"\t%4 = add i32 %3, 1\n"
		"\tret i32 %4\n"
		"}\n",
		"function i32 @_L__FPc(ptr %s)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%a = ptradd ptr %s, %i\n"
		"\t%c = load i8 %a\n"
		"\t%z = ne i8 %c, 0\n"
		"\tbrif %z, b, x\n"
		"b:\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %i\n"
		"}\n", true, true);

	//Comparing p[i] with q[i] for i from 0 to n, leaving when they differ, is memcmp(p, q, n) != 0.
	AssertIROptimised(
		"space default 4\n"
		"regcall @_memcmp\n"
		"\n"
		"function i32 @_E__FPcPcj(ptr %p, ptr %q, i32 %n)\n"
		"{\n"
		"e:\n"
		"\t%0 = ult i32 0, %n\n"
		"\tbrif %0, 0, same\n"
		"differ:\n"
		"\tret i32 0\n"
		"same:\n"
		"\tret i32 1\n"
		"0:\n"
		"\t%1 = call i16 @_memcmp(ptr %p, ptr %q, i32 %n)\n"
		"\t%2 = ne i16 %1, 0\n"
		"\tbrif %2, differ, same\n"
		"}\n",
		"function i32 @_E__FPcPcj(ptr %p, ptr %q, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%c = ult i32 %i, %n\n"
		"\tbrif %c, h, same\n"
		"h:\n"
		"\t%a = ptradd ptr %p, %i\n"
		"\t%a2 = ptradd ptr %q, %i\n"
		"\t%x = load i8 %a\n"
		"\t%y = load i8 %a2\n"
		"\t%d = ne i8 %x, %y\n"
		"\tbrif %d, differ, b\n"
		"b:\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp t\n"
		"differ:\n"
		"\tret i32 0\n"
		"same:\n"
		"\tret i32 1\n"
		"}\n", true, true);

	//Looking at p[i] for i from 0 to n, leaving with &p[i] when it is v, is memchr(p, v, n) (p + i made a pointer phi first).
	AssertIROptimised(
		"space default 4\n"
		"regcall @_memchr\n"
		"\n"
		"function ptr @_F__FPchj(ptr %p, i8 %v, i32 %n)\n"
		"{\n"
		"e:\n"
		"\t%0 = ult i32 0, %n\n"
		"\tbrif %0, 0, none\n"
		"none:\n"
		"\tret ptr 0\n"
		"0:\n"
		"\t%1 = zext i8 %v to i16\n"
		"\t%2 = call ptr @_memchr(ptr %p, i16 %1, i32 %n)\n"
		"\t%3 = eq ptr %2, 0\n"
		"\tbrif %3, none, 1\n"
		"1:\n"
		"\tret ptr %2\n"
		"}\n",
		"function ptr @_F__FPchj(ptr %p, i8 %v, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%c = ult i32 %i, %n\n"
		"\tbrif %c, h, none\n"
		"h:\n"
		"\t%a = ptradd ptr %p, %i\n"
		"\t%x = load i8 %a\n"
		"\t%d = eq i8 %x, %v\n"
		"\tbrif %d, found, b\n"
		"b:\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp t\n"
		"found:\n"
		"\tret ptr %a\n"
		"none:\n"
		"\tret ptr 0\n"
		"}\n", false, true);

	//Unsigned divides by constants of values known to be small: i < 10 in the loop (the latch goes round while i + 1 < 10),
	//so (i + 1) % 10 is a compare and subtract, i / 10 is 0 and i % 10 is i; a zext of a byte % 300 is the byte; n % 10 stays.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @t(ptr %p, i8 %b, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp h\n"
		"h:\n"
		"\t%i = phi i32 [e, 0], [h, %a]\n"
		"\t%a = add i32 %i, 1\n"
		"\t%0 = uge i32 %a, 10\n"
		"\t%1 = zext i1 %0 to i32\n"
		"\t%2 = neg i32 %1\n"
		"\t%3 = and i32 %2, 10\n"
		"\t%r = sub i32 %a, %3\n"
		"\t%s2 = add i32 %r, %i\n"
		"\tstore volatile i32 %s2, %p\n"
		"\t%c = ult i32 %a, 10\n"
		"\tbrif %c, h, x\n"
		"x:\n"
		"\t%z = zext i8 %b to i32\n"
		"\t%w = urem i32 %n, 10\n"
		"\tstore volatile i32 %z, %p\n"
		"\tstore volatile i32 %w, %p\n"
		"\tret\n"
		"}\n",
		"function void @t(ptr %p, i8 %b, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp h\n"
		"h:\n"
		"\t%i = phi i32 [e, 0], [h, %i2]\n"
		"\t%a = add i32 %i, 1\n"
		"\t%r = urem i32 %a, 10\n"
		"\t%d = udiv i32 %i, 10\n"
		"\t%m = urem i32 %i, 10\n"
		"\t%s = add i32 %r, %d\n"
		"\t%s2 = add i32 %s, %m\n"
		"\tstore volatile i32 %s2, %p\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%c = ult i32 %i2, 10\n"
		"\tbrif %c, h, x\n"
		"x:\n"
		"\t%z = zext i8 %b to i32\n"
		"\t%y = urem i32 %z, 300\n"
		"\t%w = urem i32 %n, 10\n"
		"\tstore volatile i32 %y, %p\n"
		"\tstore volatile i32 %w, %p\n"
		"\tret\n"
		"}\n");

	//Sums that cancel are what is left: (a + b) - b, (b + a) - b, a + (b - a), (b - a) + a and p + (q - p).
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @t(i32 %a, i32 %b, ptr %p, ptr %q)\n"
		"{\n"
		"e:\n"
		"\tstore ptr %q, %p\n"
		"\tret i32 %b\n"
		"}\n",
		"function i32 @t(i32 %a, i32 %b, ptr %p, ptr %q)\n"
		"{\n"
		"e:\n"
		"\t%0 = add i32 %a, %b\n"
		"\t%1 = sub i32 %0, %b\n"
		"\t%2 = add i32 %b, %1\n"
		"\t%3 = sub i32 %2, %b\n"
		"\t%4 = sub i32 %b, %3\n"
		"\t%5 = add i32 %3, %4\n"
		"\t%6 = sub i32 %5, %a\n"
		"\t%7 = add i32 %6, %a\n"
		"\t%8 = ptrtoi ptr %q to i32\n"
		"\t%9 = ptrtoi ptr %p to i32\n"
		"\t%10 = sub i32 %8, %9\n"
		"\t%11 = ptradd ptr %p, %10\n"
		"\tstore ptr %11, %p\n"
		"\tret i32 %7\n"
		"}\n");

	//The same returning i: memchr's offset from p.
	AssertIROptimised(
		"space default 4\n"
		"regcall @_memchr\n"
		"\n"
		"function i32 @_I__FPchj(ptr %p, i8 %v, i32 %n)\n"
		"{\n"
		"e:\n"
		"\t%0 = ult i32 0, %n\n"
		"\tbrif %0, 0, none\n"
		"none:\n"
		"\tret i32 -1\n"
		"0:\n"
		"\t%1 = zext i8 %v to i16\n"
		"\t%2 = call ptr @_memchr(ptr %p, i16 %1, i32 %n)\n"
		"\t%3 = eq ptr %2, 0\n"
		"\tbrif %3, none, 1\n"
		"1:\n"
		"\t%4 = ptrtoi ptr %2 to i32\n"
		"\t%5 = ptrtoi ptr %p to i32\n"
		"\t%6 = sub i32 %4, %5\n"
		"\tret i32 %6\n"
		"}\n",
		"function i32 @_I__FPchj(ptr %p, i8 %v, i32 %n)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [e, 0], [b, %i2]\n"
		"\t%c = ult i32 %i, %n\n"
		"\tbrif %c, h, none\n"
		"h:\n"
		"\t%a = ptradd ptr %p, %i\n"
		"\t%x = load i8 %a\n"
		"\t%d = eq i8 %x, %v\n"
		"\tbrif %d, found, b\n"
		"b:\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp t\n"
		"found:\n"
		"\tret i32 %i\n"
		"none:\n"
		"\tret i32 -1\n"
		"}\n", true, true);

	//Stepping a pointer until it points at v, or at a 0 first, is strchr(s, v) (with v being 0 found at the first 0).
	AssertIROptimised(
		"space default 4\n"
		"regcall @_strchr\n"
		"\n"
		"function ptr @_C__FPcc(ptr %s, i8 %v)\n"
		"{\n"
		"e:\n"
		"\t%0 = load i8 %s\n"
		"\t%1 = ne i8 %0, %v\n"
		"\tbrif %1, 0, found\n"
		"found:\n"
		"\t%2 = phi ptr [e, %s], [1, %6]\n"
		"\tret ptr %2\n"
		"none:\n"
		"\tret ptr 0\n"
		"0:\n"
		"\t%3 = eq i8 %0, 0\n"
		"\tbrif %3, none, 1\n"
		"1:\n"
		"\t%4 = zext i8 %v to i16\n"
		"\t%5 = ptradd ptr %s, 1\n"
		"\t%6 = call ptr @_strchr(ptr %5, i16 %4)\n"
		"\t%7 = eq ptr %6, 0\n"
		"\tbrif %7, none, found\n"
		"}\n",
		"function ptr @_C__FPcc(ptr %s, i8 %v)\n"
		"{\n"
		"e:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%p = phi ptr [e, %s], [b, %p2]\n"
		"\t%c = load i8 %p\n"
		"\t%d = ne i8 %c, %v\n"
		"\tbrif %d, h, found\n"
		"h:\n"
		"\t%z = eq i8 %c, 0\n"
		"\tbrif %z, none, b\n"
		"b:\n"
		"\t%p2 = ptradd ptr %p, 1\n"
		"\tjmp t\n"
		"found:\n"
		"\tret ptr %p\n"
		"none:\n"
		"\tret ptr 0\n"
		"}\n", true, true);

	//j's phi is 0 coming from z, so z goes straight to n (and j, left with one way in, joins m).
	AssertIROptimised(
		"function i32 @t(i32 %a, i32 %b)\n"
		"{\n"
		"e:\n"
		"\t%c = slt i32 %a, 0\n"
		"\tbrif %c, n, m\n"
		"m:\n"
		"\t%d = add i32 %a, %b\n"
		"\t%q = ne i32 %d, 0\n"
		"\tbrif %q, y, n\n"
		"y:\n"
		"\tret i32 1\n"
		"n:\n"
		"\tret i32 2\n"
		"}\n",
		"function i32 @t(i32 %a, i32 %b)\n"
		"{\n"
		"e:\n"
		"\t%c = slt i32 %a, 0\n"
		"\tbrif %c, z, m\n"
		"z:\n"
		"\tjmp j\n"
		"m:\n"
		"\t%d = add i32 %a, %b\n"
		"\tjmp j\n"
		"j:\n"
		"\t%p = phi i32 [z, 0], [m, %d]\n"
		"\t%q = ne i32 %p, 0\n"
		"\tbrif %q, y, n\n"
		"y:\n"
		"\tret i32 1\n"
		"n:\n"
		"\tret i32 2\n"
		"}\n");

	//b + a is a + b worked out before; b - a isn't a - b.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @w(i32 %a, i32 %b)\n"
		"{\n"
		"x:\n"
		"\t%s = add i32 %a, %b\n"
		"\t%d = sub i32 %a, %b\n"
		"\t%e = sub i32 %b, %a\n"
		"\t%u = mul i32 %s, %s\n"
		"\t%v = mul i32 %d, %e\n"
		"\t%w = add i32 %u, %v\n"
		"\tret i32 %w\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @w(i32 %a, i32 %b)\n"
		"{\n"
		"x:\n"
		"\t%s = add i32 %a, %b\n"
		"\t%t = add i32 %b, %a\n"
		"\t%d = sub i32 %a, %b\n"
		"\t%e = sub i32 %b, %a\n"
		"\t%u = mul i32 %s, %t\n"
		"\t%v = mul i32 %d, %e\n"
		"\t%w = add i32 %u, %v\n"
		"\tret i32 %w\n"
		"}\n");

	//After !(a < b), a > b is a != b, a >= b is true and b <= a too.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @u(i32 %a, i32 %b)\n"
		"{\n"
		"x:\n"
		"\t%l = slt i32 %a, %b\n"
		"\tbrif %l, y, z\n"
		"y:\n"
		"\tret i32 -1\n"
		"z:\n"
		"\t%g = ne i32 %a, %b\n"
		"\t%r = zext i1 %g to i32\n"
		"\t%s = add i32 %r, 1\n"
		"\tret i32 %s\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @u(i32 %a, i32 %b)\n"
		"{\n"
		"x:\n"
		"\t%l = slt i32 %a, %b\n"
		"\tbrif %l, y, z\n"
		"y:\n"
		"\tret i32 -1\n"
		"z:\n"
		"\t%g = sgt i32 %a, %b\n"
		"\t%r = zext i1 %g to i32\n"
		"\t%e = sge i32 %a, %b\n"
		"\t%f = sle i32 %b, %a\n"
		"\t%h = and i1 %e, %f\n"
		"\t%k = zext i1 %h to i32\n"
		"\t%s = add i32 %r, %k\n"
		"\tret i32 %s\n"
		"}\n");

	//A load from a constant table at a constant offset is what is there: a call through it is direct.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"global @_gFns, align 4, constant { ptr @_One, ptr @_Two, i32 7 }\n"
		"\n"
		"function i32 @_F()\n"
		"{\n"
		"b:\n"
		"\t%r = call i32 @_Two()\n"
		"\tret i32 %r\n"
		"}\n",
		"space default 4\n"
		"\n"
		"global @_gFns, align 4, constant { ptr @_One, ptr @_Two, i32 7 }\n"
		"\n"
		"function i32 @_F()\n"
		"{\n"
		"b:\n"
		"\t%a = ptradd ptr @_gFns, 4\n"
		"\t%p = load ptr %a\n"
		"\t%r = call i32 %p()\n"
		"\tret i32 %r\n"
		"}\n");

	//After both ways into m read [p] as %x the load in m is %x; %k2 is %k.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_F(ptr %p, i1 %c)\n"
		"{\n"
		"b:\n"
		"\t%x = load i32 %p\n"
		"\tbrif %c, l, r\n"
		"l:\n"
		"\t%u = add i32 %x, 1\n"
		"\tjmp m\n"
		"r:\n"
		"\t%w = add i32 %x, 2\n"
		"\tjmp m\n"
		"m:\n"
		"\t%k = phi i32 [l, %u], [r, %w]\n"
		"\t%s = add i32 %x, %k\n"
		"\t%t = add i32 %s, %k\n"
		"\tret i32 %t\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_F(ptr %p, i1 %c)\n"
		"{\n"
		"b:\n"
		"\t%x = load i32 %p\n"
		"\tbrif %c, l, r\n"
		"l:\n"
		"\t%u = add i32 %x, 1\n"
		"\tjmp m\n"
		"r:\n"
		"\t%w = add i32 %x, 2\n"
		"\tjmp m\n"
		"m:\n"
		"\t%k = phi i32 [l, %u], [r, %w]\n"
		"\t%k2 = phi i32 [l, %u], [r, %w]\n"
		"\t%y = load i32 %p\n"
		"\t%s = add i32 %y, %k\n"
		"\t%t = add i32 %s, %k2\n"
		"\tret i32 %t\n"
		"}\n");

	//p + (i << 2) in a loop counting i up by 1 is a pointer stepping by 4.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_Sum(ptr %p, i32 %n)\n"
		"{\n"
		"b:\n"
		"\t%0 = slt i32 0, %n\n"
		"\tbrif %0, l, x\n"
		"l:\n"
		"\t%1 = phi i32 [l, %j], [b, 0]\n"
		"\t%2 = phi i32 [l, %s2], [b, 0]\n"
		"\t%3 = phi ptr [l, %4], [b, %p]\n"
		"\t%v = load i32 %3\n"
		"\t%s2 = add i32 %2, %v\n"
		"\t%j = add i32 %1, 1\n"
		"\t%4 = ptradd ptr %3, 4\n"
		"\t%5 = slt i32 %j, %n\n"
		"\tbrif %5, l, x\n"
		"x:\n"
		"\t%6 = phi i32 [b, 0], [l, %s2]\n"
		"\tret i32 %6\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_Sum(ptr %p, i32 %n)\n"
		"{\n"
		"b:\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [b, 0], [l, %j]\n"
		"\t%s = phi i32 [b, 0], [l, %s2]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, l, x\n"
		"l:\n"
		"\t%o = shl i32 %i, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i32 %a\n"
		"\t%s2 = add i32 %s, %v\n"
		"\t%j = add i32 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %s\n"
		"}\n");

	//A loop round while i <= n, n an extended i16: round while i < n + 1, the add before the loop.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_S(i16 %n, ptr %p)\n"
		"{\n"
		"b:\n"
		"\t%m = sext i16 %n to i32\n"
		"\t%0 = sgt i16 0, %n\n"
		"\tbrif %0, x, 0\n"
		"l:\n"
		"\t%1 = phi i32 [0, 0], [l, %j]\n"
		"\tstore i32 %1, %p\n"
		"\t%j = add i32 %1, 1\n"
		"\t%2 = slt i32 %j, %4\n"
		"\tbrif %2, l, x\n"
		"x:\n"
		"\t%3 = phi i32 [b, 0], [l, %j]\n"
		"\tret i32 %3\n"
		"0:\n"
		"\t%4 = add i32 %m, 1\n"
		"\tjmp l\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_S(i16 %n, ptr %p)\n"
		"{\n"
		"b:\n"
		"\t%m = sext i16 %n to i32\n"
		"\tjmp t\n"
		"t:\n"
		"\t%i = phi i32 [b, 0], [l, %j]\n"
		"\t%c = sgt i32 %i, %m\n"
		"\tbrif %c, x, l\n"
		"l:\n"
		"\tstore i32 %i, %p\n"
		"\t%j = add i32 %i, 1\n"
		"\tjmp t\n"
		"x:\n"
		"\tret i32 %i\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's NarrowCounters: an i32 counter from 0 up to a bound that
// is an i16 sign extended counts in an i16 phi, compared with the i16
// bound; its other use (the address) gets it sign extended.  Without the
// option the counter stays 32 bits (and strength reduction steps the
// address instead).
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserNarrowCounters(void)
{
	static const char*	szText =
		"function i32 @_S__FPis(ptr %p, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%w = sext i16 %n to i32\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%c = slt i32 %i, %w\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\t%o = shl i32 %i, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\tstore i32 7, %a\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp loop\n"
		"exit:\n"
		"\tret i32 0\n"
		"}\n";

	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function i32 @_S__FPis(ptr %p, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = slt i16 0, %n\n"
		"\tbrif %0, body, exit\n"
		"body:\n"
		"\t%1 = phi i16 [entry, 0], [body, %3]\n"
		"\t%2 = sext i16 %1 to i32\n"
		"\t%o = shl i32 %2, 2\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\tstore i32 7, %a\n"
		"\t%3 = add i16 %1, 1\n"
		"\t%4 = slt i16 %3, %n\n"
		"\tbrif %4, body, exit\n"
		"exit:\n"
		"\tret i32 0\n"
		"}\n", szText, IR_TEST_INDEXED_ADDRESSES | IR_TEST_NARROW_COUNTERS);

	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function i32 @_S__FPis(ptr %p, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%w = sext i16 %n to i32\n"
		"\t%0 = slt i16 0, %n\n"
		"\tbrif %0, body, exit\n"
		"body:\n"
		"\t%1 = phi i32 [body, %i2], [entry, 0]\n"
		"\t%2 = phi ptr [body, %3], [entry, %p]\n"
		"\tstore i32 7, %2\n"
		"\t%i2 = add i32 %1, 1\n"
		"\t%3 = ptradd ptr %2, 4\n"
		"\t%4 = slt i32 %i2, %w\n"
		"\tbrif %4, body, exit\n"
		"exit:\n"
		"\tret i32 0\n"
		"}\n", szText, IR_TEST_INDEXED_ADDRESSES);
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's BiasSignedIndices.  p + sext(i) used only as a load's
// address is (p - 32768) + zext(i ^ 32768).  A scaled index p + sext(h) * 2
// with h = x >> 2 (so -8192 to 8191) is the bounded form: (p - 16384) +
// zext(h * 2 + 16384).  A scaled index of any i16 doesn't fit 16 bits and
// stays as it is.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserBiasSignedIndices(void)
{
	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function i16 @_L__FPss(ptr %p, i16 %i)\n"
		"{\n"
		"entry:\n"
		"\t%0 = ptradd ptr %p, -32768\n"
		"\t%1 = xor i16 %i, -32768\n"
		"\t%2 = zext i16 %1 to i32\n"
		"\t%a = ptradd ptr %0, %2\n"
		"\t%v = load i16 %a\n"
		"\tret i16 %v\n"
		"}\n",
		"function i16 @_L__FPss(ptr %p, i16 %i)\n"
		"{\n"
		"entry:\n"
		"\t%w = sext i16 %i to i32\n"
		"\t%a = ptradd ptr %p, %w\n"
		"\t%v = load i16 %a\n"
		"\tret i16 %v\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_BIASED_INDICES);

	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function i16 @_L__FPss(ptr %p, i16 %x)\n"
		"{\n"
		"entry:\n"
		"\t%h = ashr i16 %x, 2\n"
		"\t%0 = ptradd ptr %p, -16384\n"
		"\t%1 = shl i16 %h, 1\n"
		"\t%2 = add i16 %1, 16384\n"
		"\t%3 = zext i16 %2 to i32\n"
		"\t%a = ptradd ptr %0, %3\n"
		"\t%v = load i16 %a\n"
		"\tret i16 %v\n"
		"}\n",
		"function i16 @_L__FPss(ptr %p, i16 %x)\n"
		"{\n"
		"entry:\n"
		"\t%h = ashr i16 %x, 2\n"
		"\t%w = sext i16 %h to i32\n"
		"\t%o = shl i32 %w, 1\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i16 %a\n"
		"\tret i16 %v\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_BIASED_INDICES);

	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function i16 @_L__FPss(ptr %p, i16 %x)\n"
		"{\n"
		"entry:\n"
		"\t%w = sext i16 %x to i32\n"
		"\t%o = shl i32 %w, 1\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i16 %a\n"
		"\tret i16 %v\n"
		"}\n",
		"function i16 @_L__FPss(ptr %p, i16 %x)\n"
		"{\n"
		"entry:\n"
		"\t%w = sext i16 %x to i32\n"
		"\t%o = shl i32 %w, 1\n"
		"\t%a = ptradd ptr %p, %o\n"
		"\t%v = load i16 %a\n"
		"\tret i16 %v\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_BIASED_INDICES);
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's MergeAddressPhis: p = phi(b + sext(x), b + sext(y)) used
// only as a store's address is b + sext(phi(x, y)), which BiasSignedIndices
// then biases.  A phi that is also returned stays a phi of pointers (and
// isn't biased).
//
//////////////////////////////////////////////////////////////////////////
//
// x % d next to x / d (d not a constant) is x - (x / d) * d, one divide
// (ShareDivisions, with the W65C816's options), the divide first or not.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserSharedDivisions(void)
{
	AssertIROptimisedOptions(
		"function i32 @f(i32 %x, i32 %d)\n"
		"{\n"
		"a:\n"
		"\t%q = udiv i32 %x, %d\n"
		"\t%0 = mul i32 %q, %d\n"
		"\t%r = sub i32 %x, %0\n"
		"\t%s = xor i32 %q, %r\n"
		"\tret i32 %s\n"
		"}\n",
		"function i32 @f(i32 %x, i32 %d)\n"
		"{\n"
		"a:\n"
		"\t%q = udiv i32 %x, %d\n"
		"\t%r = urem i32 %x, %d\n"
		"\t%s = xor i32 %q, %r\n"
		"\tret i32 %s\n"
		"}\n", IR_TEST_CONSTANT_DIVISIONS);

	AssertIROptimisedOptions(
		"function i16 @f(i16 %x, i16 %d)\n"
		"{\n"
		"a:\n"
		"\t%0 = sdiv i16 %x, %d\n"
		"\t%1 = mul i16 %0, %d\n"
		"\t%r = sub i16 %x, %1\n"
		"\t%s = xor i16 %0, %r\n"
		"\tret i16 %s\n"
		"}\n",
		"function i16 @f(i16 %x, i16 %d)\n"
		"{\n"
		"a:\n"
		"\t%r = srem i16 %x, %d\n"
		"\t%q = sdiv i16 %x, %d\n"
		"\t%s = xor i16 %q, %r\n"
		"\tret i16 %s\n"
		"}\n", IR_TEST_CONSTANT_DIVISIONS);

	//The 65C64's div leaves an unsigned remainder (SetDivideRemainders): a urem moves to just after its udiv, by a
	//constant too (it has no ConstantDivisions), or a udiv is put before it and the later one is the same value.  Not
	//64 bits or signed.
	AssertIROptimisedOptions(
		"function i32 @f(i32 %x, i32 %d, i64 %y, i64 %e, i16 %z, i16 %f)\n"
		"{\n"
		"a:\n"
		"\t%q = udiv i32 %x, %d\n"
		"\t%r = urem i32 %x, %d\n"
		"\t%m = mul i32 %q, 1000\n"
		"\t%t = udiv i32 %x, 10\n"
		"\t%u = urem i32 %x, 10\n"
		"\t%0 = udiv i16 %z, %f\n"
		"\t%w = urem i16 %z, %f\n"
		"\t%p = udiv i64 %y, %e\n"
		"\t%n = add i32 %m, %r\n"
		"\t%o = urem i64 %y, %e\n"
		"\t%k = srem i32 %x, %d\n"
		"\t%g = sdiv i32 %x, %d\n"
		"\t%1 = add i32 %n, %t\n"
		"\t%2 = add i32 %1, %u\n"
		"\t%3 = zext i16 %w to i32\n"
		"\t%4 = add i32 %2, %3\n"
		"\t%5 = zext i16 %0 to i32\n"
		"\t%6 = add i32 %4, %5\n"
		"\t%7 = add i64 %p, %o\n"
		"\t%8 = trunc i64 %7 to i32\n"
		"\t%9 = add i32 %6, %8\n"
		"\t%10 = add i32 %9, %k\n"
		"\t%11 = add i32 %10, %g\n"
		"\tret i32 %11\n"
		"}\n",
		"function i32 @f(i32 %x, i32 %d, i64 %y, i64 %e, i16 %z, i16 %f)\n"
		"{\n"
		"a:\n"
		"\t%q = udiv i32 %x, %d\n"
		"\t%m = mul i32 %q, 1000\n"
		"\t%t = udiv i32 %x, 10\n"
		"\t%r = urem i32 %x, %d\n"
		"\t%u = urem i32 %x, 10\n"
		"\t%w = urem i16 %z, %f\n"
		"\t%v = udiv i16 %z, %f\n"
		"\t%p = udiv i64 %y, %e\n"
		"\t%n = add i32 %m, %r\n"
		"\t%o = urem i64 %y, %e\n"
		"\t%k = srem i32 %x, %d\n"
		"\t%g = sdiv i32 %x, %d\n"
		"\t%1 = add i32 %n, %t\n"
		"\t%2 = add i32 %1, %u\n"
		"\t%3 = zext i16 %w to i32\n"
		"\t%4 = add i32 %2, %3\n"
		"\t%5 = zext i16 %v to i32\n"
		"\t%6 = add i32 %4, %5\n"
		"\t%7 = add i64 %p, %o\n"
		"\t%8 = trunc i64 %7 to i32\n"
		"\t%9 = add i32 %6, %8\n"
		"\t%10 = add i32 %9, %k\n"
		"\t%11 = add i32 %10, %g\n"
		"\tret i32 %11\n"
		"}\n", IR_TEST_DIVIDE_REMAINDERS);
}


//////////////////////////////////////////////////////////////////////////
void TestIROptimiserMergeAddressPhis(void)
{
	static const char*	szText =
		"function RESULT @_M__FPcbss(ptr %b, i1 %c, i16 %x, i16 %y)\n"
		"{\n"
		"entry:\n"
		"\tbrif %c, left, right\n"
		"left:\n"
		"\t%wx = sext i16 %x to i32\n"
		"\t%px = ptradd ptr %b, %wx\n"
		"\tjmp join\n"
		"right:\n"
		"\t%wy = sext i16 %y to i32\n"
		"\t%py = ptradd ptr %b, %wy\n"
		"\tjmp join\n"
		"join:\n"
		"\t%p = phi ptr [left, %px], [right, %py]\n"
		"\tstore i8 7, %p\n"
		"\tRETURN\n"
		"}\n";
	CChars				szInput;

	szInput.Init(szText);
	szInput.Replace("RESULT", "void");
	szInput.Replace("RETURN", "ret");
	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function void @_M__FPcbss(ptr %b, i1 %c, i16 %x, i16 %y)\n"
		"{\n"
		"entry:\n"
		"\tbrif %c, join, right\n"
		"right:\n"
		"\tjmp join\n"
		"join:\n"
		"\t%0 = phi i16 [right, %y], [entry, %x]\n"
		"\t%1 = ptradd ptr %b, -32768\n"
		"\t%2 = xor i16 %0, -32768\n"
		"\t%3 = zext i16 %2 to i32\n"
		"\t%4 = ptradd ptr %1, %3\n"
		"\tstore i8 7, %4\n"
		"\tret\n"
		"}\n", szInput.Text(), IR_TEST_INDEXED_ADDRESSES | IR_TEST_BIASED_INDICES);
	szInput.Kill();

	szInput.Init(szText);
	szInput.Replace("RESULT", "ptr");
	szInput.Replace("RETURN", "ret ptr %p");
	AssertIROptimisedOptions(
		"space default 4\n"
		"\n"
		"function ptr @_M__FPcbss(ptr %b, i1 %c, i16 %x, i16 %y)\n"
		"{\n"
		"entry:\n"
		"\tbrif %c, left, right\n"
		"left:\n"
		"\t%wx = sext i16 %x to i32\n"
		"\t%px = ptradd ptr %b, %wx\n"
		"\tjmp join\n"
		"right:\n"
		"\t%wy = sext i16 %y to i32\n"
		"\t%py = ptradd ptr %b, %wy\n"
		"\tjmp join\n"
		"join:\n"
		"\t%p = phi ptr [left, %px], [right, %py]\n"
		"\tstore i8 7, %p\n"
		"\tret ptr %p\n"
		"}\n", szInput.Text(), IR_TEST_INDEXED_ADDRESSES | IR_TEST_BIASED_INDICES);
	szInput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's ConstantDivisions: (x & 255) / 7 is (x & 255) * 293 >> 11
// (exact for 0 to 255).  x / 7 with x's range unknown stays a udiv.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserConstantDivisions(void)
{
	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 255\n"
		"\t%0 = mul i32 %a, 293\n"
		"\t%1 = lshr i32 %0, 11\n"
		"\tret i32 %1\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 255\n"
		"\t%d = udiv i32 %a, 7\n"
		"\tret i32 %d\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS);

	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%d = udiv i32 %x, 7\n"
		"\tret i32 %d\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%d = udiv i32 %x, 7\n"
		"\tret i32 %d\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS);
}


//////////////////////////////////////////////////////////////////////////
//
// ConstantDivisions with the W65C816's costs (SetDivisionCosts): a
// reciprocal is written only where its multiply and shifts cost fewer
// cycles than the divide's routine.  A byte times a byte, and 127, over 255
// is a multiply by 32897 (three digits: shifts and adds) and a shift.  x %
// 30000 of an x under 131072 has a reciprocal, written without the costs,
// that needs two 32 bit multiplies by many digits (calls themselves): it
// stays the divide.  x / 10 of an x under 1024 alone is a multiply; with x
// % 10 beside it, where the routine leaves the remainder, the two are one
// call and stay it, and where it leaves none (two calls) both are the
// multiply.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserDivisionCosts(void)
{
	CIRW65C816DivisionCosts	cCosts;
	SIRConstantDivision		sDivision;
	SIRReciprocal			sReciprocal;

	AssertIROptimisedOptions(
		"function i32 @_D__Fii(i32 %a, i32 %b)\n"
		"{\n"
		"entry:\n"
		"\t%x = and i32 %a, 255\n"
		"\t%y = and i32 %b, 255\n"
		"\t%p = mul i32 %x, %y\n"
		"\t%s = add i32 %p, 127\n"
		"\t%0 = mul i32 %s, 32897\n"
		"\t%1 = lshr i32 %0, 23\n"
		"\tret i32 %1\n"
		"}\n",
		"function i32 @_D__Fii(i32 %a, i32 %b)\n"
		"{\n"
		"entry:\n"
		"\t%x = and i32 %a, 255\n"
		"\t%y = and i32 %b, 255\n"
		"\t%p = mul i32 %x, %y\n"
		"\t%s = add i32 %p, 127\n"
		"\t%d = udiv i32 %s, 255\n"
		"\tret i32 %d\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS | IR_TEST_DIVISION_COSTS);

	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 131071\n"
		"\t%0 = lshr i32 %a, 4\n"
		"\t%1 = mul i32 %0, 2237\n"
		"\t%2 = lshr i32 %1, 22\n"
		"\t%3 = mul i32 %2, 30000\n"
		"\t%r = sub i32 %a, %3\n"
		"\tret i32 %r\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 131071\n"
		"\t%r = urem i32 %a, 30000\n"
		"\tret i32 %r\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS);
	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 131071\n"
		"\t%r = urem i32 %a, 30000\n"
		"\tret i32 %r\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 131071\n"
		"\t%r = urem i32 %a, 30000\n"
		"\tret i32 %r\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS | IR_TEST_DIVISION_COSTS);

	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 1023\n"
		"\t%0 = mul i32 %a, 205\n"
		"\t%1 = lshr i32 %0, 11\n"
		"\tret i32 %1\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 1023\n"
		"\t%q = udiv i32 %a, 10\n"
		"\tret i32 %q\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS | IR_TEST_DIVISION_COSTS);
	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 1023\n"
		"\t%q = udiv i32 %a, 10\n"
		"\t%r = urem i32 %a, 10\n"
		"\t%s = add i32 %q, %r\n"
		"\tret i32 %s\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 1023\n"
		"\t%q = udiv i32 %a, 10\n"
		"\t%r = urem i32 %a, 10\n"
		"\t%s = add i32 %q, %r\n"
		"\tret i32 %s\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS | IR_TEST_DIVISION_COSTS | IR_TEST_DIVIDE_REMAINDERS);
	AssertIROptimisedOptions(
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 1023\n"
		"\t%0 = mul i32 %a, 205\n"
		"\t%1 = lshr i32 %0, 11\n"
		"\t%2 = mul i32 %1, 10\n"
		"\t%r = sub i32 %a, %2\n"
		"\t%s = add i32 %1, %r\n"
		"\tret i32 %s\n"
		"}\n",
		"function i32 @_D__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = and i32 %x, 1023\n"
		"\t%q = udiv i32 %a, 10\n"
		"\t%r = urem i32 %a, 10\n"
		"\t%s = add i32 %q, %r\n"
		"\tret i32 %s\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_CONSTANT_DIVISIONS | IR_TEST_DIVISION_COSTS);

	//The cycles themselves.  (x * 32897) >> 23 for x up to 65152: 200 for the shifts and adds of two words, 22 for the
	//shift, 6 for x made two words; its divide is the 16 bit routine through R816_UDiv32, a byte of quotient: 140
	//and 8 steps of 18.
	sDivision.bSigned = false;
	sDivision.bRemainder = false;
	sDivision.lliDivisor = 255;
	sReciprocal.ulliMultiplier = 32897;
	sReciprocal.iShift = 23;
	sReciprocal.iBits = 32;
	sReciprocal.iPreShift = 0;
	AssertInt(228, cCosts.ReciprocalCost(IRType(IRTK_I32), &sDivision, 127, 65152, &sReciprocal));
	AssertInt(284, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 127, 65152));

	//An x past 65535 is the 32 bit routine: 16 steps for the low word, and for a high word not below the divisor 8
	//more (its high byte is below it) or 16.  A divisor under 128 steps a byte wide (14), one of 65536 or more two
	//words wide (46).
	AssertInt(140 + 16 * 18, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 0, 65536));
	AssertInt(140 + 24 * 18, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 0, 0x00FFFFFF));
	AssertInt(140 + 24 * 18, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 0, 0x7FFFFFFF));
	sDivision.lliDivisor = 10;
	AssertInt(110 + 8 * 14, cCosts.DivideCost(IRType(IRTK_I16), &sDivision, 0, 1023));
	AssertInt(110 + 16 * 14, cCosts.DivideCost(IRType(IRTK_I16), &sDivision, 0, 32767));
	AssertInt(140 + 32 * 14, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 0, 0x7FFFFFFF));
	sDivision.lliDivisor = 100000;
	AssertInt(140 + 8 * 46, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 0, 0x00FFFFFF));
	AssertInt(140 + 16 * 46, cCosts.DivideCost(IRType(IRTK_I32), &sDivision, 0, 0x7FFFFFFF));

	//A signed one takes the signs off and puts them back (80 or 100 more), and its reciprocal adds 1 to the quotient
	//of a negative x.  x % 3 of an i16 from -32767: x made two words (16), a 32 bit multiply by 10923 that is a call
	//(230), the shift by 15 (83), the 1 (40), and the quotient times 3 taken from x in 16 bits (36 and 12).
	sDivision.bSigned = true;
	sDivision.bRemainder = true;
	sDivision.lliDivisor = 3;
	sReciprocal.ulliMultiplier = 10923;
	sReciprocal.iShift = 15;
	AssertInt(417, cCosts.ReciprocalCost(IRType(IRTK_I16), &sDivision, -32767, 3, &sReciprocal));
	AssertInt(110 + 80 + 16 * 14, cCosts.DivideCost(IRType(IRTK_I16), &sDivision, -32767, 3));
}


//////////////////////////////////////////////////////////////////////////
//
// FoldNullTests: p + 4 on the way where p was tested not null isn't null;
// a slot's address isn't null.  A phi of p (tested) and q (not tested) may
// be null, so its test stays.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserFoldNullTests(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i1 @_N__FPi(ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%c = eq ptr %p, 0\n"
		"\tbrif %c, null, nonnull\n"
		"null:\n"
		"\tret i1 0\n"
		"nonnull:\n"
		"\tret i1 1\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i1 @_N__FPi(ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%c = eq ptr %p, 0\n"
		"\tbrif %c, null, nonnull\n"
		"null:\n"
		"\tret i1 0\n"
		"nonnull:\n"
		"\t%q = ptradd ptr %p, 4\n"
		"\t%d = ne ptr %q, 0\n"
		"\tret i1 %d\n"
		"}\n");

	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i1 @_N__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%s = slot ptr 4, 4\n"
		"\tcall void @_Use__FPi(ptr %s)\n"
		"\tret i1 0\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i1 @_N__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%s = slot ptr 4, 4\n"
		"\tcall void @_Use__FPi(ptr %s)\n"
		"\t%d = eq ptr %s, 0\n"
		"\tret i1 %d\n"
		"}\n");

	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i1 @_N__FPiPib(ptr %p, ptr %q, i1 %b)\n"
		"{\n"
		"entry:\n"
		"\t%c = ne ptr %p, 0\n"
		"\tbrif %c, test, join\n"
		"test:\n"
		"\tbrif %b, join, other\n"
		"other:\n"
		"\tjmp join\n"
		"join:\n"
		"\t%r = phi ptr [test, %p], [other, %q], [entry, %q]\n"
		"\t%d = ne ptr %r, 0\n"
		"\tret i1 %d\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i1 @_N__FPiPib(ptr %p, ptr %q, i1 %b)\n"
		"{\n"
		"entry:\n"
		"\t%c = ne ptr %p, 0\n"
		"\tbrif %c, test, other\n"
		"test:\n"
		"\tbrif %b, join, other\n"
		"other:\n"
		"\tjmp join\n"
		"join:\n"
		"\t%r = phi ptr [test, %p], [other, %q]\n"
		"\t%d = ne ptr %r, 0\n"
		"\tret i1 %d\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A brif on a value a branch on the way to its block already decided is a
// jmp, though the block does other things first (CIRBlockPasses::
// FoldBranches).
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserKnownBranches(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_K__Fbi(i1 %c, i32 %x)\n"
		"{\n"
		"entry:\n"
		"\tbrif %c, yes, no\n"
		"yes:\n"
		"\tcall void @_Use__Fi(i32 %x)\n"
		"\tret i32 1\n"
		"no:\n"
		"\tcall void @_Use__Fi(i32 3)\n"
		"\tret i32 2\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_K__Fbi(i1 %c, i32 %x)\n"
		"{\n"
		"entry:\n"
		"\tbrif %c, yes, no\n"
		"yes:\n"
		"\tcall void @_Use__Fi(i32 %x)\n"
		"\tbrif %c, a, b\n"
		"a:\n"
		"\tret i32 1\n"
		"b:\n"
		"\tret i32 2\n"
		"no:\n"
		"\tcall void @_Use__Fi(i32 3)\n"
		"\tbrif %c, a, b\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// NarrowMultiplies (the W65C816's) with what the branches on the way say of an index against values that aren't
// constants: after first <= index <= last, both bytes, index - first is 0 to 255, so (index - first) * 12 is a 16 bit
// multiply and the ptradd of it IRF_InArray (BoundIndices, which also puts a constant just added to the pointer into
// the offset).  The shifts, adds and subs a back end made of a multiply are still one
// (ExtendedMultiple); branches above an if and else joined still hold (the dominators); without first <= index, or
// with a last that isn't a byte, nothing is known.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserGuardedMultiples(void)
{
	static const char*	szGuards =
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n{\nentry:\n\t%first = zext i8 %f to i16\n\t%last = zext i8 %l to i16\n"
		"\t%a = uge i16 %i, %first\n\tbrif %a, upper, out\n"
		"upper:\n\t%b = ule i16 %i, %last\n\tbrif %b, in, out\n";

	CChars	szText;

	//(index - first) * 12.
	szText.Init(szGuards);
	szText.Append("in:\n\t%d = sub i16 %i, %first\n\t%w = zext i16 %d to i32\n\t%m = mul i32 %w, 12\n\t%q = ptradd ptr %p, %m\n\tret ptr %q\n"
		"out:\n\tret ptr 0\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%0 = mul i16 %d, 12\n"
		"\t%m = zext i16 %0 to i32\n"
		"\t%q = ptradd inarray ptr %p, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n", szText.Text(), true, false, true);
	szText.Kill();

	//((index - first) * 4 - (index - first)) * 4, as the W65C816's back end makes it.
	szText.Init(szGuards);
	szText.Append("in:\n\t%d = sub i16 %i, %first\n\t%w = zext i16 %d to i32\n\t%s = shl i32 %w, 2\n\t%t = sub i32 %s, %w\n\t%m = shl i32 %t, 2\n"
		"\t%q = ptradd ptr %p, %m\n\tret ptr %q\n"
		"out:\n\tret ptr 0\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%0 = mul i16 %d, 12\n"
		"\t%m = zext i16 %0 to i32\n"
		"\t%q = ptradd inarray ptr %p, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n", szText.Text(), true, false, true);
	szText.Kill();

	//The multiply after an if and else that join.
	szText.Init(szGuards);
	szText.Append("in:\n\t%c = eq i8 %f, 3\n\tbrif %c, yes, no\n"
		"yes:\n\tcall void @_Use__Fv()\n\tjmp join\n"
		"no:\n\tjmp join\n"
		"join:\n\t%d = sub i16 %i, %first\n\t%w = zext i16 %d to i32\n\t%m = mul i32 %w, 12\n\t%q = ptradd ptr %p, %m\n\tret ptr %q\n"
		"out:\n\tret ptr 0\n}\n");
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%c = eq i8 %f, 3\n"
		"\tbrif %c, yes, join\n"
		"yes:\n"
		"\tcall void @_Use__Fv()\n"
		"\tjmp join\n"
		"join:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%0 = mul i16 %d, 12\n"
		"\t%m = zext i16 %0 to i32\n"
		"\t%q = ptradd inarray ptr %p, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n", szText.Text(), true, false, true);
	szText.Kill();

	//Without first <= index the difference may be negative: 65535 * 12 doesn't fit.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%w = zext i16 %d to i32\n"
		"\t%m = mul i32 %w, 12\n"
		"\t%q = ptradd ptr %p, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n",
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n{\nentry:\n\t%first = zext i8 %f to i16\n\t%last = zext i8 %l to i16\n"
		"\t%b = ule i16 %i, %last\n\tbrif %b, in, out\n"
		"in:\n\t%d = sub i16 %i, %first\n\t%w = zext i16 %d to i32\n\t%m = mul i32 %w, 12\n\t%q = ptradd ptr %p, %m\n\tret ptr %q\n"
		"out:\n\tret ptr 0\n}\n", true, false, true);

	//A constant the pointer was given just before, for this alone, goes into the 16 bit offset: p + ((index - first) * 12 + 12).
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%0 = mul i16 %d, 12\n"
		"\t%1 = add i16 %0, 12\n"
		"\t%2 = zext i16 %1 to i32\n"
		"\t%q = ptradd inarray ptr %p, %2\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n",
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%base = ptradd ptr %p, 12\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%w = zext i16 %d to i32\n"
		"\t%m = mul i32 %w, 12\n"
		"\t%q = ptradd ptr %base, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n", true, false, true);

	//Not one worked out in another block, or used for something else too.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%base = ptradd ptr %p, 12\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%0 = mul i16 %d, 12\n"
		"\t%m = zext i16 %0 to i32\n"
		"\t%q = ptradd inarray ptr %base, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr %base\n"
		"}\n",
		"function ptr @_G__FPcjhh(ptr %p, i16 %i, i8 %f, i8 %l)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%last = zext i8 %l to i16\n"
		"\t%base = ptradd ptr %p, 12\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%w = zext i16 %d to i32\n"
		"\t%m = mul i32 %w, 12\n"
		"\t%q = ptradd ptr %base, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr %base\n"
		"}\n", true, false, true);

	//A last of 16 bits bounds nothing.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function ptr @_G__FPcjhj(ptr %p, i16 %i, i8 %f, i16 %last)\n"
		"{\n"
		"entry:\n"
		"\t%first = zext i8 %f to i16\n"
		"\t%a = uge i16 %i, %first\n"
		"\tbrif %a, upper, out\n"
		"upper:\n"
		"\t%b = ule i16 %i, %last\n"
		"\tbrif %b, in, out\n"
		"in:\n"
		"\t%d = sub i16 %i, %first\n"
		"\t%w = zext i16 %d to i32\n"
		"\t%m = mul i32 %w, 12\n"
		"\t%q = ptradd ptr %p, %m\n"
		"\tret ptr %q\n"
		"out:\n"
		"\tret ptr 0\n"
		"}\n",
		"function ptr @_G__FPcjhj(ptr %p, i16 %i, i8 %f, i16 %last)\n{\nentry:\n\t%first = zext i8 %f to i16\n"
		"\t%a = uge i16 %i, %first\n\tbrif %a, upper, out\n"
		"upper:\n\t%b = ule i16 %i, %last\n\tbrif %b, in, out\n"
		"in:\n\t%d = sub i16 %i, %first\n\t%w = zext i16 %d to i32\n\t%m = mul i32 %w, 12\n\t%q = ptradd ptr %p, %m\n\tret ptr %q\n"
		"out:\n\tret ptr 0\n}\n", true, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
// ReduceDivisions (the W65C816's): a counter that can't wrap divided by constants each time round has its remainders,
// and the quotient a udiv wants, kept as counters of their own (16 bits wide for these divisors), stepped in the
// latch; nothing is divided going round.  A counter stepped by 5 up to a bound that isn't known may wrap, and is
// left.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserCounterDivisions(void)
{
	AssertIROptimisedOptions(
		"function i32 @_D__Fj(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%e = ult i32 0, %n\n"
		"\tbrif %e, loop, out\n"
		"loop:\n"
		"\t%i = phi i32 [loop, %i2], [entry, 0]\n"
		"\t%s = phi i32 [loop, %s2], [entry, 0]\n"
		"\t%0 = phi i16 [loop, %9], [entry, 0]\n"
		"\t%1 = phi i32 [loop, %11], [entry, 0]\n"
		"\t%2 = phi i16 [loop, %17], [entry, 0]\n"
		"\t%3 = zext i16 %2 to i32\n"
		"\t%t = add i32 %s, %1\n"
		"\t%s2 = add i32 %t, %3\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%4 = add i16 %0, 1\n"
		"\t%5 = uge i16 %4, 251\n"
		"\t%6 = zext i1 %5 to i16\n"
		"\t%7 = neg i16 %6\n"
		"\t%8 = and i16 %7, 251\n"
		"\t%9 = sub i16 %4, %8\n"
		"\t%10 = zext i1 %5 to i32\n"
		"\t%11 = add i32 %1, %10\n"
		"\t%12 = add i16 %2, 1\n"
		"\t%13 = uge i16 %12, 3\n"
		"\t%14 = zext i1 %13 to i16\n"
		"\t%15 = neg i16 %14\n"
		"\t%16 = and i16 %15, 3\n"
		"\t%17 = sub i16 %12, %16\n"
		"\t%c = ult i32 %i2, %n\n"
		"\tbrif %c, loop, out\n"
		"out:\n"
		"\t%x = phi i32 [entry, 0], [loop, %s2]\n"
		"\tret i32 %x\n"
		"}\n",
		"function i32 @_D__Fj(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%e = ult i32 0, %n\n"
		"\tbrif %e, loop, out\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%s = phi i32 [entry, 0], [loop, %s2]\n"
		"\t%q = udiv i32 %i, 251\n"
		"\t%r = urem i32 %i, 3\n"
		"\t%t = add i32 %s, %q\n"
		"\t%s2 = add i32 %t, %r\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%c = ult i32 %i2, %n\n"
		"\tbrif %c, loop, out\n"
		"out:\n"
		"\t%x = phi i32 [entry, 0], [loop, %s2]\n"
		"\tret i32 %x\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_COUNTER_DIVISIONS);

	AssertIROptimisedOptions(
		"function i16 @_D__Ftt(i16 %a, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%e = ult i16 %a, %n\n"
		"\tbrif %e, loop, out\n"
		"loop:\n"
		"\t%i = phi i16 [entry, %a], [loop, %i2]\n"
		"\t%s = phi i16 [entry, 0], [loop, %s2]\n"
		"\t%r = urem i16 %i, 7\n"
		"\t%s2 = add i16 %s, %r\n"
		"\t%i2 = add i16 %i, 5\n"
		"\t%c = ult i16 %i2, %n\n"
		"\tbrif %c, loop, out\n"
		"out:\n"
		"\t%x = phi i16 [entry, 0], [loop, %s2]\n"
		"\tret i16 %x\n"
		"}\n",
		"function i16 @_D__Ftt(i16 %a, i16 %n)\n"
		"{\n"
		"entry:\n"
		"\t%e = ult i16 %a, %n\n"
		"\tbrif %e, loop, out\n"
		"loop:\n"
		"\t%i = phi i16 [entry, %a], [loop, %i2]\n"
		"\t%s = phi i16 [entry, 0], [loop, %s2]\n"
		"\t%r = urem i16 %i, 7\n"
		"\t%s2 = add i16 %s, %r\n"
		"\t%i2 = add i16 %i, 5\n"
		"\t%c = ult i16 %i2, %n\n"
		"\tbrif %c, loop, out\n"
		"out:\n"
		"\t%x = phi i16 [entry, 0], [loop, %s2]\n"
		"\tret i16 %x\n"
		"}\n", IR_TEST_INDEXED_ADDRESSES | IR_TEST_COUNTER_DIVISIONS);
}


//////////////////////////////////////////////////////////////////////////
//
// Constants added to and taken from a value one after another come to one add or sub (CIRFoldingPasses::
// IntegerOffsets): (x + 8) + 8 - 1 is x + 15; ones that come to nothing leave the value, and to the least value an
// add of it.  Not through a sum something else uses too.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserIntegerOffsets(void)
{
	AssertIROptimised(
		"function i32 @_R__Fj(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%c = add i32 %x, 15\n"
		"\t%d = and i32 %c, -8\n"
		"\tret i32 %d\n"
		"}\n",
		"function i32 @_R__Fj(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = add i32 %x, 8\n"
		"\t%b = add i32 %a, 8\n"
		"\t%c = sub i32 %b, 1\n"
		"\t%d = and i32 %c, -8\n"
		"\tret i32 %d\n"
		"}\n");
	AssertIROptimised(
		"function i16 @_R__Fs(i16 %x)\n"
		"{\n"
		"entry:\n"
		"\t%e = add i16 %x, -32768\n"
		"\t%f = add i16 %x, %e\n"
		"\tret i16 %f\n"
		"}\n",
		"function i16 @_R__Fs(i16 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = sub i16 %x, 5\n"
		"\t%b = add i16 %a, 2\n"
		"\t%c = add i16 %b, 3\n"
		"\t%d = sub i16 %x, 32767\n"
		"\t%e = sub i16 %d, 1\n"
		"\t%f = add i16 %c, %e\n"
		"\tret i16 %f\n"
		"}\n");
	AssertIROptimised(
		"function i32 @_R__Fj(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = add i32 %x, 8\n"
		"\t%b = add i32 %a, 8\n"
		"\t%c = mul i32 %a, %b\n"
		"\tret i32 %c\n"
		"}\n",
		"function i32 @_R__Fj(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%a = add i32 %x, 8\n"
		"\t%b = add i32 %a, 8\n"
		"\t%c = mul i32 %a, %b\n"
		"\tret i32 %c\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// NarrowTruncations: trunc(zext(a) + zext(b)) to i8 is an i8 add.  Not when
// the i32 add is also stored.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserNarrowTruncations(void)
{
	AssertIROptimised(
		"function i8 @_T__Fcc(i8 %a, i8 %b)\n"
		"{\n"
		"entry:\n"
		"\t%t = add i8 %a, %b\n"
		"\tret i8 %t\n"
		"}\n",
		"function i8 @_T__Fcc(i8 %a, i8 %b)\n"
		"{\n"
		"entry:\n"
		"\t%x = zext i8 %a to i32\n"
		"\t%y = zext i8 %b to i32\n"
		"\t%s = add i32 %x, %y\n"
		"\t%t = trunc i32 %s to i8\n"
		"\tret i8 %t\n"
		"}\n");

	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i8 @_T__FccPi(i8 %a, i8 %b, ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%x = zext i8 %a to i32\n"
		"\t%y = zext i8 %b to i32\n"
		"\t%s = add i32 %x, %y\n"
		"\tstore i32 %s, %p\n"
		"\t%t = trunc i32 %s to i8\n"
		"\tret i8 %t\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i8 @_T__FccPi(i8 %a, i8 %b, ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%x = zext i8 %a to i32\n"
		"\t%y = zext i8 %b to i32\n"
		"\t%s = add i32 %x, %y\n"
		"\tstore i32 %s, %p\n"
		"\t%t = trunc i32 %s to i8\n"
		"\tret i8 %t\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// UnifySlotTypes: a slot stored as a pointer and loaded as an i32 (a
// pointer's size) is loaded as a pointer and converted, so it is promoted:
// what is left is the ptrtoi of the pointer stored.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserUnifySlotTypes(void)
{
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_U__FPv(ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%x = ptrtoi ptr %p to i32\n"
		"\tret i32 %x\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_U__FPv(ptr %p)\n"
		"{\n"
		"entry:\n"
		"\t%s = slot ptr 4, 4\n"
		"\tstore ptr %p, %s\n"
		"\t%x = load i32 %s\n"
		"\tret i32 %x\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// RotateLoops: a loop testing at its head tests once on the way in (0 < n)
// and at the bottom of the body after that; the head goes.
//
//////////////////////////////////////////////////////////////////////////
void TestIROptimiserRotateLoops(void)
{
	AssertIROptimised(
		"function void @_R__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = slt i32 0, %n\n"
		"\tbrif %0, body, exit\n"
		"body:\n"
		"\t%1 = phi i32 [entry, 0], [body, %i2]\n"
		"\tcall void @_F__Fi(i32 %1)\n"
		"\t%i2 = add i32 %1, 1\n"
		"\t%2 = slt i32 %i2, %n\n"
		"\tbrif %2, body, exit\n"
		"exit:\n"
		"\tret\n"
		"}\n",
		"function void @_R__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%c = slt i32 %i, %n\n"
		"\tbrif %c, body, exit\n"
		"body:\n"
		"\tcall void @_F__Fi(i32 %i)\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp loop\n"
		"exit:\n"
		"\tret\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Parses szText (which must parse) into pcModule, then numbers its first
// function's blocks and finds their predecessors.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* TestIRParseFunction(CIRModule* pcModule, const char* szText)
{
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRFunction*	pcFunction;

	cErrors.Init();
	cParser.Init(pcModule, &cErrors);
	AssertTrue(cParser.Parse(szText));
	cParser.Kill();
	AssertString("", cErrors.Text());
	cErrors.Kill();

	pcFunction = pcModule->GetFunction(0);
	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Prints pcFunction and checks it is szExpected.
//
//////////////////////////////////////////////////////////////////////////
void AssertIRFunction(const char* szExpected, CIRModule* pcModule, CIRFunction* pcFunction)
{
	CIRPrinter	cPrinter;
	CChars		szPrinted;

	szPrinted.Init();
	cPrinter.Init(pcModule, &szPrinted);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
	AssertString(szExpected, szPrinted.Text());
	szPrinted.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CIRSSAUpdater.  A diamond a -> l, r -> j defining the value in l and r:
// at j's start it is a new phi of both, at l's start (nothing defined
// before it) 0, and at l's end what l defines.  A loop s -> h -> b -> h,
// defined in s and b: at h's start a phi of both (found while it looks
// round the loop back to h), and at the exit's start (one predecessor, h)
// that phi.
//
// A block whose only predecessor is itself (an unreachable self loop)
// starts with 0, as a block nothing reaches does (it used to make
// ValueAtStart and ValueAtEnd call each other for ever).
//
//////////////////////////////////////////////////////////////////////////
void TestIRSSAUpdater(void)
{
	CIRModule		cModule;
	CIRFunction*	pcFunction;
	CIRSSAUpdater	cUpdater;
	SIRReference	sValue;
	SIRReference	sOther;

	cModule.Init();
	pcFunction = TestIRParseFunction(&cModule,
		"function i32 @f(i1 %c, i32 %p)\n"
		"{\n"
		"a:\n\tbrif %c, l, r\n"
		"l:\n\t%x = add i32 %p, 1\n\tjmp j\n"
		"r:\n\t%y = add i32 %p, 2\n\tjmp j\n"
		"j:\n\tret i32 %p\n"
		"}\n");

	cUpdater.Init(pcFunction);
	cUpdater.Begin(IRType(IRTK_I32));
	cUpdater.Define(pcFunction->FindBlock("l"), pcFunction->FindTemporary("x"));
	cUpdater.Define(pcFunction->FindBlock("r"), pcFunction->FindTemporary("y"));
	sValue = cUpdater.ValueAtStart(pcFunction->FindBlock("j"));
	AssertTrue(sValue.IsTemporary());
	AssertTrue(sValue.Equals(cUpdater.ValueAtEnd(pcFunction->FindBlock("j"))));
	AssertTrue(sValue.Equals(cUpdater.ValueAtStart(pcFunction->FindBlock("j"))));
	AssertTrue(pcFunction->FindTemporary("x").Equals(cUpdater.ValueAtEnd(pcFunction->FindBlock("l"))));
	sOther = cUpdater.ValueAtStart(pcFunction->FindBlock("l"));
	AssertTrue(sOther.IsConstant());
	AssertLong(0, pcFunction->GetSigned(sOther));
	cUpdater.Kill();
	AssertIRFunction(
		"function i32 @f(i1 %c, i32 %p)\n"
		"{\n"
		"a:\n"
		"\tbrif %c, l, r\n"
		"l:\n"
		"\t%x = add i32 %p, 1\n"
		"\tjmp j\n"
		"r:\n"
		"\t%y = add i32 %p, 2\n"
		"\tjmp j\n"
		"j:\n"
		"\t%0 = phi i32 [l, %x], [r, %y]\n"
		"\tret i32 %p\n"
		"}\n", &cModule, pcFunction);
	cModule.Kill();

	cModule.Init();
	pcFunction = TestIRParseFunction(&cModule,
		"function i32 @f(i1 %c, i32 %p)\n"
		"{\n"
		"s:\n\t%x = add i32 %p, 1\n\tjmp h\n"
		"h:\n\tbrif %c, b, e\n"
		"b:\n\t%y = add i32 %p, 2\n\tjmp h\n"
		"e:\n\tret i32 %p\n"
		"}\n");

	cUpdater.Init(pcFunction);
	cUpdater.Begin(IRType(IRTK_I32));
	cUpdater.Define(pcFunction->FindBlock("s"), pcFunction->FindTemporary("x"));
	cUpdater.Define(pcFunction->FindBlock("b"), pcFunction->FindTemporary("y"));
	sValue = cUpdater.ValueAtStart(pcFunction->FindBlock("e"));
	AssertTrue(sValue.IsTemporary());
	AssertTrue(sValue.Equals(cUpdater.ValueAtStart(pcFunction->FindBlock("h"))));
	AssertTrue(sValue.Equals(cUpdater.ValueAtStart(pcFunction->FindBlock("b"))));
	cUpdater.Kill();
	AssertIRFunction(
		"function i32 @f(i1 %c, i32 %p)\n"
		"{\n"
		"s:\n"
		"\t%x = add i32 %p, 1\n"
		"\tjmp h\n"
		"h:\n"
		"\t%0 = phi i32 [s, %x], [b, %y]\n"
		"\tbrif %c, b, e\n"
		"b:\n"
		"\t%y = add i32 %p, 2\n"
		"\tjmp h\n"
		"e:\n"
		"\tret i32 %p\n"
		"}\n", &cModule, pcFunction);
	cModule.Kill();

	//A block in a loop with one predecessor (b, between the head and the latch) sees the head's phi.
	cModule.Init();
	pcFunction = TestIRParseFunction(&cModule,
		"function i32 @f(i1 %c, i32 %p)\n"
		"{\n"
		"s:\n\t%x = add i32 %p, 1\n\tjmp h\n"
		"h:\n\tbrif %c, b, e\n"
		"b:\n\tjmp l\n"
		"l:\n\t%y = add i32 %p, 2\n\tjmp h\n"
		"e:\n\tret i32 %p\n"
		"}\n");
	cUpdater.Init(pcFunction);
	cUpdater.Begin(IRType(IRTK_I32));
	cUpdater.Define(pcFunction->FindBlock("s"), pcFunction->FindTemporary("x"));
	cUpdater.Define(pcFunction->FindBlock("l"), pcFunction->FindTemporary("y"));
	sValue = cUpdater.ValueAtStart(pcFunction->FindBlock("b"));
	AssertTrue(sValue.IsTemporary());
	AssertTrue(sValue.Equals(cUpdater.ValueAtStart(pcFunction->FindBlock("h"))));
	cUpdater.Kill();
	cModule.Kill();

	cModule.Init();
	pcFunction = TestIRParseFunction(&cModule,
		"function i32 @f(i32 %p)\n"
		"{\n"
		"s:\n\t%x = add i32 %p, 1\n\tret i32 %x\n"
		"z:\n\tjmp z\n"
		"}\n");
	cUpdater.Init(pcFunction);
	cUpdater.Begin(IRType(IRTK_I32));
	cUpdater.Define(pcFunction->FindBlock("s"), pcFunction->FindTemporary("x"));
	sValue = cUpdater.ValueAtStart(pcFunction->FindBlock("z"));
	AssertTrue(sValue.IsConstant());
	AssertLong(0, pcFunction->GetSigned(sValue));
	cUpdater.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CIRLiveness of a loop.  Blocks take 4 positions and 2 per instruction:
// entry 0 (its add at 2, its jmp at 4), loop 6 (8, 10, jmp 12), exit 14
// (ret 16).  %k (defined in entry, used in the loop) and %n are live across
// the back edge, so to the loop's end; %i lives from the loop's start to
// its use; %i2 from its definition through exit's ret.  Weights are 1 per
// definition or use, 8 in a loop.
//
//////////////////////////////////////////////////////////////////////////
void TestIRLiveness(void)
{
	CIRModule		cModule;
	CIRFunction*	pcFunction;
	CIRDominators	cDominators;
	CIRLiveness		cLiveness;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcExit;
	SIRReference	sK;
	SIRReference	sN;
	SIRReference	sI;
	SIRReference	sI2;
	SIRRange*		psRanges;

	cModule.Init();
	pcFunction = TestIRParseFunction(&cModule,
		"function i32 @f(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%k = add i32 %n, 1\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [loop, %i2]\n"
		"\t%i2 = add i32 %i, %k\n"
		"\t%c = slt i32 %i2, %n\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n"
		"\tret i32 %i2\n"
		"}\n");
	pcEntry = pcFunction->FindBlock("entry");
	pcLoop = pcFunction->FindBlock("loop");
	pcExit = pcFunction->FindBlock("exit");
	sK = pcFunction->FindTemporary("k");
	sN = pcFunction->FindTemporary("n");
	sI = pcFunction->FindTemporary("i");
	sI2 = pcFunction->FindTemporary("i2");

	cDominators.Init();
	cDominators.Compute(pcFunction);
	cLiveness.Init();
	cLiveness.Compute(pcFunction, &cDominators);

	AssertInt(0, cLiveness.Start(pcEntry));
	AssertInt(2, cLiveness.Position(pcEntry, 0));
	AssertInt(4, cLiveness.Jump(pcEntry));
	AssertInt(6, cLiveness.Start(pcLoop));
	AssertInt(10, cLiveness.Position(pcLoop, 1));
	AssertInt(12, cLiveness.Jump(pcLoop));
	AssertInt(14, cLiveness.Start(pcExit));
	AssertInt(0, cLiveness.LoopDepth(pcEntry));
	AssertInt(1, cLiveness.LoopDepth(pcLoop));
	AssertInt(0, cLiveness.LoopDepth(pcExit));

	AssertSize(1, cLiveness.NumRanges(sK));
	psRanges = cLiveness.GetRanges(sK);
	AssertInt(3, psRanges[0].iFrom);
	AssertInt(14, psRanges[0].iTo);
	AssertInt(1 + 8, cLiveness.GetWeight(sK));

	AssertSize(1, cLiveness.NumRanges(sN));
	psRanges = cLiveness.GetRanges(sN);
	AssertInt(0, psRanges[0].iFrom);
	AssertInt(14, psRanges[0].iTo);
	AssertInt(1 + 1 + 8, cLiveness.GetWeight(sN));

	AssertSize(1, cLiveness.NumRanges(sI));
	psRanges = cLiveness.GetRanges(sI);
	AssertInt(6, psRanges[0].iFrom);
	AssertInt(9, psRanges[0].iTo);
	AssertInt(8 + 8, cLiveness.GetWeight(sI));

	AssertSize(1, cLiveness.NumRanges(sI2));
	psRanges = cLiveness.GetRanges(sI2);
	AssertInt(9, psRanges[0].iFrom);
	AssertInt(17, psRanges[0].iTo);
	AssertInt(8 + 8 + 1, cLiveness.GetWeight(sI2));

	AssertFalse(CIRLiveness::Intersect(cLiveness.GetRanges(sI), 1, cLiveness.GetRanges(sI2), 1));
	AssertTrue(CIRLiveness::Intersect(cLiveness.GetRanges(sI), 1, cLiveness.GetRanges(sK), 1));
	AssertInt(1, CIRLoops::Weight(0, IR_LOOP_WEIGHT_MAX_DEPTH));
	AssertInt(64, CIRLoops::Weight(2, IR_LOOP_WEIGHT_MAX_DEPTH));
	AssertInt(32768, CIRLoops::Weight(5, IR_LOOP_WEIGHT_MAX_DEPTH));
	AssertInt(32768, CIRLoops::Weight(9, IR_LOOP_WEIGHT_MAX_DEPTH));
	AssertInt(4096, CIRLoops::Weight(5, 4));
	AssertInt(512, CIRLoops::Weight(7, 3));

	cLiveness.Kill();
	cDominators.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Dominance frontiers: s -> a; a -> b, c (a diamond) -> d; d -> a (the
// latch), e.  b's and c's frontier is d (the join); d's (the latch's) is a
// (the head), and so is a's own.  s, e and the unreachable x have none.
//
//////////////////////////////////////////////////////////////////////////
void TestIRDominanceFrontiers(void)
{
	CIRModule		cModule;
	CIRFunction*	pcFunction;
	CIRDominators	cDominators;
	CIRBlock*		pcA;
	CIRBlock*		pcD;

	cModule.Init();
	pcFunction = TestIRParseFunction(&cModule,
		"function void @f(i1 %c)\n"
		"{\n"
		"s:\n\tjmp a\n"
		"a:\n\tbrif %c, b, c\n"
		"b:\n\tjmp d\n"
		"c:\n\tjmp d\n"
		"x:\n\tjmp d\n"
		"d:\n\tbrif %c, e, a\n"
		"e:\n\tret\n"
		"}\n");
	pcA = pcFunction->FindBlock("a");
	pcD = pcFunction->FindBlock("d");

	cDominators.Init();
	cDominators.Compute(pcFunction);
	cDominators.ComputeFrontiers();
	AssertSize(1, cDominators.NumFrontiers(pcFunction->FindBlock("b")));
	AssertPointer(pcD, cDominators.GetFrontier(pcFunction->FindBlock("b"), 0));
	AssertSize(1, cDominators.NumFrontiers(pcFunction->FindBlock("c")));
	AssertPointer(pcD, cDominators.GetFrontier(pcFunction->FindBlock("c"), 0));
	AssertSize(1, cDominators.NumFrontiers(pcD));
	AssertPointer(pcA, cDominators.GetFrontier(pcD, 0));
	AssertSize(1, cDominators.NumFrontiers(pcA));
	AssertPointer(pcA, cDominators.GetFrontier(pcA, 0));
	AssertSize(0, cDominators.NumFrontiers(pcFunction->FindBlock("s")));
	AssertSize(0, cDominators.NumFrontiers(pcFunction->FindBlock("e")));
	AssertSize(0, cDominators.NumFrontiers(pcFunction->FindBlock("x")));
	cDominators.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The flags hot (on sdiv, udiv, srem and urem), inbank (ptradd) and
// thiscall (call), and the float comparisons not in
// TestIRRoundTripEverything, print back as they were parsed.
//
//////////////////////////////////////////////////////////////////////////
void TestIRRoundTripFlags(void)
{
	AssertIRRoundTrip(
		"space default 4\n"
		"\n"
		"function i32 @_Flags__FPvif(ptr %p, i32 %x, f32 %f)\n"
		"{\n"
		"entry:\n"
		"\t%a = sdiv hot i32 %x, 3\n"
		"\t%b = udiv hot i32 %a, 5\n"
		"\t%c = srem hot i32 %b, 7\n"
		"\t%d = urem hot i32 %c, 9\n"
		"\t%q = ptradd inbank ptr %p, %d\n"
		"\t%r = call thiscall i32 @_Get__1AFv(ptr %q)\n"
		"\tcall thiscall void @_Set__1AFi(ptr %q, i32 %r)\n"
		"\t%c1 = fne f32 %f, 0x3F800000\n"
		"\t%c2 = flt f32 %f, 0x3F800000\n"
		"\t%c3 = fle f32 %f, 0x3F800000\n"
		"\t%c4 = fgt f32 %f, 0x3F800000\n"
		"\t%c5 = fge f32 %f, 0x3F800000\n"
		"\t%c6 = fordered f32 %f, %f\n"
		"\tbrif %c6, yes, no\n"
		"yes:\n"
		"\tret i32 %r\n"
		"no:\n"
		"\tret i32 0\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A flag on an opcode that doesn't allow it.
//
//////////////////////////////////////////////////////////////////////////
void TestIRVerifierFlagErrors(void)
{
	AssertIRErrors("@f b: Flag not allowed on the opcode. [add]\n",
		"function void @f()\n{\nb:\n\t%x = add hot i32 1, 2\n\tret\n}\n");
	AssertIRErrors("@f b: Flag not allowed on the opcode. [mul]\n",
		"function void @f()\n{\nb:\n\t%x = mul thiscall i32 1, 2\n\tret\n}\n");
	AssertIRErrors("@f b: Flag not allowed on the opcode. [ptradd]\n",
		"space default 4\n\nfunction void @f(ptr %p)\n{\nb:\n\t%q = ptradd volatile ptr %p, 1\n\tret\n}\n");
	AssertIRErrors("@f b: Flag not allowed on the opcode. [load]\n",
		"space default 4\n\nfunction void @f(ptr %p)\n{\nb:\n\t%x = load inbank i32 %p\n\tret\n}\n");
	AssertIRErrors("@f b: Flag not allowed on the opcode. [sdiv]\n",
		"function void @f()\n{\nb:\n\t%x = sdiv inbank i32 1, 2\n\tret\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// IRCompareIntegers (the optimiser's folding and loop guards): signed
// comparisons sign extend from the type's width, unsigned ones don't, and
// both see only the type's bits.
//
//////////////////////////////////////////////////////////////////////////
void TestIRCompareIntegers(void)
{
	AssertTrue(IRCompareIntegers(IRO_SLt, IRType(IRTK_I8), 0xFF, 1));
	AssertFalse(IRCompareIntegers(IRO_ULt, IRType(IRTK_I8), 0xFF, 1));
	AssertTrue(IRCompareIntegers(IRO_UGe, IRType(IRTK_I8), 0xFF, 0x80));
	AssertTrue(IRCompareIntegers(IRO_SLe, IRType(IRTK_I16), 0x8000, 0x7FFF));
	AssertTrue(IRCompareIntegers(IRO_Eq, IRType(IRTK_I8), 0x1FF, 0xFF));
	AssertFalse(IRCompareIntegers(IRO_Ne, IRType(IRTK_I8), 0x1FF, 0xFF));
	AssertTrue(IRCompareIntegers(IRO_SGt, IRType(IRTK_I64), 1, 0xFFFFFFFFFFFFFFFFULL));
	AssertFalse(IRCompareIntegers(IRO_UGt, IRType(IRTK_I64), 1, 0xFFFFFFFFFFFFFFFFULL));
}


//////////////////////////////////////////////////////////////////////////
//
// A block that ends in throw: the function is left with an exception.  It has
// no successors, as ret and unreachable have none.  The block passes skip the
// empty block before one and join the block that only jumps to one, and the
// throw stays the ending.
//
//////////////////////////////////////////////////////////////////////////
void TestIRThrow(void)
{
	AssertIRRoundTrip(
		"function i32 @_Checked__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%c = slt i32 %x, 0\n"
		"\tbrif %c, bad, good\n"
		"bad:\n"
		"\tthrow\n"
		"good:\n"
		"\tret i32 %x\n"
		"}\n");

	AssertIRErrors("(5, 2): Instruction after the block's jump. [ret]\n",
		"function void @f()\n{\nb:\n\tthrow\n\tret\n}\n");
	AssertIRErrors("(4, 2): Expected an instruction. [throws]\n",
		"function void @f()\n{\nb:\n\tthrows\n}\n");

	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function i32 @_Checked__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%c = slt i32 %x, 0\n"
		"\tbrif %c, out, done\n"
		"out:\n"
		"\tcall void @_Report__Fv()\n"
		"\tthrow\n"
		"done:\n"
		"\tret i32 %x\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function i32 @_Checked__Fi(i32 %x)\n"
		"{\n"
		"entry:\n"
		"\t%c = slt i32 %x, 0\n"
		"\tbrif %c, bad, good\n"
		"bad:\n"
		"\tjmp out\n"
		"out:\n"
		"\tcall void @_Report__Fv()\n"
		"\tjmp fail\n"
		"fail:\n"
		"\tthrow\n"
		"good:\n"
		"\tjmp done\n"
		"done:\n"
		"\tret i32 %x\n"
		"}\n");

	//Code after a throw's block that nothing else reaches goes, as it does after a ret.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_Always__Fv()\n"
		"{\n"
		"entry:\n"
		"\tcall void @_Report__Fv()\n"
		"\tthrow\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function void @_Always__Fv()\n"
		"{\n"
		"entry:\n"
		"\tcall void @_Report__Fv()\n"
		"\tjmp fail\n"
		"fail:\n"
		"\tthrow\n"
		"after:\n"
		"\tcall void @_Report__Fv()\n"
		"\tret\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// thrown is whether the call straight before it was left with an exception.
// Nothing may come between the two.  Two of them are never one (each speaks
// of its own call), and the back ends' pass that moves a loop's step to
// after the call that last reads the counter puts it after the thrown too.
//
//////////////////////////////////////////////////////////////////////////
void TestIRThrown(void)
{
	CIRModule			cModule;
	CCPPErrors			cErrors;
	CIRParser			cParser;
	CIRVerifier			cVerifier;
	CIRPrinter			cPrinter;
	CIRBackendPasses	cPasses;
	CIROptimiser		cOptimiser;
	CChars				szPrinted;
	const char*			szTwice =
		"space default 4\n"
		"\n"
		"function i32 @_Twice__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%a = call i32 @_Get__Fv()\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, fail, next\n"
		"next:\n"
		"\t%b = call i32 @_Get__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, fail, done\n"
		"done:\n"
		"\t%s = add i32 %a, %b\n"
		"\tret i32 %s\n"
		"fail:\n"
		"\tthrow\n"
		"}\n";

	AssertIRRoundTrip(
		"space default 4\n"
		"\n"
		"function void @_Tested__FPv(ptr %p)\n"
		"{\n"
		"entry:\n"
		"\tcall void %p(i32 1)\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, fail, done\n"
		"done:\n"
		"\tret\n"
		"fail:\n"
		"\tthrow\n"
		"}\n");

	AssertIRErrors("@f b: thrown not straight after a call.\n",
		"function void @f()\n{\nb:\n\t%t = thrown i1\n\tret\n}\n");
	AssertIRErrors("@f b: thrown not straight after a call.\n",
		"function i32 @f()\n{\nb:\n\t%r = call i32 @g()\n\t%s = add i32 %r, 1\n\t%t = thrown i1\n\tret i32 %s\n}\n");
	AssertIRErrors("@f b: Type not allowed for the opcode. [thrown]\n",
		"function void @f()\n{\nb:\n\tcall void @g()\n\t%t = thrown i32\n\tret\n}\n");

	//The second call's thrown is not the first's.
	AssertIROptimised(szTwice, szTwice);

	//One that nothing reads goes.
	AssertIROptimised(
		"space default 4\n"
		"\n"
		"function void @_Untested__Fv()\n"
		"{\n"
		"entry:\n"
		"\tcall void @_Get__Fv()\n"
		"\tret\n"
		"}\n",
		"space default 4\n"
		"\n"
		"function void @_Untested__Fv()\n"
		"{\n"
		"entry:\n"
		"\tcall void @_Get__Fv()\n"
		"\t%t = thrown i1\n"
		"\tret\n"
		"}\n");

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"space default 4\n"
		"\n"
		"function void @_Each__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [next, %j]\n"
		"\t%j = add i32 %i, 1\n"
		"\tcall void @_Use__Fi(i32 %i)\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, fail, next\n"
		"next:\n"
		"\t%c = slt i32 %j, %n\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"fail:\n"
		"\tthrow\n"
		"}\n"));
	cParser.Kill();
	cPasses.Init();
	cPasses.SinkSteps(cModule.GetFunction(0));
	cPasses.Kill();
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	cVerifier.Kill();
	AssertString("", cErrors.Text());
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	AssertString(
		"space default 4\n"
		"\n"
		"function void @_Each__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%i = phi i32 [entry, 0], [next, %j]\n"
		"\tcall void @_Use__Fi(i32 %i)\n"
		"\t%t = thrown i1\n"
		"\t%j = add i32 %i, 1\n"
		"\tbrif %t, fail, next\n"
		"next:\n"
		"\t%c = slt i32 %j, %n\n"
		"\tbrif %c, loop, exit\n"
		"exit:\n"
		"\tret\n"
		"fail:\n"
		"\tthrow\n"
		"}\n", szPrinted.Text());
	szPrinted.Kill();
	cErrors.Kill();
	cModule.Kill();

	//The optimiser keeps a thrown straight after its call: what both ways out of its brif work out isn't hoisted
	//between them (HoistCommon put the add there, and TestScene's W65C816 build with exceptions didn't verify).
	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"space default 4\n"
		"\n"
		"function i32 @_Both__Fii(i32 %a, i32 %b)\n"
		"{\n"
		"entry:\n"
		"\tcall void @_Get__Fv()\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, fail, done\n"
		"fail:\n"
		"\t%p = add i32 %a, %b\n"
		"\tcall void @_Use__Fi(i32 %p)\n"
		"\tthrow\n"
		"done:\n"
		"\t%q = add i32 %a, %b\n"
		"\tret i32 %q\n"
		"}\n"));
	cParser.Kill();
	cOptimiser.Init(&cModule);
	cOptimiser.Optimise(cModule.GetFunction(0));
	cOptimiser.Kill();
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	cVerifier.Kill();
	AssertString("", cErrors.Text());
	cErrors.Kill();
	cModule.Kill();

	//IsolateThrowns (for a back end that branches on the carry at the call): a thrown that isn't the last thing in its
	//block, read by its brif alone, becomes one that is, and a phi of the two ways.  One that is already is left.
	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"space default 4\n"
		"\n"
		"function i32 @_Value__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%r = call i32 @_Get__Fv()\n"
		"\t%t = thrown i1\n"
		"\t%w = zext i1 %t to i32\n"
		"\t%s = add i32 %r, %w\n"
		"\tbrif %t, a, b\n"
		"a:\n"
		"\tret i32 %s\n"
		"b:\n"
		"\tcall void @_Get__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, c, a\n"
		"c:\n"
		"\tthrow\n"
		"}\n"));
	cParser.Kill();
	cPasses.Init();
	AssertTrue(cPasses.IsolateThrowns(cModule.GetFunction(0)));
	AssertFalse(cPasses.IsolateThrowns(cModule.GetFunction(0)));
	cPasses.Kill();
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	cVerifier.Kill();
	AssertString("", cErrors.Text());
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	AssertString(
		"space default 4\n"
		"\n"
		"function i32 @_Value__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%r = call i32 @_Get__Fv()\n"
		"\t%0 = thrown i1\n"
		"\tbrif %0, 1, 2\n"
		"a:\n"
		"\tret i32 %s\n"
		"b:\n"
		"\tcall void @_Get__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, c, a\n"
		"c:\n"
		"\tthrow\n"
		"0:\n"
		"\t%t = phi i1 [1, 1], [2, 0]\n"
		"\t%w = zext i1 %t to i32\n"
		"\t%s = add i32 %r, %w\n"
		"\tbrif %t, a, b\n"
		"1:\n"
		"\tjmp 0\n"
		"2:\n"
		"\tjmp 0\n"
		"}\n", szPrinted.Text());
	szPrinted.Kill();
	cErrors.Kill();
	cModule.Kill();

	//OrderBlocks (ExceptionsPlan.txt stage 9d): the blocks only an exception reaches, a tested call's landing and what
	//only it reaches, go after the others in their order, so each tested call's normal way falls through.
	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(
		"space default 4\n"
		"\n"
		"function i32 @_Order__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%r = call i32 @_Get__Fv()\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, land, on\n"
		"land:\n"
		"\tcall void @_Undo__Fv()\n"
		"\tjmp out\n"
		"out:\n"
		"\tthrow\n"
		"on:\n"
		"\t%s = call i32 @_Get__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, out, done\n"
		"done:\n"
		"\t%v = add i32 %r, %s\n"
		"\tret i32 %v\n"
		"}\n"));
	cParser.Kill();
	cPasses.Init();
	cPasses.OrderBlocks(cModule.GetFunction(0), false);
	cPasses.Kill();
	cVerifier.Init(&cModule, &cErrors);
	AssertTrue(cVerifier.Verify());
	cVerifier.Kill();
	AssertString("", cErrors.Text());
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintModule();
	cPrinter.Kill();
	AssertString(
		"space default 4\n"
		"\n"
		"function i32 @_Order__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%r = call i32 @_Get__Fv()\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, land, on\n"
		"on:\n"
		"\t%s = call i32 @_Get__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, out, done\n"
		"done:\n"
		"\t%v = add i32 %r, %s\n"
		"\tret i32 %v\n"
		"land:\n"
		"\tcall void @_Undo__Fv()\n"
		"\tjmp out\n"
		"out:\n"
		"\tthrow\n"
		"}\n", szPrinted.Text());
	szPrinted.Kill();
	cErrors.Kill();
	cModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// nothrow @name: the function is never left with an exception.  The thrown
// after a call to it is false, so the code for its exception goes; the
// thrown after any other call stays.  A function marked nothrow can't reach
// a throw, except past the thrown of a call to another that is.
//
//////////////////////////////////////////////////////////////////////////
void TestIRNoThrow(void)
{
	CIRModule	cModule;
	CCPPErrors	cErrors;
	CIRParser	cParser;

	//A copy of a function that never throws never throws.
	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse("nothrow @f\n\nfunction void @f()\n{\nb:\n\tret\n}\n\nfunction void @g()\n{\nb:\n\tret\n}\n"));
	cParser.Kill();
	AssertTrue(cModule.IsNoThrow(cModule.CopyFunction(cModule.GetFunction(0), "f.copy")->muiSymbol));
	AssertFalse(cModule.IsNoThrow(cModule.CopyFunction(cModule.GetFunction(1), "g.copy")->muiSymbol));
	AssertFalse(IRMayThrow(&cModule, cModule.GetFunction(0)));
	cErrors.Kill();
	cModule.Kill();

	AssertIRRoundTrip(
		"space default 4\n"
		"nowrites @_Quiet__Fv\n"
		"nothrow @_Quiet__Fv\n"
		"nothrow @_Outside__Fv\n"
		"\n"
		"function void @_Quiet__Fv()\n"
		"{\n"
		"entry:\n"
		"\tcall void @_Outside__Fv()\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, fail, done\n"
		"done:\n"
		"\tret\n"
		"fail:\n"
		"\tthrow\n"
		"}\n");

	AssertIRErrors("(1, 9): Expected '@'. [5]\n", "nothrow 5\n");
	AssertIRErrors("@f: A nothrow function can reach a throw.\n",
		"nothrow @f\n\nfunction void @f()\n{\nb:\n\tcall void @g()\n\t%t = thrown i1\n\tbrif %t, c, d\nc:\n\tthrow\nd:\n\tret\n}\n");
	AssertIRErrors("@f: A nothrow function can reach a throw.\n",
		"nothrow @f\n\nfunction void @f()\n{\nb:\n\tbrif 1, c, d\nc:\n\tthrow\nd:\n\tret\n}\n");

	//The test after the call to the nothrow function folds and its landing block goes; the other's stays.
	AssertIROptimised(
		"space default 4\n"
		"nothrow @_Safe__Fv\n"
		"\n"
		"function i32 @_Both__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%a = call i32 @_Safe__Fv()\n"
		"\t%b = call i32 @_Risky__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, landb, done\n"
		"landb:\n"
		"\tcall void @_Undo__Fv()\n"
		"\tthrow\n"
		"done:\n"
		"\t%s = add i32 %a, %b\n"
		"\tret i32 %s\n"
		"}\n",
		"space default 4\n"
		"nothrow @_Safe__Fv\n"
		"\n"
		"function i32 @_Both__Fv()\n"
		"{\n"
		"entry:\n"
		"\t%a = call i32 @_Safe__Fv()\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, landa, next\n"
		"landa:\n"
		"\tcall void @_Abandon__Fv()\n"
		"\tthrow\n"
		"next:\n"
		"\t%b = call i32 @_Risky__Fv()\n"
		"\t%u = thrown i1\n"
		"\tbrif %u, landb, done\n"
		"landb:\n"
		"\tcall void @_Undo__Fv()\n"
		"\tthrow\n"
		"done:\n"
		"\t%s = add i32 %a, %b\n"
		"\tret i32 %s\n"
		"}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Inlines the call(s) in the function named szCaller of szText, verifies,
// and (bOptimise) optimises it; the function must then print as szExpected.
//
//////////////////////////////////////////////////////////////////////////
static void AssertIRInlined(const char* szExpected, const char* szText, const char* szCaller, bool bOptimise)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRVerifier		cVerifier;
	CIRInliner		cInliner;
	CIROptimiser	cOptimiser;
	CIRPrinter		cPrinter;
	CChars			szPrinted;
	CIRFunction*	pcCaller;

	cErrors.Init();
	cModule.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(szText));
	cParser.Kill();
	pcCaller = cModule.FindFunction((uint32)cModule.FindSymbol(szCaller));
	AssertNotNull(pcCaller);
	cInliner.Init(&cModule, 64, 64, 8);
	cInliner.InlineCalls(pcCaller);
	cInliner.Kill();
	pcCaller->ComputePredecessors();
	cVerifier.Init(&cModule, &cErrors);
	cVerifier.Verify();
	cVerifier.Kill();
	AssertString("", cErrors.Text());
	if (bOptimise)
	{
		cOptimiser.Init(&cModule);
		cOptimiser.Optimise(pcCaller);
		cOptimiser.Kill();
		cVerifier.Init(&cModule, &cErrors);
		cVerifier.Verify();
		cVerifier.Kill();
		AssertString("", cErrors.Text());
	}
	szPrinted.Init();
	cPrinter.Init(&cModule, &szPrinted);
	cPrinter.PrintFunction(pcCaller);
	cPrinter.Kill();
	AssertString(szExpected, szPrinted.Text());
	szPrinted.Kill();
	cModule.Kill();
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A callee that returns on one path and throws on the other, inlined at a
// tested call: its throw goes to the caller's code for the exception and its
// return goes on, with no test left.  One that only returns leaves no code
// for the exception.  A call with no test of a callee that throws stays a
// call; so does nothing else.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInlineThrows(void)
{
#define TEST_IR_CHECKED \
	"function i32 @_Checked__Fi(i32 %x)\n{\nentry:\n\t%c = slt i32 %x, 0\n\tbrif %c, bad, good\nbad:\n\tthrow\ngood:\n\t%d = add i32 %x, %x\n\tret i32 %d\n}\n\n"
#define TEST_IR_TESTED_CALLER(callee) \
	"function i32 @_Caller__Fi(i32 %n)\n{\nentry:\n\t%r = call i32 @" callee "(i32 %n)\n\t%t = thrown i1\n\tbrif %t, land, on\n" \
	"land:\n\tcall void @_Undo__Fv()\n\tthrow\non:\n\t%s = add i32 %r, 1\n\tret i32 %s\n}\n"

	//As the inliner leaves it: the callee's throw jumps to the caller's code for the exception, its return goes on.
	AssertIRInlined(
		"function i32 @_Caller__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp 1\n"
		"land:\n"
		"\tcall void @_Undo__Fv()\n"
		"\tthrow\n"
		"on:\n"
		"\t%s = add i32 %r, 1\n"
		"\tret i32 %s\n"
		"0:\n"
		"\t%r = phi i32 [3, %1]\n"
		"\tjmp on\n"
		"1:\n"
		"\t%0 = slt i32 %n, 0\n"
		"\tbrif %0, 2, 3\n"
		"2:\n"
		"\tjmp land\n"
		"3:\n"
		"\t%1 = add i32 %n, %n\n"
		"\tjmp 0\n"
		"}\n",
		"space default 4\n\n" TEST_IR_CHECKED TEST_IR_TESTED_CALLER("_Checked__Fi"), "_Caller__Fi", false);

	//Optimised: each way out of the callee goes straight where the caller's test would have sent it.
	AssertIRInlined(
		"function i32 @_Caller__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = slt i32 %n, 0\n"
		"\tbrif %0, land, 0\n"
		"land:\n"
		"\tcall void @_Undo__Fv()\n"
		"\tthrow\n"
		"0:\n"
		"\t%1 = add i32 %n, %n\n"
		"\t%s = add i32 %1, 1\n"
		"\tret i32 %s\n"
		"}\n",
		"space default 4\n\n" TEST_IR_CHECKED TEST_IR_TESTED_CALLER("_Checked__Fi"), "_Caller__Fi", true);

	//Something between the thrown and its brif (not what a front end writes): the thrown is a phi of the ways out.
	AssertIRInlined(
		"function i32 @_Caller__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp 1\n"
		"land:\n"
		"\tthrow\n"
		"on:\n"
		"\tret i32 %s\n"
		"0:\n"
		"\t%r = phi i32 [3, %1], [2, 0]\n"
		"\t%t = phi i1 [3, 0], [2, 1]\n"
		"\t%s = add i32 %r, 1\n"
		"\tbrif %t, land, on\n"
		"1:\n"
		"\t%0 = slt i32 %n, 0\n"
		"\tbrif %0, 2, 3\n"
		"2:\n"
		"\tjmp 0\n"
		"3:\n"
		"\t%1 = add i32 %n, %n\n"
		"\tjmp 0\n"
		"}\n",
		"space default 4\n\n" TEST_IR_CHECKED
		"function i32 @_Caller__Fi(i32 %n)\n{\nentry:\n\t%r = call i32 @_Checked__Fi(i32 %n)\n\t%t = thrown i1\n\t%s = add i32 %r, 1\n\tbrif %t, land, on\n"
		"land:\n\tthrow\non:\n\tret i32 %s\n}\n", "_Caller__Fi", false);

	//A callee that only returns: the caller's code for an exception goes.
	AssertIRInlined(
		"function i32 @_Caller__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = add i32 %n, %n\n"
		"\t%s = add i32 %0, 1\n"
		"\tret i32 %s\n"
		"}\n",
		"space default 4\n\n"
		"function i32 @_Twice__Fi(i32 %x)\n{\nentry:\n\t%d = add i32 %x, %x\n\tret i32 %d\n}\n\n"
		TEST_IR_TESTED_CALLER("_Twice__Fi"), "_Caller__Fi", true);

	//No test after the call: the callee's throw would be the caller's, so the call stays.
	AssertIRInlined(
		"function i32 @_Caller__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%r = call i32 @_Checked__Fi(i32 %n)\n"
		"\tret i32 %r\n"
		"}\n",
		"space default 4\n\n" TEST_IR_CHECKED
		"function i32 @_Caller__Fi(i32 %n)\n{\nentry:\n\t%r = call i32 @_Checked__Fi(i32 %n)\n\tret i32 %r\n}\n", "_Caller__Fi", false);

	//A callee marked nothrow whose throw is behind the test of a call to another that is: the throw isn't reached, in
	//the callee or in its copy, and the caller (nothrow too) verifies before it is optimised and after.
	AssertIRInlined(
		"function i32 @_Caller__Fi(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%0 = call i32 @_Safe__Fi(i32 %n)\n"
		"\t%s = add i32 %0, 1\n"
		"\tret i32 %s\n"
		"}\n",
		"space default 4\n"
		"nothrow @_Safe__Fi\n"
		"nothrow @_Wraps__Fi\n"
		"nothrow @_Caller__Fi\n\n"
		"function i32 @_Wraps__Fi(i32 %x)\n{\nentry:\n\t%v = call i32 @_Safe__Fi(i32 %x)\n\t%t = thrown i1\n\tbrif %t, bad, good\nbad:\n\tthrow\ngood:\n\tret i32 %v\n}\n\n"
		TEST_IR_TESTED_CALLER("_Wraps__Fi"), "_Caller__Fi", true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIR(void)
{
	BeginTests();

	TestIRCompareIntegers();
	TestIRRoundTripLoop();
	TestIRRoundTripRegisterCall();
	TestIRRoundTripNoWrites();
	TestIRRoundTripEverything();
	TestIRRoundTripSpaces();
	TestIRBuild();
	TestIRParseForwardReferences();
	TestIRRoundTripNumbered();
	TestIRTargetOpcodes();
	TestIRLarge();
	TestIRParserErrors();
	TestIRVerifierErrors();
	TestIRVerifierBroken();
	TestIRDominators();
	TestIRLoops();
	TestIRDefaultSpace();
	TestIRComparisonOpcodes();
	TestIRComparisonColumns();
	TestIROpcodeClasses();
	TestIRPassTable();
	TestIRSymbolAttributes();
	TestIREdgeCondition();
	TestIROptimiserPointerConstantWidth();
	TestIROptimiserHoistableDivisions();
	TestIROptimiser();
	TestIROptimiserConstantsRight();
	TestIROptimiserFoldedInlinedPhis();
	TestIROptimiserLoopInvariants();
	TestIROptimiserRestrict();
	TestIROptimiserRestrictParameters();
	TestIRImportsRestrict();
	TestIRImportsCalls();
	TestIRInlineForwarders();
	TestIRInlineRecursive();
	TestIROptimiserLoopCounters();
	TestIROptimiserZeroExtendedCounters();
	TestIROptimiserSharedDivisions();
	TestIROptimiserSinkLoads();
	TestIROptimiserSplitSlotNames();
	TestIROptimiserNoWritesCalls();
	TestIROptimiserExitValues();
	TestIROptimiserSinkAndHoist();
	TestIROptimiserReduceMultiplies();
	TestIROptimiserTimesZero();
	TestIROptimiserPointerOffsets();
	TestIROptimiserNarrowCounters();
	TestIROptimiserBiasSignedIndices();
	TestIROptimiserMergeAddressPhis();
	TestIROptimiserConstantDivisions();
	TestIROptimiserDivisionCosts();
	TestIROptimiserFoldNullTests();
	TestIROptimiserKnownBranches();
	TestIROptimiserGuardedMultiples();
	TestIROptimiserCounterDivisions();
	TestIROptimiserIntegerOffsets();
	TestIROptimiserNarrowTruncations();
	TestIROptimiserUnifySlotTypes();
	TestIROptimiserRotateLoops();
	TestIRSSAUpdater();
	TestIRLiveness();
	TestIRDominanceFrontiers();
	TestIRRoundTripFlags();
	TestIRCopyFunction();
	TestIRVerifierFlagErrors();
	TestIRVerifierUnusedOperand();
	TestIRThrow();
	TestIRThrown();
	TestIRNoThrow();
	TestIRInlineThrows();

	TestStatistics();
}
