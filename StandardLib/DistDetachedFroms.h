#ifndef __DIST_DETACHED_FROMS_H__
#define __DIST_DETACHED_FROMS_H__
#include "ObjectPointerArrays.h"


class CDistDetachedFroms
{
public:
	CArrayBaseObjectPtr		apcDetached;

	void Init(void);
	void Kill(void);
	void Add(CBaseObject* pcObject);
};


#endif // __DIST_DETACHED_FROMS_H__
