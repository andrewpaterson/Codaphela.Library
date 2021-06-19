#ifndef __WINDOWS_SHARED_MEMORY_FILE_H__
#define __WINDOWS_SHARED_MEMORY_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/WindowsHeaders.h"
#include "SharedMemoryDescriptor.h"
#include "SharedMemoryResult.h"


class CWindowsSharedMemoryFile
{
protected:
	HANDLE	mhMapFile;
	char	mszName[64];

public:
	void						Init(void);
	void						Init(char* szName);
	void						Reinit(void);
	void						Reinit(char* szName);
	BOOL						Kill(void);

	BOOL						IsNamed(void);
	char*						GetName(void);

	SSharedMemoryResult			Create(size_t uiSize);
	SSharedMemoryResult			Open(void);
	void						Close(void);

	SSharedMemoryDescriptor*	Map(size_t uiSize);
};


#endif // !__WINDOWS_SHARED_MEMORY_FILE_H__

