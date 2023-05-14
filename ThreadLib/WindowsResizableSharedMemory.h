#ifndef __WINDOWS_RESIZABLE_SHARED_MEMORY_H__
#define __WINDOWS_RESIZABLE_SHARED_MEMORY_H__
#include "WindowsSharedMemoryCoordinator.h"
#include "WindowsSharedMemoryFile.h"


struct SSharedMemoryDescriptor;
class CResizableSharedMemory
{
private:
	CWindowsSharedMemoryCoordinator	mcCoordinator;
	SWindowsSharedMemoryFileName*	mpcCoordinatorMappedFile;

	CWindowsSharedMemoryFile		mcMappedFile;
	SSharedMemoryDescriptor*		mpsDescriptor;
	void*							mpvMemory;

	CChars							mszSharedName;
	CChars							mszDebugIdentifier;

	CWindowsSharedMemoryFile		mcHoldingMappedFile;
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


#endif // !__WINDOWS_RESIZABLE_SHARED_MEMORY_H__

