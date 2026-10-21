#ifdef LINUX_GNU_32
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "BaseLib/NativeThread.h"
#include "NamedSharedObject_Linux.h"
#include "InterProcessMutex.h"


//A Windows named mutex is shared between processes, recursive, and reports when
//its owner died holding it (WAIT_ABANDONED).  On Linux the mutex is a lock word
//in named shared memory that is taken with compare and swap, so that processes
//that can not call pthreads (a JVM) can share it.
//
//The layout is read and written by other languages so it must not change:
//
//	offset 0, int64:	The owner, or 0 when unlocked.  The owner is the process
//						id in the high 32 bits and a thread id, unique within
//						the process, in the low 32 bits.  Set with compare and
//						swap to lock and cleared with a release store to unlock.
//	offset 8, int32:	How many times the owner has locked it.  Only the owner
//						reads or writes it.
//	offset 12, int32:	Unused.
//
//A waiter that finds the owning process has exited takes the lock, as a robust
//pthread mutex does when its owner died.  A thread that exits while its
//process carries on is not noticed.
struct SInterProcessLock
{
	int64	iOwner;
	int32	iCount;
	int32	iUnused;
};


static_assert(sizeof(SInterProcessLock) == 16, "SInterProcessLock is shared with other languages and must be 16 bytes.");


//Spins this many times, then yields this many times, then sleeps between tries.
//It only sleeps while one owner keeps the lock.  While the lock changes hands it
//keeps yielding: a waiter that slept would only try once per sleep and lose the
//lock every time to threads that take it again as soon as they let it go.
#define INTER_PROCESS_LOCK_SPINS				64
#define INTER_PROCESS_LOCK_YIELDS				1024
#define INTER_PROCESS_LOCK_SLEEP_NANOSECONDS	50000
//While sleeping, checks whether the owner is still running every this many tries.
#define INTER_PROCESS_LOCK_OWNER_CHECK_TRIES	256


struct SLinuxInterProcessMutex
{
	int						iFile;
	SInterProcessLock*		psLock;
};


