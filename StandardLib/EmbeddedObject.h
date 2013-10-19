#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__
#include "BaseLib/ArrayEmbedded.h"
#include "BaseLib/Validation.h"
#include "CoreLib/IndexedGeneral.h"
#include "StackPointer.h"
#include "StackPointers.h"
#include "Unknown.h"


#define ROOT_DIST_TO_ROOT			 0
#define UNATTACHED_DIST_TO_ROOT		-1
#define CLEARED_DIST_TO_ROOT		-2
#define MAX_DIST_TO_ROOT			 MAX_INT


class CEmbeddedObject;
typedef CArrayTemplate<CEmbeddedObject*>	CArrayEmbeddedObjectPtr;


class CBaseObject;
typedef CArrayTemplate<CBaseObject*>		CArrayBaseObjectPtr;
typedef CArrayEmbedded<CBaseObject*, 32>	CArrayEmbeddedBaseObjectPtr;

class CPointer;
typedef CArrayEmbedded<CPointer*, 32>	CArrayPointerPtr;


class CObject;
class CObjects;
class CObjectSerialiser;
class CObjectDeserialiser;
class CEmbeddedObject : public CUnknown
{
friend class CPointer;
BASE_FUNCTIONS(CEmbeddedObject);
protected:
	CBaseObject*						mpcEmbedded;  //Object that 'this' is embedded in.
	CArrayEmbedded<CBaseObject*, 6>		mapHeapFroms;  //Objects on the heap that 'this' is pointed to from.  This is a BaseOject not an EmbeddedObject because HollowEmbeddedObjects cannot point to anything.
	CStackPointer*						mpcStackFroms;  //Objects on the stack that 'this' is pointed to from.  

public:
								CEmbeddedObject();
								~CEmbeddedObject();
	virtual void				KillDontFree(void) =0;
	virtual void				KillInternalData(void);
			void				KillFroms(void);
	virtual BOOL				Save(CObjectSerialiser* pcFile) =0;
	virtual BOOL				Load(CObjectDeserialiser* pcFile) =0;
	virtual BOOL				IsHollow(void) =0;
	virtual int					RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject);
	virtual void				SetDistToRootAndSetPointedTosExpectedDistToRoot(int iDistToRoot) =0;
	virtual int					GetDistToRoot(void) =0;
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
			BOOL				IsAllocatedInObjects(void);
	virtual void				ValidateConsistency(void) =0;
			void				ValidateFroms(void);
			void				ValidateBaseObjectDetail(void);

	virtual int					NumTos(void) =0;
	virtual int					UnsafeNumEmbeddedObjectTos(void);
	virtual void				GetTos(CArrayEmbeddedObjectPtr* papcTos) =0;
	virtual void				UnsafeGetEmbeddedObjectTos(CArrayEmbeddedObjectPtr* papcTos);

			BOOL				HasHeapPointers(void);
			void				AddHeapFrom(CBaseObject* pcFromObject);
			void				AddHeapFrom(CBaseObject* pcFromObject, BOOL bValidate);
			void				RemoveHeapFrom(CBaseObject* pcFromObject);
	virtual int					NumHeapFroms(void);
			CBaseObject*		GetHeapFrom(int iFromIndex);

			void				AddStackFrom(CPointer* pcPointer);
			void				AddStackFroms(CStackPointer* pcStackPointer);
			BOOL				HasStackPointers(void);
			void				RemoveStackFrom(CPointer* pcPointer);
			void				RemoveStackFromTryKill(CPointer* pcPointer, BOOL bKillIfNoRoot);
	virtual int					NumStackFroms(void);
			CStackPointer*		GetFirstStackFrom(void);

			int					NumTotalFroms(void);

	virtual CObjects*			GetObjects(void) =0;
			void				PrintObject(CChars* psz, BOOL bEmbedded = FALSE);
			void				LogNotExpectedToBeEmbedded(char* szMethod);
			void				ValidateNotEmbedded(char* szMethod);

protected:
	virtual void				TryKill(BOOL bKillIfNoRoot) =0;
	virtual void				RemoveAllHeapFroms(void);
	virtual void				RemoveAllStackFroms(void);
			BOOL				PrivateRemoveHeapFrom(CBaseObject* pcFrom);
	virtual void				GetStackFroms(CArrayPointerPtr* papcFroms);
	virtual void				GetHeapFroms(CArrayEmbeddedBaseObjectPtr* papcFroms);
	virtual CStackPointers*		GetStackPointers(void) =0;
	virtual CEmbeddedObject*	GetClosestFromToRoot(void);
};


#endif // __EMBEDDED_OBJECT__

