#ifndef __SHARED_MEMORY_COORDINATOR_H__
#define __SHARED_MEMORY_COORDINATOR_H__
#include "SharedMemoryFile.h"
#include "AtomicInt.h"


struct SSharedMemoryFileName
{
public:
	char	mszName[64];
	int		miIndex;

	void	Init(int iIndex, char* szName);
};


struct SResizableSharedMemoryFile : SSharedMemoryDescriptor
{
	int								iNumSharedMemoryFiles;
	int								iValidSharedMemoryIndex;
	CAtomicInt						miClients;
	SSharedMemoryFileName	macMemoryFiles[1]; //...
};


class CSharedMemoryCoordinator
{
protected:
	CSharedMemoryFile			mcMemory;
	SResizableSharedMemoryFile*	mpsDescriptor;
	
public:
	bool							Init(char* szCoordinatorMemoryName);
	void							Kill(void);

	SSharedMemoryFileName*	Add(char* szName);
	SSharedMemoryFileName*	GetCurrent(void);

	void							Invalidate(void);
};


#endif // __SHARED_MEMORY_COORDINATOR_H__

