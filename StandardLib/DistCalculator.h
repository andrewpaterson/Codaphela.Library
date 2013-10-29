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

	CArrayBaseObjectPtr*	Calculate(CBaseObject* pcFromChanged);
};


#endif // __DIST_CALCULATOR_H__

