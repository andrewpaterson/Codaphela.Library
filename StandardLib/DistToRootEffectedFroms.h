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
	CArrayBaseObjectPtr		mapcUnattched;
	CArrayBaseObjectPtr		mapcLowestFroms;

public:
	void					Init(void);
	void					Kill(void);

	void					AddExpectedDist(CBaseObject* pcObject, int iExpectedDist);
	void					AddUnattached(CBaseObject* pcObject);
	SDistToRoot*			GetLowest(void);
	CBaseObject*			GetUnattached(void);
	int						NumExpectedDists(void);
	SDistToRoot*			GetExpectedDist(int iIndex);
	SDistToRoot*			GetExpectedDist(CBaseObject* pcObject);
	BOOL					ContainsUnattached(CBaseObject* pcObject);
	void					RemoveExpectedDist(int iIndex);
	void					RemoveExpectedDist(SDistToRoot* psDistToRoot);
	void					RemoveUnattached(CBaseObject* pcBaseObject);
	void					MarkExpectedDistLowestFroms(void);
	void					MarkUnattachedLowestFroms(void);
	CArrayBaseObjectPtr*	GetLowestFroms(void);
	void					AddChangedFromAsLowest(CBaseObject* pcFromChanged);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

