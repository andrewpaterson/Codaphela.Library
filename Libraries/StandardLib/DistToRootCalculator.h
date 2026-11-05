#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistCalculatorParameters.h"
#include "DistToRootExpectedDists.h"


//An object to check while clearing distances to the root, and the distance the cleared object that points to it had.
struct SDistToRootClearCandidate
{
	CBaseObject*	pcObject;
	int				iClearedFromDistToRoot;
};

typedef CArrayLargeTemplate<SDistToRootClearCandidate>	CArrayDistToRootClearCandidates;  //Large: freeing a big graph queues one per object.


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
	void	ClearInvalidDistToRoot(CBaseObject* pcObject, int iClearedFromDistToRoot, CDistCalculatorParameters* pcParameters, CArrayDistToRootClearCandidates* pacCandidates);
	bool	HasCloserFrom(CBaseObject* pcObject);

	void	UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters, CDistToRootExpectedDists* pcExpectedDists);
	void	UpdateAttachedTosDistToRoot(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CDistToRootExpectedDists* pcExpectedDists);
	void	AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistToRootExpectedDists* pcExpectedDists);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

