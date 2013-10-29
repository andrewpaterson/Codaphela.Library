#ifndef __DIST_TO_STACK_CALCULATOR_H__
#define __DIST_TO_STACK_CALCULATOR_H__
#include "DistCalculatorParameters.h"


class CDistToStackCalculator
{
private:

public:
	void			Init(void);
	void			Kill(void);

	void			Calculate(CDistCalculatorParameters* pcDetached);
	int				UpdateDistToStackForObjectsWithStackPointers(CDistCalculatorParameters* pcDetached);
	void			InitialiseCompletelyDetached(CDistCalculatorParameters* pcDetached);
	void			ResetObjectsToUnknownDistToStack(CDistCalculatorParameters* pcDetached);
	int				UpdateDistToStackForObjectsWithFromStackDist(CDistCalculatorParameters* pcDetached);
	void			UpdateDistToStackForAllObjects(CDistCalculatorParameters* pcDetached);
};


#endif // __DIST_TO_STACK_CALCULATOR_H__

