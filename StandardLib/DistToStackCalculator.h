#ifndef __DIST_TO_STACK_CALCULATOR_H__
#define __DIST_TO_STACK_CALCULATOR_H__
#include "DistDetachedFroms.h"


class CDistToStackCalculator
{
private:

public:
	void			Init(void);
	void			Kill(void);

	void			Calculate(CDistDetachedFroms* pcDetached);
	int				UpdateDistToStackForObjectsWithStackPointers(CDistDetachedFroms* pcDetached);
	void			InitialiseCompletelyDetached(CDistDetachedFroms* pcDetached);
	void			ResetObjectsToUnknownDistToStack(CDistDetachedFroms* pcDetached);
	int				UpdateDistToStackForObjectsWithFromStackDist(CDistDetachedFroms* pcDetached);
	void			UpdateDistToStackForAllObjects(CDistDetachedFroms* pcDetached);
};


#endif // __DIST_TO_STACK_CALCULATOR_H__

