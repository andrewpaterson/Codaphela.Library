#ifndef __INTER_PROCESS_NAME_H__
#define __INTER_PROCESS_NAME_H__
#include "BaseLib/Chars.h"


//Prefixes that keep the names of different kinds of named shared object apart.
#define INTER_PROCESS_MEMORY_PREFIX		"Local\\SharedMemory:"
#define INTER_PROCESS_MUTEX_PREFIX		"Local\\ProcessMutex:"
#define INTER_PROCESS_CHANNEL_PREFIX	"Local\\Channel:"
#define INTER_PROCESS_SIGNAL_PREFIX		"Local\\Signal:"


//Initialises psz to the prefix followed by szName.
void InterProcessName(CChars* psz, const char* szPrefix, char* szName);


#endif // __INTER_PROCESS_NAME_H__

