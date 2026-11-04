#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistCalculatorParameters.h"


//Keeps the distances to the root exact (a breadth first search from the root) as heap froms change.  Objects only give
//access to their froms, tos, distances and flags; the walks are here.
class CDistToRootCalculator
{
public:
	void	Calculate(CBaseObject* pcFromRemoved, CDistCalculatorParameters* pcParameters);
	void	CollectAndClearInvalidDistToRoot(CBaseObject* pcFromRemoved, CDistCalculatorParameters* pcParameters);
	void	ResetClearedToUnattached(CDistCalculatorParameters* pcParameters);

	void	FromAdded(CBaseObject* pcObject, int iExpectedDistToRoot);

protected:
	void	ClearInvalidDistToRoot(CBaseObject* pcObject, int iClearedFromDistToRoot, CDistCalculatorParameters* pcParameters);
	bool	HasCloserFrom(CBaseObject* pcObject);

	void	UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters);
	void	UpdateAttachedTosDistToRoot(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);
	void	AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

