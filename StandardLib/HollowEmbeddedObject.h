#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
BASE_FUNCTIONS(CHollowEmbeddedObject);
public:
	void				KillDontFree(void);
	BOOL				Save(CObjectSerialiser* pcFile);
	BOOL				Load(CObjectDeserialiser* pcFile);
	void				RemoveTo(CEmbeddedObject* pcTo);
	int					GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(int iIndex);
	BOOL				IsHollow(void);
	CBaseObject*		Dehollow(void);
	int					GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	void				AddFrom(CBaseObject* pcFrom);
	void				RemoveFrom(CBaseObject* mpcEmbedding);
	void				TryKill(void);
};


#endif // __HOLLOW_EMBEDDED_OBJECT_H__

