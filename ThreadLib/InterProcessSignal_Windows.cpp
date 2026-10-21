#ifdef _WIN32
#include "BaseLib/Logger.h"
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/WindowsError.h"
#include "InterProcessSignal.h"


static_assert(sizeof(HANDLE) <= INTER_PROCESS_SIGNAL_HANDLE_SIZE, "INTER_PROCESS_SIGNAL_HANDLE_SIZE is too small for a HANDLE.");


//WaitOnAddress only works within a process so the waiter sleeps on a named
//auto-reset event.  The sequence is still kept so a Wake between BeginWait and
//Wait is seen without the event: the event stays set and Wait returns at once.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static HANDLE& EventHandle(uint8* paHandle)
{
	return *(HANDLE*)paHandle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessSignal::Create(void)
{
	HANDLE	hEvent;

	hEvent = CreateEventA(
				NULL,				// default security descriptor
				false,				// auto-reset
				false,				// not set
				mszName.Text());	// object name

	EventHandle(maHandle) = hEvent;
	if (hEvent == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Could not create event [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessSignal::Connect(void)
{
	HANDLE	hEvent;

	hEvent = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, false, mszName.Text());

	EventHandle(maHandle) = hEvent;
	if (hEvent == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Could not open event [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessSignal::Close(void)
{
	if (EventHandle(maHandle) != NULL)
	{
		CloseHandle(EventHandle(maHandle));
		EventHandle(maHandle) = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessSignal::Wait(int iSequence, int iTimeoutMilliseconds)
{
	DWORD	dwResult;

	if (mpsShared->iSequence.Get() != iSequence)
	{
		mpsShared->iWaiters.Decrement();
		return true;
	}

	dwResult = WaitForSingleObject(EventHandle(maHandle), iTimeoutMilliseconds >= 0 ? (DWORD)iTimeoutMilliseconds : INFINITE);
	mpsShared->iWaiters.Decrement();
	return dwResult == WAIT_OBJECT_0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessSignal::Wake(void)
{
	//Orders what the waker did before the read of the waiters; see BeginWait.
	MemoryBarrier();
	if (mpsShared->iWaiters.Get() == 0)
	{
		return;
	}
	mpsShared->iSequence.Increment();
	SetEvent(EventHandle(maHandle));
}


#endif // _WIN32
