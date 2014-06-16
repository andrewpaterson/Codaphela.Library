#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__
#include "BaseLib/Validation.h"
#include "CoreLib/IndexedGeneral.h"
#include "StackPointer.h"
#include "StackPointers.h"
#include "ObjectPointerArrays.h"
#include "DistToRootCalculator.h"
#include "DistToStackCalculator.h"
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

	virtual void				Kill(void);
	virtual void				Kill(BOOL bHeapFromChanged);
	virtual void				KillDontFree(void) =0;
	virtual void				KillInternalData(void);
			void				PostRemapFroms(void);
	virtual BOOL				Save(CObjectSerialiser* pcFile) =0;
	virtual BOOL				Load(CObjectDeserialiser* pcFile) =0;
	virtual BOOL				IsHollow(void) =0;
	virtual int					RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject);
	virtual BOOL				SetDistToRoot(int iDistToRoot) =0;
	virtual void				SetPointedTosExpectedDistToRoot(int iDistToRoot) =0;

	virtual int					GetDistToRoot(void) =0;
	virtual int					GetDistToStack(void) =0;
	virtual OIndex				GetOI(void);
	virtual BOOL				IsNamed(void);
	virtual char*				GetName(void);
	virtual BOOL				IsDirty(void);
	virtual void				ClearIndex(void);
	virtual void				ClearName(void);
	virtual BOOL				IsBaseObject(void);
	virtual void				RemoveTo(CEmbeddedObject* pcTo) =0;
			CBaseObject*		GetEmbeddingContainer(void);
			BOOL				IsEmbedded(void);
			BOOL				IsNotEmbedded(void);
	virtual BOOL				IsInitialised(void) =0;
			BOOL				IsInStack(void);
	virtual int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	virtual int					GetNumEmbedded(void) =0;
			void				SetEmbedded(CBaseObject* pcEmbedded);
	virtual CEmbeddedObject*	GetEmbeddedObject(int iIndex) =0;
	virtual CBaseObject*		Dehollow(void) =0;
			BOOL				IsAllocatedInObjects(void);
	virtual void				ValidateConsistency(void) =0;
			void				ValidateFrom(CBaseObject* pcBaseObject);
			void				ValidateFroms(void);
			void				ValidateTo(CEmbeddedObject* pcPointedTo);
	virtual void				ValidateTos(void);
			void				ValidateBaseObjectDetail(void);

	virtual int					NumTos(void) =0;
	virtual int					UnsafeNumEmbeddedObjectTos(void);
	virtual void				GetTos(CArrayEmbeddedObjectPtr* papcTos) =0;
	virtual void				UnsafeGetEmbeddedObjectTos(CArrayEmbeddedObjectPtr* papcTos);
	virtual void				CollectAndClearTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	virtual int					CollectEmbeddedObjectDetachedFroms(CDistCalculatorParameters* pcParameters) =0;

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
	virtual CBaseObject*		GetClosestFromToStack(void);

			int					NumTotalFroms(void);
			BOOL				ContainsFrom(CEmbeddedObject* pcBaseObject);
	virtual CBaseObject*		GetClosestFromToRoot(void);

	virtual CObjects*			GetObjects(void) =0;
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
	virtual void				GetStackFroms(CArrayPointerPtr* papcFroms);
	virtual void				GetHeapFroms(CArrayEmbeddedBaseObjectPtr* papcFroms);
	virtual CStackPointers*		GetStackPointers(void) =0;
	virtual CBaseObject*		GetClosestFromForCanFindRoot(void);
	virtual void				UpdateAttachedEmbeddedObjectTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist) =0;
};


#endif // __EMBEDDED_OBJECT__

