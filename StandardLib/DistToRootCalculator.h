#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistCalculatorParameters.h"


class CDistToRootCalculator
{
friend class CDistCalculator;
public:
	void	Calculate(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);

protected:
	void	CollectAndClearInvalidRootDistances(CBaseObject* pcFromChanged, CDistCalculatorParameters* pcParameters);
	void	UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters);
	void	ResetObjectsToUnattachedDistToRoot(CDistCalculatorParameters* pcParameters);
	void	ValidateExpectedDistStartingObjects(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

