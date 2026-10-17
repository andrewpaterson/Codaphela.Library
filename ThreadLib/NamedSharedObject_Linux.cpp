#ifdef LINUX_GNU_32
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
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

	if (flock(iFile, LOCK_EX | LOCK_NB) != 0)
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
	flock(iFile, LOCK_UN);
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

	if ((flock(iFile, LOCK_EX) != 0) || (ftruncate(iFile, RoundUpToPage(uiSize)) != 0))
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
	flock(iFile, LOCK_SH);
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
	if (flock(iFile, LOCK_SH) != 0)
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
	//Only the last handle can take the exclusive lock.
	if (flock(iFile, LOCK_EX | LOCK_NB) == 0)
	{
		UnlinkIfSameObject(iFile, szPosixName);
	}
	close(iFile);
}


#endif // LINUX_GNU_32

