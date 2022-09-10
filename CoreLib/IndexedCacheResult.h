#ifndef __INDEXED_CACHE_RESULT_H__
#define __INDEXED_CACHE_RESULT_H__
#include "BaseLib/Define.h"


class CIndexedCacheResult
{
protected:
	void*	mpvCache;

public:
	void	Fail(void);
	void	Succeed(void* pvCache);
	bool	IsCached(void);
	void*	GetCache(void);
};


#endif // !__INDEXED_CACHE_RESULT_H__


