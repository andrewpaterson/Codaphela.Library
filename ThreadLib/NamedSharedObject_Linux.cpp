#ifdef LINUX_GNU
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "NamedSharedObject_Linux.h"


#define NAMED_SHARED_OBJECT_PAGE_SIZE		4096
#define NAMED_SHARED_OBJECT_DIRECTORY		"/dev/shm"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//Locks the whole file with an open file description lock.  These behave as
//flock does (they belong to the file descriptor and are released when it is
//closed, even if the process crashed) but they are fcntl record locks, so they
//also conflict with the fcntl locks other languages take (Java FileLock).
//iType is F_RDLCK, F_WRLCK or F_UNLCK.  Returns 0, or -1 with errno set.
static int LockFile(int iFile, short iType, bool bWait)
{
	struct flock	sLock;

	memset(&sLock, 0, sizeof(sLock));
	sLock.l_type = iType;
	sLock.l_whence = SEEK_SET;
	sLock.l_start = 0;
	sLock.l_len = 0;  //To the end of the file, however large it grows.
	return fcntl(iFile, bWait ? F_OFD_SETLKW : F_OFD_SETLK, &sLock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void NamedSharedObjectName(CChars* psz, const char* szPrefix, char* szName)
{
	//POSIX shared memory names are a single '/' followed by no other '/'.
	psz->Init(szName);
	psz->Replace('/', '_');
	psz->Insert(0, szPrefix);
	psz->Insert(0, "/");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static size_t RoundUpToPage(size_t uiSize)
{
	return ((uiSize + NAMED_SHARED_OBJECT_PAGE_SIZE - 1) / NAMED_SHARED_OBJECT_PAGE_SIZE) * NAMED_SHARED_OBJECT_PAGE_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void UnlinkIfSameObject(int iFile, char* szPosixName)
{
	CChars		szPath;
	struct stat	sFileStat;
	struct stat	sPathStat;

	//Only unlink the name if it still refers to this object and not to a new
	//object created with the same name.
	szPath.Init(NAMED_SHARED_OBJECT_DIRECTORY)->Append(szPosixName);
	if ((fstat(iFile, &sFileStat) == 0) && (stat(szPath.Text(), &sPathStat) == 0))
	{
		if ((sFileStat.st_dev == sPathStat.st_dev) && (sFileStat.st_ino == sPathStat.st_ino))
		{
			shm_unlink(szPosixName);
		}
	}
	szPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool RemoveIfStale(int iFile, char* szPosixName)
{
	struct stat	sStat;
	bool		bStale;

	if (LockFile(iFile, F_WRLCK, false) != 0)
	{
		//Somebody holds it.
		return false;
	}

	//A creator holds its lock from before it sets the size so an object with no
	//size is still being created.
	bStale = (fstat(iFile, &sStat) == 0) && (sStat.st_size > 0);
	if (bStale)
	{
		UnlinkIfSameObject(iFile, szPosixName);
	}
	LockFile(iFile, F_UNLCK, false);
	return bStale;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CreateNamedSharedObject(char* szPosixName, size_t uiSize)
{
	int		iFile;
	int		iExisting;
	int		iErrno;
	bool	bStale;

	iFile = shm_open(szPosixName, O_RDWR | O_CREAT | O_EXCL, 0600);
	if ((iFile == -1) && (errno == EEXIST))
	{
		iExisting = shm_open(szPosixName, O_RDWR, 0600);
		if (iExisting == -1)
		{
			bStale = (errno == ENOENT);
		}
		else
		{
			bStale = RemoveIfStale(iExisting, szPosixName);
			close(iExisting);
		}

		if (!bStale)
		{
			errno = EEXIST;
			return -1;
		}
		iFile = shm_open(szPosixName, O_RDWR | O_CREAT | O_EXCL, 0600);
	}

	if (iFile == -1)
	{
		return -1;
	}

	if ((LockFile(iFile, F_WRLCK, true) != 0) || (ftruncate(iFile, RoundUpToPage(uiSize)) != 0))
	{
		iErrno = errno;
		shm_unlink(szPosixName);
		close(iFile);
		errno = iErrno;
		return -1;
	}
	return iFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PublishNamedSharedObject(int iFile)
{
	LockFile(iFile, F_RDLCK, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int OpenNamedSharedObject(char* szPosixName)
{
	int			iFile;
	struct stat	sStat;

	iFile = shm_open(szPosixName, O_RDWR, 0600);
	if (iFile == -1)
	{
		return -1;
	}

	if (RemoveIfStale(iFile, szPosixName))
	{
		close(iFile);
		errno = ENOENT;
		return -1;
	}

	//Waits while the object is being created.
	if (LockFile(iFile, F_RDLCK, true) != 0)
	{
		close(iFile);
		return -1;
	}

	//The last handle may have been closed before the lock was taken; the object
	//is then unlinked.  A creator that failed leaves it with no size.
	if ((fstat(iFile, &sStat) != 0) || (sStat.st_nlink == 0) || (sStat.st_size == 0))
	{
		close(iFile);
		errno = ENOENT;
		return -1;
	}
	return iFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CloseNamedSharedObject(int iFile, char* szPosixName)
{
	//Only the last handle can take the exclusive lock.  The shared lock is let go
	//first; otherwise two handles closing together would each be stopped by the
	//other's shared lock and neither would remove the name.
	LockFile(iFile, F_UNLCK, false);
	if (LockFile(iFile, F_WRLCK, false) == 0)
	{
		UnlinkIfSameObject(iFile, szPosixName);
	}
	close(iFile);
}


#endif // LINUX_GNU

