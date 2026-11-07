#ifndef __BASE_OBJECT_H__
#define __BASE_OBJECT_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/IndexedGeneral.h"
#include "EmbeddedObject.h"
#include "Class.h"


//Invalidated is set when the object on the file system is changed and must be reloaded.  This objects does not use it.
#define OBJECT_FLAGS_INVALIDATED				  0x04

//Dirty must be manually set when an object needs to be written from memory to indexed data.  Objects are - by default always dirty.
#define OBJECT_FLAGS_DIRTY						  0x08

//Set when an object's Free has been called: it is in the batch being freed.  The distance walks pass over it and
//TryFree leaves it.  An object that is killed should be removed from Memory, so one still there with this set is broken.
#define OBJECT_FLAGS_FREED						  0x10

//Debug flag marking whether or not an object has had it's graph dumped yet.
#define OBJECT_FLAGS_DUMPED						  0x20

//Set when an object was read from or written to the database.  A dirty stored object is saved by Flush and before it
//is freed even when it has no path to the root (it was loaded by name, its holders are only in the database).  Objects
//that never reach the database are freed without saving.
#define OBJECT_FLAGS_STORED						  0x40

// Tested for sanity is only valid whilst the scene graph is calling ValidateConsistency.  It stops the graph from walking already tested objects.
#define OBJECT_FLAGS_TESTED_FOR_SANITY			  0x80

//Object initialisation life-cycle
#define OBJECT_FLAGS_CALLED_CONSTRUCTOR			 0x100
#define OBJECT_FLAGS_CALLED_ALLOCATE			 0x200
#define OBJECT_FLAGS_CALLED_INIT				 0x400
#define OBJECT_FLAGS_CALLED_CLASS				 0x800
#define OBJECT_FLAGS_CALLED_KILL				 0x001

//Every flag but CALLED_CONSTRUCTOR and CALLED_CLASS is the embedding container's: an embedded object reads and sets
//those of the object it is embedded in (as it does its distance to root).  Constructing and classing are each object's own.
#define OBJECT_FLAGS_CONTAINER	((uint16)~(OBJECT_FLAGS_CALLED_CONSTRUCTOR | OBJECT_FLAGS_CALLED_CLASS))

//0x0002, 0x1000, 0x2000, 0x4000 and 0x8000 are free.  The distance calculations keep what they find in the
//walk stamp (WALK_STATE_...), not in flags.

#define MAX_NAMED_OBJECT_NAME_LENGTH	4096


//An object's walk stamp is the base of the walk that last reached it plus what that walk found (the low bits).  Each
//walk takes a new base (CObjects::StartWalk), so a stamp from an older walk reads as not reached and nothing needs
//clearing afterwards.
#define WALK_STAMP_STEP			8
#define WALK_STAMP_STATE_MASK	(WALK_STAMP_STEP - 1)

//What a distance calculation found an object to be (the low bits of its walk stamp); 0 is not reached by this walk.
#define WALK_STATE_TOUCHED				1  //Its distance to the root was cleared, or it was collected as detached.
#define WALK_STATE_DETACHED_WALKED		2  //Found detached from the root; what points to it was walked.
#define WALK_STATE_REACHED_FROM_STACK	3  //Detached from the root but reached from the stack (walked too).


//Walks the heap froms of an object and of the objects embedded in it (a hollow object's hollow embedded objects too),
//reading them where they are: a hub has thousands.  See CBaseObject::StartHeapFromIteration.
struct SHeapFromIterator
{
	CEmbeddedObject*	pcHolder;  //The object (or one embedded in it) whose froms are being walked.
	size				uiHolder;
	size				uiNumHolders;
	size				uiFrom;  //The next of pcHolder's froms.
	size				uiNumFroms;  //pcHolder's.
};


class CObjectReader;
class CObjectWriter;
class CObjects;
class CObjectRemapFrom;
class CBaseObject : public CEmbeddedObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CArrayCommonObject;
friend class CIndexObject;
friend class CMapObject;
friend class CObject;
friend class CObjects;
friend class CObjectRemapFrom;
friend class CEmbeddedObject;
friend class CObjectWriter;
friend class CObjectReader;
friend class CNamedIndexedObjects;

