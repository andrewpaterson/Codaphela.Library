#ifndef __DIST_TO_STACK_CALCULATOR_H__
#define __DIST_TO_STACK_CALCULATOR_H__
#include "DistCalculatorParameters.h"


class CDistToStackCalculator
{
private:

public:
	void			Calculate(CBaseObject* pcFromChanged, CDistCalculatorParameters* pcParameters);
	void			CalculateFromTouched(CDistCalculatorParameters* pcParameters);

	int				CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters);
	void			InitialiseCompletelyDetached(CDistCalculatorParameters* pcParameters);
	void			ResetObjectsToUnknownDistToStack(CDistCalculatorParameters* pcParameters);
	int				UpdateDistToStackForObjectsWithFromStackDist(CDistCalculatorParameters* pcParameters);
	void			UpdateDistToStackForAllObjects(CDistCalculatorParameters* pcParameters);
	void			ResetObjectsToUnattachedDistToRoot(CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_STACK_CALCULATOR_H__

