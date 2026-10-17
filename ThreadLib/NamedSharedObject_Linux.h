#ifndef __NAMED_SHARED_OBJECT_LINUX_H__
#define __NAMED_SHARED_OBJECT_LINUX_H__
#ifdef LINUX_GNU_32
#include <stddef.h>
#include "BaseLib/Chars.h"


//Named POSIX shared memory that is destroyed when its last handle is closed, as
//Windows named objects are.  Each handle holds a shared flock on its file
//descriptor and the kernel releases it when the process exits, even if it
//crashed, so a name that nobody holds is stale and is treated as not existing.

//The kinds of named shared object, passed to NamedSharedObjectName as szPrefix.
#define NAMED_SHARED_OBJECT_MUTEX		"Mutex."
#define NAMED_SHARED_OBJECT_MEMORY		"SharedMemory."

void	NamedSharedObjectName(CChars* psz, const char* szPrefix, char* szName);

//Returns a file descriptor for a new object of at least uiSize bytes, rounded up
//to whole pages, or -1 with errno set (EEXIST if a live object has the name).
//The object can not be opened until PublishNamedSharedObject is called.
int		CreateNamedSharedObject(char* szPosixName, size_t uiSize);
void	PublishNamedSharedObject(int iFile);

//Returns a file descriptor, or -1 if there is no live object with the name.
int		OpenNamedSharedObject(char* szPosixName);

void	CloseNamedSharedObject(int iFile, char* szPosixName);


#endif // LINUX_GNU_32
#endif // __NAMED_SHARED_OBJECT_LINUX_H__

