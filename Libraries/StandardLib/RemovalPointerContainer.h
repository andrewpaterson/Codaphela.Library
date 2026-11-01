#ifndef __REMOVAL_POINTER_CONTAINER_H__
#define __REMOVAL_POINTER_CONTAINER_H__
#include "ObjectPointerArrays.h"


class CRemovalPointerContainer
{
protected:
	CArrayBaseObjectPtr		mapcKilled;
	CArrayBaseObjectPtr*	mpapcKilled;

public:
	void					Init(CArrayBaseObjectPtr* papcKilled, bool bFreeArray);
	void					Kill(void);

	CArrayBaseObjectPtr*	GetKilled(void);
};


#endif // __OBJECT_REMOVAL_CONTEXT_H__

