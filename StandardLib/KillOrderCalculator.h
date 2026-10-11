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
};


#endif // __KILL_ORDER_CALCULATOR_H__

