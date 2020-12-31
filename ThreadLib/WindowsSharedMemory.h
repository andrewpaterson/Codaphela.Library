#ifndef __WINDOWS_SHARED_MEMORY_H__
#define __WINDOWS_SHARED_MEMORY_H__
#include "WindowsSharedMemoryCoordinator.h"
#include "WindowsSharedMemoryFile.h"


struct SSharedMemory;
class CSharedMemory
{
private:
	CWindowsSharedMemoryCoordinator	mcCoordinator;
	SWindowsSharedMemoryFileName*	mpcCoordinatorMappedFile;

	CWindowsSharedMemoryFile		mcMappedFile;
	SSharedMemory*					mpsDescriptor;
	void*							mpvMemory;

	CChars							mszSharedName;
	CChars							mszDebugIdentifier;

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
};


#endif // __WINDOWS_SHARED_MEMORY_H__

