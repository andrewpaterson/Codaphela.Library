#ifndef __INDEXED_CACHE_RESULT_H__
#define __INDEXED_CACHE_RESULT_H__
#include "Define.h"


class CIndexedCacheResult
{
protected:
	void*	mpvCache;

public:
	void	Fail(void);
	void	Succeed(void* pvCache);
	bool	IsCached(void) const;
	void*	GetCache(void) const;
};


#endif // __INDEXED_CACHE_RESULT_H__