static_assert(sizeof(SLinuxInterProcessMutex) <= INTER_PROCESS_MUTEX_HANDLE_SIZE, "INTER_PROCESS_MUTEX_HANDLE_SIZE is too small for SLinuxInterProcessMutex.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SLinuxInterProcessMutex& LinuxMutex(uint8* paMutex)
{
	return *(SLinuxInterProcessMutex*)paMutex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SInterProcessLock* MapLinuxLock(int iFile)
{
	void*	pvMemory;

	pvMemory = mmap(NULL, sizeof(SInterProcessLock), PROT_READ | PROT_WRITE, MAP_SHARED, iFile, 0);
	if (pvMemory == MAP_FAILED)
	{
		return NULL;
	}
	return (SInterProcessLock*)pvMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static thread_local int64	tiLockOwner = 0;

static void ForgetLockOwner(void)
{
	tiLockOwner = 0;
}

static int64 CurrentLockOwner(void)
{
	static bool	bAtFork = (pthread_atfork(NULL, NULL, ForgetLockOwner) == 0);

	(void)bAtFork;
	if (tiLockOwner == 0)
	{
		tiLockOwner = ((int64)getpid() << 32) | (int64)(uint32)syscall(SYS_gettid);
	}
	return tiLockOwner;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsLockOwnerDead(int64 iOwner)
{
	pid_t	iProcess;

	iProcess = (pid_t)(iOwner >> 32);
	if (iProcess <= 0)
	{
		return false;
	}
	return (kill(iProcess, 0) == -1) && (errno == ESRCH);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void LockBackOff(int iTry)
{
	struct timespec	sSleep;

	if (iTry < INTER_PROCESS_LOCK_SPINS)
	{
#if defined(__x86_64__) || defined(__i386__)
		__builtin_ia32_pause();
#endif
	}
	else if (iTry < INTER_PROCESS_LOCK_SPINS + INTER_PROCESS_LOCK_YIELDS)
	{
		ThreadYield();
	}
	else
	{
		sSleep.tv_sec = 0;
		sSleep.tv_nsec = INTER_PROCESS_LOCK_SLEEP_NANOSECONDS;
		nanosleep(&sSleep, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Init(char* szName)
{
	mszName.Init(szName);
	LinuxMutex(maMutex).iFile = -1;
	LinuxMutex(maMutex).psLock = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Kill(void)
{
	mszName.Kill();
	LinuxMutex(maMutex).iFile = -1;
	LinuxMutex(maMutex).psLock = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Create(void)
{
	CChars				szName;
	int					iFile;
	int					iErrno;
	SInterProcessLock*	psLock;
	int					iAttempt;
	bool				bCreated;

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MUTEX, mszName.Text());

	//Like CreateMutex; an existing mutex is opened instead.  It may be closed
	//by its last handle between failing to create and opening so try again.
	bCreated = false;
	iFile = -1;
	for (iAttempt = 0; iAttempt < 100; iAttempt++)
	{
		iFile = CreateNamedSharedObject(szName.Text(), sizeof(SInterProcessLock));
		if (iFile != -1)
		{
			bCreated = true;
			break;
		}
		iErrno = errno;
		if (iErrno != EEXIST)
		{
			szName.Kill();
			return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
		}

		iFile = OpenNamedSharedObject(szName.Text());
		if (iFile != -1)
		{
			break;
		}
	}

	if (iFile == -1)
	{
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", mszName.Text(), "].", NULL);
	}

	psLock = MapLinuxLock(iFile);
	if (psLock == NULL)
	{
		iErrno = errno;
		CloseNamedSharedObject(iFile, szName.Text());
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}

	//A new object is filled with zeros, which is an unlocked lock.
	if (bCreated)
	{
		PublishNamedSharedObject(iFile);
	}
	szName.Kill();

	LinuxMutex(maMutex).iFile = iFile;
	LinuxMutex(maMutex).psLock = psLock;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Connect(void)
{
	CChars				szName;
	int					iFile;
	int					iErrno;
	SInterProcessLock*	psLock;

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MUTEX, mszName.Text());

	iFile = OpenNamedSharedObject(szName.Text());
	if (iFile == -1)
	{
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", LinuxErrorCodeToString(errno), "].", NULL);
	}

	psLock = MapLinuxLock(iFile);
	if (psLock == NULL)
	{
		iErrno = errno;
		CloseNamedSharedObject(iFile, szName.Text());
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}

	szName.Kill();
	LinuxMutex(maMutex).iFile = iFile;
	LinuxMutex(maMutex).psLock = psLock;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Close(void)
{
	CChars						szName;
	SLinuxInterProcessMutex*	psHandle;

	psHandle = &LinuxMutex(maMutex);
	if (psHandle->iFile == -1)
	{
		return;
	}

	munmap(psHandle->psLock, sizeof(SInterProcessLock));
	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MUTEX, mszName.Text());
	CloseNamedSharedObject(psHandle->iFile, szName.Text());
	szName.Kill();

	LinuxMutex(maMutex).iFile = -1;
	LinuxMutex(maMutex).psLock = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Lock(void)
{
	SInterProcessLock*	psLock;
	int64				iSelf;
	int64				iOwner;
	int64				iLastOwner;
	int					iTry;

	psLock = LinuxMutex(maMutex).psLock;
	if (psLock == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Lock failed, mutex [", mszName.Text(), "] is not open.", NULL);
	}

	iSelf = CurrentLockOwner();
	if (__atomic_load_n(&psLock->iOwner, __ATOMIC_ACQUIRE) == iSelf)
	{
		psLock->iCount++;
		return true;
	}

	iLastOwner = 0;
	for (iTry = 0; ; iTry++)
	{
		iOwner = 0;
		if (__atomic_compare_exchange_n(&psLock->iOwner, &iOwner, iSelf, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
		{
			psLock->iCount = 1;
			return true;
		}

		if ((iOwner != iLastOwner) && (iTry > INTER_PROCESS_LOCK_SPINS))
		{
			iTry = INTER_PROCESS_LOCK_SPINS;
		}
		iLastOwner = iOwner;

		if ((iTry >= INTER_PROCESS_LOCK_SPINS + INTER_PROCESS_LOCK_YIELDS) && (iTry % INTER_PROCESS_LOCK_OWNER_CHECK_TRIES == 0))
		{
			//iOwner holds the owner the compare and swap found.
			if (IsLockOwnerDead(iOwner))
			{
				if (__atomic_compare_exchange_n(&psLock->iOwner, &iOwner, iSelf, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				{
					//Like EOWNERDEAD: the lock is held but what it protects may
					//be inconsistent.
					psLock->iCount = 1;
					return false;
				}
			}
		}
		LockBackOff(iTry);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Unlock(void)
{
	SInterProcessLock*	psLock;

	psLock = LinuxMutex(maMutex).psLock;
	if (psLock == NULL)
	{
		return false;
	}
	if (__atomic_load_n(&psLock->iOwner, __ATOMIC_RELAXED) != CurrentLockOwner())
	{
		return false;
	}

	if (psLock->iCount > 1)
	{
		psLock->iCount--;
		return true;
	}
	psLock->iCount = 0;
	__atomic_store_n(&psLock->iOwner, 0, __ATOMIC_RELEASE);
	return true;
}


#endif // LINUX_GNU_32
