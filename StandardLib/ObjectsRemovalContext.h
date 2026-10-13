#ifndef __OBJECT_REMOVAL_CONTEXT_H__
#define __OBJECT_REMOVAL_CONTEXT_H__
#include "ObjectPointerArrays.h"


typedef CArrayTemplateEmbeddedPtr<CArrayBaseObjectPtr, 4>	CArrayArrayBaseObjectPtrE4;


class CObjectRemovalContext
{
protected:
	CArrayArrayBaseObjectPtrE4	maapcRemovedObjects;
	CArrayBaseObjectPtr			mapcRemainingObjects;
	CArrayBaseObjectPtr			mapcKilledObjects;

public:
	void					Init(void);
	void					Kill(void);

	CArrayBaseObjectPtr*	Start(CArrayBaseObjectPtr* papcKilled);
	bool					Done(void);

	void					Push(CArrayBaseObjectPtr* papcKilled);
	CArrayBaseObjectPtr*	Pull(void);

	bool					IsBusy(void);
};


#endif // __OBJECT_REMOVAL_CONTEXT_H__

