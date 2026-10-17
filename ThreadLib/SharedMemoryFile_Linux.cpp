#ifdef LINUX_GNU_32
#include <atomic>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "SharedMemoryFile.h"


//A Windows file mapping is destroyed when its last handle is closed but a POSIX
//shared memory name stays until it is unlinked.  The first page of the shared
//memory counts the open handles so that the last Close can unlink the name.
//The SSharedMemoryDescriptor and the memory follow that page.
//
//Windows file mappings are whole pages.  A view can grow into the rest of the
//last page but MapViewOfFile fails past it; CResizableSharedMemory relies on
//both (TestSharedMemory expects resizes within a page to keep the same file).
//mmap succeeds past the end of the file and accessing it raises SIGBUS, so the
//file is whole pages and Map checks the size itself.
#define SHARED_MEMORY_HEADER_SIZE	4096
#define SHARED_MEMORY_PAGE_SIZE		4096


struct SLinuxSharedMemoryHeader
{
	std::atomic<int>	iHandles;
};


struct SLinuxSharedMemoryFile
{
	int							iFile;
	SSharedMemoryDescriptor*	psView;
	size_t						uiViewSize;
};


static_assert(sizeof(SLinuxSharedMemoryFile) <= SHARED_MEMORY_FILE_HANDLE_SIZE, "SHARED_MEMORY_FILE_HANDLE_SIZE is too small for SLinuxSharedMemoryFile.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SLinuxSharedMemoryFile* LinuxFile(uint8* paMapFile)
{
	return (SLinuxSharedMemoryFile*)paMapFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void LinuxSharedMemoryName(CChars* psz, char* szName)
{
	//POSIX shared memory names are a single '/' followed by no other '/'.
	psz->Init(szName);
	psz->Replace('/', '_');
	psz->Insert(0, "/SharedMemory.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static size_t RoundUpToPage(size_t uiSize)
{
	return ((uiSize + SHARED_MEMORY_PAGE_SIZE - 1) / SHARED_MEMORY_PAGE_SIZE) * SHARED_MEMORY_PAGE_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SLinuxSharedMemoryHeader* MapHeader(int iFile)
{
	void*	pvMemory;

	pvMemory = mmap(NULL, SHARED_MEMORY_HEADER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, iFile, 0);
	if (pvMemory == MAP_FAILED)
	{
		return NULL;
	}
	return (SLinuxSharedMemoryHeader*)pvMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void UnmapHeader(SLinuxSharedMemoryHeader* psHeader)
{
	munmap(psHeader, SHARED_MEMORY_HEADER_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool AddHandleIfOpen(int iFile)
{
	SLinuxSharedMemoryHeader*	psHeader;
	int							iHandles;
	bool						bAdded;

	psHeader = MapHeader(iFile);
	if (psHeader == NULL)
	{
		return false;
	}

	//A count of zero means the last handle was closed and the name is being unlinked.
	bAdded = false;
	iHandles = psHeader->iHandles.load();
	while (iHandles > 0)
	{
		if (psHeader->iHandles.compare_exchange_weak(iHandles, iHandles + 1))
		{
			bAdded = true;
			break;
		}
	}

	UnmapHeader(psHeader);
	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Init(void)
{
	memset(this, 0, sizeof(CSharedMemoryFile));
	LinuxFile(maMapFile)->iFile = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Init(char* szName)
{
	memset(maMapFile, 0, SHARED_MEMORY_FILE_HANDLE_SIZE);
	LinuxFile(maMapFile)->iFile = -1;
	StrCpySafe(mszName, szName, 64);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryFile::Kill(void)
{
	if (LinuxFile(maMapFile)->iFile != -1)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Kill shared memory file [", mszName, "].  It's handle has not been closed.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult CSharedMemoryFile::Create(size_t uiSize)
{
	size_t						iAdjustedSize;
	SSharedMemoryDescriptor*	psDescriptor;
	SLinuxSharedMemoryHeader*	psHeader;
	SLinuxSharedMemoryFile*		psFile;
	CChars						szName;
	int							iFile;
	int							iErrno;

	iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);
	psFile = LinuxFile(maMapFile);

	if (psFile->iFile != -1)
	{
		gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  Map file handle must be NULL.", NULL);
		return SSharedMemoryResult(SMR_FileHandleAlreadySet);
	}

	LinuxSharedMemoryName(&szName, mszName);
	iFile = shm_open(szName.Text(), O_RDWR | O_CREAT | O_EXCL, 0600);
	if (iFile == -1)
	{
		iErrno = errno;
		szName.Kill();
		if (iErrno == EEXIST)
		{
			gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  File already exists.", NULL);
			return SSharedMemoryResult(SMR_FileHandleAlreadySet);
		}
		gcLogger.Warning2(__METHOD__, " Could not create file mapping object [", mszName, "].  Failed with [", LinuxErrorCodeToString(iErrno), "].", NULL);
		return SSharedMemoryResult(SMR_CannotCreate);
	}

	if (ftruncate(iFile, SHARED_MEMORY_HEADER_SIZE + RoundUpToPage(iAdjustedSize)) == -1)
	{
		iErrno = errno;
		close(iFile);
		shm_unlink(szName.Text());
		szName.Kill();
		gcLogger.Warning2(__METHOD__, " Could not create file mapping object [", mszName, "].  Failed with [", LinuxErrorCodeToString(iErrno), "].", NULL);
		return SSharedMemoryResult(SMR_CannotCreate);
	}

	psHeader = MapHeader(iFile);
	psFile->iFile = iFile;
	psDescriptor = Map(0);
	if ((psDescriptor == NULL) || (psHeader == NULL))
	{
		iErrno = errno;
		if (psDescriptor)
		{
			Unmap(psDescriptor);
		}
		if (psHeader)
		{
			UnmapHeader(psHeader);
		}
		close(iFile);
		psFile->iFile = -1;
		shm_unlink(szName.Text());
		szName.Kill();
		gcLogger.Warning2(__METHOD__, " Could not map view of file  [", mszName, "].  Failed with [", LinuxErrorCodeToString(iErrno), "].", NULL);
		return SSharedMemoryResult(SMR_CannotMap);
	}
	szName.Kill();

	memset(psDescriptor, 0, sizeof(SSharedMemoryDescriptor));
	psDescriptor->uiMagic = SHARED_MEMORY_MAGIC;
	psDescriptor->uiSize = uiSize;
	psDescriptor->iInvalid = SHARED_MEMORY_VALID;
	psDescriptor->iMapCount = 0;
	strcpy(psDescriptor->szName, "Not Set");

	Unmap(psDescriptor);

	//Open fails until there is a handle so the descriptor is set before anyone can see it.
	psHeader->iHandles.store(1);
	UnmapHeader(psHeader);

	return SSharedMemoryResult(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult CSharedMemoryFile::Open(void)
{
	SSharedMemoryDescriptor*	psDescriptor;
	size_t						uiSize;
	SLinuxSharedMemoryFile*		psFile;
	CChars						szName;
	int							iFile;
	struct stat					sStat;

	psFile = LinuxFile(maMapFile);
	if (psFile->iFile != -1)
	{
		gcLogger.Error2(__METHOD__, " Could not open mapping [", mszName, "].  Map file handle must be NULL.", NULL);
		return SSharedMemoryResult(SMR_FileHandleAlreadySet);
	}

	LinuxSharedMemoryName(&szName, mszName);
	iFile = shm_open(szName.Text(), O_RDWR, 0600);
	szName.Kill();
	if (iFile == -1)
	{
		return SSharedMemoryResult(SMR_CannotOpen);
	}

	//A file that is still being created may not have its size yet.
	if ((fstat(iFile, &sStat) == -1) || ((size_t)sStat.st_size < SHARED_MEMORY_HEADER_SIZE + sizeof(SSharedMemoryDescriptor)))
	{
		close(iFile);
		return SSharedMemoryResult(SMR_CannotOpen);
	}

	if (!AddHandleIfOpen(iFile))
	{
		close(iFile);
		return SSharedMemoryResult(SMR_CannotOpen);
	}
	psFile->iFile = iFile;

	psDescriptor = Map(0);

	if (psDescriptor == NULL)
	{
		gcLogger.Warning2(__METHOD__, " Could not map view of file  [", mszName, "].  Failed with [", LinuxErrorCodeToString(errno), "].", NULL);
		Close();
		return SSharedMemoryResult(SMR_CannotMap);
	}

	if (psDescriptor->iInvalid == SHARED_MEMORY_INVALID)
	{
		Unmap(psDescriptor);
		Close();
		return SSharedMemoryResult(SMR_Invalid);
	}

	uiSize = (size_t)psDescriptor->uiSize;
	Unmap(psDescriptor);

	return SSharedMemoryResult(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Close(void)
{
	SLinuxSharedMemoryFile*		psFile;
	SLinuxSharedMemoryHeader*	psHeader;
	CChars						szName;

	psFile = LinuxFile(maMapFile);
	if (psFile->iFile != -1)
	{
		psHeader = MapHeader(psFile->iFile);
		if (psHeader)
		{
			if (psHeader->iHandles.fetch_sub(1) == 1)
			{
				LinuxSharedMemoryName(&szName, mszName);
				shm_unlink(szName.Text());
				szName.Kill();
			}
			UnmapHeader(psHeader);
		}

		close(psFile->iFile);
		psFile->iFile = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryDescriptor* CSharedMemoryFile::Map(size_t uiSize)
{
	SSharedMemoryDescriptor*	psDescriptor;
	size_t						iAdjustedSize;
	SLinuxSharedMemoryFile*		psFile;
	struct stat					sStat;
	void*						pvMemory;

	psFile = LinuxFile(maMapFile);
	if (psFile->iFile != -1)
	{
		iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);

		if (fstat(psFile->iFile, &sStat) == -1)
		{
			return NULL;
		}
		if (SHARED_MEMORY_HEADER_SIZE + iAdjustedSize > (size_t)sStat.st_size)
		{
			errno = ENOMEM;
			return NULL;
		}

		pvMemory = mmap(NULL, iAdjustedSize, PROT_READ | PROT_WRITE, MAP_SHARED, psFile->iFile, SHARED_MEMORY_HEADER_SIZE);
		if (pvMemory == MAP_FAILED)
		{
			return NULL;
		}

		//munmap needs the size of the view; this class only has one view at a time.
		psDescriptor = (SSharedMemoryDescriptor*)pvMemory;
		psFile->psView = psDescriptor;
		psFile->uiViewSize = iAdjustedSize;
		return psDescriptor;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Unmap(SSharedMemoryDescriptor* psDescriptor)
{
	SLinuxSharedMemoryFile*		psFile;

	psFile = LinuxFile(maMapFile);
	if ((psDescriptor == NULL) || (psDescriptor != psFile->psView))
	{
		gcLogger.Error2(__METHOD__, " Could not unmap view of file [", mszName, "].  It is not the current view.", NULL);
		return;
	}

	munmap(psDescriptor, psFile->uiViewSize);
	psFile->psView = NULL;
	psFile->uiViewSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSharedMemoryFile::GetLastErrorString(void)
{
	return LinuxErrorCodeToString(errno);
}


#endif // LINUX_GNU_32

