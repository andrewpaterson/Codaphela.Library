#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CObjects;
class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
CONSTRUCTABLE(CHollowEmbeddedObject);
public:
	void				FreeInternal(bool bAllocatedInObjects) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	int					GetDistToStack(void);
	int					GetDistToRoot(void);
	bool				SetDistToRoot(int iDistToRoot) override;
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
	size				GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(size iIndex);
	bool				IsHollow(void);
	CBaseObject*		Dehollow(void);
	int					GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjectsThisIn(void);
	CStackPointers*		GetStackPointers(void);
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	size				NumPointerTos(void);
	void				ValidateConsistency(void);
	bool				IsInitialised(void);
	bool				IsDirty(void);
	void				SetDirty(bool bDirty);
	bool				IsRoot(void);
	bool				TestedForRoot(void);
	CClass*				GetClass(void);

protected:
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	size				CollectDetachedFroms(CDistCalculatorParameters* pcParameters);
};


#endif // __HOLLOW_EMBEDDED_OBJECT_H__

