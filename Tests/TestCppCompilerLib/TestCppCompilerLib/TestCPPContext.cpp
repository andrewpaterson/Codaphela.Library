#include "CppCompilerLib/CPP65Type.h"
#include "CppCompilerLib/CPPClass.h"
#include "CppCompilerLib/CPPContext.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A function is entered and left: the function it was entered from comes
// back, and whether that one had a this, however the one between changed
// it.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPContextFunction(void)
{
	CCPP65Types		cTypes;
	CCPPContext		cContext;
	CCPPClass		cClass;
	CCPPFunction	cFree;
	CCPPFunction	cMember;
	CCPPFunction	cStatic;

	cTypes.Init(2, 4);
	cClass.Init(NULL, -1);
	cFree.Init(NULL, 1, CPPFK_Function, &cTypes.msVoid);
	cMember.Init(&cClass, 2, CPPFK_Function, &cTypes.msVoid);
	cStatic.Init(&cClass, 3, CPPFK_Function, &cTypes.msVoid);
	cStatic.mbStatic = true;
	cContext.Init();

	AssertNull(cContext.GetFunction());
	AssertFalse(cContext.HasThis());

	cContext.EnterFunction(&cMember);
	AssertPointer(&cMember, cContext.GetFunction());
	AssertTrue(cContext.HasThis());

	//Another function inside it: a free one and a static member have no this.
	cContext.EnterFunction(&cFree);
	AssertPointer(&cFree, cContext.GetFunction());
	AssertFalse(cContext.HasThis());
	cContext.EnterFunction(&cStatic);
	AssertPointer(&cStatic, cContext.GetFunction());
	AssertFalse(cContext.HasThis());
	cContext.LeaveFunction();
	AssertPointer(&cFree, cContext.GetFunction());
	cContext.LeaveFunction();
	AssertPointer(&cMember, cContext.GetFunction());
	AssertTrue(cContext.HasThis());

	//What is written with no this, in the member function: this comes back when it is left, and a function entered
	//and left inside it does not bring it back early.
	cContext.EnterNoThis();
	AssertFalse(cContext.HasThis());
	AssertPointer(&cMember, cContext.GetFunction());
	cContext.EnterFunction(&cFree);
	cContext.LeaveFunction();
	AssertFalse(cContext.HasThis());
	cContext.LeaveNoThis();
	AssertTrue(cContext.HasThis());

	cContext.LeaveFunction();
	AssertNull(cContext.GetFunction());
	AssertFalse(cContext.HasThis());
	AssertFalse(cContext.IsBroken());

	cContext.Kill();
	cStatic.Kill();
	cMember.Kill();
	cFree.Kill();
	cClass.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What is being worked out is known while it is, by what it is of and the
// template arguments there are; a warning's check is one at a time whatever
// they are.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPContextWorking(void)
{
	CCPPContext				cContext;
	int						iFirst;
	int						iSecond;
	CCPPTemplateArguments*	pcBindings;

	cContext.Init();
	pcBindings = (CCPPTemplateArguments*)&iSecond;	//Only its address is asked of.

	AssertFalse(cContext.IsWorking(CPPWK_Initialiser, &iFirst));
	AssertTrue(cContext.EnterWorking(CPPWK_Initialiser, &iFirst));
	AssertTrue(cContext.IsWorking(CPPWK_Initialiser, &iFirst));

	//The same again is refused, and nothing is entered for it.
	AssertFalse(cContext.EnterWorking(CPPWK_Initialiser, &iFirst));

	//Another thing, and another kind of working on the same thing, are not it.
	AssertFalse(cContext.IsWorking(CPPWK_Initialiser, &iSecond));
	AssertFalse(cContext.IsWorking(CPPWK_ConstantType, &iFirst));
	AssertTrue(cContext.EnterWorking(CPPWK_ConstantType, &iFirst));

	//With other template arguments it is another thing; with the first again, the same.
	cContext.EnterBindings(pcBindings);
	AssertFalse(cContext.IsWorking(CPPWK_Initialiser, &iFirst));
	AssertTrue(cContext.EnterWorking(CPPWK_Initialiser, &iFirst));
	AssertFalse(cContext.EnterWorking(CPPWK_Initialiser, &iFirst));
	cContext.LeaveWorking();
	cContext.LeaveBindings();
	AssertTrue(cContext.IsWorking(CPPWK_Initialiser, &iFirst));

	//A warning's check is of nothing, and is the one check whatever the template arguments.
	AssertTrue(cContext.EnterWorking(CPPWK_Warning, NULL));
	cContext.EnterBindings(pcBindings);
	AssertTrue(cContext.IsWorking(CPPWK_Warning, NULL));
	AssertFalse(cContext.EnterWorking(CPPWK_Warning, NULL));
	cContext.LeaveBindings();
	cContext.LeaveWorking();
	AssertFalse(cContext.IsWorking(CPPWK_Warning, NULL));

	cContext.LeaveWorking();
	cContext.LeaveWorking();
	AssertFalse(cContext.IsWorking(CPPWK_Initialiser, &iFirst));
	AssertFalse(cContext.IsBroken());
	cContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An operand that isn't evaluated, and quiet code, are entered and left:
// one inside another is the same, and one left that was not the last thing
// entered breaks the context, which the unit reports (CheckContext).
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPContextUnevaluatedAndQuiet(void)
{
	CCPPContext	cContext;

	cContext.Init();
	AssertFalse(cContext.IsUnevaluated());
	cContext.EnterUnevaluated();
	cContext.EnterUnevaluated();
	AssertTrue(cContext.IsUnevaluated());
	cContext.LeaveUnevaluated();
	AssertTrue(cContext.IsUnevaluated());
	cContext.LeaveUnevaluated();
	AssertFalse(cContext.IsUnevaluated());

	//Quiet code fails when something in it does, and says so when it ends; the next begins unfailed.
	AssertFalse(cContext.IsQuiet());
	AssertTrue(cContext.BeginQuiet());
	AssertFalse(cContext.BeginQuiet());
	AssertTrue(cContext.IsQuiet());
	cContext.SetFailed();
	AssertFalse(cContext.EndQuiet());
	AssertTrue(cContext.IsQuiet());
	AssertFalse(cContext.EndQuiet());
	AssertFalse(cContext.IsQuiet());
	AssertTrue(cContext.BeginQuiet());
	AssertTrue(cContext.EndQuiet());

	//A trial's failing is its own: the quiet code around it has not failed.
	AssertTrue(cContext.BeginQuiet());
	AssertFalse(cContext.BeginTrial());
	cContext.SetFailed();
	AssertFalse(cContext.EndTrial());
	AssertTrue(cContext.EndQuiet());
	AssertFalse(cContext.IsBroken());

	//Quiet code ended inside an operand that was entered after it began: the operand was the last thing entered.
	cContext.BeginQuiet();
	cContext.EnterUnevaluated();
	cContext.EndQuiet();
	AssertTrue(cContext.IsBroken());
	AssertTrue(cContext.IsQuiet());
	AssertTrue(cContext.IsUnevaluated());
	cContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CCPPContext, where the compiler is in a unit: each part of it is entered
// and left (SemanticLayerPlan.txt stage 3).
//
//////////////////////////////////////////////////////////////////////////
void TestCPPContext(void)
{
	BeginTests();

	TestCPPContextFunction();
	TestCPPContextWorking();
	TestCPPContextUnevaluatedAndQuiet();

	TestStatistics();
}
