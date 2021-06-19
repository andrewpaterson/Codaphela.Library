#ifndef __WINDOWS_SHARED_MEMORY_H__
#define __WINDOWS_SHARED_MEMORY_H__
#include "WindowsSharedMemoryFile.h"


class CSharedMemory
{
protected:
	CWindowsSharedMemoryFile	mcMappedFile;
	SSharedMemoryDescriptor*	mpsDescriptor;
	void*						mpvMemory;

public:
	void	Init(char* szName);
	BOOL	Kill(void);

	char*	GetName(void);

	void*	Create(size_t uiSize);
	void*	Open(void);
	void	Close(void);
};


#endif // !__WINDOWS_SHARED_MEMORY_H__

