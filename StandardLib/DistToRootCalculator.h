#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistCalculatorParameters.h"


class CDistToRootCalculator
{
public:
	void Calculate(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);

	void CollectAndClearInvalidRootDistances(CBaseObject* pcFromChanged, CDistCalculatorParameters* pcParameters);
	void UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters);
	void ClearTouchedFlagsAndDetach(CDistCalculatorParameters* pcParameters);
	void ValidateExpectedDistStartingObjects(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

