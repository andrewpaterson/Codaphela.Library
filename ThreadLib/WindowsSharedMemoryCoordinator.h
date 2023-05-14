#ifndef __WINDOWS_SHARED_MEMORY_COORDINATOR_H__
#define __WINDOWS_SHARED_MEMORY_COORDINATOR_H__
#include "WindowsSharedMemoryFile.h"
#include <atomic>


struct SWindowsSharedMemoryFileName
{
public:
	char	mszName[64];
	int		miIndex;

	void	Init(int iIndex, char* szName);
};


struct SResizableWindowsSharedMemoryFile : SSharedMemoryDescriptor
{
	int								iNumSharedMemoryFiles;
	int								iValidSharedMemoryIndex;
	std::atomic<int>				miClients;
	SWindowsSharedMemoryFileName	macMemoryFiles[1]; //...
};


class CWindowsSharedMemoryCoordinator
{
protected:
	CWindowsSharedMemoryFile			mcMemory;
	SResizableWindowsSharedMemoryFile*	mpsDescriptor;
	
public:
	bool							Init(char* szCoordinatorMemoryName);
	void							Kill(void);

	SWindowsSharedMemoryFileName*	Add(char* szName);
	SWindowsSharedMemoryFileName*	GetCurrent(void);

	void							Invalidate(void);
};


#endif // !__WINDOWS_SHARED_MEMORY_COORDINATOR_H__

