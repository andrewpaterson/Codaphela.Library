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
	SSharedMemoryDescriptor*					mpsDescriptor;
	void*							mpvMemory;

	CChars							mszSharedName;
	CChars							mszDebugIdentifier;

	CWindowsSharedMemoryFile		mcHoldingMappedFile;
	SSharedMemoryDescriptor*			mpsHoldingDescriptor;

public:
	void	Init(char* szMemoryName, char* szDebugIdentifier = NULL);
	void	Kill(void);

	void*	Touch(void);

	BOOL	Create(size_t uiSize);
	BOOL	Connect(void);
	int		Close(void);

	void*	IncreaseSize(void);
	void*	Resize(size_t uiSize);

	size_t	GetSize(void);

protected:
	void	InitCoordinator(char* szMemoryName);
	BOOL	ReinitCoordinator(void);

	BOOL	Map(size_t uiSize);
	BOOL	Remap(size_t uiSize);

	void	HoldValid(void);
	void	TouchHolding(void);
};


#endif // __WINDOWS_RESIZABLE_SHARED_MEMORY_H__

