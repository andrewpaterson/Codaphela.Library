#ifndef __SHARED_MEMORY_CHANNEL_H__
#define __SHARED_MEMORY_CHANNEL_H__
#include "AtomicInt.h"
#include "SharedMemory.h"
#include "SharedMemoryQueue.h"
#include "ProcessChannel.h"


#define SHARED_MEMORY_CHANNEL_WAITING	0	//Not connected yet.
#define SHARED_MEMORY_CHANNEL_OPEN		1
#define SHARED_MEMORY_CHANNEL_CLOSED	2


//In shared memory; the state of the process that Created the channel and of
//the process that Connected to it.
struct SSharedMemoryChannelState
{
	CAtomicInt	aiState[2];
};


//A process channel over two CSharedMemoryQueues, one for each direction.  One
//process Creates it by name and the other Connects with the same name.
//Receive has nothing to wake it so it yields, then sleeps, while it waits.
class CSharedMemoryChannel : public CProcessChannel
{
protected:
	CSharedMemoryQueue			mcSend;
	CSharedMemoryQueue			mcReceive;
	CSharedMemory				mcState;
	SSharedMemoryChannelState*	mpsState;
	int							miSide;
	bool						mbOpen;

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
	void			Fail(bool bSend, bool bReceive);
	EChannelResult	Pop(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, size uiElementSize);
};


#endif // __SHARED_MEMORY_CHANNEL_H__

