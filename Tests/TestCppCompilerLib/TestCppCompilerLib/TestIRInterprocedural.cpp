#include "CppCompilerLib/IRInterprocedural.h"
#include "CppCompilerLib/IROptimiser.h"
#include "CppCompilerLib/IRParser.h"
#include "CppCompilerLib/IRPrinter.h"
#include "TestLib/Assert.h"


//The unit's functions for CIRInterprocedural, without a front end: the module's, each kept until it's removed, unless
//the generator's text is pretended to name it (mbNamed).
class CTestIRUnit : public CIRUnitFunctions
{
public:
	CIRModule						mcModule;
	CArrayTemplate<CIRFunction*>	mapcFunctions;
	CArrayTemplate<size>			mauiCopiedFrom;		//Per AddSpecialised.
	bool							mbNamed;

	void Init(const char* szText)
	{
		CCPPErrors		cErrors;
		CIRParser		cParser;
		CIRFunction*	pcFunction;
		size			ui;

		mcModule.Init();
		cErrors.Init();
		cParser.Init(&mcModule, &cErrors);
		AssertTrue(cParser.Parse(szText));
		cParser.Kill();
		cErrors.Kill();
		mapcFunctions.Init();
		mauiCopiedFrom.Init();
		for (ui = 0; ui < mcModule.NumFunctions(); ui++)
		{
			pcFunction = mcModule.GetFunction(ui);
			mapcFunctions.Add(&pcFunction);
		}
		mbNamed = false;
	}

	void Kill(void)
	{
		mauiCopiedFrom.Kill();
		mapcFunctions.Kill();
		mcModule.Kill();
	}

	size NumUnitFunctions(void)
	{
		return mapcFunctions.NumElements();
	}

	CIRFunction* GetUnitFunction(size uiFunction)
	{
		return *mapcFunctions.Get(uiFunction);
	}

	void InitOptimiser(CIROptimiser* pcOptimiser)
	{
		pcOptimiser->Init(&mcModule);
	}

	void AddSpecialised(size uiFunction, CIRFunction* pcCopy)
	{
		mapcFunctions.Add(&pcCopy);
		mauiCopiedFrom.Add(&uiFunction);
	}

	bool RemoveSpecialised(size uiFunction)
	{
		if (mbNamed)
		{
			return false;
		}
		mcModule.RemoveFunction(*mapcFunctions.Get(uiFunction));
		*mapcFunctions.Get(uiFunction) = NULL;
		return true;
	}

	CIRFunction* Find(const char* szName)
	{
		int	iSymbol;

		iSymbol = mcModule.FindSymbol(szName);
		return (iSymbol >= 0) ? mcModule.FindFunction((uint32)iSymbol) : NULL;
	}

