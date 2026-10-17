#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__
#include "SharedMemoryFile.h"


class CSharedMemory
{
protected:
	CSharedMemoryFile	mcMappedFile;
	SSharedMemoryDescriptor*	mpsDescriptor;
	void*						mpvMemory;

public:
	void	Init(char* szName);
	bool	Kill(void);

	char*	GetName(void);

	void*	Create(size_t uiSize);
	void*	Open(void);
	void	Close(void);
};


#endif // __SHARED_MEMORY_H__

