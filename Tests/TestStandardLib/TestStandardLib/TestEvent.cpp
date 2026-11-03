#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "StandardLib/Listener.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/ListenerCall.h"
#include "TestLib/Assert.h"


class CTestObjectIsListenerWithEvent;
struct STestEventContext1;
struct STestEventContext2;
class CTestListener : public CListener
{
CONSTRUCTABLE(CTestListener)
public:
	virtual void Style1(CUnknown* pcSource, void* pvContext) {};
	virtual void Style2(CUnknown* pcSource, void* pvContext) {};
};


class CTestAnotherListener : public CListener
{
CONSTRUCTABLE(CTestAnotherListener)
public:
	int	iThisIsNotTheRightWayToUseListeners;

	CTestAnotherListener()
	{
		iThisIsNotTheRightWayToUseListeners = 0;
	}

	virtual void Another(CUnknown* pcSource, void* pvContext)
	{ 
		iThisIsNotTheRightWayToUseListeners = 1;
	};
};


class CTestBadListener : public CListener
{
CONSTRUCTABLE(CTestBadListener)
public:
};


struct STestEventContext1
{
	int		iNumber;
	char	szFour[4];
};


struct STestEventContext2
{
	char	szSixteen[12];
};


enum EWhatHappen
{
	WH_SomeOneSetUpUsTheBomb,
	WH_WeGetSignal,
	WH_WhatYouSay,
	WH_MoveZigForGreatJustice,
};


class CTestObjectIsListenerWithEvent : public CUnknown, public CListenerCall, public CTestListener
{
CONSTRUCTABLE(CTestObjectIsListenerWithEvent)
public:
	EWhatHappen	meWhatHappen;
	int			miBored;

	void Init(void)
	{
		CListenerCall::Init();
		AddAllowedClass<CTestListener>();
		AddAllowedClass<CTestAnotherListener>();
		AddListener<CTestListener>(this);
		meWhatHappen = WH_SomeOneSetUpUsTheBomb;
		miBored = 0;
	}
	
	void Kill(void)
	{
		CListenerCall::Kill();
	}

	void MakeEventStyle1Happen(void)
	{
		STestEventContext1	sContext;
		
		sContext.iNumber = 3;
		memcpy(sContext.szFour, "abc", 4);
		CallListeners(&CTestListener::Style1, this, &sContext);
	}
	
	void MakeEventStyle2Happen(void)
	{
		STestEventContext2	sContext;

		memcpy(sContext.szSixteen, "def cat bad", 12);
		CallListeners(&CTestListener::Style2, this, &sContext);
	}

