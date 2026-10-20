#include <string.h>
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/Timer.h"
#include "BaseLib/NativeThread.h"
#include "InterProcessName.h"
#include "SharedMemoryChannel.h"


//The process that Created the channel sends on "name:A" and receives on
//"name:B".  The process that Connected does the opposite.  Each element in a
//queue is an SProcessChannelHeader followed by the data.
#define SHARED_MEMORY_CHANNEL_CREATOR		0
#define SHARED_MEMORY_CHANNEL_CONNECTOR		1

//While waiting, Receive yields this many times and then sleeps between polls,
//so a reply that comes quickly is seen at once and a long wait does not use a
//whole processor.
#define SHARED_MEMORY_CHANNEL_YIELDS			1024
#define SHARED_MEMORY_CHANNEL_SLEEP_MS			1


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryChannel::Init(void)
{
	mpsState = NULL;
	miSide = SHARED_MEMORY_CHANNEL_CREATOR;
	mbOpen = false;
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
	mpsState->aiState[SHARED_MEMORY_CHANNEL_CREATOR].Set(SHARED_MEMORY_CHANNEL_OPEN);
	mpsState->aiState[SHARED_MEMORY_CHANNEL_CONNECTOR].Set(SHARED_MEMORY_CHANNEL_WAITING);

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
	bool	bTimed;
	int		iPolls;

	if (!mbOpen)
	{
		return CR_Closed;
	}

	bTimed = iTimeoutMilliseconds > 0;
	if (bTimed)
	{
		cTimer.Init();
	}

	for (iPolls = 0; ; iPolls++)
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
				if (bTimed)
				{
					cTimer.Kill();
				}
				return Pop(puiType, pvData, uiMaxSize, puiSize, uiElementSize);
			}
		}

		if (iOtherState == SHARED_MEMORY_CHANNEL_CLOSED)
		{
			if (bTimed)
			{
				cTimer.Kill();
			}
			return CR_Closed;
		}

		if (iTimeoutMilliseconds == 0)
		{
			return CR_Timeout;
		}
		if (bTimed)
		{
			cTimer.Update();
			if (cTimer.GetTotalTimeInMillieconds() >= iTimeoutMilliseconds)
			{
				cTimer.Kill();
				return CR_Timeout;
			}
		}

		if (iPolls < SHARED_MEMORY_CHANNEL_YIELDS)
		{
			ThreadYield();
		}
		else
		{
			ThreadSleep(SHARED_MEMORY_CHANNEL_SLEEP_MS);
		}
	}
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
	}
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

