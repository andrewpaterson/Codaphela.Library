#ifndef __WINDOWS_SHARED_MEMORY_FILE_H__
#define __WINDOWS_SHARED_MEMORY_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/WindowsHeaders.h"


enum ESharedMemoryResult
{
	SMR_Success,
	SMR_NotSet,
	SMR_CannotCreate,
	SMR_CannotMap,
	SMR_FileHandleAlreadySet,
	SMR_CannotOpen,
	SMR_Invalid
};


struct SSharedMemoryResult
{
	size_t					uiSize;
	ESharedMemoryResult		eResult;

			SSharedMemoryResult();
			SSharedMemoryResult(size_t uiSize);
			SSharedMemoryResult(ESharedMemoryResult eRsult);
	
	size_t	GetSize(void);
	BOOL	IsFailed(void);
	BOOL	IsSuccess(void);
	BOOL	IsInvalid(void);
};


class CWindowsSharedMemoryFile
{
protected:
	HANDLE	mhMapFile;
	char	mszName[64];

public:
	void				Init(void);
	void				Init(char* szName);
	void				Reinit(void);
	void				Reinit(char* szName);
	BOOL				Kill(void);

	BOOL				IsNamed(void);
	char*				GetName(void);

	SSharedMemoryResult	Create(size_t uiSize);
	SSharedMemoryResult	Open(void);
	void				Close(void);

	SSharedMemory*		Map(size_t uiSize);
};


#endif // __WINDOWS_SHARED_MEMORY_FILE_H__

