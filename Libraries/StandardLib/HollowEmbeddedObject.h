#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CObjects;
class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
CONSTRUCTABLE(CHollowEmbeddedObject)
public:
	void				FreeInternal() override;
	void				DoneKill(bool bAllocatedInObjects) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	int					GetDistToRoot(void);
	bool				SetDistToRoot(int iDistToRoot) override;
	size				NumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(size iIndex);
	bool				IsHollow(void);
	CBaseObject*		Dehollow(void);
	size				GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjectsThisIn(void);
	void				ValidateConsistency(void);
	bool				IsInitialised(void);
	bool				IsDirty(void);
	void				SetDirty(bool bDirty);
	bool				IsRoot(void);
	bool				TestedForRoot(void);
	CClass*				GetClass(void);
	bool				HasNes(void) override;
	bool				HasNesPointerToThis(CEmbeddedObject* pcBaseObject) override;

protected:
};


#endif // __HOLLOW_EMBEDDED_OBJECT_H__

