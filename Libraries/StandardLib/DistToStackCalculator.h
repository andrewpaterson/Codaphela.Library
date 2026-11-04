#ifndef __DIST_TO_STACK_CALCULATOR_H__
#define __DIST_TO_STACK_CALCULATOR_H__
#include "DistCalculatorParameters.h"


//Finds which objects detached from the root can still be reached from the stack; the rest are completely detached
//and can be freed.  Objects only give access to their froms, tos, distances and flags; the walks are here.
class CDistToStackCalculator
{
public:
	void	Calculate(CBaseObject* pcFromRemoved, CDistCalculatorParameters* pcParameters);
	void	CalculateFromTouched(CDistCalculatorParameters* pcParameters);
	bool	IsHeldFromStackOrRoot(CBaseObject* pcObject);

	void	ResetObjectsToUnknownDistToStack(CDistCalculatorParameters* pcParameters);

protected:
	size	CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters);
	size	CollectDetachedAndSetDistToStackZero(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcFroms);
	bool	CollectDetached(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcFroms);
	void	InitialiseCompletelyDetached(CDistCalculatorParameters* pcParameters);
	void	UpdateDistToStackForObjectsWithFromStackDist(CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcUpdated);
	void	UpdateDistToStackForAllObjects(CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_STACK_CALCULATOR_H__

