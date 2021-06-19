#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CObjects;
class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
CONSTRUCTABLE(CHollowEmbeddedObject);
public:
	void				InternalFree(void);
	BOOL				Save(CObjectWriter* pcFile);
	BOOL				Load(CObjectReader* pcFile);
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	int					GetDistToStack(void);
	int					GetDistToRoot(void);
	BOOL				SetDistToRoot(int iDistToRoot);
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
	uint16				GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(uint16 iIndex);
	BOOL				IsHollow(void);
	CBaseObject*		Dehollow(void);
	int					GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjectsThisIn(void);
	CStackPointers*		GetStackPointers(void);
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	int					NumPointerTos(void);
	void				ValidateConsistency(void);
	BOOL				IsInitialised(void);
	BOOL				IsDirty(void);
	void				SetDirty(BOOL bDirty);

protected:
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters);
};


#endif // !__HOLLOW_EMBEDDED_OBJECT_H__

