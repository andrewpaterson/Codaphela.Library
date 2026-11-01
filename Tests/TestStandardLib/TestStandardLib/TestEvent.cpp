#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Chars.h"
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
void TestEvent(void)
{
	BeginTests();

	TestVirtualCall();
	
	TestStatistics();
}

