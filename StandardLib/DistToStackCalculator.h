#ifndef __DIST_TO_STACK_CALCULATOR_H__
#define __DIST_TO_STACK_CALCULATOR_H__
#include "DistCalculatorParameters.h"


class CDistToStackCalculator
{
private:

public:
	void			Init(void);
	void			Kill(void);

	void			CalculateFromTouched(CDistCalculatorParameters* pcDetached);
	int				CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters);
	void			InitialiseCompletelyDetached(CDistCalculatorParameters* pcDetached);
	void			ResetObjectsToUnknownDistToStack(CDistCalculatorParameters* pcDetached);
	int				UpdateDistToStackForObjectsWithFromStackDist(CDistCalculatorParameters* pcDetached);
	void			UpdateDistToStackForAllObjects(CDistCalculatorParameters* pcDetached);
	void			UpdateAttachedAndDetachedDistToRoot(CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_STACK_CALCULATOR_H__

