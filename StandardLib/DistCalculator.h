#ifndef __DIST_CALCULATOR_H__
#define __DIST_CALCULATOR_H__
#include "DistToRootCalculator.h"
#include "DistToStackCalculator.h"


class CDistCalculator
{
private:
	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;

	CDistDetachedFroms			cDetached;
	CDistToRootEffectedFroms	cEffectedFroms;

public:
	void Init(void);
	void Kill(void);

	void Calculate(CBaseObject* pcFromChanged);
};
#endif // __DIST_CALCULATOR_H__

