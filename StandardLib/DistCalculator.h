#ifndef __DIST_CALCULATOR_H__
#define __DIST_CALCULATOR_H__
#include "DistToRootCalculator.h"
#include "DistToStackCalculator.h"


class CDistCalculator
{
private:
	CDistToRootCalculator		mcDistToRootCalculator;
	CDistToStackCalculator		mcDistToStackCalculator;
	
	CDistCalculatorParameters	mcParameters;

public:
	void					Init(void);
	void					Kill(void);

	CArrayBlockObjectPtr*	Calculate(CBaseObject* pcFromChanged, bool bHeapFromChanged);
	CArrayBlockObjectPtr*	CalculateHeapFromChanged(CBaseObject* pcFromChanged);
	CArrayBlockObjectPtr*	CalculateStackFromChanged(CBaseObject* pcFromChanged);
	void					ClearTouchedFlags(CDistCalculatorParameters* pcParameters);
	void					ChangeClearedDistToUnattachedDist(CDistCalculatorParameters* pcParameters);
};


#endif // !__DIST_CALCULATOR_H__

