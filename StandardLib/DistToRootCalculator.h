#ifndef __DIST_TO_ROOT_CALCULATOR_H__
#define __DIST_TO_ROOT_CALCULATOR_H__
#include "BaseLib/ArrayTemplate.h"


class CBaseObject;


struct SDistToRoot
{
	CBaseObject*	pcObject;
	int				iExpectedDist;
};


typedef CArrayTemplate<SDistToRoot>		CArrayDistToRoot;


class CDistToRootCalculator
{
public:
	CArrayDistToRoot	macExpectedDists;

	void			Init(void);
	void			Kill(void);

	void			Add(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*	GetLowest(void);
	int				NumElements(void);
	SDistToRoot*	Get(int iIndex);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

