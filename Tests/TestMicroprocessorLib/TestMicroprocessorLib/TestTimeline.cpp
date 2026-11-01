#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "MicroprocessorLib/LongTime.h"
#include "MicroprocessorLib/Timeline.h"
#include "TestLib/Assert.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "name@time;" when it executes, "removed:name;" when it is removed
// from its owner and "freed:name;" when it is killed.  It can add or
// remove another event when it executes.  Its owner is a pointer it clears
// when it is removed from its owner.
//
//////////////////////////////////////////////////////////////////////////
class CTestLogEvent : public CEvent
{
CONSTRUCTABLE(CTestLogEvent);
public:
	const char*		mszName;
	CChars*			mpszLog;
	CTestLogEvent**	mppcOwner;
	CEvent*			mpcAddOnExecute;
	CEvent*			mpcRemoveOnExecute;

	void Init(int64 iTime, const char* szName, CChars* pszLog)
	{
		CEvent::Init(iTime);
		mszName = szName;
		mpszLog = pszLog;
		mppcOwner = NULL;
		mpcAddOnExecute = NULL;
		mpcRemoveOnExecute = NULL;
	}

	void Kill(void) override
	{
		mpszLog->Append("freed:");
		mpszLog->Append(mszName);
		mpszLog->Append(";");
	}

	void Execute(CTimeline* pcTimeline) override
	{
		mpszLog->Append(mszName);
		mpszLog->Append("@");
		mpszLog->Append(miTime);
		mpszLog->Append(";");
		if (mpcAddOnExecute)
		{
			if (!pcTimeline->AddFutureEvent(mpcAddOnExecute))
			{
				SafeKill(mpcAddOnExecute);
			}
		}
		if (mpcRemoveOnExecute)
		{
			pcTimeline->Remove(mpcRemoveOnExecute);
		}
		CEvent::Execute(pcTimeline);
	}

