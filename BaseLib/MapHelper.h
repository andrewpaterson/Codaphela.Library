#ifndef __MAP_HELPER_H__
#define __MAP_HELPER_H__
#include "PrimitiveTypes.h"
#include "Define.h"


//This is WAY too big.  Should be be, like, 64 bytes.
#ifdef __W65C816__
#define MAX_KEY_SIZE ((size)260)	//FAT32's longest path: keys are mostly 8 byte indexes, a few are names or paths.  Key buffers are often on the W65C816's stack, in bank 0.
#else
#define MAX_KEY_SIZE ((size)(4 KB))
#endif // __W65C816__


lsize	MinDataSize(lsize uiDataSize, lsize uiMaxDataSize);
bool	CopyString(char* pvDest, char* pvSource, size iSourceLength, size iMaxDestLength);
void	CopyData(void* pvDest, void* pvSource, size iSourceLength, size iMaxDestLength);

size	StrDataSize(const char* pszData);
size	StrKeySize(const char* pszKey);


#endif // __MAP_HELPER_H__

