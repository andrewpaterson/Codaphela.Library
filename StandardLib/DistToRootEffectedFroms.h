#ifndef __DIST_TO_ROOT_EFFECTED_FROMS_H__
#define __DIST_TO_ROOT_EFFECTED_FROMS_H__
#include "ObjectPointerArrays.h"


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
	CArrayDistToRoot		macExpectedDists;
	CArrayBaseObjectPtr		mapcLowestFroms;

public:
	void					Init(void);
	void					Kill(void);

	void					Add(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*			GetLowest(void);
	int						NumElements(void);
	SDistToRoot*			Get(int iIndex);
	SDistToRoot*			Get(CBaseObject* pcObject);
	void					Remove(int iIndex);
	void					Remove(SDistToRoot* psDistToRoot);
	void					MarkLowestFroms(void);
	CArrayBaseObjectPtr*	GetLowestFroms(void);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

