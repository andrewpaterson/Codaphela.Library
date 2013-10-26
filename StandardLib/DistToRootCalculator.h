#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistToRootEffectedFroms.h"
#include "DistDetachedFroms.h"


class CDistToRootCalculator
{
private:
	CBaseObject*	mpcFromChanged;

public:
	void Init(void);
	void Kill(void);

	void AddFromChanged(CBaseObject* pcObject);
	void Calculate(void);
	void Calculate(CDistDetachedFroms* pcDetached);
	void Calculate(CDistToRootEffectedFroms* pcEffectedFroms, CDistDetachedFroms* pcDetached);
	void UpdateTosDistToRoot(CDistToRootEffectedFroms* pcEffectedFroms);
	void RemoveDetachedLowest(CDistToRootEffectedFroms* pcEffectedFroms);
	void ClearTosUpdatedTosFlags(CDistToRootEffectedFroms* pcEffectedFroms);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

