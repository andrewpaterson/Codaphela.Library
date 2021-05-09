#ifndef __MAP_HELPER_H__
#define __MAP_HELPER_H__
#include "Define.h"


#define MAX_KEY_SIZE (4 KB)


size_t	MinDataSize(size_t uiDataSize, size_t uiMaxDataSize);
BOOL	CopyString(char* pvDest, char* pvSource, size_t iSourceLength, size_t iMaxDestLength);
void	CopyData(char* pvDest, char* pvSource, size_t iSourceLength, size_t iMaxDestLength);


#endif // __MAP_HELPER_H__


