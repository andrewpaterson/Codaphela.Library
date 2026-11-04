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

protected:
	size	CollectDetachedFromRoot(CDistCalculatorParameters* pcParameters);
	size	CollectDetachedFromRoot(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcFroms);
	bool	CollectDetached(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcFroms);
	void	InitialiseCompletelyDetached(CDistCalculatorParameters* pcParameters);
	void	MarkPointedToFromReached(CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcReached);
	bool	HasFromReachedFromStack(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);
	void	MarkReachedFromStack(CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_STACK_CALCULATOR_H__

