#include "BaseLib/Chars.h"
#include "StandardLib/MemberCalls.h"
#include "TestLib/Assert.h"


class CTestMemberCallsTarget
{
public:
	CChars			mszCalled;
	CMemberCalls*	mpcCalls;
	bool			mbAnswer;
	int				miValue;

	void Init(CMemberCalls* pcCalls)
	{
		mszCalled.Init();
		mpcCalls = pcCalls;
		mbAnswer = false;
		miValue = 0;
	}

	void Kill(void)
	{
		mszCalled.Kill();
	}

	void Plain(void)
	{
		mszCalled.Append("Plain ");
	}

	void With(CChars* psz)
	{
		mszCalled.Append("With ");
		mszCalled.Append(psz->Text());
		mszCalled.Append(" ");
	}

	void WithValue(CChars* psz, int iValue)
	{
		mszCalled.Append("WithValue ");
		mszCalled.Append(psz->Text());
		mszCalled.Append(" ");
		miValue = iValue;
	}

	void RemovesItself(void)
	{
		mszCalled.Append("RemovesItself ");
		mpcCalls->Remove(this);
	}

	bool Answer(void)
	{
		return mbAnswer;
	}
};


//Puts the target's base second, so a pointer to it isn't the same as a pointer to the object.
class CTestMemberCallsOther
{
public:
	int		miOther;
};


class CTestMemberCallsDerived : public CTestMemberCallsOther, public CTestMemberCallsTarget
{
public:
	void Derived(void)
	{
		mszCalled.Append("Derived ");
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemberCallShapes(void)
{
	CMemberCalls			cCalls;
	CTestMemberCallsTarget	cTarget;
	CChars					sz;
	int						iValue;

	cCalls.Init();
	cTarget.Init(&cCalls);
	sz.Init("Bob");
	iValue = 7;

	cCalls.Add()->Init(&cTarget, &CTestMemberCallsTarget::Plain);
	cCalls.Add()->Init(&cTarget, &CTestMemberCallsTarget::With);
	cCalls.Add()->Init(&cTarget, &CTestMemberCallsTarget::WithValue);
	AssertSize(3, cCalls.NumCalls());

	cCalls.Call(&sz, &iValue);
	AssertString("Plain With Bob WithValue Bob ", cTarget.mszCalled.Text());
	AssertInt(7, cTarget.miValue);

	sz.Kill();
	cTarget.Kill();
	cCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemberCallAsk(void)
{
	SMemberCall				sCall;
	CTestMemberCallsTarget	cTarget;

	cTarget.Init(NULL);
	sCall.Clear();
	AssertFalse(sCall.IsSet());

	sCall.Init(&cTarget, &CTestMemberCallsTarget::Answer);
	AssertTrue(sCall.IsSet());
	AssertFalse(sCall.Ask());
	cTarget.mbAnswer = true;
	AssertTrue(sCall.Ask());

	cTarget.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemberCallsRemove(void)
{
	CMemberCalls			cCalls;
	CTestMemberCallsTarget	cFirst;
	CTestMemberCallsTarget	cSecond;

	cCalls.Init();
	cFirst.Init(&cCalls);
	cSecond.Init(&cCalls);

	cCalls.Add()->Init(&cFirst, &CTestMemberCallsTarget::Plain);
	cCalls.Add()->Init(&cSecond, &CTestMemberCallsTarget::Plain);
	cCalls.Add()->Init(&cFirst, &CTestMemberCallsTarget::Plain);

	AssertTrue(cCalls.Remove(&cFirst));
	AssertSize(1, cCalls.NumCalls());
	AssertFalse(cCalls.Remove(&cFirst));

	cCalls.Call(NULL, NULL);
	AssertString("", cFirst.mszCalled.Text());
	AssertString("Plain ", cSecond.mszCalled.Text());

	cSecond.Kill();
	cFirst.Kill();
	cCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A call removed by an earlier one isn't called.  One added while they're
// called isn't called until the next time.
//
//////////////////////////////////////////////////////////////////////////
void TestMemberCallsChangedWhileCalled(void)
{
	CMemberCalls			cCalls;
	CTestMemberCallsTarget	cRemover;
	CTestMemberCallsTarget	cAfter;

	cCalls.Init();
	cRemover.Init(&cCalls);
	cAfter.Init(&cCalls);

	cCalls.Add()->Init(&cRemover, &CTestMemberCallsTarget::RemovesItself);
	cCalls.Add()->Init(&cRemover, &CTestMemberCallsTarget::Plain);
	cCalls.Add()->Init(&cAfter, &CTestMemberCallsTarget::Plain);

	cCalls.Call(NULL, NULL);
	AssertString("RemovesItself ", cRemover.mszCalled.Text());
	AssertString("Plain ", cAfter.mszCalled.Text());
	AssertSize(1, cCalls.NumCalls());

	cAfter.Kill();
	cRemover.Kill();
	cCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function is called on the right part of an object whose class
// derives from the function's after another base.
//
//////////////////////////////////////////////////////////////////////////
void TestMemberCallsDerived(void)
{
	CMemberCalls			cCalls;
	CTestMemberCallsDerived	cDerived;

	cCalls.Init();
	cDerived.Init(&cCalls);

	cCalls.Add()->Init(&cDerived, &CTestMemberCallsTarget::Plain);
	cCalls.Add()->Init(&cDerived, &CTestMemberCallsDerived::Derived);
	cCalls.Call(NULL, NULL);
	AssertString("Plain Derived ", cDerived.mszCalled.Text());

	cDerived.Kill();
	cCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemberCalls(void)
{
	BeginTests();

	TestMemberCallShapes();
	TestMemberCallAsk();
	TestMemberCallsRemove();
	TestMemberCallsChangedWhileCalled();
	TestMemberCallsDerived();

	TestStatistics();
}

