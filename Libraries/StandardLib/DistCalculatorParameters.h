#ifndef __DIST_CALCULATOR_PARAMETERS_H__
#define __DIST_CALCULATOR_PARAMETERS_H__
#include "BaseLib/Chars.h"
#include "ObjectPointerArrays.h"


class CBaseObject;


//What one distance calculation shares between the dist to root and the dist to stack calculators: its walk (the
//stamp states of the objects it reaches), the objects it touched and the objects found detached.  Its result is the
//completely detached objects: those that can be freed.
class CDistCalculatorParameters
{
private:
	uint32					muiWalk;  //This calculation's walk stamp base.
	CArrayBaseObjectPtrE32	mapcTouched;
	CArrayEmbeddedObjectPtr	mapcTos;  //One object's pointer tos at a time, its memory kept between them.
	CArrayBaseObjectPtrE32	mapcDetachedFromRoot;
	CArrayBaseObjectPtr		mapcCompletelyDetached;

public:
	void					Init(void);
	void					Kill(void);

	CArrayEmbeddedObjectPtr*	StartPointerTos(void);

	void					AddDetachedFromRoot(CBaseObject* pcObject);
	size					NumDetachedFromRoot(void);
	CBaseObject*			GetDetachedFromRoot(size iIndex);

	void					AddCompletelyDetached(CBaseObject* pcObject);
	size					NumCompletelyDetached(void);
	CBaseObject*			GetCompletelyDetached(size iIndex);
	void					RemoveCompletelyDetached(size iIndex);
	void					CopyRootDetachedToCompletelyDetached(void);
	CArrayBaseObjectPtr*	GetCompletelyDetachedArray(void);

	uint32					GetWalkState(CBaseObject* pcObject);
	void					SetWalkState(CBaseObject* pcObject, uint32 uiState);
	void					MarkReachedFromStack(CBaseObject* pcObject);
	bool					IsReachedFromStack(CBaseObject* pcObject);

	void					AddTouched(CBaseObject* pcObject);
	size					NumTouched(void);
	CBaseObject*			GetTouched(size iIndex);


	void					Print(CChars* psz, bool bCommaSeparate = true);
	void					Dump(void);

private:
	void					PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate);
	void					PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate);
};


#endif // __DIST_CALCULATOR_PARAMETERS_H__

