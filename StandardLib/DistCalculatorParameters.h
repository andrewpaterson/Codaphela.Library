#ifndef __DIST_TO_ROOT_EFFECTED_FROMS_H__
#define __DIST_TO_ROOT_EFFECTED_FROMS_H__
#include "BaseLib/Chars.h"
#include "ObjectPointerArrays.h"


class CBaseObject;


struct SDistToRoot
{
	CBaseObject*	pcObject;
	int				iExpectedDist;
};


typedef CArrayTemplate<SDistToRoot>		CArrayDistToRoot;


class CDistCalculatorParameters
{
private:
	CArrayBaseObjectPtrE32		mapcTouched;
	CArrayDistToRoot						macExpectedDists;
	CArrayBaseObjectPtrE32		mapcDetachedFromRoot;

	CArrayBaseObjectPtr					mapcCompletelyDetached;

public:
	void					Init(void);
	void					Kill(void);

	void					AddExpectedDist(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*			GetLowestExpectedDist(void);
	int						NumExpectedDists(void);
	SDistToRoot*			GetExpectedDist(int iIndex);
	SDistToRoot*			GetExpectedDist(CBaseObject* pcObject);
	void					RemoveExpectedDist(int iIndex);
	void					RemoveExpectedDist(SDistToRoot* psDistToRoot);


	void					AddDetachedFromRoot(CBaseObject* pcObject);
	int						NumDetachedFromRoot(void);
	CBaseObject*			GetDetachedFromRoot(int iIndex);

	void					AddCompletelyDetached(CBaseObject* pcObject);
	int						NumCompletelyDetached(void);
	CBaseObject*			GetCompletelyDetached(int iIndex);
	void					RemoveCompletelyDetached(int iIndex);
	void					CopyRootDetachedToCompletelyDetached(void);
	CArrayBaseObjectPtr*	GetCompletelyDetachedArray(void);

	void					AddTouched(CBaseObject* pcObject);
	int						NumTouched(void);
	CBaseObject*			GetTouched(int iIndex);

	void					ClearTouchedFlags(void);

	void					Print(CChars* psz, bool bCommaSeparate = true);
	void					Dump(void);

	void					PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate);
	void					PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate);
	void					PrintArray(CChars* psz, CArrayDistToRoot* pcArray, bool bCommaSeparate);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

