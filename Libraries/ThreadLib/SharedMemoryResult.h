#ifndef __SHARED_MEMORY_RESULT_H__
#define __SHARED_MEMORY_RESULT_H__
#include "BaseLib/DataTypes.h"


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
	SSharedMemoryResult(ESharedMemoryResult eResult);

	size_t	GetSize(void) const;
	bool	IsFailed(void) const;
	bool	IsSuccess(void) const;
	bool	IsInvalid(void) const;
};


struct SSharedMemoryMap
{
	size_t	uiSize;
	void*	pvMemory;

	SSharedMemoryMap();
	SSharedMemoryMap(void* pvMemory, size_t uiSize);
};


#endif // __SHARED_MEMORY_RESULT_H__


