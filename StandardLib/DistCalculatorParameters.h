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
	CArrayTemplateEmbeddedBaseObjectPtr		mapcTouched;
	CArrayDistToRoot				macExpectedDists;
	CArrayTemplateEmbeddedBaseObjectPtr		mapcDetachedFromRoot;

	CArrayBlockObjectPtr				mapcCompletelyDetached;

public:
	void					Init(void);
	void					Kill(void);
	void					Dump(void);

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
	CBaseObject**			GetNextDetachedFromRoot(void);
	void					RemoveDetachedFromRoot(CBaseObject** ppcObject);

	void					AddCompletelyDetached(CBaseObject* pcObject);
	int						NumCompletelyDetached(void);
	CBaseObject*			GetCompletelyDetached(int iIndex);
	void					RemoveCompletelyDetached(int iIndex);
	void					CopyRootDetachedToCompletelyDetached(void);
	CArrayBlockObjectPtr*	GetCompletelyDetachedArray(void);

	void					AddTouched(CBaseObject* pcObject);
	int						NumTouched(void);
	CBaseObject*			GetTouched(int iIndex);

	void					ClearTouchedFlags(void);

	void					PrintArray(CChars* psz, CArrayTemplateEmbeddedBaseObjectPtr* pcArray);
	void					PrintArray(CChars* psz, CArrayBlockObjectPtr* pcArray);
	void					PrintArray(CChars* psz, CArrayDistToRoot* pcArray);
};


#endif // !__DIST_TO_ROOT_CALCULATOR_H__

