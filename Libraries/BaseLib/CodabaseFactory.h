#ifndef __CODABASE_FACTORY_H__
#define __CODABASE_FACTORY_H__
#include "Codabase.h"


//The caches of a Codabase that holds little, such as a CLayeredConnection's Removed Codabase.
#define CODABASE_SMALL_DATA_CACHE_SIZE		(256 KB)
#define CODABASE_SMALL_INDEX_CACHE_SIZE		(256 KB)
#define CODABASE_SMALL_NAMES_CACHE_SIZE		(64 KB)


class CCodabaseFactory
{
public:
	static CCodabase*	Create(const char* szDirectory, EIndexWriteThrough eWriteThrough, EDurableSync eSync);
	static CCodabase*	Create(const char* szDirectory, EIndexWriteThrough eWriteThrough, EDurableSync eSync, lsize uiDataCacheSize, lsize uiIndexCacheSize, lsize uiNamesCacheSize);
	static CCodabase*	CreateSmall(const char* szDirectory, EIndexWriteThrough eWriteThrough, EDurableSync eSync);
};


#endif // __CODABASE_FACTORY_H__

