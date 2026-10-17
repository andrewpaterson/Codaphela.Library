#ifdef LINUX_GNU_32
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "NamedSharedObject_Linux.h"
#include "InterProcessMutex.h"


//A Windows named mutex is shared between processes, recursive, and reports when
//its owner died holding it (WAIT_ABANDONED).  A robust, process shared,
//recursive pthread mutex in named shared memory behaves the same way.
struct SLinuxInterProcessMutex
{
	int					iFile;
	pthread_mutex_t*	psMutex;
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
pthread_mutex_t* MapLinuxMutex(int iFile)
{
	void*	pvMemory;

	pvMemory = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, iFile, 0);
	if (pvMemory == MAP_FAILED)
	{
		return NULL;
	}
	return (pthread_mutex_t*)pvMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Init(char* szName)
{
	mszName.Init(szName);
	LinuxMutex(maMutex).iFile = -1;
	LinuxMutex(maMutex).psMutex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Kill(void)
{
	mszName.Kill();
	LinuxMutex(maMutex).iFile = -1;
	LinuxMutex(maMutex).psMutex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Create(void)
{
	CChars					szName;
	int						iFile;
	int						iErrno;
	pthread_mutex_t*		psMutex;
	pthread_mutexattr_t		sAttributes;
	int						iAttempt;

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MUTEX, mszName.Text());

	//Like CreateMutex; an existing mutex is opened instead.  It may be closed
	//by its last handle between failing to create and opening so try again.
	for (iAttempt = 0; iAttempt < 100; iAttempt++)
	{
		iFile = CreateNamedSharedObject(szName.Text(), sizeof(pthread_mutex_t));
		if (iFile != -1)
		{
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
			psMutex = MapLinuxMutex(iFile);
			if (psMutex == NULL)
			{
				iErrno = errno;
				CloseNamedSharedObject(iFile, szName.Text());
				szName.Kill();
				return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
			}
			szName.Kill();
			LinuxMutex(maMutex).iFile = iFile;
			LinuxMutex(maMutex).psMutex = psMutex;
			return true;
		}
	}

	if (iFile == -1)
	{
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", mszName.Text(), "].", NULL);
	}

	psMutex = MapLinuxMutex(iFile);
	if (psMutex == NULL)
	{
		iErrno = errno;
		CloseNamedSharedObject(iFile, szName.Text());
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not create mutex [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}

	pthread_mutexattr_init(&sAttributes);
	pthread_mutexattr_setpshared(&sAttributes, PTHREAD_PROCESS_SHARED);
	pthread_mutexattr_setrobust(&sAttributes, PTHREAD_MUTEX_ROBUST);
	pthread_mutexattr_settype(&sAttributes, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(psMutex, &sAttributes);
	pthread_mutexattr_destroy(&sAttributes);

	PublishNamedSharedObject(iFile);
	szName.Kill();

	LinuxMutex(maMutex).iFile = iFile;
	LinuxMutex(maMutex).psMutex = psMutex;
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
	pthread_mutex_t*	psMutex;

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MUTEX, mszName.Text());
	iFile = OpenNamedSharedObject(szName.Text());
	if (iFile == -1)
	{
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", LinuxErrorCodeToString(errno), "].", NULL);
	}

	psMutex = MapLinuxMutex(iFile);
	if (psMutex == NULL)
	{
		iErrno = errno;
		CloseNamedSharedObject(iFile, szName.Text());
		szName.Kill();
		return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", LinuxErrorCodeToString(iErrno), "].", NULL);
	}
	szName.Kill();

	LinuxMutex(maMutex).iFile = iFile;
	LinuxMutex(maMutex).psMutex = psMutex;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Close(void)
{
	CChars					szName;
	SLinuxInterProcessMutex*	psHandle;

	psHandle = &LinuxMutex(maMutex);
	if (psHandle->iFile == -1)
	{
		return;
	}

	munmap(psHandle->psMutex, sizeof(pthread_mutex_t));

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MUTEX, mszName.Text());
	CloseNamedSharedObject(psHandle->iFile, szName.Text());
	szName.Kill();

	LinuxMutex(maMutex).iFile = -1;
	LinuxMutex(maMutex).psMutex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Lock(void)
{
	SLinuxInterProcessMutex*	psHandle;
	int						iResult;

	psHandle = &LinuxMutex(maMutex);
	if (psHandle->psMutex == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Lock failed, mutex [", mszName.Text(), "] is not open.", NULL);
	}

	iResult = pthread_mutex_lock(psHandle->psMutex);
	if (iResult == 0)
	{
		return true;
	}
	else if (iResult == EOWNERDEAD)
	{
		// The thread got ownership of an abandoned mutex
		pthread_mutex_consistent(psHandle->psMutex);
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
	SLinuxInterProcessMutex*	psHandle;

	psHandle = &LinuxMutex(maMutex);
	if (psHandle->psMutex == NULL)
	{
		return false;
	}

	if (pthread_mutex_unlock(psHandle->psMutex) != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}


#endif // LINUX_GNU_32

