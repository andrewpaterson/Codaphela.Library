#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__
#include "BaseLib/Validation.h"
#include "CoreLib/IndexedGeneral.h"
#include "StackPointer.h"
#include "StackPointers.h"
#include "ObjectPointerArrays.h"
#include "DistToRootCalculator.h"
#include "DistToStackCalculator.h"
#include "Primitive.h"
#include "Unknown.h"


#define ROOT_DIST_TO_ROOT			 0
#define UNATTACHED_DIST_TO_ROOT		-1
#define CLEARED_DIST_TO_ROOT		-2
#define MAX_DIST_TO_ROOT			 MAX_INT

#define MIN_STACK_DIST_TO_STACK		 0
#define MIN_HEAP_DIST_TO_STACK		 1
#define UNKNOWN_DIST_TO_STACK		-2


class CObject;
class CObjects;
class CObjectWriter;
class CObjectReader;
class CEmbeddedObject : public CUnknown
{
friend class CPointer;
CONSTRUCTABLE(CEmbeddedObject);
protected:
	CBaseObject*								mpcEmbedded;  //Object that 'this' is embedded in.
	CArrayTemplateEmbedded<CBaseObject*, 6>		mapHeapFroms;  //Objects on the heap that 'this' is pointed to from.  This is a BaseOject not an EmbeddedObject because HollowEmbeddedObjects cannot point to anything.
	CStackPointer*								mpcStackFroms;  //Objects on the stack that 'this' is pointed to from.  

public:
								CEmbeddedObject();
								~CEmbeddedObject();

			void				Kill(void);
	virtual void				KillInternal(BOOL bHeapFromChanged);

	virtual void				Evict(void);

	virtual void				FreeInternal(void) =0;
	virtual void				FreePointers(void);
			void				PostRemapFroms(void);
	virtual BOOL				Save(CObjectWriter* pcFile) =0;
	virtual BOOL				Load(CObjectReader* pcFile) =0;
	virtual BOOL				IsHollow(void) =0;
	virtual int					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject);
	virtual BOOL				SetDistToRoot(int iDistToRoot) =0;
	virtual void				SetPointerTosExpectedDistToRoot(int iDistToRoot) =0;
	virtual BOOL				TestedForRoot(void) =0;
	virtual int					GetDistToRoot(void) =0;
	virtual int					GetDistToStack(void) =0;
	virtual OIndex				GetIndex(void);
	virtual BOOL				IsNamed(void);
	virtual char*				GetName(void);
	virtual void				ClearIdentifiers(void);
	virtual BOOL				IsDirty(void) =0;
	virtual void				SetDirty(BOOL bDirty) =0;
	virtual BOOL				IsBaseObject(void);
	virtual void				RemovePointerTo(CEmbeddedObject* pcTo) =0;
			CBaseObject*		GetEmbeddingContainer(void);
			BOOL				IsEmbedded(void);
			BOOL				IsNotEmbedded(void);
	virtual BOOL				IsInitialised(void) =0;
			BOOL				IsInStack(void);
	virtual BOOL				IsRoot(void) =0;
	virtual int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	virtual uint16				GetNumEmbedded(void) =0;
			void				SetEmbedding(CBaseObject* pcEmbedded);
	virtual CEmbeddedObject*	GetEmbeddedObject(uint16 iIndex) =0;
	virtual CBaseObject*		Dehollow(void) =0;
			BOOL				IsAllocatedInObjects(void);
	virtual void				ValidateConsistency(void) =0;
			void				ValidateFrom(CBaseObject* pcBaseObject);
			void				ValidateFroms(void);
			void				ValidatePointerTo(CEmbeddedObject* pcPointedTo);
	virtual void				ValidatePointerTos(void);

	virtual int					NumPointerTos(void) =0;
	virtual int					BaseNumPointerTos(void);
	virtual void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) =0;
	virtual void				BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	virtual void				CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	virtual int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters) =0;

			BOOL				HasHeapFroms(void);
			void				AddHeapFrom(CBaseObject* pcFromObject, BOOL bValidate);
			void				RemoveHeapFrom(CBaseObject* pcFromObject, BOOL bValidate);
	virtual int					NumHeapFroms(void);
			CBaseObject*		GetHeapFrom(int iFromIndex);
			void				UnsafeAddHeapFrom(CBaseObject* pcFromObject);

			void				AddStackFrom(CPointer* pcPointer);
			void				AddStackFroms(CStackPointer* pcStackPointer);
			BOOL				HasStackPointers(void);
			void				PrivateRemoveStackFrom(CPointer* pcPointer);
			void				RemoveStackFromTryKill(CPointer* pcPointer, BOOL bKillIfNoRoot);
	virtual int					NumStackFroms(void);
			CStackPointer*		GetFirstStackFrom(void);
	virtual CEmbeddedObject*	GetClosestFromToStack(void);

			int					NumTotalFroms(void);
			BOOL				ContainsFrom(CEmbeddedObject* pcBaseObject);
	virtual CEmbeddedObject*	GetClosestFromToRoot(void);

			CObjects*			GetObjects(void);
	virtual CObjects*			GetObjectsThisIn(void) =0;
			void				PrintObject(CChars* psz, BOOL bEmbedded = FALSE);
			void				LogNotExpectedToBeEmbedded(char* szMethod);
			void				LogExpectedToBeInitialised(char* szMethod);
			void				ValidateInitialised(char* szMethod);
			void				ValidateNotEmbedded(char* szMethod);
			BOOL				TestRemoveHeapFrom(CBaseObject* pcFromObject);
			void				TestRemoveStackFrom(CPointer* pcPointer);

protected:
	virtual void				RemoveAllHeapFroms(void);
	virtual void				RemoveAllStackFroms(void);
			BOOL				PrivateRemoveHeapFrom(CBaseObject* pcFrom);
	virtual void				GetStackFroms(CArrayTypedPointerPtr* papcFroms);
	virtual void				GetHeapFroms(CArrayTemplateEmbeddedBaseObjectPtr* papcFroms);
	virtual CStackPointers*		GetStackPointers(void) =0;
	virtual CEmbeddedObject*	GetClosestFromForCanFindRoot(void);
	virtual void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist) =0;
};


#endif // !__EMBEDDED_OBJECT__

