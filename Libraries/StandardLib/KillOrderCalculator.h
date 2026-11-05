#ifndef __KILL_ORDER_CALCULATOR_H__
#define __KILL_ORDER_CALCULATOR_H__
#include "ObjectPointerArrays.h"


class CKillOrderCalculator
{
protected:
	CArrayBaseObjectPtr*	mpapcKilled;
	CArrayBaseObjectPtr		mapcKilledNesObjects;

public:
	void					Init(CArrayBaseObjectPtr* papcKilled);
	void					Kill(void);

	CArrayBaseObjectPtr*	Calculate(void);

protected:
	void					OrderNesObjects(void);
	bool					IsNesPointedToFromWaiting(CBaseObject* pcObject, uint32 uiWaiting);
};


#endif // __KILL_ORDER_CALCULATOR_H__

