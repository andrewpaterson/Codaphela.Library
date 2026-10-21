#include <string.h>
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/Timer.h"
#include "BaseLib/NativeThread.h"
#ifdef _WIN32
#include "BaseLib/WindowsHeaders.h"
#endif
#include "InterProcessName.h"
#include "SharedMemoryChannel.h"


//The process that Created the channel sends on "name:A" and receives on
//"name:B".  The process that Connected does the opposite.  Each element in a
//queue is an SProcessChannelHeader followed by the data.
#define SHARED_MEMORY_CHANNEL_CREATOR		0
#define SHARED_MEMORY_CHANNEL_CONNECTOR		1

//While waiting, Receive spins this many times (about a tenth of a
//millisecond), so a reply that comes quickly is seen at once, and then sleeps
//on its signal until the other process sends.  It spins with a pause, not by
//yielding: with more waiting threads than processors yielding switched between
//them millions of times a second and halved throughput.  The timeout is only
//timed after the spin, so the timer costs nothing while a conversation is busy.
//A sleep is cut short after SHARED_MEMORY_CHANNEL_SIGNAL_MS in case a wake was
//lost (the other process died while sending).  After a Receive that timed out
//the next one does not spin, so an idle receiver that keeps calling Receive
//only wakes when its timeout runs out.
#define SHARED_MEMORY_CHANNEL_SPIN_PAUSES		2000
#define SHARED_MEMORY_CHANNEL_SIGNAL_MS			100

