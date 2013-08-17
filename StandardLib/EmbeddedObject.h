#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__
#include "BaseLib/ArrayEmbedded.h"
#include "CoreLib/IndexedGeneral.h"
#include "Unknown.h"


#define ROOT_DIST_TO_ROOT			 0
#define UNATTACHED_DIST_TO_ROOT		-1
#define CLEARED_DIST_TO_ROOT		-2


class CEmbeddedObject;
typedef CArrayTemplate<CEmbeddedObject*>	CArrayEmbeddedObjectPtr;


class CBaseObject;
typedef CArrayTemplate<CBaseObject*>		CArrayBaseObjectPtr;
typedef CArrayEmbedded<CBaseObject*, 32>	CArrayEmbeddedBaseObjectPtr;


class CObject;
class CObjects;
class CObjectSerialiser;
class CObjectDeserialiser;
class CEmbeddedObject : public CUnknown
{
BASE_FUNCTIONS(CEmbeddedObject);
protected:
	CBaseObject*						mpcEmbedded;  //Object that 'this' is embedded in.
	CArrayEmbedded<CBaseObject*, 6>		mapHeapFroms;  //Objects on the heap that 'this' is pointed from
	int									miStackFroms;  //Objects on the stack that 'this' is pointed from.  

public:
								CEmbeddedObject();
	virtual void				KillDontFree(void) =0;
			void				KillFroms(void);
	virtual BOOL				Save(CObjectSerialiser* pcFile) =0;
	virtual BOOL				Load(CObjectDeserialiser* pcFile) =0;
	virtual BOOL				IsHollow(void) =0;
	virtual void				RemoveFrom(CBaseObject* mpcEmbedding) =0;
	virtual int					RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject);
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

	virtual BOOL				HasStackPointers(void);
			void				CopyFroms(CEmbeddedObject* pcSource);
	virtual void				AddFrom(CBaseObject* pcFrom) =0;
	virtual int					NumHeapFroms(void);
	virtual int					NumStackFroms(void);
	virtual int					NumTotalFroms(void);
			void				AddStackFrom(void);
			void				RemoveStackFrom(void);
			CBaseObject*		PrivateGetFrom(int iFrom);
			CBaseObject*		TestGetFrom(int iFromIndex);
protected:
	virtual void			TryKill(void) =0;
			void			RemoveAllFroms(void);
			BOOL			PrivateRemoveFrom(CBaseObject* pcFrom);
			void			GetFroms(CArrayEmbeddedBaseObjectPtr* papcFroms);
	virtual void			RecurseGetFroms(CArrayEmbeddedBaseObjectPtr* papcFroms);
	virtual CObjects*		GetObjects(void) =0;
};


#endif // __EMBEDDED_OBJECT__

