#include "CppParserLib/IRParser.h"
#include "CppParserLib/IRPrinter.h"
#include "CppParserLib/IRVerifier.h"
#include "CppParserLib/IROptimiser.h"
#include "CppParserLib/IRLoops.h"
#include "CppParserLib/IRImports.h"
#include "CppParserLib/IRInliner.h"
#include "CppParserLib/IRSSAUpdater.h"
#include "CppParserLib/IRLiveness.h"
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
// W65C816 spaces: far by default, near and zero.
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
	AssertIRErrors("(1, 1): Expected space, global, function or restrict. [fun]\n", "fun");
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
#define IR_TEST_NARROW_MULTIPLIES	0x04	//SetNarrowMultiplies (the W65C816's).
#define IR_TEST_NARROW_COUNTERS		0x08	//SetNarrowCounters (the W65C816's).
#define IR_TEST_BIASED_INDICES		0x10	//SetBiasedIndices (the W65C816's [p],Y).
#define IR_TEST_CONSTANT_DIVISIONS	0x20	//SetConstantDivisions (the W65C816's).


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

	//Every instruction parsed is from line 7: the ones optimising makes must have a line too (7, or 0 for none).
	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	CIRBlock::suiSource = 7;
	AssertTrue(cParser.Parse(szText));
	CIRBlock::suiSource = 0;
	cParser.Kill();
	cOptimiser.Init(&cModule);
	cOptimiser.SetIndexedAddresses((uiOptions & IR_TEST_INDEXED_ADDRESSES) != 0);
	cOptimiser.SetNarrowMultiplies((uiOptions & IR_TEST_NARROW_MULTIPLIES) != 0);
	cOptimiser.SetNarrowCounters((uiOptions & IR_TEST_NARROW_COUNTERS) != 0);
	cOptimiser.SetBiasedIndices((uiOptions & IR_TEST_BIASED_INDICES) != 0);
	cOptimiser.SetConstantDivisions((uiOptions & IR_TEST_CONSTANT_DIVISIONS) != 0);
	if (uiOptions & IR_TEST_LIBRARY)
	{
		cOptimiser.SetLibraryInt(IRType(IRTK_I16));
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
	AssertInt(1, CIRLiveness::LoopWeight(0));
	AssertInt(64, CIRLiveness::LoopWeight(2));
	AssertInt(32768, CIRLiveness::LoopWeight(5));
	AssertInt(32768, CIRLiveness::LoopWeight(9));

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
//
//////////////////////////////////////////////////////////////////////////
void TestIR(void)
{
	BeginTests();

	TestIRRoundTripLoop();
	TestIRRoundTripEverything();
	TestIRRoundTripSpaces();
	TestIRBuild();
	TestIRParseForwardReferences();
	TestIRRoundTripNumbered();
	TestIRTargetOpcodes();
	TestIRLarge();
	TestIRParserErrors();
	TestIRVerifierErrors();
	TestIRDominators();
	TestIRLoops();
	TestIROptimiser();
	TestIROptimiserConstantsRight();
	TestIROptimiserLoopInvariants();
	TestIROptimiserRestrict();
	TestIROptimiserRestrictParameters();
	TestIRImportsRestrict();
	TestIRInlineRecursive();
	TestIROptimiserLoopCounters();
	TestIROptimiserZeroExtendedCounters();
	TestIROptimiserSharedDivisions();
	TestIROptimiserSinkLoads();
	TestIROptimiserExitValues();
	TestIROptimiserSinkAndHoist();
	TestIROptimiserReduceMultiplies();
	TestIROptimiserTimesZero();
	TestIROptimiserNarrowCounters();
	TestIROptimiserBiasSignedIndices();
	TestIROptimiserMergeAddressPhis();
	TestIROptimiserConstantDivisions();
	TestIROptimiserFoldNullTests();
	TestIROptimiserNarrowTruncations();
	TestIROptimiserUnifySlotTypes();
	TestIROptimiserRotateLoops();
	TestIRSSAUpdater();
	TestIRLiveness();
	TestIRDominanceFrontiers();
	TestIRRoundTripFlags();
	TestIRVerifierFlagErrors();

	TestStatistics();
}