	void RemoveFromOwner(void) override
	{
		if (mppcOwner)
		{
			*mppcOwner = NULL;
			mppcOwner = NULL;
		}
		mpszLog->Append("removed:");
		mpszLog->Append(mszName);
		mpszLog->Append(";");
		CEvent::RemoveFromOwner();
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTestLogEvent* NewTestLogEvent(int64 iTime, const char* szName, CChars* pszLog)
{
	CTestLogEvent*	pcEvent;

	pcEvent = NewMalloc<CTestLogEvent>();
	pcEvent->Init(iTime, szName, pszLog);
	return pcEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineLongTime(void)
{
	CChars	sz;

	AssertLong(1024, NanosecondsToTime((int64)1));
	AssertLong(1536, NanosecondsToTime(1.5));
	AssertDouble(1.5, TimeToNanoseconds(1536), 6);
	AssertLong(1024000, SecondsToTime(0.000001));
	AssertDouble(0.000001, TimeToSeconds(1024000), 9);
	AssertLong(1024000, FrequencyToTime(1000000.0));

	sz.Init();
	AppendNanoseconds(&sz, 1536);
	AssertString("1.5ns", sz.Text());
	sz.Clear();
	AppendNanoseconds(&sz, 0);
	AssertString("0.0ns", sz.Text());
	sz.Clear();
	AppendNanoseconds(&sz, -1536);
	AssertString("-1.5ns", sz.Text());
	sz.Clear();
	AppendNanoseconds(&sz, NanosecondsToTime((int64)1000));
	AssertString("1000.0ns", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineOrder(void)
{
	CTimeline		cTimeline;
	CChars			szLog;
	CTestLogEvent*			pcPast;
	STimelineData			sData;
	CSimultaneousEvents*	pcEvents;
	SLogConfig				sLogConfig;

	szLog.Init();
	cTimeline.Init();

	//Events run in time order, simultaneous ones in the order they were added.
	AssertTrue(cTimeline.AddFutureEvent(NewTestLogEvent(30, "D", &szLog)));
	AssertTrue(cTimeline.AddFutureEvent(NewTestLogEvent(10, "A", &szLog)));
	AssertTrue(cTimeline.AddFutureEvent(NewTestLogEvent(20, "C", &szLog)));
	AssertTrue(cTimeline.AddFutureEvent(NewTestLogEvent(10, "B", &szLog)));
	AssertSize(3, cTimeline.NumSimultaneousEvents());
	pcEvents = cTimeline.GetFirstSimultaneousEvents();
	AssertLong(10, pcEvents->GetTime());
	AssertSize(2, pcEvents->NumEvents());
	pcEvents = cTimeline.GetNextSimultaneousEvents(pcEvents);
	AssertLong(20, pcEvents->GetTime());
	pcEvents = cTimeline.GetNextSimultaneousEvents(pcEvents);
	AssertLong(30, pcEvents->GetTime());
	AssertNull(cTimeline.GetNextSimultaneousEvents(pcEvents));
	AssertTrue(cTimeline.IsValid());

	//An executed event is removed from its owner then freed with the others at its time.
	AssertTrue(cTimeline.RunSimultaneous());
	AssertLog("A@10;removed:A;B@10;removed:B;freed:A;freed:B;");
	AssertLong(10, cTimeline.GetTime());
	AssertLong(10, cTimeline.GetEventTime());
	AssertLong(0, cTimeline.GetPreviousEventTime());

	cTimeline.Run();
	AssertLog("C@20;removed:C;freed:C;D@30;removed:D;freed:D;");
	AssertLong(30, cTimeline.GetTime());
	AssertLong(30, cTimeline.GetEventTime());
	AssertLong(20, cTimeline.GetPreviousEventTime());
	AssertSize(0, cTimeline.NumSimultaneousEvents());
	AssertFalse(cTimeline.RunSimultaneous());

	//An event at or before the current time isn't added and stays the caller's.
	pcPast = NewTestLogEvent(30, "Past", &szLog);
	AssertFalse(cTimeline.AddEvent(pcPast));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cTimeline.AddFutureEvent(pcPast));
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(0, cTimeline.NumSimultaneousEvents());
	SafeKill(pcPast);
	AssertLog("freed:Past;");

	//An event can't be added twice.
	pcPast = NewTestLogEvent(40, "Twice", &szLog);
	AssertTrue(cTimeline.AddFutureEvent(pcPast));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cTimeline.AddFutureEvent(pcPast));
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(1, cTimeline.GetFirstSimultaneousEvents()->NumEvents());

	cTimeline.Save(&sData);
	AssertLong(30, sData.iTime);
	AssertLong(20, sData.iPreviousEventTime);
	AssertLong(30, sData.iEventTime);

	//Killing the timeline frees the events still waiting, without removing them from their owners.
	cTimeline.Kill();
	AssertLog("freed:Twice;");

	cTimeline.Init();
	cTimeline.Load(&sData);
	AssertLong(30, cTimeline.GetTime());
	AssertLong(20, cTimeline.GetPreviousEventTime());
	cTimeline.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineRunToTime(void)
{
	CTimeline	cTimeline;
	CChars		szLog;

	szLog.Init();
	cTimeline.Init();
	cTimeline.AddFutureEvent(NewTestLogEvent(10, "A", &szLog));
	cTimeline.AddFutureEvent(NewTestLogEvent(20, "B", &szLog));
	cTimeline.AddFutureEvent(NewTestLogEvent(50, "C", &szLog));

	//Events up to and including the target time run, then the time is the target.
	AssertTrue(cTimeline.RunToTime(20));
	AssertLog("A@10;removed:A;freed:A;B@20;removed:B;freed:B;");
	AssertLong(20, cTimeline.GetTime());

	AssertFalse(cTimeline.RunToTime(15));
	AssertLog("");
	AssertLong(35, cTimeline.GetTime());

	AssertTrue(cTimeline.RunToTime(100));
	AssertLog("C@50;removed:C;freed:C;");
	AssertLong(135, cTimeline.GetTime());

	//With nothing to run the time still moves on.
	AssertFalse(cTimeline.RunToTime(5));
	AssertLong(140, cTimeline.GetTime());

	cTimeline.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineRemove(void)
{
	CTimeline		cTimeline;
	CChars			szLog;
	CTestLogEvent*	pcA;
	CTestLogEvent*	pcB;
	CTestLogEvent*	pcC;
	CTestLogEvent*	pcOwned;
	CArrayEventPtr	apcEvents;

	szLog.Init();
	cTimeline.Init();

	//Removing an event takes it from its owner and frees it.  The last event at a time takes the time with it.
	pcA = NewTestLogEvent(10, "A", &szLog);
	pcB = NewTestLogEvent(20, "B", &szLog);
	pcOwned = pcB;
	pcB->mppcOwner = &pcOwned;
	cTimeline.AddFutureEvent(pcA);
	cTimeline.AddFutureEvent(pcB);
	AssertTrue(cTimeline.Remove(pcB));
	AssertLog("removed:B;freed:B;");
	AssertNull(pcOwned);
	AssertSize(1, cTimeline.NumSimultaneousEvents());
	AssertNull(cTimeline.FindSimultaneousEvents(20));

	//An event that isn't on the timeline isn't touched.
	pcC = NewTestLogEvent(30, "C", &szLog);
	AssertFalse(cTimeline.Remove(pcC));
	AssertLog("");
	SafeKill(pcC);
	AssertLog("freed:C;");

	//RemoveAll.
	pcB = NewTestLogEvent(10, "B", &szLog);
	cTimeline.AddFutureEvent(pcB);
	apcEvents.Init();
	apcEvents.Add(pcA);
	apcEvents.Add(pcB);
	AssertTrue(cTimeline.RemoveAll(&apcEvents));
	apcEvents.Kill();
	AssertLog("removed:A;freed:A;removed:B;freed:B;");
	AssertSize(0, cTimeline.NumSimultaneousEvents());

	cTimeline.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineChangesWhileRunning(void)
{
	CTimeline		cTimeline;
	CChars			szLog;
	CTestLogEvent*	pcA;
	CTestLogEvent*	pcB;
	CTestLogEvent*	pcC;
	CTestLogEvent*	pcOwned;
	SLogConfig		sLogConfig;

	szLog.Init();
	cTimeline.Init();

	//An event at the same time removed by one that executes first doesn't execute.  It is freed once the others have run.
	pcA = NewTestLogEvent(10, "A", &szLog);
	pcB = NewTestLogEvent(10, "B", &szLog);
	pcC = NewTestLogEvent(20, "C", &szLog);
	pcOwned = pcB;
	pcB->mppcOwner = &pcOwned;
	pcA->mpcRemoveOnExecute = pcB;
	cTimeline.AddFutureEvent(pcA);
	cTimeline.AddFutureEvent(pcB);
	cTimeline.AddFutureEvent(pcC);
	AssertTrue(cTimeline.RunSimultaneous());
	AssertLog("A@10;removed:B;removed:A;freed:A;freed:B;");
	AssertNull(pcOwned);

	//An event removed at a later time is freed straight away.  An event can add later events.
	pcA = NewTestLogEvent(15, "A", &szLog);
	pcB = NewTestLogEvent(17, "B", &szLog);
	pcA->mpcRemoveOnExecute = pcC;
	pcA->mpcAddOnExecute = pcB;
	cTimeline.AddFutureEvent(pcA);
	cTimeline.Run();
	AssertLog("A@15;removed:C;freed:C;removed:A;freed:A;B@17;removed:B;freed:B;");
	AssertLong(17, cTimeline.GetTime());

	//An event can't add one at its own time, it would be in the past.
	pcA = NewTestLogEvent(20, "A", &szLog);
	pcA->mpcAddOnExecute = NewTestLogEvent(20, "Now", &szLog);
	cTimeline.AddFutureEvent(pcA);
	sLogConfig = gcLogger.SetSilent();
	cTimeline.Run();
	gcLogger.SetConfig(&sLogConfig);
	AssertLog("A@20;freed:Now;removed:A;freed:A;");
	AssertSize(0, cTimeline.NumSimultaneousEvents());

	cTimeline.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Many times added in a scrambled order, some removed, then run.  The
// red-black tree stays valid and the events run in time order.
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineManyTimes(void)
{
	CTimeline		cTimeline;
	CChars			szLog;
	CTestLogEvent*	apcEvents[1000];
	int64			iTime;
	int64			iPrevious;
	size			i;
	size			uiExecuted;
	CSimultaneousEvents*	pcEvents;

	szLog.Init();
	cTimeline.Init();

	//Times 1 to 500, twice each, in a scrambled order: 7 is coprime with 500.
	for (i = 0; i < 1000; i++)
	{
		iTime = (int64)(((i * 7) % 500) + 1);
		apcEvents[i] = NewTestLogEvent(iTime, "E", &szLog);
		AssertTrue(cTimeline.AddFutureEvent(apcEvents[i]));
	}
	AssertSize(500, cTimeline.NumSimultaneousEvents());
	AssertTrue(cTimeline.IsValid());

	//Remove both events at every third time, which takes those times out of the tree, and one event at every other fifth.
	for (i = 0; i < 1000; i++)
	{
		iTime = apcEvents[i]->GetTime();
		if ((iTime % 3) == 0)
		{
			AssertTrue(cTimeline.Remove(apcEvents[i]));
			apcEvents[i] = NULL;
		}
		else if (((iTime % 5) == 0) && (i < 500))
		{
			AssertTrue(cTimeline.Remove(apcEvents[i]));
			apcEvents[i] = NULL;
		}
	}
	AssertTrue(cTimeline.IsValid());
	AssertSize(500 - 166, cTimeline.NumSimultaneousEvents());

	//The times are in order and none of them is a multiple of three.
	iPrevious = 0;
	pcEvents = cTimeline.GetFirstSimultaneousEvents();
	while (pcEvents)
	{
		AssertTrue(pcEvents->GetTime() > iPrevious);
		AssertTrue((pcEvents->GetTime() % 3) != 0);
		iPrevious = pcEvents->GetTime();
		pcEvents = cTimeline.GetNextSimultaneousEvents(pcEvents);
	}

	szLog.Clear();
	uiExecuted = 0;
	while (cTimeline.RunSimultaneous())
	{
		AssertTrue(cTimeline.IsValid());
		uiExecuted++;
	}
	AssertSize(500 - 166, uiExecuted);
	AssertLong(500, cTimeline.GetTime());
	AssertSize(0, cTimeline.NumSimultaneousEvents());

	cTimeline.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimelineEventID(void)
{
	CTestLogEvent	cEvent;
	CChars			szLog;
	CChars			sz;

	szLog.Init();
	CEvent::ResetNextID();
	New<CTestLogEvent>(&cEvent);
	cEvent.Init(1024, "A", &szLog);
	AssertLong(1, cEvent.GetID());
	cEvent.Init(2048, "B", &szLog);
	AssertLong(2, cEvent.GetID());

	//A loaded event keeps its id and later ones come after it.
	cEvent.CEvent::Init(3072, 10);
	AssertLong(10, cEvent.GetID());
	cEvent.CEvent::Init(4096);
	AssertLong(11, cEvent.GetID());
	AssertFalse(cEvent.IsExecuted());
	AssertFalse(cEvent.IsRemoved());

	sz.Init();
	cEvent.PrintShort(&sz);
	AssertString("TestLog: Time [4096]", sz.Text());
	sz.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTimeline(void)
{
	BeginTests();

	TestTimelineLongTime();
	TestTimelineOrder();
	TestTimelineRunToTime();
	TestTimelineRemove();
	TestTimelineChangesWhileRunning();
	TestTimelineManyTimes();
	TestTimelineEventID();

	TestStatistics();
}

