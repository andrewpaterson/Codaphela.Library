#ifdef LINUX_GNU_32
#include <atomic>
#include <thread>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "InterProcessMutex.h"


//A Windows named mutex is shared between processes, recursive, and reports when
//its owner died holding it (WAIT_ABANDONED).  A robust, process shared,
//recursive pthread mutex in named shared memory behaves the same way.
struct SLinuxInterProcessMutex
{
	std::atomic<int>	iInitialised;
	pthread_mutex_t		sMutex;
};


static_assert(sizeof(SLinuxInterProcessMutex*) <= INTER_PROCESS_MUTEX_HANDLE_SIZE, "INTER_PROCESS_MUTEX_HANDLE_SIZE is too small for a mutex pointer.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetMutexHandle(uint8* paMutex, SLinuxInterProcessMutex* psMutex)
{
	memcpy(paMutex, &psMutex, sizeof(SLinuxInterProcessMutex*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLinuxInterProcessMutex* GetMutexHandle(uint8* paMutex)
{
	SLinuxInterProcessMutex*	psMutex;

	memcpy(&psMutex, paMutex, sizeof(SLinuxInterProcessMutex*));
	return psMutex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LinuxMutexName(CChars* psz, char* szName)
{
	//POSIX shared memory names are a single '/' followed by no other '/'.
	psz->Init(szName);
	psz->Replace('/', '_');
	psz->Insert(0, "/Mutex.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLinuxInterProcessMutex* MapLinuxMutex(int iFile)
{
	void*	pvMemory;

	pvMemory = mmap(NULL, sizeof(SLinuxInterProcessMutex), PROT_READ | PROT_WRITE, MAP_SHARED, iFile, 0);
	if (pvMemory == MAP_FAILED)
	{
		return NULL;
	}
	return (SLinuxInterProcessMutex*)pvMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Init(char* szName)
{
	mszName.Init(szName);
	SetMutexHandle(maMutex, NULL);
	mbCreated = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Kill(void)
{
	mszName.Kill();
	SetMutexHandle(maMutex, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Create(void)
{
	CChars						szName;
	int							iFile;
	int							iErrno;
	SLinuxInterProcessMutex*	psMutex;
	pthread_mutexattr_t			sAttributes;

	LinuxMutexName(&szName, mszName.Text());
	iFile = shm_open(szName.Text(), O_RDWR | O_CREAT | O_EXCL, 0600);
	if (iFile == -1)
	{
		iErrno = errno;
		szName.Kill();
		if (iErrno == EEXIST)
		{
			//Like CreateMutex on an existing name; use the existing mutex.
			return Connect();
		}
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}

	if (ftruncate(iFile, sizeof(SLinuxInterProcessMutex)) == -1)
	{
		iErrno = errno;
		close(iFile);
		shm_unlink(szName.Text());
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}

	psMutex = MapLinuxMutex(iFile);
	iErrno = errno;
	close(iFile);
	if (psMutex == NULL)
	{
		shm_unlink(szName.Text());
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}
	szName.Kill();

	pthread_mutexattr_init(&sAttributes);
	pthread_mutexattr_setpshared(&sAttributes, PTHREAD_PROCESS_SHARED);
	pthread_mutexattr_setrobust(&sAttributes, PTHREAD_MUTEX_ROBUST);
	pthread_mutexattr_settype(&sAttributes, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&psMutex->sMutex, &sAttributes);
	pthread_mutexattr_destroy(&sAttributes);
	psMutex->iInitialised.store(1, std::memory_order_release);

	SetMutexHandle(maMutex, psMutex);
	mbCreated = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Connect(void)
{
	CChars						szName;
	int							iFile;
	int							iErrno;
	SLinuxInterProcessMutex*	psMutex;

	LinuxMutexName(&szName, mszName.Text());
	iFile = shm_open(szName.Text(), O_RDWR, 0600);
	szName.Kill();
	if (iFile == -1)
	{
		return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", LinuxErrorCodeToString(errno), "].", NULL);
	}

	psMutex = MapLinuxMutex(iFile);
	iErrno = errno;
	close(iFile);
	if (psMutex == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}

	//The creating process may not have finished initialising the mutex.
	while (psMutex->iInitialised.load(std::memory_order_acquire) == 0)
	{
		std::this_thread::yield();
	}

	SetMutexHandle(maMutex, psMutex);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Close(void)
{
	CChars						szName;
	SLinuxInterProcessMutex*	psMutex;

	psMutex = GetMutexHandle(maMutex);
	if (psMutex == NULL)
	{
		return;
	}

	munmap(psMutex, sizeof(SLinuxInterProcessMutex));
	SetMutexHandle(maMutex, NULL);

	//A Windows mutex goes away when its last handle is closed but a POSIX name
	//stays until unlinked.  Processes that have already connected keep it.
	if (mbCreated)
	{
		LinuxMutexName(&szName, mszName.Text());
		shm_unlink(szName.Text());
		szName.Kill();
		mbCreated = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Lock(void)
{
	SLinuxInterProcessMutex*	psMutex;
	int							iResult;

	psMutex = GetMutexHandle(maMutex);
	if (psMutex == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Lock failed, mutex [", mszName.Text(), "] is not open.", NULL);
	}

	iResult = pthread_mutex_lock(&psMutex->sMutex);
	if (iResult == 0)
	{
		return true;
	}
	else if (iResult == EOWNERDEAD)
	{
		// The thread got ownership of an abandoned mutex
		pthread_mutex_consistent(&psMutex->sMutex);
		return false;
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Lock failed with [", LinuxErrorCodeToString(iResult), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Unlock(void)
{
	SLinuxInterProcessMutex*	psMutex;

	psMutex = GetMutexHandle(maMutex);
	if (psMutex == NULL)
	{
		return false;
	}

	if (pthread_mutex_unlock(&psMutex->sMutex) != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}


#endif // LINUX_GNU_32

