#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__
#include "CoreLib/IndexedGeneral.h"
#include "Unknown.h"


#define ROOT_DIST_TO_ROOT			 0
#define UNATTACHED_DIST_TO_ROOT		-1
#define CLEARED_DIST_TO_ROOT		-2


class CEmbeddedObject;
typedef CArrayTemplate<CEmbeddedObject*>	CArrayEmbeddedObjectPtr;


class CBaseObject;
class CObjectSerialiser;
class CObjectDeserialiser;
class CEmbeddedObject : public CUnknown
{
BASE_FUNCTIONS(CEmbeddedObject);
protected:
	CBaseObject*	mpcEmbedded;  //Object that 'this' is embedded in.

public:
								CEmbeddedObject();
	virtual BOOL				Save(CObjectSerialiser* pcFile) =0;
	virtual BOOL				Load(CObjectDeserialiser* pcFile) =0;
	virtual void				AddFrom(CBaseObject* pcFrom);
	virtual BOOL				IsHollow(void) =0;
	virtual void				RemoveFrom(CBaseObject* mpcEmbedding);
	virtual int					NumFroms(void);
	virtual CBaseObject*		GetFrom(int i);
	virtual int					RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject);
	virtual void				CopyFroms(CEmbeddedObject* pcOld);
	virtual void				SetDistToRoot(int iDistToRoot);
	virtual int					DistToRoot(void);
	virtual OIndex				GetOI(void);
	virtual BOOL				IsNamed(void);
	virtual char*				GetName(void);
	virtual BOOL				IsDirty(void);
	virtual void				ClearIndex(void);
	virtual BOOL				IsBaseObject(void);
	virtual void				RemoveTo(CEmbeddedObject* pcTo) =0;
			CBaseObject*		GetEmbeddingContainer(void);
			BOOL				IsEmbedded(void);
			BOOL				IsNotEmbedded(void);
	virtual int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	virtual int					GetNumEmbedded(void) =0;
			void				SetEmbedded(CBaseObject* pcEmbedded);
	virtual CEmbeddedObject*	GetEmbeddedObject(int iIndex) =0;
	virtual CBaseObject*		Dehollow(void) =0;
};


#endif // __EMBEDDED_OBJECT__