	void Style1(CUnknown* pcSource, void* pvContext) override;
	void Style2(CUnknown* pcSource, void* pvContext) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestObjectIsListenerWithEvent::Style1(CUnknown* pcSource, void* pvContext)
{
	CTestObjectIsListenerWithEvent*	pvthis;
	STestEventContext1*				psContext;

	pvthis = (CTestObjectIsListenerWithEvent*)this;
	psContext = (STestEventContext1*)pvContext;
	
	pvthis->miBored = 1;
	
	if (psContext->iNumber == 3)
	{
		if ((psContext->szFour[0] == 'a') && (psContext->szFour[1] == 'b') && (psContext->szFour[2] == 'c') && (psContext->szFour[3] == 0))
		{
			((CTestObjectIsListenerWithEvent*)pcSource)->meWhatHappen = WH_WeGetSignal;
			return;
		}
	}
	((CTestObjectIsListenerWithEvent*)pcSource)->meWhatHappen = WH_WhatYouSay;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestObjectIsListenerWithEvent::Style2(CUnknown* pcSource, void* pvContext)
{
	CTestObjectIsListenerWithEvent*	_this;
	STestEventContext2*				psContext;

	_this = (CTestObjectIsListenerWithEvent*)this;
	psContext = (STestEventContext2*)pvContext;

	_this->miBored = 2;
	if (memcmp(psContext->szSixteen, "def cat bad", 12) == 0)
	{
		((CTestObjectIsListenerWithEvent*)pcSource)->meWhatHappen = WH_MoveZigForGreatJustice;
	}
	else
	{
		((CTestObjectIsListenerWithEvent*)pcSource)->meWhatHappen = WH_WhatYouSay;
	}
}


class CTestObjectIsListener : public CUnknown, public CTestListener
{
CONSTRUCTABLE(CTestObjectIsListener)
public:
	CChars	mszAlsoBored;

	void Init(void)
	{
		mszAlsoBored.Init("Nothing to see here");
	}

	void Kill(void)
	{
		//An unknown's memory is freed without its destructor running so it unhooks its listener here.
		RemoveFromListenerCalls();
		mszAlsoBored.Kill();
	}

	void Style1(CUnknown* pcSource, void* pvContext) override;
	void Style2(CUnknown* pcSource, void* pvContext) override;
};



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestObjectIsListener::Style1(CUnknown* pcSource, void* pvContext)
{
	mszAlsoBored.Set("Sup my homies");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestObjectIsListener::Style2(CUnknown* pcSource, void* pvContext)
{
	mszAlsoBored.Set("Wikky wikky free styling");
}


void TestVirtualCall(void)
{
	MemoryInit();
	UnknownsInit();

	CTestObjectIsListenerWithEvent*		pcTest;
	CTestObjectIsListener*				pcListener;
	CTestAnotherListener				cAnother1;
	CTestAnotherListener				cAnother2;
	CTestAnotherListener				cAnother3;
	bool								bResult;

	pcTest = UMalloc(CTestObjectIsListenerWithEvent);
	pcTest->Init();

	pcListener = UMalloc(CTestObjectIsListener);
	pcListener->Init();

	AssertInt(WH_SomeOneSetUpUsTheBomb, pcTest->meWhatHappen);
	AssertString("Nothing to see here", pcListener->mszAlsoBored.Text());
	AssertInt(0, pcTest->miBored);

	bResult = pcTest->AddListener<CTestListener>(pcListener);
	AssertTrue(bResult);
	bResult = pcTest->AddListener(&cAnother1);
	AssertTrue(bResult);
	bResult = pcTest->AddListener(&cAnother2);
	AssertTrue(bResult);

	pcTest->MakeEventStyle1Happen();
	AssertInt(WH_WeGetSignal, pcTest->meWhatHappen);
	AssertString("Sup my homies", pcListener->mszAlsoBored.Text());
	AssertInt(1, pcTest->miBored);

	pcTest->MakeEventStyle2Happen();
	AssertInt(WH_MoveZigForGreatJustice, pcTest->meWhatHappen);
	AssertString("Wikky wikky free styling", pcListener->mszAlsoBored.Text());
	AssertInt(2, pcTest->miBored);

	pcTest->CallListeners(&CTestAnotherListener::Another, pcTest, NULL);
	AssertInt(1, cAnother1.iThisIsNotTheRightWayToUseListeners);
	AssertInt(1, cAnother2.iThisIsNotTheRightWayToUseListeners);
	AssertInt(0, cAnother3.iThisIsNotTheRightWayToUseListeners);

	bResult = pcTest->AddListener<CTestBadListener>(NULL);
	AssertFalse(bResult);

	pcListener->Kill();
	pcTest->Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CTestCountingListener : public CListener
{
CONSTRUCTABLE(CTestCountingListener)
public:
	int						miCalls;
	CListenerCall*			mpcCall;
	CTestCountingListener*	mpcRemove;
	CTestCountingListener*	mpcAdd;

	CTestCountingListener()
	{
		miCalls = 0;
		mpcCall = NULL;
		mpcRemove = NULL;
		mpcAdd = NULL;
	}

	virtual void Counted(CUnknown* pcSource, void* pvContext)
	{
		miCalls++;
		if (mpcRemove)
		{
			mpcCall->RemoveListener(mpcRemove);
		}
		if (mpcAdd)
		{
			mpcCall->AddListener(mpcAdd);
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListenerDestroyedBeforeCall(void)
{
	MemoryInit();

	CListenerCall			cCall;
	CTestCountingListener	cStays;

	cCall.Init();
	cCall.AddAllowedClass<CTestCountingListener>();
	AssertTrue(cCall.AddListener(&cStays));

	{
		CTestCountingListener	cGoes;

		AssertTrue(cCall.AddListener(&cGoes));
		AssertSize(2, cCall.NumListeners());
		AssertSize(1, cGoes.NumListenerCalls());
	}

	//The listener on the stack removed itself from the call when it went out of scope.
	AssertSize(1, cCall.NumListeners());
	cCall.CallListeners(&CTestCountingListener::Counted, NULL, NULL);
	AssertInt(1, cStays.miCalls);

	cCall.Kill();
	AssertSize(0, cStays.NumListenerCalls());

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListenerCallKilledBeforeListener(void)
{
	MemoryInit();

	CListenerCall*			pcCall;
	CTestCountingListener	cListener;

	pcCall = NewMalloc<CListenerCall>();
	pcCall->Init();
	pcCall->AddAllowedClass<CTestCountingListener>();
	AssertTrue(pcCall->AddListener(&cListener));
	AssertSize(1, cListener.NumListenerCalls());

	//The call removes itself from its listeners so the listener's destructor doesn't touch the freed call.
	SafeKill(pcCall);
	AssertSize(0, cListener.NumListenerCalls());

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListenerChangedDuringCall(void)
{
	MemoryInit();

	CListenerCall			cCall;
	CTestCountingListener	cRemover;
	CTestCountingListener	cRemoved;
	CTestCountingListener	cAfter;
	CTestCountingListener	cAdded;
	CTestCountingListener	cSelfRemover;

	cCall.Init();
	cCall.AddAllowedClass<CTestCountingListener>();

	//The first removes the second before its turn and adds another; the last removes itself.
	cRemover.mpcCall = &cCall;
	cRemover.mpcRemove = &cRemoved;
	cRemover.mpcAdd = &cAdded;
	cSelfRemover.mpcCall = &cCall;
	cSelfRemover.mpcRemove = &cSelfRemover;
	AssertTrue(cCall.AddListener(&cRemover));
	AssertTrue(cCall.AddListener(&cRemoved));
	AssertTrue(cCall.AddListener(&cSelfRemover));
	AssertTrue(cCall.AddListener(&cAfter));

	cCall.CallListeners(&CTestCountingListener::Counted, NULL, NULL);
	AssertInt(1, cRemover.miCalls);
	AssertInt(0, cRemoved.miCalls);
	AssertInt(1, cSelfRemover.miCalls);
	AssertInt(1, cAfter.miCalls);
	AssertInt(0, cAdded.miCalls);  //Added during the call so it's called from the next one.
	AssertSize(3, cCall.NumListeners());
	AssertSize(0, cRemoved.NumListenerCalls());
	AssertSize(0, cSelfRemover.NumListenerCalls());
	AssertSize(1, cAdded.NumListenerCalls());

	cRemover.mpcAdd = NULL;
	cCall.CallListeners(&CTestCountingListener::Counted, NULL, NULL);
	AssertInt(2, cRemover.miCalls);
	AssertInt(2, cAfter.miCalls);
	AssertInt(1, cAdded.miCalls);
	AssertInt(1, cSelfRemover.miCalls);

	cCall.Kill();
	AssertSize(0, cRemover.NumListenerCalls());
	AssertSize(0, cAfter.NumListenerCalls());
	AssertSize(0, cAdded.NumListenerCalls());

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListenerAddedTwice(void)
{
	MemoryInit();

	CListenerCall			cCall;
	CTestCountingListener	cListener;

	cCall.Init();
	cCall.AddAllowedClass<CTestCountingListener>();

	AssertTrue(cCall.AddListener(&cListener));
	AssertFalse(cCall.AddListener(&cListener));
	AssertSize(1, cCall.NumListeners());
	AssertSize(1, cListener.NumListenerCalls());

	cCall.CallListeners(&CTestCountingListener::Counted, NULL, NULL);
	AssertInt(1, cListener.miCalls);

	AssertTrue(cCall.RemoveListener(&cListener));
	AssertFalse(cCall.RemoveListener(&cListener));
	AssertSize(0, cCall.NumListeners());
	AssertSize(0, cListener.NumListenerCalls());

	cCall.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListenerInManyCalls(void)
{
	MemoryInit();

	CListenerCall			cCall1;
	CListenerCall			cCall2;
	CListenerCall			cCall3;

	cCall1.Init();
	cCall1.AddAllowedClass<CTestCountingListener>();
	cCall2.Init();
	cCall2.AddAllowedClass<CTestCountingListener>();
	cCall3.Init();
	cCall3.AddAllowedClass<CTestCountingListener>();

	{
		CTestCountingListener	cListener;

		//More calls than the listener's array embeds.
		AssertTrue(cCall1.AddListener(&cListener));
		AssertTrue(cCall2.AddListener(&cListener));
		AssertTrue(cCall3.AddListener(&cListener));
		AssertSize(3, cListener.NumListenerCalls());

		AssertTrue(cCall2.RemoveListener(&cListener));
		AssertSize(2, cListener.NumListenerCalls());
		AssertTrue(cCall2.AddListener(&cListener));
		AssertSize(3, cListener.NumListenerCalls());
	}

	AssertSize(0, cCall1.NumListeners());
	AssertSize(0, cCall2.NumListeners());
	AssertSize(0, cCall3.NumListeners());

	cCall3.Kill();
	cCall2.Kill();
	cCall1.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEvent(void)
{
	BeginTests();

	TestVirtualCall();
	TestListenerDestroyedBeforeCall();
	TestListenerCallKilledBeforeListener();
	TestListenerChangedDuringCall();
	TestListenerAddedTwice();
	TestListenerInManyCalls();

	TestStatistics();
}

