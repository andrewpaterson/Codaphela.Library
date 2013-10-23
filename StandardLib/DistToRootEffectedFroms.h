#ifndef __DIST_TO_ROOT_EFFECTED_FROMS_H__
#define __DIST_TO_ROOT_EFFECTED_FROMS_H__
#include "BaseLib/ArrayTemplate.h"


class CBaseObject;


struct SDistToRoot
{
	CBaseObject*	pcObject;
	int				iExpectedDist;
};


typedef CArrayTemplate<SDistToRoot>		CArrayDistToRoot;


class CDistToRootEffectedFroms
{
private:
	CArrayDistToRoot	macExpectedDists;

public:
	void			Init(void);
	void			Kill(void);

	void			Add(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*	GetLowest(void);
	SDistToRoot*	GetSecondLowest(SDistToRoot* pcLowest);
	int				NumElements(void);
	SDistToRoot*	Get(int iIndex);
	SDistToRoot*	Get(CBaseObject* pcObject);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

