#include "CppCompilerLib/IRParser.h"
#include "CppCompilerLib/IRVerifier.h"
#include "CppCompilerLib/IRInterpreter.h"
#include "TestLib/Assert.h"


#define IR_TEST_FAILED	((int64)0x7FFFFFFFFFFFFFFFLL)


//////////////////////////////////////////////////////////////////////////
//
// Runs @Test of the IR text with up to three integer arguments.  What it
// returns (sign extended from its type), or IR_TEST_FAILED with why in
// pszError.
//
//////////////////////////////////////////////////////////////////////////
static int64 TestIRRun(const char* szText, size uiNumArguments, int64 lliFirst, int64 lliSecond, int64 lliThird, CChars* pszError, size uiSteps = IR_INTERPRETER_STEPS, size uiDepth = IR_INTERPRETER_DEPTH)
{
	CIRModule		cModule;
	CCPPErrors		cErrors;
	CIRParser		cParser;
	CIRVerifier		cVerifier;
	CIRInterpreter	cInterpreter;
	CIRFunction*	pcFunction;
	SIRValue		asArguments[3];
	SIRValue		sResult;
	int64			lliResult;
	int				iSymbol;
	bool			bResult;

	cModule.Init();
	cErrors.Init();
	cParser.Init(&cModule, &cErrors);
	bResult = cParser.Parse(szText);
	cParser.Kill();
	if (bResult)
	{
		cVerifier.Init(&cModule, &cErrors);
		bResult = cVerifier.Verify();
		cVerifier.Kill();
	}
	AssertString("", cErrors.Text());
	cErrors.Kill();

	lliResult = IR_TEST_FAILED;
	iSymbol = cModule.FindSymbol("Test");
	pcFunction = (bResult && (iSymbol >= 0)) ? cModule.FindFunction((uint32)iSymbol) : NULL;
	AssertNotNull(pcFunction);
	if (pcFunction)
	{
		asArguments[0].Set(IRTruncate((uiNumArguments > 0) ? pcFunction->TypeOf(*pcFunction->masParameters.Get(0)) : IRType(IRTK_I64), (uint64)lliFirst));
		asArguments[1].Set(IRTruncate((uiNumArguments > 1) ? pcFunction->TypeOf(*pcFunction->masParameters.Get(1)) : IRType(IRTK_I64), (uint64)lliSecond));
		asArguments[2].Set(IRTruncate((uiNumArguments > 2) ? pcFunction->TypeOf(*pcFunction->masParameters.Get(2)) : IRType(IRTK_I64), (uint64)lliThird));
		cInterpreter.Init();
		cInterpreter.SetLimits(uiSteps, uiDepth);
		if (cInterpreter.Call(&cModule, pcFunction, asArguments, uiNumArguments, &sResult))
		{
			lliResult = pcFunction->msReturnType.IsInteger() ? IRSignExtend(pcFunction->msReturnType, sResult.ulliBits) : (int64)sResult.ulliBits;
		}
		else if (pszError)
		{
			pszError->Append(cInterpreter.GetError());
		}
		cInterpreter.Kill();
	}
	cModule.Kill();
	return lliResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertIRValue(int64 lliExpected, const char* szText, size uiNumArguments = 0, int64 lliFirst = 0, int64 lliSecond = 0, int64 lliThird = 0)
{
	CChars	szError;

	szError.Init();
	AssertLong(lliExpected, TestIRRun(szText, uiNumArguments, lliFirst, lliSecond, lliThird, &szError));
	AssertString("", szError.Text());
	szError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertIRNoValue(const char* szExpected, const char* szText, size uiNumArguments = 0, int64 lliFirst = 0, int64 lliSecond = 0, size uiSteps = IR_INTERPRETER_STEPS, size uiDepth = IR_INTERPRETER_DEPTH)
{
	CChars	szError;

	szError.Init();
	AssertLong(IR_TEST_FAILED, TestIRRun(szText, uiNumArguments, lliFirst, lliSecond, 0, &szError, uiSteps, uiDepth));
	AssertString(szExpected, szError.Text());
	szError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Integers at each width: results wrap to the type, signed and unsigned
// division and comparison differ, conversions widen as they say.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRInterpreterIntegers(void)
{
	const char*	szWrap =
		"space default 4\n"
		"\n"
		"function i32 @Test(i8 %a, i8 %b)\n"
		"{\n"
		"b0:\n"
		"\t%s = add i8 %a, %b\n"
		"\t%z = zext i8 %s to i32\n"
		"\t%x = sext i8 %s to i32\n"
		"\t%m = mul i32 %z, 1000\n"
		"\t%r = add i32 %m, %x\n"
		"\tret i32 %r\n"
		"}\n";
	const char*	szDivide =
		"space default 4\n"
		"\n"
		"function i32 @Test(i16 %a, i16 %b)\n"
		"{\n"
		"b0:\n"
		"\t%q = sdiv i16 %a, %b\n"
		"\t%u = udiv i16 %a, %b\n"
		"\t%m = srem i16 %a, %b\n"
		"\t%c = slt i16 %a, %b\n"
		"\t%d = ult i16 %a, %b\n"
		"\t%qx = sext i16 %q to i32\n"
		"\t%ux = zext i16 %u to i32\n"
		"\t%mx = sext i16 %m to i32\n"
		"\t%cx = zext i1 %c to i32\n"
		"\t%dx = zext i1 %d to i32\n"
		"\t%t1 = mul i32 %ux, 10\n"
		"\t%t2 = add i32 %t1, %qx\n"
		"\t%t3 = mul i32 %t2, 10\n"
		"\t%t4 = add i32 %t3, %mx\n"
		"\t%t5 = mul i32 %t4, 10\n"
		"\t%t6 = add i32 %t5, %cx\n"
		"\t%t7 = mul i32 %t6, 10\n"
		"\t%t8 = add i32 %t7, %dx\n"
		"\tret i32 %t8\n"
		"}\n";
	const char*	szShift =
		"space default 4\n"
		"\n"
		"function i64 @Test(i64 %a, i64 %n)\n"
		"{\n"
		"b0:\n"
		"\t%l = shl i64 %a, %n\n"
		"\t%r = ashr i64 %l, 1\n"
		"\t%x = xor i64 %r, -1\n"
		"\t%g = neg i64 %x\n"
		"\tret i64 %g\n"
		"}\n";

	//200 + 100 is 44 in a byte, either way it is widened.
	AssertIRValue(44044, szWrap, 2, 200, 100);
	//100 + 100 is 200: 200 unsigned, -56 signed.
	AssertIRValue(200000 - 56, szWrap, 2, 100, 100);

	//-7 and 2: -7 / 2 is -3 and -7 % 2 is -1; as unsigned 65529 / 2 is 32764; -7 < 2 signed, not unsigned.
	AssertIRValue(((((32764 * 10) + -3) * 10 + -1) * 10 + 1) * 10 + 0, szDivide, 2, -7, 2);
	AssertIRValue((((((3 * 10) + 3) * 10 + 1) * 10 + 0) * 10) + 0, szDivide, 2, 7, 2);

	//((3 << 4) >> 1) is 24; -(~24) is 25.
	AssertIRValue(25, szShift, 2, 3, 4);

	AssertIRNoValue("It divides by zero, or the least number by -1", szDivide, 2, 7, 0);
	AssertIRNoValue("It shifts by the width of the number or more", szShift, 2, 3, 64);
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks, phis (read together), branches, a loop, and calls: a function
// that calls itself, with a limit on how deep and on how long.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRInterpreterControl(void)
{
	const char*	szLoop =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\tjmp loop\n"
		"loop:\n"
		"\t%a = phi i32 [entry, 0], [body, %b]\n"
		"\t%b = phi i32 [entry, 1], [body, %c]\n"
		"\t%i = phi i32 [entry, 0], [body, %i2]\n"
		"\t%t = slt i32 %i, %n\n"
		"\tbrif %t, body, exit\n"
		"body:\n"
		"\t%c = add i32 %a, %b\n"
		"\t%i2 = add i32 %i, 1\n"
		"\tjmp loop\n"
		"exit:\n"
		"\tret i32 %a\n"
		"}\n";
	const char*	szFactorial =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%c = sle i32 %n, 1\n"
		"\tbrif %c, one, more\n"
		"one:\n"
		"\tret i32 1\n"
		"more:\n"
		"\t%m = sub i32 %n, 1\n"
		"\t%f = call i32 @Test(i32 %m)\n"
		"\t%r = mul i32 %n, %f\n"
		"\tret i32 %r\n"
		"}\n";
	const char*	szForever =
		"space default 4\n"
		"\n"
		"function i32 @Test()\n"
		"{\n"
		"b0:\n"
		"\tjmp again\n"
		"again:\n"
		"\tjmp again\n"
		"}\n";
	const char*	szDeeper =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%m = add i32 %n, 1\n"
		"\t%f = call i32 @Test(i32 %m)\n"
		"\tret i32 %f\n"
		"}\n";
	const char*	szOther =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%f = call i32 @Elsewhere(i32 %n)\n"
		"\tret i32 %f\n"
		"}\n";

	//The tenth Fibonacci number: the phis of a and b are read before either is written.
	AssertIRValue(55, szLoop, 1, 10);
	AssertIRValue(0, szLoop, 1, 0);
	AssertIRValue(3628800, szFactorial, 1, 10);

	AssertIRNoValue("It takes too long to work out", szForever, 0, 0, 0, 10000);
	AssertIRNoValue("Its calls go too deep to be worked out", szDeeper, 1, 0, 0, IR_INTERPRETER_STEPS, 100);
	AssertIRNoValue("It calls what isn't a function whose value can be worked out [Elsewhere]", szOther, 1, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Memory is objects: slots, arguments where they were put, and data that
// never changes.  What is outside an object, never written, not the
// program's to write or gone is no value.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRInterpreterMemory(void)
{
	const char*	szSlots =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"entry:\n"
		"\t%a = slot ptr 16, 4\n"
		"\tjmp fill\n"
		"fill:\n"
		"\t%i = phi i32 [entry, 0], [fill, %i2]\n"
		"\t%o = shl i32 %i, 2\n"
		"\t%p = ptradd ptr %a, %o\n"
		"\t%v = mul i32 %i, %i\n"
		"\tstore i32 %v, %p\n"
		"\t%i2 = add i32 %i, 1\n"
		"\t%c = slt i32 %i2, 4\n"
		"\tbrif %c, fill, read\n"
		"read:\n"
		"\t%q = shl i32 %n, 2\n"
		"\t%r = ptradd ptr %a, %q\n"
		"\t%x = load i32 %r\n"
		"\t%lo = load i16 %r\n"
		"\t%lx = zext i16 %lo to i32\n"
		"\t%s = add i32 %x, %lx\n"
		"\tret i32 %s\n"
		"}\n";
	const char*	szUnwritten =
		"space default 4\n"
		"\n"
		"function i32 @Test()\n"
		"{\n"
		"b0:\n"
		"\t%a = slot ptr 4, 4\n"
		"\tstore i16 1, %a\n"
		"\t%x = load i32 %a\n"
		"\tret i32 %x\n"
		"}\n";
	const char*	szData =
		"space default 4\n"
		"\n"
		"global @gTable, align 4, local, constant { i32 10, i16 -2, i8 7, ptr @gText+1, zero 3 }\n"
		"\n"
		"global @gText, align 1, local, constant { bytes \"abc\\00\" }\n"
		"\n"
		"global @gCounter, align 4 { i32 5 }\n"
		"\n"
		"function i32 @Test(i32 %what)\n"
		"{\n"
		"b0:\n"
		"\t%a = load i32 @gTable\n"
		"\t%p1 = ptradd ptr @gTable, 4\n"
		"\t%b = load i16 %p1\n"
		"\t%bx = sext i16 %b to i32\n"
		"\t%p2 = ptradd ptr @gTable, 7\n"
		"\t%t = load ptr %p2\n"
		"\t%ch = load i8 %t\n"
		"\t%cx = zext i8 %ch to i32\n"
		"\t%s1 = add i32 %a, %bx\n"
		"\t%s2 = add i32 %s1, %cx\n"
		"\t%c0 = eq i32 %what, 0\n"
		"\tbrif %c0, done, more\n"
		"done:\n"
		"\tret i32 %s2\n"
		"more:\n"
		"\t%c1 = eq i32 %what, 1\n"
		"\tbrif %c1, write, more2\n"
		"write:\n"
		"\tstore i32 1, @gTable\n"
		"\tret i32 0\n"
		"more2:\n"
		"\t%c2 = eq i32 %what, 2\n"
		"\tbrif %c2, variable, past\n"
		"variable:\n"
		"\t%g = load i32 @gCounter\n"
		"\tret i32 %g\n"
		"past:\n"
		"\t%p3 = ptradd ptr @gText, 2\n"
		"\t%w = load i32 %p3\n"
		"\tret i32 %w\n"
		"}\n";
	const char*	szDangling =
		"space default 4\n"
		"\n"
		"function ptr @Local()\n"
		"{\n"
		"b0:\n"
		"\t%a = slot ptr 4, 4\n"
		"\tstore i32 3, %a\n"
		"\tret ptr %a\n"
		"}\n"
		"\n"
		"function i32 @Test()\n"
		"{\n"
		"b0:\n"
		"\t%p = call ptr @Local()\n"
		"\t%x = load i32 %p\n"
		"\tret i32 %x\n"
		"}\n";
	const char*	szArguments =
		"space default 4\n"
		"\n"
		"function i32 @Twice(i16 %a, i32 %b)\n"
		"{\n"
		"b0:\n"
		"\t%p = argaddr ptr 1\n"
		"\t%x = load i32 %p\n"
		"\t%y = add i32 %x, %x\n"
		"\tstore i32 %y, %p\n"
		"\t%z = load i32 %p\n"
		"\t%q = argaddr ptr 0\n"
		"\t%s = load i16 %q\n"
		"\t%sx = sext i16 %s to i32\n"
		"\t%r = add i32 %z, %sx\n"
		"\t%r2 = add i32 %r, %b\n"
		"\tret i32 %r2\n"
		"}\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%r = call i32 @Twice(i16 -1, i32 %n)\n"
		"\tret i32 %r\n"
		"}\n";
	const char*	szNull =
		"space default 4\n"
		"\n"
		"function i32 @Test(ptr %p)\n"
		"{\n"
		"b0:\n"
		"\t%x = load i32 %p\n"
		"\tret i32 %x\n"
		"}\n";

	//Element 3 is 9, and its low half is 9 too.
	AssertIRValue(18, szSlots, 1, 3);
	AssertIRNoValue("It reads or writes outside an object", szSlots, 1, 4);
	AssertIRNoValue("It reads or writes outside an object", szSlots, 1, -1);
	AssertIRNoValue("It reads a variable that has no value yet", szUnwritten);

	//10 + -2 + 'b': the table's pointer is into the text.
	AssertIRValue(10 - 2 + 'b', szData, 1, 0);
	AssertIRNoValue("It writes what can't be changed [gTable]", szData, 1, 1);
	AssertIRNoValue("It reads what isn't a constant [gCounter]", szData, 1, 2);
	AssertIRNoValue("It reads or writes outside an object [gText]", szData, 1, 3);

	AssertIRNoValue("It reads or writes a local of a function that has returned", szDangling);

	//2n where it was put, -1 beside it, and n as the parameter still is.
	AssertIRValue(20 - 1 + 10, szArguments, 1, 10);
	AssertIRNoValue("It reads or writes through a null pointer", szNull, 1, 0);
	AssertIRNoValue("It reads or writes through a pointer made from a number", szNull, 1, 1234);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer is an object and an offset whatever it is carried as: copied
// a piece at a time as integers, compared, subtracted, and called.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRInterpreterPointers(void)
{
	const char*	szPieces =
		"space default 4\n"
		"\n"
		"function i32 @Test()\n"
		"{\n"
		"b0:\n"
		"\t%x = slot ptr 4, 4\n"
		"\t%from = slot ptr 6, 2\n"
		"\t%to = slot ptr 6, 2\n"
		"\tstore i32 77, %x\n"
		"\tstore i8 1, %from\n"
		"\t%f1 = ptradd ptr %from, 1\n"
		"\tstore ptr %x, %f1\n"
		"\t%f5 = ptradd ptr %from, 5\n"
		"\tstore i8 2, %f5\n"
		"\t%a = load i32 %from\n"
		"\tstore i32 %a, %to\n"
		"\t%f4 = ptradd ptr %from, 4\n"
		"\t%b = load i16 %f4\n"
		"\t%t4 = ptradd ptr %to, 4\n"
		"\tstore i16 %b, %t4\n"
		"\t%t1 = ptradd ptr %to, 1\n"
		"\t%p = load ptr %t1\n"
		"\t%v = load i32 %p\n"
		"\tret i32 %v\n"
		"}\n";
	const char*	szCompare =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %what)\n"
		"{\n"
		"b0:\n"
		"\t%a = slot ptr 8, 4\n"
		"\t%b = slot ptr 8, 4\n"
		"\t%a4 = ptradd ptr %a, 4\n"
		"\t%same = eq ptr %a, %a\n"
		"\t%other = ne ptr %a, %b\n"
		"\t%null = ne ptr %a, 0\n"
		"\t%less = ult ptr %a, %a4\n"
		"\t%ai = ptrtoi ptr %a to i32\n"
		"\t%a4i = ptrtoi ptr %a4 to i32\n"
		"\t%d = sub i32 %a4i, %ai\n"
		"\t%s1 = zext i1 %same to i32\n"
		"\t%s2 = zext i1 %other to i32\n"
		"\t%s3 = zext i1 %null to i32\n"
		"\t%s4 = zext i1 %less to i32\n"
		"\t%t1 = add i32 %s1, %s2\n"
		"\t%t2 = add i32 %t1, %s3\n"
		"\t%t3 = add i32 %t2, %s4\n"
		"\t%t4 = mul i32 %t3, 10\n"
		"\t%t5 = add i32 %t4, %d\n"
		"\t%c = eq i32 %what, 0\n"
		"\tbrif %c, done, bad\n"
		"done:\n"
		"\tret i32 %t5\n"
		"bad:\n"
		"\t%c1 = eq i32 %what, 1\n"
		"\tbrif %c1, order, number\n"
		"order:\n"
		"\t%o = ult ptr %a, %b\n"
		"\t%ox = zext i1 %o to i32\n"
		"\tret i32 %ox\n"
		"number:\n"
		"\t%m = mul i32 %ai, 2\n"
		"\tret i32 %m\n"
		"}\n";
	const char*	szFunctions =
		"space default 4\n"
		"\n"
		"global @gTable, align 4, local, constant { ptr @Double, ptr @Square }\n"
		"\n"
		"function i32 @Double(i32 %x)\n"
		"{\n"
		"b0:\n"
		"\t%r = add i32 %x, %x\n"
		"\tret i32 %r\n"
		"}\n"
		"\n"
		"function i32 @Square(i32 %x)\n"
		"{\n"
		"b0:\n"
		"\t%r = mul i32 %x, %x\n"
		"\tret i32 %r\n"
		"}\n"
		"\n"
		"function i32 @Test(i32 %which, i32 %x)\n"
		"{\n"
		"b0:\n"
		"\t%o = shl i32 %which, 2\n"
		"\t%p = ptradd ptr @gTable, %o\n"
		"\t%f = load ptr %p\n"
		"\t%r = call i32 %f(i32 %x)\n"
		"\tret i32 %r\n"
		"}\n";

	//The pointer is at 1 to 4 of six bytes, so each piece carries part of it.
	AssertIRValue(77, szPieces);
	AssertIRValue(44, szCompare, 1, 0);
	AssertIRNoValue("It compares pointers to different objects", szCompare, 1, 1);
	AssertIRNoValue("It uses a pointer as a number", szCompare, 1, 2);
	AssertIRValue(14, szFunctions, 2, 0, 7);
	AssertIRValue(49, szFunctions, 2, 1, 7);
}


//////////////////////////////////////////////////////////////////////////
//
// Floating point, as the host's float and double.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRInterpreterFloats(void)
{
	const char*	szFloats =
		"space default 4\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%f = sitof i32 %n to f32\n"
		"\t%h = fmul f32 %f, 0x3FC00000\n"
		"\t%d = fext f32 %h to f64\n"
		"\t%e = fadd f64 %d, 0x3FD0000000000000\n"
		"\t%c = fgt f64 %e, 0x4000000000000000\n"
		"\t%i = ftosi f64 %e to i32\n"
		"\t%cx = zext i1 %c to i32\n"
		"\t%t = mul i32 %i, 10\n"
		"\t%r = add i32 %t, %cx\n"
		"\tret i32 %r\n"
		"}\n";
	const char*	szTooBig =
		"space default 4\n"
		"\n"
		"function i32 @Test()\n"
		"{\n"
		"b0:\n"
		"\t%i = ftosi f64 0x4202A05F20000000 to i16\n"
		"\t%x = sext i16 %i to i32\n"
		"\tret i32 %x\n"
		"}\n";

	//3 * 1.5 + 0.25 is 4.75: 4 as an integer, and more than 2.  -3 gives -4.25: -4, and not.
	AssertIRValue(41, szFloats, 1, 3);
	AssertIRValue(-40, szFloats, 1, -3);
	AssertIRNoValue("It converts a floating point number to an integer that can't hold it", szTooBig);
}


//////////////////////////////////////////////////////////////////////////
//
// An exception can't be worked out: a function that throws on one path has
// a value on the other only.  thrown is false after a call that returned.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRInterpreterThrow(void)
{
	const char*	szChecked =
		"space default 4\n"
		"\n"
		"function i32 @Checked(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%c = slt i32 %n, 0\n"
		"\tbrif %c, bad, good\n"
		"bad:\n"
		"\tthrow\n"
		"good:\n"
		"\tret i32 %n\n"
		"}\n"
		"\n"
		"function i32 @Test(i32 %n)\n"
		"{\n"
		"b0:\n"
		"\t%r = call i32 @Checked(i32 %n)\n"
		"\t%t = thrown i1\n"
		"\tbrif %t, caught, fine\n"
		"caught:\n"
		"\tret i32 -1\n"
		"fine:\n"
		"\t%s = add i32 %r, 1\n"
		"\tret i32 %s\n"
		"}\n";

	AssertIRValue(6, szChecked, 1, 5);
	AssertIRNoValue("It throws an exception", szChecked, 1, -5);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterpreter(void)
{
	BeginTests();

	TestIRInterpreterIntegers();
	TestIRInterpreterControl();
	TestIRInterpreterMemory();
	TestIRInterpreterPointers();
	TestIRInterpreterFloats();
	TestIRInterpreterThrow();

	TestStatistics();
}
