#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CObjects;
class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
BASE_FUNCTIONS(CHollowEmbeddedObject);
public:
	void				KillDontFree(void);
	BOOL				Save(CObjectSerialiser* pcFile);
	BOOL				Load(CObjectDeserialiser* pcFile);
	void				RemoveTo(CEmbeddedObject* pcTo);
	int					GetDistToRoot(void);
	void				SetDistToRootAndSetPointedTosExpectedDistToRoot(int iDistToRoot);
	int					GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(int iIndex);
	BOOL				IsHollow(void);
	CBaseObject*		Dehollow(void);
	int					GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjects(void);
	CStackPointers*		GetStackPointers(void);
	void				GetTos(CArrayEmbeddedObjectPtr* papcTos);
	int					NumTos(void);
	void				ValidateConsistency(void);
};


#endif // __HOLLOW_EMBEDDED_OBJECT_H__

