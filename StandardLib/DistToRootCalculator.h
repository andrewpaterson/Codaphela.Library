#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistCalculatorParameters.h"


class CDistToRootCalculator
{
private:
	CBaseObject*	mpcFromChanged;

public:
	void Init(void);
	void Kill(void);

	void AddFromChanged(CBaseObject* pcObject);
	void Calculate(void);
	void Calculate(CDistCalculatorParameters* pcParameters);
	void CollectStartingObjectsAndSetClearedToRoot(CDistCalculatorParameters* pcParameters);
	void UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters);
	void UpdateAttachedAndDetachedDistToRoot(CDistCalculatorParameters* pcParameters);
	void ClearTouchedFlagsAndDetach(CDistCalculatorParameters* pcParameters);
	void ValidateStartingObject(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

