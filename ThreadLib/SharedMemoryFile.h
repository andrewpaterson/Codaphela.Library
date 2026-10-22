#ifndef __SHARED_MEMORY_FILE_H__
#define __SHARED_MEMORY_FILE_H__
#include "BaseLib/Chars.h"
#include "SharedMemoryDescriptor.h"
#include "SharedMemoryResult.h"


//Large enough to hold a Windows file mapping HANDLE or the Linux shared memory
//file descriptor and its current view.
#define SHARED_MEMORY_FILE_HANDLE_SIZE	24

//The longest name a shared memory file has, and the size of the buffers that
//hold one with its terminator (some are in shared memory).  A longer name is
//shortened with ShortenInterProcessName.
#define SHARED_MEMORY_NAME_LENGTH		63
#define SHARED_MEMORY_NAME_SIZE			(SHARED_MEMORY_NAME_LENGTH + 1)
static_assert(sizeof(((SSharedMemoryDescriptor*)0)->szName) == SHARED_MEMORY_NAME_SIZE, "SSharedMemoryDescriptor::szName must hold a shared memory name.");


class CSharedMemoryFile
{
protected:
	alignas(8) uint8	maMapFile[SHARED_MEMORY_FILE_HANDLE_SIZE];
	char	mszName[SHARED_MEMORY_NAME_SIZE];

public:
	void						Init(void);
	void						Init(char* szName);
	void						Reinit(void);
	void						Reinit(char* szName);
	bool						Kill(void);

	bool						IsNamed(void);
	char*						GetName(void);

	SSharedMemoryResult			Create(size_t uiSize);
	SSharedMemoryResult			Open(void);
	void						Close(void);

	SSharedMemoryDescriptor*	Map(size_t uiSize);
	void						Unmap(SSharedMemoryDescriptor* psDescriptor);

	char*						GetLastErrorString(void);
};


#endif // __SHARED_MEMORY_FILE_H__

