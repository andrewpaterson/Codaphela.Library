#ifndef __SHARED_MEMORY_CHANNEL_H__
#define __SHARED_MEMORY_CHANNEL_H__
#include "AtomicInt.h"
#include "SharedMemory.h"
#include "SharedMemoryQueue.h"
#include "InterProcessSignal.h"
#include "ProcessChannel.h"


#define SHARED_MEMORY_CHANNEL_WAITING	0	//Not connected yet.
#define SHARED_MEMORY_CHANNEL_OPEN		1
#define SHARED_MEMORY_CHANNEL_CLOSED	2


//In shared memory; index 0 is the process that Created the channel and index
//1 the process that Connected to it.  Other languages read and write it so the
//layout must not change:
//
//	offset 0, int32[2]:		aiState, SHARED_MEMORY_CHANNEL_WAITING, _OPEN or
//							_CLOSED.
//	offset 8, int32[2]:		aiCanWake, 1 if that process Wakes the other's
//							signal after every Send and on Close.  A process
//							only sleeps on its signal while the other can wake
//							it; otherwise it polls.
//	offset 64, 8 bytes:		asSignal[0], waited on by the creator.
//	offset 128, 8 bytes:	asSignal[1], waited on by the connector.
//
//The signals are 64 bytes apart so neither shares a cache line with the other
//or with the states.  A channel made by a version without signals is all zero
//past the states, so neither side sleeps.
struct SSharedMemoryChannelSignal
{
	SInterProcessSignal	sSignal;
	int					aiPadding[14];
};

struct SSharedMemoryChannelState
{
	CAtomicInt					aiState[2];
	CAtomicInt					aiCanWake[2];
	int							aiPadding[12];
	SSharedMemoryChannelSignal	asSignal[2];
};


//A process channel over two CSharedMemoryQueues, one for each direction.  One
//process Creates it by name and the other Connects with the same name.
//Receive spins for a moment and then sleeps until the other process sends.
class CSharedMemoryChannel : public CProcessChannel
{
protected:
	CSharedMemoryQueue			mcSend;
	CSharedMemoryQueue			mcReceive;
	CSharedMemory				mcState;
	SSharedMemoryChannelState*	mpsState;
	CInterProcessSignal			macSignal[2];
	int							miSide;
	bool						mbOpen;
	bool						mbSpin;

public:
	void			Init(void);
	void			Kill(void);

	bool			Create(char* szName);
	bool			Connect(char* szName);

	using			CProcessChannel::Send;
	bool			Send(uint32 uiType, void* pvData, size uiSize);
	EChannelResult	Receive(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, int iTimeoutMilliseconds);
	void			Close(void);
	bool			IsOpen(void);

protected:
	void			QueueName(CChars* psz, char* szName, char* szDirection);
	bool			InitSignals(char* szName, bool bCreate);
	void			KillSignals(void);
	void			Fail(bool bSend, bool bReceive);
	EChannelResult	Pop(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, size uiElementSize);
	void			WaitForSignal(int iTimeoutMilliseconds);
};


#endif // __SHARED_MEMORY_CHANNEL_H__

