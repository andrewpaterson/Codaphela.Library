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
	BOOL				Save(CObjectSerialiser* pcFile);
	BOOL				Load(CObjectDeserialiser* pcFile);
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	int					GetDistToStack(void);
	int					GetDistToRoot(void);
	BOOL				SetDistToRoot(int iDistToRoot);
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
	unsigned short int	GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(unsigned short int iIndex);
	BOOL				IsHollow(void);
	CBaseObject*		Dehollow(void);
	int					GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjects(void);
	CStackPointers*		GetStackPointers(void);
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	int					NumPointerTos(void);
	void				ValidateConsistency(void);
	BOOL				IsInitialised(void);

protected:
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters);
};


#endif // __HOLLOW_EMBEDDED_OBJECT_H__

