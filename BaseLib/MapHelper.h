#ifndef __MAP_HELPER_H__
#define __MAP_HELPER_H__
#include "PrimitiveTypes.h"
#include "Define.h"


#define MAX_KEY_SIZE ((size)(4 KB))


size	MinDataSize(size uiDataSize, size uiMaxDataSize);
bool	CopyString(char* pvDest, char* pvSource, size iSourceLength, size iMaxDestLength);
void	CopyData(void* pvDest, void* pvSource, size iSourceLength, size iMaxDestLength);

size	StrDataSize(char* pszData);
size	StrKeySize(char* pszKey);


#endif // __MAP_HELPER_H__

