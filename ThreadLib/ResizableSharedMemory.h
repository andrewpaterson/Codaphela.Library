#ifndef __RESIZABLE_SHARED_MEMORY_H__
#define __RESIZABLE_SHARED_MEMORY_H__
#include "BaseLib/PrimitiveTypes.h"
#include "SharedMemoryDescriptor.h"
#include "SharedMemoryCoordinator.h"
#include "SharedMemoryFile.h"


struct SSharedMemoryDescriptor;
class CResizableSharedMemory
{
private:
	CSharedMemoryCoordinator	mcCoordinator;
	SSharedMemoryFileName*	mpcCoordinatorMappedFile;

	CSharedMemoryFile		mcMappedFile;
	SSharedMemoryDescriptor*		mpsDescriptor;
	void*							mpvMemory;

	CChars							mszSharedName;
	CChars							mszDebugIdentifier;

	CSharedMemoryFile		mcHoldingMappedFile;
	SSharedMemoryDescriptor*		mpsHoldingDescriptor;

public:
	bool				Init(char* szMemoryName, char* szDebugIdentifier = NULL);
	void				Kill(void);

	SSharedMemoryMap	Touch(void);

	bool				Create(size_t uiSize);
	bool				Connect(void);
	int					Close(void);

	SSharedMemoryMap	IncreaseSize(size_t uiMore);
	SSharedMemoryMap	Resize(size_t uiSize);

	size_t				GetSize(void);

protected:
	bool	InitCoordinator(char* szMemoryName);
	bool	ReinitCoordinator(void);

	bool	Map(size_t uiSize);
	bool	Remap(size_t uiSize);

	void	HoldValid(void);
	void	TouchHolding(void);
};


#endif // __RESIZABLE_SHARED_MEMORY_H__

