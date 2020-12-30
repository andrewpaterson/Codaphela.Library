#ifndef __WINDOWS_SHARED_MEMORY_H__
#define __WINDOWS_SHARED_MEMORY_H__
#include "WindowsSharedMemoryFile.h"


struct SSharedMemory;
class CSharedMemory
{
private:
	CWindowsSharedMemoryFile	mcMappedFiles[2];
	SSharedMemory*				mpsDescriptor;
	void*						mpvMemory;
	CChars						mszSharedName;
	CChars						mszDebugIdentifier;
	int							miSharedMemory;

public:
	void	Init(char* szName, char* szDebugIdentifier = NULL);
	void	Kill(void);

	void*	Touch(void);

	BOOL	Create(size_t uiSize);
	BOOL	Connect(void);
	int		Close(void);

	void*	IncreaseSize(void);
	void*	Resize(size_t uiSize);

	size_t	GetSize(void);

protected:
	BOOL	Connect(int iCurrentMemory, int iNextMemory);
	BOOL	Map(int iSharedMemory, size_t uiSize);
	BOOL	Remap(int iSharedMemory, size_t uiSize);
	BOOL	Create(int iSharedMemory, size_t uiSize);
	int		Close(SSharedMemory* psDescriptor, int iSharedMemory);
};


#endif // __WINDOWS_SHARED_MEMORY_H__