//When the other process can not wake it Receive polls: it yields for this long
//and then sleeps between polls.  The yielding is timed, not counted, because a
//sleep of 1ms can take a whole scheduler tick (about 15ms on Windows); a reply
//that took a tick each time would make a conversation of many messages crawl.
#define SHARED_MEMORY_CHANNEL_YIELD_MS			10
#define SHARED_MEMORY_CHANNEL_SLEEP_MS			1


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SpinPause(void)
{
#ifdef _WIN32
	YieldProcessor();
#else
	__builtin_ia32_pause();
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::Init(void)
{
	mpsState = NULL;
	miSide = SHARED_MEMORY_CHANNEL_CREATOR;
	mbOpen = false;
	mbSpin = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::Kill(void)
{
	Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::QueueName(CChars* psz, char* szName, char* szDirection)
{
	psz->Init(szName);
	psz->Append(szDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryChannel::InitSignals(char* szName, bool bCreate)
{
	CChars	sz;
	CChars	szSide;
	int		i;
	bool	bResult;

	InterProcessName(&sz, INTER_PROCESS_SIGNAL_PREFIX, szName);
	for (i = 0; i < 2; i++)
	{
		szSide.Init(sz.Text());
		szSide.Append(i == SHARED_MEMORY_CHANNEL_CREATOR ? ":A" : ":B");
		macSignal[i].Init(szSide.Text(), &mpsState->asSignal[i].sSignal);
		szSide.Kill();
		bResult = bCreate ? macSignal[i].Create() : macSignal[i].Connect();
		if (!bResult)
		{
			for (; i >= 0; i--)
			{
				macSignal[i].Kill();
			}
			sz.Kill();
			return false;
		}
	}
	sz.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::KillSignals(void)
{
	macSignal[SHARED_MEMORY_CHANNEL_CREATOR].Kill();
	macSignal[SHARED_MEMORY_CHANNEL_CONNECTOR].Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryChannel::Create(char* szName)
{
	CChars	sz;
	bool	bSend;
	bool	bReceive;

	InterProcessName(&sz, INTER_PROCESS_CHANNEL_PREFIX, szName);
	mcState.Init(sz.Text());
	sz.Kill();
	mpsState = (SSharedMemoryChannelState*)mcState.Create(sizeof(SSharedMemoryChannelState));
	if (mpsState == NULL)
	{
		mcState.Kill();
		return false;
	}
	memset(mpsState->asSignal, 0, sizeof(mpsState->asSignal));
	mpsState->aiCanWake[SHARED_MEMORY_CHANNEL_CONNECTOR].Set(0);
	mpsState->aiCanWake[SHARED_MEMORY_CHANNEL_CREATOR].Set(1);
	mpsState->aiState[SHARED_MEMORY_CHANNEL_CREATOR].Set(SHARED_MEMORY_CHANNEL_OPEN);
	mpsState->aiState[SHARED_MEMORY_CHANNEL_CONNECTOR].Set(SHARED_MEMORY_CHANNEL_WAITING);
	if (!InitSignals(szName, true))
	{
		mpsState->aiState[SHARED_MEMORY_CHANNEL_CREATOR].Set(SHARED_MEMORY_CHANNEL_CLOSED);
		mcState.Close();
		mcState.Kill();
		mpsState = NULL;
		return false;
	}

	QueueName(&sz, szName, ":A");
	bSend = mcSend.Init(sz.Text(), 0);
	sz.Kill();
	QueueName(&sz, szName, ":B");
	bReceive = mcReceive.Init(sz.Text(), 0);
	sz.Kill();

	miSide = SHARED_MEMORY_CHANNEL_CREATOR;
	if (!bSend || !bReceive)
	{
		Fail(bSend, bReceive);
		return false;
	}
	mbOpen = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryChannel::Connect(char* szName)
{
	CChars	sz;
	bool	bSend;
	bool	bReceive;

	InterProcessName(&sz, INTER_PROCESS_CHANNEL_PREFIX, szName);
	mcState.Init(sz.Text());
	sz.Kill();
	mpsState = (SSharedMemoryChannelState*)mcState.Open();
	if (mpsState == NULL)
	{
		mcState.Kill();
		return false;
	}
	if (!InitSignals(szName, false))
	{
		mcState.Close();
		mcState.Kill();
		mpsState = NULL;
		return false;
	}

	QueueName(&sz, szName, ":B");
	bSend = mcSend.Init(sz.Text());
	sz.Kill();
	QueueName(&sz, szName, ":A");
	bReceive = mcReceive.Init(sz.Text());
	sz.Kill();

	miSide = SHARED_MEMORY_CHANNEL_CONNECTOR;
	if (!bSend || !bReceive)
	{
		Fail(bSend, bReceive);
		return false;
	}
	mbOpen = true;

	mpsState->aiCanWake[SHARED_MEMORY_CHANNEL_CONNECTOR].Set(1);
	mpsState->aiState[SHARED_MEMORY_CHANNEL_CONNECTOR].Set(SHARED_MEMORY_CHANNEL_OPEN);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryChannel::Send(uint32 uiType, void* pvData, size uiSize)
{
	CStackMemory<>			cTemp;
	SProcessChannelHeader*	psHeader;
	bool					bResult;

	if (!mbOpen)
	{
		return false;
	}

	psHeader = BuildMessage(&cTemp, uiType, pvData, uiSize);
	if (psHeader == NULL)
	{
		return false;
	}

	bResult = mcSend.Push(psHeader, sizeof(SProcessChannelHeader) + uiSize);
	cTemp.Kill();
	if (bResult)
	{
		macSignal[1 - miSide].Wake();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EChannelResult CSharedMemoryChannel::Receive(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, int iTimeoutMilliseconds)
{
	size	uiElementSize;
	int		iOtherState;
	CTimer	cTimer;
	bool	bTimer;
	bool	bSleep;
	int		iSpins;

	if (!mbOpen)
	{
		return CR_Closed;
	}

	bTimer = false;
	bSleep = false;
	iSpins = 0;
	for (;;)
	{
		//Read the other state before looking in the queue so a message sent
		//just before it closed is still received.
		iOtherState = mpsState->aiState[1 - miSide].Get();

		//Only takes the queue's mutex when there may be a message, so an idle
		//receiver does not hold up the sender.
		if (mcReceive.MayHaveElements())
		{
			uiElementSize = mcReceive.PeekSize();
			if (uiElementSize != 0)
			{
				if (bTimer)
				{
					cTimer.Kill();
				}
				mbSpin = true;
				return Pop(puiType, pvData, uiMaxSize, puiSize, uiElementSize);
			}
		}

		if (iOtherState == SHARED_MEMORY_CHANNEL_CLOSED)
		{
			if (bTimer)
			{
				cTimer.Kill();
			}
			return CR_Closed;
		}

		if (iTimeoutMilliseconds == 0)
		{
			return CR_Timeout;
		}

		if (mbSpin && (iSpins < SHARED_MEMORY_CHANNEL_SPIN_PAUSES))
		{
			iSpins++;
			SpinPause();
			continue;
		}

		if (!bTimer)
		{
			cTimer.Init();
			bTimer = true;
		}
		else
		{
			cTimer.Update();
		}
		if ((iTimeoutMilliseconds > 0) && (cTimer.GetTotalTimeInMillieconds() >= iTimeoutMilliseconds))
		{
			cTimer.Kill();
			mbSpin = false;
			return CR_Timeout;
		}

		if (mpsState->aiCanWake[1 - miSide].Get() != 0)
		{
			WaitForSignal(iTimeoutMilliseconds > 0 ? iTimeoutMilliseconds - (int)cTimer.GetTotalTimeInMillieconds() : SHARED_MEMORY_CHANNEL_SIGNAL_MS);
			continue;
		}

		if (!bSleep)
		{
			bSleep = cTimer.GetTotalTimeInMillieconds() >= SHARED_MEMORY_CHANNEL_YIELD_MS;
		}

		if (bSleep)
		{
			ThreadSleep(SHARED_MEMORY_CHANNEL_SLEEP_MS);
		}
		else
		{
			ThreadYield();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::WaitForSignal(int iTimeoutMilliseconds)
{
	CInterProcessSignal*	pcSignal;
	int						iSequence;

	if (iTimeoutMilliseconds > SHARED_MEMORY_CHANNEL_SIGNAL_MS)
	{
		iTimeoutMilliseconds = SHARED_MEMORY_CHANNEL_SIGNAL_MS;
	}
	else if (iTimeoutMilliseconds < 1)
	{
		iTimeoutMilliseconds = 1;
	}

	//Looks once more after BeginWait; a message or close after that wakes the signal.
	pcSignal = &macSignal[miSide];
	iSequence = pcSignal->BeginWait();
	if (mcReceive.MayHaveElements() || (mpsState->aiState[1 - miSide].Get() == SHARED_MEMORY_CHANNEL_CLOSED))
	{
		pcSignal->CancelWait();
		return;
	}
	pcSignal->Wait(iSequence, iTimeoutMilliseconds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EChannelResult CSharedMemoryChannel::Pop(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, size uiElementSize)
{
	CStackMemory<>			cTemp;
	SProcessChannelHeader*	psHeader;
	size					uiPoppedSize;

	if (uiElementSize < sizeof(SProcessChannelHeader))
	{
		return CR_Error;
	}

	//Only this process receives from this queue so the element peeked is the one popped.
	psHeader = (SProcessChannelHeader*)cTemp.Init(uiElementSize);
	if (!mcReceive.Pop(psHeader, &uiPoppedSize, uiElementSize) ||
		(psHeader->uiSize != uiPoppedSize - sizeof(SProcessChannelHeader)))
	{
		cTemp.Kill();
		return CR_Error;
	}

	*puiType = psHeader->uiType;
	if (puiSize)
	{
		*puiSize = psHeader->uiSize;
	}

	if (psHeader->uiSize > uiMaxSize)
	{
		cTemp.Kill();
		return CR_TooLarge;
	}

	if (psHeader->uiSize > 0)
	{
		memcpy(pvData, RemapSinglePointer(psHeader, sizeof(SProcessChannelHeader)), psHeader->uiSize);
	}
	cTemp.Kill();
	return CR_Message;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::Close(void)
{
	if (!mbOpen)
	{
		return;
	}

	//Messages already sent stay in the queue for the other process.
	mpsState->aiState[miSide].Set(SHARED_MEMORY_CHANNEL_CLOSED);
	macSignal[1 - miSide].Wake();
	KillSignals();
	mcSend.Kill();
	mcReceive.Kill();
	mcState.Close();
	mcState.Kill();
	mpsState = NULL;
	mbOpen = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::Fail(bool bSend, bool bReceive)
{
	if (bSend)
	{
		mcSend.Kill();
	}
	if (bReceive)
	{
		mcReceive.Kill();
	}
	//A process that failed to Connect never opened its side so it is left
	//waiting.  Closing it would tell the creator the channel is finished while
	//the connector may still be retrying (it may have found the state before
	//the creator made the queues).
	if (miSide == SHARED_MEMORY_CHANNEL_CREATOR)
	{
		mpsState->aiState[miSide].Set(SHARED_MEMORY_CHANNEL_CLOSED);
		macSignal[1 - miSide].Wake();
	}
	KillSignals();
	mcState.Close();
	mcState.Kill();
	mpsState = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryChannel::IsOpen(void)
{
	return mbOpen;
}

