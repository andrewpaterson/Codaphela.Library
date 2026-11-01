#ifndef __INTER_PROCESS_SIGNAL_H__
#define __INTER_PROCESS_SIGNAL_H__
#include "BaseLib/Chars.h"
#include "AtomicInt.h"


//Large enough to hold a Windows event HANDLE.
#define INTER_PROCESS_SIGNAL_HANDLE_SIZE	8


//In shared memory.  Other languages wait on and wake it so the layout must not
//change:
//
//	offset 0, int32:	The sequence.  Wake adds one to it when a process is
//						waiting.  On Linux the waiter sleeps on it with futex.
//	offset 4, int32:	How many processes are between BeginWait and the end of
//						Wait (or CancelWait).
struct SInterProcessSignal
{
	CAtomicInt	iSequence;
	CAtomicInt	iWaiters;
};


//Lets one process sleep until another wakes it, without either polling.  The
//waiter calls BeginWait, looks once more for whatever it waits for, and then
//calls Wait with the sequence BeginWait returned (or CancelWait if it found it).
//The waker makes what the waiter waits for visible and then calls Wake.  A wake
//between BeginWait and Wait is not lost: Wait returns at once.  Wait can also
//return early for no reason so the waiter must always look again.
//
//Wake costs a fence and a read unless a process is waiting.
//
//On Linux the signal is a futex on the sequence and the name is not used.  On
//Windows it is a named auto-reset event; one process Creates it and the others
//Connect to it by name.
class CInterProcessSignal
{
protected:
	CChars					mszName;
	SInterProcessSignal*	mpsShared;
	alignas(8) uint8		maHandle[INTER_PROCESS_SIGNAL_HANDLE_SIZE];

public:
	void	Init(char* szName, SInterProcessSignal* psShared);
	void	Kill(void);

	bool	Create(void);
	bool	Connect(void);
	void	Close(void);

	int		BeginWait(void);
	bool	Wait(int iSequence, int iTimeoutMilliseconds);
	void	CancelWait(void);
	void	Wake(void);
};


#endif // __INTER_PROCESS_SIGNAL_H__