CONSTRUCTABLE(CBaseObject)
protected:
	OIndex				moi;
	CCharsImmutable		mon;
	CObjects*			mpcObjectsThisIn;
	CClass*				mpcClass;
	int					miDistToRoot;	//EMBEDDED_DIST_TO_ROOT in an embedded object: its distance is its embedding container's.
	uint32				muiWalkStamp;
	uint16				muiFlags;
	uint16				muiNumEmbedded;
	uint16				muiPreInits;
	uint16				muiPostInits;

public:
									CBaseObject();
									CBaseObject(const CBaseObject&) = delete;  //Copying would duplicate the pointer bookkeeping.  See CMapEntry(const CMapEntry&).
									~CBaseObject();

protected:
	virtual	void					Class(void) =0;

public:
			bool					InitIdentifiers(const char* szName, OIndex oi);
			void					Kill(void) override;
	virtual void					Free(void) =0;
	virtual bool					Flush(void);

			bool					Save(CObjectWriter* pcFile) override;
			bool					Load(CObjectReader* pcFile) override;

			OIndex					GetIndex(void) override;
			void					FreeIdentifiers(void) override;

			bool					IsRoot(void);
	virtual bool					IsSubRoot(void);
			bool					IsHollow(void);
	virtual bool					IsCollection(void) =0;
	virtual bool					IsObject(void) =0;
			bool					IsNamed(void) override;
			bool					IsInvalidated(void);
			bool					IsDirty(void);
			bool					IsStored(void);
			bool					IsInitialised(void);
			bool					HasClass(void);
			CClass*					GetClass(void);
			bool					IsKilled(void);
			bool					IsNamed(const char* szName);

			char*					GetName(void) override;
			lsize					SerialisedSize(void);
	virtual char*					PrintIdentifier(CChars* psz);
			void					PrintState(CChars* psz);
			void					PrintFlagsShorthand(CChars* psz);
			void					PrintFlags(CChars* psz);
			void					Print(CChars* psz) override;

			size					NumEmbedded(void);

			void					SetDirty(bool bDirty);
			void					SetStored(bool bStored);
			bool					Destroy(void);
			int						GetDistToRoot(void);
			bool					IsKnownHeldFromStack(void);
			void					SetKnownHeldFromStack(void);
			void					StackFromAdded(void);
			void					HeapFromAdded(CBaseObject* pcFrom);
			bool					IsOlderThan(CBaseObject* pcObject);
			uint32					GetWalkStamp(void);
			void					SetWalkStamp(uint32 uiWalkStamp);
	virtual bool					SetDistToRoot(int iDistToRoot) override;
			CBaseObject*			StartHeapFromIteration(SHeapFromIterator* psIter);
			CBaseObject*			IterateHeapFrom(SHeapFromIterator* psIter);
			void					GetHeapFroms(CArrayBaseObjectPtrE32* papcFroms) override;
			void					AddHeapFromContainers(CArrayBaseObjectPtr* papcFroms);


			bool					TestedForSanity(void);
			CObjects*				GetObjectsThisIn(void);
			CClasses*				GetClasses(void);
			CClass*					CompleteClass(CClasses* pcClasses);
			void					CopyFields(CEmbeddedObject* pcOther) override;

			bool					ContainsPointerTo(CEmbeddedObject* pcEmbedded);
			CEmbeddedObject* 		TestGetPointerTo(size iToIndex);
			size 					TestGetNumEmbeddedFromFlags(void);
			void					ClearFlagNumEmbedded(void);
			void					SetFlag(uint16 iFlag, uint16 iFlagValue);
			void					EmbedIn(CBaseObject* pcContainer);
			uint16					GetFlags(void);
			bool					CanFindRoot(void);
			CBaseObject*			GetClosestFromToRoot(void);
			CBaseObject*			GetClosestHeapFrom(uint32 uiSkipWalkStamp);
			bool					HasHeapFromCloserThan(int iDistToRoot);
			void					StopDestructor(void);

			bool					HasNes(void) override;
			void					GetNesPointerToThis(CArrayEmbeddedObjectPtrE8* papNesObjectsPointingToThis);
			bool					IsNesPointedTo(void);
			bool					HasNesPointerToThis(CEmbeddedObject* pcBaseObject) override;

			void					DumpFroms(void);
			void					DumpPointerTos(void);
	virtual void					Dump(void);
			void					DumpFlags(void);

			void					ValidateFlagSet(uint16 iFlag, const char* szFlag);
			void					ValidateFlagNotSet(uint16 iFlag, const char* szFlag);
			void					ValidateFlags(void);
			void					ValidateDistToRoot(void);
			void					ValidateKnownHeldFromStack(void);
			void					ValidateIndex(void);
			void					ValidateObjectsThisIn(void);
			void					ValidateCanFindRoot(void);
	virtual void					ValidateEmbeddedConsistency(void);
	virtual void					ValidateObjectIdentifiers(void);
			void					ValidateBaseObjectDetail(void);
			void					ValidateAllocation(void);
			bool					FailHasClassFlag(char* szMethod);
			bool					FailHasClassField(char* szMethod);
			void					ValidateInitCalled(void);
			void					ValidateKillCalled(void);
	virtual void					ValidateInternalConsistency(void);
	
