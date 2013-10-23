#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "DistToRootEffectedFroms.h"


class CDistToRootCalculator
{
private:
	CBaseObject*	mpcFromChanged;

public:
	void Init(void);
	void Kill(void);

	void AddFromChanged(CBaseObject* pcObject);
	void Calculate(void);
	void Calculate(CDistToRootEffectedFroms* pcEffectedFroms);
	void UpdateTosDistToRoot(CDistToRootEffectedFroms* pcEffectedFroms);
	void ClearFlagsAndFindDetached(CDistToRootEffectedFroms* pcEffectedFroms);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

