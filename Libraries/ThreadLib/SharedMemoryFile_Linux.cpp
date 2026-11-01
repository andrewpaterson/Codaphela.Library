#ifdef LINUX_GNU
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "NamedSharedObject_Linux.h"
#include "InterProcessName.h"
#include "SharedMemoryFile.h"


//Windows file mappings are whole pages.  A view can grow into the rest of the
//last page but MapViewOfFile fails past it; CResizableSharedMemory relies on
//both (TestSharedMemory expects resizes within a page to keep the same file).
//mmap succeeds past the end of the file and accessing it raises SIGBUS, so the
//file is whole pages (see CreateNamedSharedObject) and Map checks the size.
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
static SLinuxSharedMemoryFile& LinuxFile(uint8* paMapFile)
{
	return *(SLinuxSharedMemoryFile*)paMapFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Init(void)
{
	memset(this, 0, sizeof(CSharedMemoryFile));
	LinuxFile(maMapFile).iFile = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Init(char* szName)
{
	CChars	sz;

	memset(maMapFile, 0, SHARED_MEMORY_FILE_HANDLE_SIZE);
	LinuxFile(maMapFile).iFile = -1;
	ShortenInterProcessName(&sz, szName, SHARED_MEMORY_NAME_LENGTH);
	StrCpySafe(mszName, sz.Text(), SHARED_MEMORY_NAME_SIZE);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryFile::Kill(void)
{
	if (LinuxFile(maMapFile).iFile != -1)
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
	SLinuxSharedMemoryFile*		psFile;
	CChars						szName;
	int							iFile;
	int							iErrno;

	iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);
	psFile = &LinuxFile(maMapFile);

	if (psFile->iFile != -1)
	{
		gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  Map file handle must be NULL.", NULL);
		return SSharedMemoryResult(SMR_FileHandleAlreadySet);
	}

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MEMORY, mszName);
	iFile = CreateNamedSharedObject(szName.Text(), iAdjustedSize);
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

	psFile->iFile = iFile;
	psDescriptor = Map(0);
	if (psDescriptor == NULL)
	{
		iErrno = errno;
		CloseNamedSharedObject(iFile, szName.Text());
		psFile->iFile = -1;
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

	PublishNamedSharedObject(iFile);

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

	psFile = &LinuxFile(maMapFile);
	if (psFile->iFile != -1)
	{
		gcLogger.Error2(__METHOD__, " Could not open mapping [", mszName, "].  Map file handle must be NULL.", NULL);
		return SSharedMemoryResult(SMR_FileHandleAlreadySet);
	}

	NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MEMORY, mszName);
	iFile = OpenNamedSharedObject(szName.Text());
	szName.Kill();
	if (iFile == -1)
	{
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
	CChars						szName;

	psFile = &LinuxFile(maMapFile);
	if (psFile->iFile != -1)
	{
		NamedSharedObjectName(&szName, NAMED_SHARED_OBJECT_MEMORY, mszName);
		CloseNamedSharedObject(psFile->iFile, szName.Text());
		szName.Kill();
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

	psFile = &LinuxFile(maMapFile);
	if (psFile->iFile != -1)
	{
		iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);

		if (fstat(psFile->iFile, &sStat) == -1)
		{
			return NULL;
		}
		if (iAdjustedSize > (size_t)sStat.st_size)
		{
			errno = ENOMEM;
			return NULL;
		}

		pvMemory = mmap(NULL, iAdjustedSize, PROT_READ | PROT_WRITE, MAP_SHARED, psFile->iFile, 0);
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

	psFile = &LinuxFile(maMapFile);
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


#endif // LINUX_GNU

