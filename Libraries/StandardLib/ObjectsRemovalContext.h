#ifndef __OBJECT_REMOVAL_CONTEXT_H__
#define __OBJECT_REMOVAL_CONTEXT_H__
#include "RemovalPointerContainer.h"


typedef CArrayTemplateEmbedded<CRemovalPointerContainer, 4>	CArrayRemovalPointerContainerE4;


class CObjectRemovalContext
{
protected:
	CArrayRemovalPointerContainerE4		maapcRemovedObjects;
	CArrayBaseObjectPtr					mapcRemainingObjects;
	CArrayBaseObjectPtr					mapcKilledObjects;

public:
	void					Init(void);
	void					Kill(void);

	CArrayBaseObjectPtr*	Start(CArrayBaseObjectPtr* papcKilled);
	CArrayBaseObjectPtr*	Done(void);

	void					Push(CArrayBaseObjectPtr* papcKilled);

	bool					IsBusy(void);

	void					Clear(void);
};


#endif // __OBJECT_REMOVAL_CONTEXT_H__