protected:
			void					Allocate(CObjects* pcObjects) override;
			void					PreClass(void);
			void					SetClass(CClass* pcClass);

			void					PreInit(void);
			void					PostInit(void);
	virtual void					Initialised(void);
	virtual	void					PreFields(void);
	virtual	void					EmbedFields(void) =0;

			void					FreeInternal(void) override;
			void					DoneKill(bool bAllocatedInObjects) override;
			void					KillInternal(bool bHeapFromChanged, bool bValidateNotEmbedded) override;

	virtual bool					ClearPointerTos(bool bFree);
			bool					DestructOnStack(char* szMethod, bool bKill);

			bool					SaveManaged(CObjectWriter* pcFile);
			bool					LoadManaged(CObjectReader* pcFile);

			bool					SaveHeapFroms(CObjectWriter* pcFile);
			bool					LoadHeapFroms(CObjectReader* pcFile);

			bool					IsBaseObject(void);
			size					NumHeapFromHolders(void);
			uint16					GetNumEmbeddedFromFlags(void);
			void					SetFlagNumEmbedded(size iNumEmbedded);
			void					ContainerPreInit(void);
			void					ContainerPostInit(void);

			bool					SaveEmbeddedObjectsManaged(CObjectWriter* pcFile);
			bool					SavePointers(CObjectWriter* pcFile);
			bool					SavePrimitives(CObjectWriter* pcFile);
			bool					SaveUnmanaged(CObjectWriter* pcFile);
			bool					SaveUnmanagedField(CObjectWriter* pcFile, CUnmanagedField* pcField);
			bool					SaveEmbeddedObjectsHeapFroms(CObjectWriter* pcFile);
			bool					LoadEmbeddedObjectsManaged(CObjectReader* pcFile);
			bool					LoadPointers(CObjectReader* pcFile);
			bool					LoadPrimitives(CObjectReader* pcFile);
			bool					LoadUnmanaged(CObjectReader* pcFile);
			bool					LoadUnmanagedField(CObjectReader* pcFile, CUnmanagedField* pcField);
			bool					LoadEmbeddedObjectsHeapFroms(CObjectReader* pcFile);


			void					Nes(CPointerField* pcField);

			CPointerField*			Pointer(CPointer* pcPointer, const char* szFieldName);
			CEmbeddedObjectField*	Embedded(CBaseObject* pcObject, const char* szFieldName);

			CPrimitiveField*		Primitive(Int8* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(UInt8* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Int16* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(UInt16* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Int32* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(UInt32* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Int64* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(UInt64* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Bool* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Float32* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Float64* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Char8* pcPrimitive, const char* szFieldName);
			CPrimitiveField*		Primitive(Char16* pcPrimitive, const char* szFieldName);

			void					UnmanagedInt8(int8* pc, const char* szFieldName);
			void					UnmanagedInt8(int8* pac, size uiLength, const char* szFieldName);
			void					UnmanagedInt16(int16* pi, const char* szFieldName);
			void					UnmanagedInt16(int16* pai, size uiLength, const char* szFieldName);
			void					UnmanagedInt32(int32* pi, const char* szFieldName);
			void					UnmanagedInt32(int32* pai, size uiLength, const char* szFieldName);
			void					UnmanagedInt64(int64* pi, const char* szFieldName);
			void					UnmanagedInt64(int64* pai, size uiLength, const char* szFieldName);
			void					UnmanagedSInt(int* pi, const char* szFieldName);
			void					UnmanagedSInt(int* pai, size uiLength, const char* szFieldName);
			void					UnmanagedUInt8(uint8* pc, const char* szFieldName);
			void					UnmanagedUInt8(uint8* pac, size uiLength, const char* szFieldName);
			void					UnmanagedUInt16(uint16* pi, const char* szFieldName);
			void					UnmanagedUInt16(uint16* pai, size uiLength, const char* szFieldName);
			void					UnmanagedUInt32(uint32* pi, const char* szFieldName);
			void					UnmanagedUInt32(uint32* pai, size uiLength, const char* szFieldName);
			void					UnmanagedUInt64(uint64* pi, const char* szFieldName);
			void					UnmanagedUInt64(uint64* pai, size uiLength, const char* szFieldName);
			void					UnmanagedSUInt(uint* pi, const char* szFieldName);
			void					UnmanagedSUInt(uint* pai, size uiLength, const char* szFieldName);
			void					UnmanagedSize(size* pi, const char* szFieldName);
			void					UnmanagedSize(size* pai, size uiLength, const char* szFieldName);
			void					UnmanagedFloat(float32* pf, const char* szFieldName);
			void					UnmanagedFloat(float32* paf, size uiLength, const char* szFieldName);
			void					UnmanagedFloat(float64* pf, const char* szFieldName);
			void					UnmanagedFloat(float64* paf, size uiLength, const char* szFieldName);
			void					UnmanagedFloat(float128* pf, const char* szFieldName);
			void					UnmanagedFloat(float128* paf, size uiLength, const char* szFieldName);
			void					UnmanagedChar(char8* pc, const char* szFieldName);
			void					UnmanagedChar(char8* pac, size uiLength, const char* szFieldName);
			void					UnmanagedChar(char16* pc, const char* szFieldName);
			void					UnmanagedChar(char16* pac, size uiLength, const char* szFieldName);
			void					UnmanagedBool(bool* pb, const char* szFieldName);
			void					UnmanagedBool(bool* pab, size uiLength, const char* szFieldName);

			void					UnmanagedEnum(void* pe, const char* szFieldName);

			void					UnmanagedString(CChars* pcChars, const char* szFieldName);
			void					UnmanagedString(CCharsImmutable* pcChars, const char* szFieldName);
			void					UnmanagedNumber(CNumber* pcNumber, const char* szFieldName);
			void					UnmanagedDate(CDate* pcDate, const char* szFieldName);

			void					UnmanagedData(void* pv, size uiSizeof, const char* szFieldName);
			void					UnmanagedUnknown(void* pv, size uiSizeof, const char* szFieldName);
			void					UnmanagedPointer(void** pv, const char* szFieldName);

			void					Unmanaged2Float(SFloat32Vec2* pf, const char* szFieldName);
			void					Unmanaged2Float(SFloat32Vec2* pf, size uiLength, const char* szFieldName);
			void					Unmanaged3Float(SFloat32Vec3* pf, const char* szFieldName);
			void					Unmanaged3Float(SFloat32Vec3* pf, size uiLength, const char* szFieldName);
			void					Unmanaged4Float(SFloat32Vec4* pf, const char* szFieldName);
			void					Unmanaged4Float(SFloat32Vec4* pf, size uiLength, const char* szFieldName);
			void					Unmanaged4x4Float(SFloat4x4* pf, const char* szFieldName);
			void					Unmanaged4x4Float(SFloat4x4* pf, size uiLength, const char* szFieldName);

			void					Unmanaged2Int(SInt32Vec2* pf, const char* szFieldName);
			void					Unmanaged2Int(SInt32Vec2* pf, size uiLength, const char* szFieldName);
			void					Unmanaged3Int(SInt32Vec3* pf, const char* szFieldName);
			void					Unmanaged3Int(SInt32Vec3* pf, size uiLength, const char* szFieldName);
			void					Unmanaged4Int(SInt32Vec4* pf, const char* szFieldName);
			void					Unmanaged4Int(SInt32Vec4* pf, size uiLength, const char* szFieldName);

private:
			bool					ClipName(void);
};


#endif // __BASE_OBJECT_H__

