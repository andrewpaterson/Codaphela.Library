#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__
#include "BaseLib/WindowsHeaders.h"


#define SHARED_MEMORY_MAGIC	0xe749c0a8d3c51108


struct SSharedMemory
{
	unsigned long long int	uiMagic;
	unsigned long long int	uiSize;
	char					szName[128];
};


class CSharedMemory
{
private:
	HANDLE			hMapFile;
	SSharedMemory*	mpsDescriptor;
	void*			mpvMemory;

public:
	BOOL	Create(char* szName, size_t iSize);
	BOOL	Connect(char* szName, size_t iSize);
	void	Close(void);
	void*	GetMemory(void);
};


#endif // __SHARED_MEMORY_H__


