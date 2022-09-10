#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CObjects;
class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
CONSTRUCTABLE(CHollowEmbeddedObject);
public:
	void				FreeInternal(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	int					GetDistToStack(void);
	int					GetDistToRoot(void);
	bool				SetDistToRoot(int iDistToRoot) override;
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
	uint16				GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(uint16 iIndex);
	bool				IsHollow(void);
	CBaseObject*		Dehollow(void);
	int					GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjectsThisIn(void);
	CStackPointers*		GetStackPointers(void);
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	int					NumPointerTos(void);
	void				ValidateConsistency(void);
	bool				IsInitialised(void);
	bool				IsDirty(void);
	void				SetDirty(bool bDirty);
	bool				IsRoot(void);
	bool				TestedForRoot(void);

protected:
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters);
};


#endif // !__HOLLOW_EMBEDDED_OBJECT_H__