	size Index(const char* szName)
	{
		size	ui;

		for (ui = 0; ui < mapcFunctions.NumElements(); ui++)
		{
			if (*mapcFunctions.Get(ui) == Find(szName))
			{
				return ui;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	//The function as printed, or "" if it has gone.
	void Print(const char* szName, CChars* psz)
	{
		CIRPrinter		cPrinter;
		CIRFunction*	pcFunction;

		psz->Init();
		pcFunction = Find(szName);
		if (pcFunction)
		{
			cPrinter.Init(&mcModule, psz);
			cPrinter.PrintFunction(pcFunction);
			cPrinter.Kill();
		}
	}

	bool Calls(const char* szCaller, const char* szCallee)
	{
		CChars	sz;
		CChars	szCall;
		bool	bCalls;

		Print(szCaller, &sz);
		szCall.Init("@");
		szCall.Append(szCallee);
		szCall.Append('(');
		bCalls = sz.Contains(szCall.Text());
		szCall.Kill();
		sz.Kill();
		return bCalls;
	}
};


//A count (a shift's) that is strong, and a value that isn't.
#define TEST_IR_SHIFT_BY \
	"function i32 @Shift(i32 %x, i32 %n), local\n{\n0:\n\t%r = shl i32 %x, %n\n\tret i32 %r\n}\n"


//////////////////////////////////////////////////////////////////////////
//
// IsRecursive: through the module's functions, directly or round a cycle,
// not through another unit's function or to a callee that recurses without
// coming back.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralRecursion(void)
{
	CTestIRUnit	cUnit;

	cUnit.Init(
		"function void @Self()\n{\n0:\n\tcall void @Self()\n\tret\n}\n"
		"function void @Even()\n{\n0:\n\tcall void @Odd()\n\tret\n}\n"
		"function void @Odd()\n{\n0:\n\tcall void @Even()\n\tret\n}\n"
		"function void @Leaf()\n{\n0:\n\tret\n}\n"
		"function void @Caller()\n{\n0:\n\tcall void @Self()\n\tcall void @Leaf()\n\tcall void @Outside()\n\tret\n}\n");
	AssertTrue(CIRInterprocedural::IsRecursive(&cUnit.mcModule, cUnit.Find("Self")));
	AssertTrue(CIRInterprocedural::IsRecursive(&cUnit.mcModule, cUnit.Find("Even")));
	AssertTrue(CIRInterprocedural::IsRecursive(&cUnit.mcModule, cUnit.Find("Odd")));
	AssertFalse(CIRInterprocedural::IsRecursive(&cUnit.mcModule, cUnit.Find("Leaf")));
	AssertFalse(CIRInterprocedural::IsRecursive(&cUnit.mcModule, cUnit.Find("Caller")));
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// MarkNoWrites: a store to a global writes, and so does what calls it, or
// another unit's function; functions calling each other and storing nothing
// don't, nor does a store to a function's own slot.  CallsNoWrites gives the
// callers of those that don't.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralNoWrites(void)
{
	CTestIRUnit			cUnit;
	CIRInterprocedural	cPasses;

	cUnit.Init(
		"space default 4\n\n"
		"function void @Poke()\n{\n0:\n\tstore i32 1, @G\n\tret\n}\n"
		"function void @CallsPoke()\n{\n0:\n\tcall void @Poke()\n\tret\n}\n"
		"function void @CallsOutside()\n{\n0:\n\tcall void @Outside()\n\tret\n}\n"
		"function void @Even()\n{\n0:\n\tcall void @Odd()\n\tret\n}\n"
		"function void @Odd()\n{\n0:\n\tcall void @Even()\n\tret\n}\n"
		"function i32 @Frame()\n{\n0:\n\t%s = slot ptr 4, 4\n\tstore i32 1, %s\n\t%v = load i32 %s\n\tret i32 %v\n}\n"
		"function i32 @CallsFrame()\n{\n0:\n\t%v = call i32 @Frame()\n\tret i32 %v\n}\n");
	cPasses.Init(&cUnit.mcModule, &cUnit);
	AssertFalse(cPasses.CallsNoWrites(cUnit.Index("Even")));
	cPasses.MarkNoWrites();
	AssertFalse(cUnit.mcModule.IsNoWrites(cUnit.Find("Poke")->muiSymbol));
	AssertFalse(cUnit.mcModule.IsNoWrites(cUnit.Find("CallsPoke")->muiSymbol));
	AssertFalse(cUnit.mcModule.IsNoWrites(cUnit.Find("CallsOutside")->muiSymbol));
	AssertTrue(cUnit.mcModule.IsNoWrites(cUnit.Find("Even")->muiSymbol));
	AssertTrue(cUnit.mcModule.IsNoWrites(cUnit.Find("Odd")->muiSymbol));
	AssertTrue(cUnit.mcModule.IsNoWrites(cUnit.Find("Frame")->muiSymbol));
	AssertTrue(cUnit.mcModule.IsNoWrites(cUnit.Find("CallsFrame")->muiSymbol));
	AssertTrue(cPasses.CallsNoWrites(cUnit.Index("CallsFrame")));
	AssertTrue(cPasses.CallsNoWrites(cUnit.Index("Even")));
	AssertFalse(cPasses.CallsNoWrites(cUnit.Index("CallsPoke")));
	AssertFalse(cPasses.CallsNoWrites(cUnit.Index("Frame")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Frame")));
	cPasses.Kill();
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// DeadParameters: what isn't read, as bits; a member function's this stays;
// none if a parameter's address is taken.  RemoveDeadArguments: the
// parameter and its calls' arguments go, and both functions are changed.
// Not for a function another unit can call, or whose address is taken.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralDeadArguments(void)
{
	CTestIRUnit			cUnit;
	CIRInterprocedural	cPasses;
	CChars				sz;

	cUnit.Init(
		"space default 4\n\n"
		"function i32 @Pick(i32 %a, i32 %b, i32 %c), local\n{\n0:\n\tret i32 %b\n}\n"
		"function i32 @Global(i32 %a, i32 %b)\n{\n0:\n\tret i32 %b\n}\n"
		"function i32 @Taken(i32 %a, i32 %b), local\n{\n0:\n\tret i32 %b\n}\n"
		"function i32 @Member(ptr %this, i32 %a), local, thiscall\n{\n0:\n\tret i32 0\n}\n"
		"function i32 @Address(i32 %a), local\n{\n0:\n\t%p = argaddr ptr 0\n\tret i32 0\n}\n"
		"function i32 @Caller(i32 %x)\n{\n0:\n"
		"\t%r = call i32 @Pick(i32 1, i32 %x, i32 %x)\n"
		"\t%s = call i32 @Global(i32 %x, i32 %r)\n"
		"\t%t = call i32 @Taken(i32 %x, i32 %s)\n"
		"\tstore ptr @Taken, @G\n"
		"\tret i32 %t\n}\n");
	AssertInt(5, (int)CIRInterprocedural::DeadParameters(cUnit.Find("Pick")));
	AssertInt(2, (int)CIRInterprocedural::DeadParameters(cUnit.Find("Member")));
	AssertInt(0, (int)CIRInterprocedural::DeadParameters(cUnit.Find("Address")));

	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.RemoveDeadArguments();
	AssertSize(1, cUnit.Find("Pick")->masParameters.NumElements());
	AssertSize(2, cUnit.Find("Global")->masParameters.NumElements());
	AssertSize(2, cUnit.Find("Taken")->masParameters.NumElements());
	AssertTrue(cPasses.IsChanged(cUnit.Index("Pick")));
	AssertTrue(cPasses.IsChanged(cUnit.Index("Caller")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Global")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Taken")));
	cUnit.Print("Caller", &sz);
	AssertTrue(sz.Contains("call i32 @Pick(i32 %x)"));
	AssertTrue(sz.Contains("call i32 @Global(i32 %x, i32 %r)"));
	sz.Kill();
	cPasses.Kill();
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A count with one value at every call: the parameter's uses are the constant,
// no copy is made, and the calls still pass it.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralOneValue(void)
{
	CTestIRUnit			cUnit;
	CIRInterprocedural	cPasses;
	CChars				sz;

	cUnit.Init(
		TEST_IR_SHIFT_BY
		"function i32 @Caller(i32 %x)\n{\n0:\n\t%r = call i32 @Shift(i32 %x, i32 3)\n\tret i32 %r\n}\n");
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	AssertSize(2, cUnit.NumUnitFunctions());
	cUnit.Print("Shift", &sz);
	AssertTrue(sz.Contains("shl i32 %x, 3"));
	sz.Kill();
	AssertTrue(cPasses.IsChanged(cUnit.Index("Shift")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Caller")));
	AssertTrue(cUnit.Calls("Caller", "Shift"));
	cPasses.Kill();
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A count with a few values: a copy for each (a negative one's name has an
// m), the calls go to them and the function goes, or stays when something else
// names it.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralCopies(void)
{
	static const char*	szText =
		"function i64 @Shift(i64 %x, i64 %n), local\n{\n0:\n\t%r = shl i64 %x, %n\n\tret i64 %r\n}\n"
		"function i64 @Caller(i64 %x)\n{\n0:\n"
		"\t%a = call i64 @Shift(i64 %x, i64 3)\n"
		"\t%b = call i64 @Shift(i64 %a, i64 -1)\n"
		"\t%c = call i64 @Shift(i64 %b, i64 3)\n"
		"\tret i64 %c\n}\n";
	CTestIRUnit			cUnit;
	CIRInterprocedural	cPasses;
	CChars				sz;

	cUnit.Init(szText);
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	AssertSize(4, cUnit.NumUnitFunctions());
	AssertSize(2, cUnit.mauiCopiedFrom.NumElements());
	AssertTrue(NULL == cUnit.Find("Shift"));
	AssertTrue(NULL == *cUnit.mapcFunctions.Get(0));
	AssertFalse(cPasses.IsChanged(0));
	AssertTrue(cPasses.IsChanged(cUnit.Index("Caller")));
	AssertTrue(cPasses.IsChanged(cUnit.Index("Shift__K1_3")));
	AssertTrue(cPasses.IsChanged(cUnit.Index("Shift__K1_m1")));
	AssertTrue(cUnit.mcModule.IsLocal(cUnit.Find("Shift__K1_3")->muiSymbol));
	AssertTrue(cUnit.Calls("Caller", "Shift__K1_3"));
	AssertTrue(cUnit.Calls("Caller", "Shift__K1_m1"));
	AssertFalse(cUnit.Calls("Caller", "Shift"));
	cUnit.Print("Shift__K1_3", &sz);
	AssertTrue(sz.Contains("shl i64 %x, 3"));
	sz.Kill();
	cPasses.Kill();
	cUnit.Kill();

	cUnit.Init(szText);
	cUnit.mbNamed = true;
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	AssertTrue(NULL != cUnit.Find("Shift"));
	AssertTrue(NULL != cUnit.Find("Shift__K1_3"));
	AssertFalse(cUnit.Calls("Caller", "Shift"));
	cPasses.Kill();
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A function that calls itself with constants: its copies call each other, so
// it goes and nothing calls a function that has gone.  One that passes itself
// a value that isn't a constant isn't specialised.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralRecursiveCopies(void)
{
	CTestIRUnit			cUnit;
	CIRInterprocedural	cPasses;
	CIRFunction*		pcFunction;
	CIRBlock*			pcBlock;
	SIRInstruction*		psInstruction;
	size				ui;
	size				uiBlock;
	size				uiInstruction;

	cUnit.Init(
		"function i32 @Fold(i32 %x, i32 %n), local\n{\n0:\n"
		"\t%c = eq i32 %x, 0\n"
		"\tbrif %c, 1, 2\n"
		"1:\n\t%r = shl i32 1, %n\n\tret i32 %r\n"
		"2:\n\t%y = sub i32 %x, 1\n"
		"\t%a = call i32 @Fold(i32 %y, i32 3)\n"
		"\t%b = call i32 @Fold(i32 %y, i32 5)\n"
		"\t%s = add i32 %a, %b\n"
		"\tret i32 %s\n}\n"
		"function i32 @Caller(i32 %x)\n{\n0:\n\t%r = call i32 @Fold(i32 %x, i32 3)\n\tret i32 %r\n}\n");
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	for (ui = 0; ui < cUnit.NumUnitFunctions(); ui++)
	{
		pcFunction = cUnit.GetUnitFunction(ui);
		for (uiBlock = 0; pcFunction && (uiBlock < pcFunction->NumBlocks()); uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				if (psInstruction->uiOpcode == IRO_Call)
				{
					AssertTrue(NULL != cUnit.mcModule.FindFunction(psInstruction->asArguments[0].Index()));
				}
			}
		}
	}
	AssertTrue(NULL == cUnit.Find("Fold"));
	AssertTrue(cUnit.Calls("Caller", "Fold__K1_3"));
	AssertTrue(cUnit.Calls("Fold__K1_3", "Fold__K1_5"));
	AssertFalse(cUnit.Calls("Fold__K1_3", "Fold"));
	AssertSize(4, cUnit.NumUnitFunctions());
	cPasses.Kill();
	cUnit.Kill();

	cUnit.Init(
		"function i32 @Fold(i32 %x, i32 %n), local\n{\n0:\n"
		"\t%c = eq i32 %x, 0\n"
		"\tbrif %c, 1, 2\n"
		"1:\n\t%r = shl i32 1, %n\n\tret i32 %r\n"
		"2:\n\t%y = sub i32 %x, 1\n"
		"\t%v = xor i32 %n, %y\n"
		"\t%a = call i32 @Fold(i32 %y, i32 %v)\n"
		"\tret i32 %a\n}\n"
		"function i32 @Caller(i32 %x)\n{\n0:\n\t%a = call i32 @Fold(i32 %x, i32 3)\n\t%b = call i32 @Fold(i32 %a, i32 5)\n\tret i32 %b\n}\n");
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	AssertTrue(NULL != cUnit.Find("Fold"));
	AssertSize(2, cUnit.NumUnitFunctions());
	cPasses.Kill();
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What isn't specialised: a parameter that doesn't count, one that isn't a
// constant at every call, one with too many values (IR_SPECIALISE_VALUES), a
// function another unit can call or whose address is taken.  A parameter
// that is passed on to one that counts is worth it.
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterproceduralNotSpecialised(void)
{
	CTestIRUnit			cUnit;
	CIRInterprocedural	cPasses;
	CChars				sz;

	cUnit.Init(
		"function i32 @Weak(i32 %x, i32 %n), local\n{\n0:\n\t%r = add i32 %x, %n\n\tret i32 %r\n}\n"
		"function i32 @Unknown(i32 %x, i32 %n), local\n{\n0:\n\t%r = shl i32 %x, %n\n\tret i32 %r\n}\n"
		"function i32 @Many(i32 %x, i32 %n), local\n{\n0:\n\t%r = shl i32 %x, %n\n\tret i32 %r\n}\n"
		"function i32 @Global(i32 %x, i32 %n)\n{\n0:\n\t%r = shl i32 %x, %n\n\tret i32 %r\n}\n"
		"function i32 @Taken(i32 %x, i32 %n), local\n{\n0:\n\t%r = shl i32 %x, %n\n\tret i32 %r\n}\n"
		"function i32 @Caller(i32 %x)\n{\n0:\n"
		"\t%a = call i32 @Weak(i32 %x, i32 3)\n"
		"\t%b = call i32 @Unknown(i32 %a, i32 %x)\n"
		"\t%c = call i32 @Many(i32 %b, i32 1)\n"
		"\t%d = call i32 @Many(i32 %c, i32 2)\n"
		"\t%e = call i32 @Many(i32 %d, i32 3)\n"
		"\t%f = call i32 @Many(i32 %e, i32 4)\n"
		"\t%g = call i32 @Global(i32 %f, i32 5)\n"
		"\t%h = call i32 @Taken(i32 %g, i32 6)\n"
		"\tstore ptr @Taken, @G\n"
		"\tret i32 %h\n}\n");
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	AssertSize(6, cUnit.NumUnitFunctions());
	AssertFalse(cPasses.IsChanged(cUnit.Index("Weak")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Unknown")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Many")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Global")));
	AssertFalse(cPasses.IsChanged(cUnit.Index("Taken")));
	cPasses.Kill();
	cUnit.Kill();

	cUnit.Init(
		"function i32 @Outer(i32 %x, i32 %n), local\n{\n0:\n\t%m = add i32 %n, 1\n\t%r = call i32 @Shift(i32 %x, i32 %m)\n\tret i32 %r\n}\n"
		TEST_IR_SHIFT_BY
		"function i32 @Caller(i32 %x)\n{\n0:\n\t%r = call i32 @Outer(i32 %x, i32 2)\n\tret i32 %r\n}\n");
	cPasses.Init(&cUnit.mcModule, &cUnit);
	cPasses.SpecialiseConstantArguments();
	cUnit.Print("Outer", &sz);
	AssertTrue(sz.Contains("i32 3)"));
	sz.Kill();
	cUnit.Print("Shift", &sz);
	AssertTrue(sz.Contains("shl i32 %x, 3"));
	sz.Kill();
	cPasses.Kill();
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRInterprocedural(void)
{
	BeginTests();

	TestIRInterproceduralRecursion();
	TestIRInterproceduralNoWrites();
	TestIRInterproceduralDeadArguments();
	TestIRInterproceduralOneValue();
	TestIRInterproceduralCopies();
	TestIRInterproceduralRecursiveCopies();
	TestIRInterproceduralNotSpecialised();

	TestStatistics();
}
