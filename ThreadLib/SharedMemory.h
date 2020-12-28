#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__
#include "BaseLib/Chars.h"
#include "BaseLib/WindowsHeaders.h"


#define SHARED_MEMORY_MAGIC	0xe749c0a8d3c51108


struct SSharedMemory
{
	unsigned long long int	uiMagic;
	unsigned long long int	uiSize;
	int						iConnectionId;
	char					szName[128];
};


class CSharedMemory
{
private:
	HANDLE			mhMapFile;
	SSharedMemory*	mpsDescriptor;
	void*			mpvMemory;
	CChars			mszName;
	int				miConnectionId;

public:
	void	Init(char* szName);
	void	Kill(void);

	BOOL	Create(size_t uiSize);
	BOOL	Connect(void);
	void	Close(void);
	void*	GetMemory(void);

protected:
	BOOL	Touch(size_t uiSize);
};


#endif // __SHARED_MEMORY_H__


