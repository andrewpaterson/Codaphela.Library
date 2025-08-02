/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __OBJECTS_H__
#define __OBJECTS_H__
#include "BaseLib/DataConnection.h"
#include "BaseLib/IndexedGeneral.h"
#include "NamedIndexedObjects.h"
#include "Unknowns.h"
#include "Null.h"
#include "ObjectsSource.h"
#include "Root.h"
#include "IndexGenerator.h"
#include "Pointer.h"
#include "Classes.h"
#include "DistCalculator.h"


#define ROOT_NAME	"GraphRoot"

#define CLEAR_MEMORY_CHUNK_SIZE		16384

Ptr<CRoot> ORoot(void);

template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> OMalloc(Args ... args);

template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMalloc(char* szObjectName, Args ... args);

template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMalloc(const char* szObjectName, Args ... args);


#define  LOG_OBJECT_ALLOCATION(pcObject) LogObjectAllocation(pcObject, __ENGINE_PRETTY_FUNCTION__)
#define  LOG_OBJECT_ALLOCATION_FAILURE(Class, Index, Name) LogObjectAllocationFailure(Class, Index, Name, __ENGINE_PRETTY_FUNCTION__)
#define  LOG_OBJECT_DESTRUCTION(pcObject) LogObjectDestruction(pcObject, __ENGINE_PRETTY_FUNCTION__)

typedef void (*AllocationCallback)(CBaseObject*);
typedef void (*HollocationCallback)(CBaseObject*);
typedef void (*DestructionCallback)(CBaseObject*);


class CHollowObject;
class CObjects
{
friend class CBaseObject;
friend class CInternalObjectDeserialiser;
protected:
	bool					mbInitialised;
	CUnknowns*				mpcUnknownsAllocatingFrom;

	CNamedIndexedObjects	mcMemory;			//Objects (BaseObject*) allocated in Unknowns referenced by name and OIndex.  
	CObjectsSource			mcSource;			//An object found on disk will be deserialised and then placed in memory.
	CClasses				mcClasses;

	CDataConnection*		mpcDataConnection;
	CSequenceConnection*	mpcSequenceConnection;

	CStackPointers*			mpcStackPointers;

	CDistCalculator			mcDistCalculator;

	AllocationCallback		mAllocationCallback;
	HollocationCallback		mHollocationCallback;
	DestructionCallback		mDestructionCallback;

public:
												CObjects();
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
						void					Kill(void);

	template<class M>	void					AddConstructor(void);
	template<class M>	CObjectSource*			AddSource(CAbstractFile* pcFile, char* szFilename);

						bool					Flush(void);
						bool					ForceSave(CBaseObject* pcObject);

						CPointer				Get(OIndex oi);
						CPointer				Get(char* szObjectName);
	template<class M> 	Ptr<M>					Get(OIndex oi);
	template<class M>	Ptr<M>					Get(char* szObjectName);
						bool					Contains(char* szObjectName);
						bool					Contains(OIndex oi);

	template<class M> 	Ptr<M>					Malloc(void);
	template<class M>	Ptr<M>					Malloc(char* szObjectName);

						Ptr<CRoot>				Root(void);

						bool					Remove(CArrayBlockObjectPtr* papcKilled);

						CPointer				Null(void);
	template<class M>	Ptr<M>					Null(void);

						size					NumMemoryIndexes(void);
						size					NumMemoryNames(void);
						int64					NumDatabaseIndexes(void);
						int64					NumDatabaseNames(void);

						CSequenceConnection*	GetIndexGenerator(void);
						CNamedIndexedObjects*	GetMemory(void);
						bool					EvictInMemory(void);

						CBaseObject*			Dehollow(OIndex oi);
						CBaseObject*			Dehollow(char* szObjectName);

						bool					ReplaceBaseObject(CBaseObject* pvExisting, CBaseObject* pvObject);
						bool					RemoveMemoryIdentifiers(CBaseObject* pvObject);

						CStackPointers*			GetStackPointers(void);
						CClasses*				GetClasses(void);

						OIndex					StartMemoryIteration(SIndexesIterator* psIter);
						OIndex					IterateMemory(SIndexesIterator* psIter);
						CPointer				TestGetFromMemory(OIndex oi);
						CPointer				TestGetFromMemory(char* szName);

						void					PrintMemoryNames(CChars* psz);
						void					DumpMemoryUseIteration(void);
						void					DumpMemoryUseRecursion(void);
						void					DumpNames(void);
						void					DumpGraph(void);
						void					DumpMemoryNames(void);

						void					ValidateEmpty(void);
						void					ValidateObjectsConsistency(void);

						void					SetAllocationCallback(AllocationCallback fAllocationCallback);
						void					SetHollocationCallback(HollocationCallback fHollocationCallback);
						void					SetDestructionCallback(DestructionCallback fDestructionCallback);

protected:
						Ptr<CRoot>				GetRoot(void);
						bool					HasRoot(void);

						bool					AddIntoMemory(CBaseObject* pvObject);
						bool					AddIntoMemoryWithIndex(CBaseObject* pvObject);
						bool					AddIntoMemoryWithNameAndIndex(CBaseObject* pvObject);
	template<class M> 	Ptr<M>					PointTo(M* pcObject);
	template<class M> 	Ptr<M>					PointToSetDirty(M* pcObject);
	template<class M>	M*						AllocateUninitialisedByTemplate(void);
	template<class M>	M*						AllocateUninitialisedByTemplate(const char* szObjectName);
	template<class M>	M*						AllocateUninitialisedByTemplate(OIndex oi, size uiAdditionalBytes);
	template<class M>	M*						AllocateUninitialisedByTemplate(const char* szObjectName, OIndex oi, size uiAdditionalBytes);
						CBaseObject*			AllocateUninitialisedByClassName(const char* szClassName, OIndex oi);
						CBaseObject*			AllocateUninitialisedByClassName(const char* szClassName, const char* szObjectName, OIndex oi);
						bool					ValidateCanAllocate(const char* szClassName);
						bool					ValidateCanAllocate(void);
						CBaseObject*			GetFromMemory(OIndex oi);
						CBaseObject*			GetFromMemory(char* szObjectName);
						CBaseObject*			GetFromMemory(char* szObjectName, OIndex oi);
						CBaseObject*			GetFromDatabase(OIndex oi);
						CBaseObject*			GetFromDatabase(char* szObjectName);
						CBaseObject*			GetFromSources(char* szObjectName);
						OIndex					GetNextIndex(void);
						
						void					KillDontFreeObjects(CArrayBlockObjectPtr* papcObjectPts);
						void					KillObjects(CArrayBlockObjectPtr* papcObjectPts);
						void					FreeObject(CBaseObject* pvObject);

						void					RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcObject, int iLevel, bool bEmbedded);
						void					ValidateSceneGraph(void);
						void					ValidateIndexedObjects(void);
						void					ClearValidationFlags(void);
						void					RecurseValidateSceneGraph(CBaseObject* pcBaseObject);
						CHollowObject*			AllocateHollow(size uiNumEmbedded, OIndex oi);
						CHollowObject*			AllocateHollow(size uiNumEmbedded, const char* szObjectName, OIndex oi);
						void					AppenedHollowEmbeddedObjects(CBaseObject* pcHollow, size uiNumEmbedded, void* pvEmbedded) ;
						void					PrintMemoryUseIteration(CChars* psz);
						void					PrintMemoryUseRecursion(CChars* psz);

public:
						CBaseObject*			AllocateUninitialisedByClassNameAndAddIntoMemory(char* szClassName);
						CBaseObject*			AllocateNamedUninitialisedByClassNameAndAddIntoMemory(char* szClassName, char* szObjectName);

						CBaseObject*			GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName(char* szClassName, char* szObjectName);  //This overwrites an existing object with a new object (with the same name).
						CBaseObject*			AllocateForInternalDeserialisationWithIndex(char* szClassName, OIndex oi);
						CHollowObject*			AllocateHollowWithIndex(OIndex oi, size uiNumEmbedded);
						CBaseObject*			GetNamedObjectInMemoryOrAllocateHollow(char* szObjectName, size uiNumEmbedded);
						CHollowObject*			AllocateHollowWithNameAndIndex(char* szObjectName, OIndex oi, size uiNumEmbedded);

protected:
						CBaseObject*			AllocateForInternalDeserialisationWithNameAndIndex(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting);
};


extern CObjects	gcObjects;
extern bool		gbObjects;

void ObjectsInit(void);
void ObjectsInit(CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
void ObjectsInit(CUnknowns* pcUnknowns, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
void ObjectsKill(void);
bool ObjectsValidate(void);
bool ObjectsFlush(void);
void ObjectsDump(void);

void LogObjectAllocation(CBaseObject* pcObject, char* szMethod);
void LogObjectDestruction(CBaseObject* pcObject, char* szMethod);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(OIndex oi, size uiAdditionalBytes)
{
	SpecificClass*	pcObject;
	bool			bResult;

	bResult = ValidateCanAllocate();
	if (!bResult)
	{
		return NULL;
	}

	pcObject = mpcUnknownsAllocatingFrom->Add<SpecificClass>(uiAdditionalBytes);
	if (pcObject)
	{
		pcObject->Allocate(this);
		pcObject->InitIdentifiers("", oi);
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(const char* szObjectName, OIndex oi, size uiAdditionalBytes)
{
	SpecificClass*	pcObject;
	bool			bResult;

	bResult = ValidateCanAllocate();
	if (!bResult)
	{
		return NULL;
	}

	pcObject = mpcUnknownsAllocatingFrom->Add<SpecificClass>(uiAdditionalBytes);
	if (pcObject)
	{
		pcObject->Allocate(this);
		pcObject->InitIdentifiers(szObjectName, oi);
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(void)
{
	OIndex	oi;

	oi = GetIndexGenerator()->GetNext();
	return AllocateUninitialisedByTemplate<SpecificClass>(oi, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(const char* szObjectName)
{
	OIndex	oi;

	oi = GetNextIndex();
	return AllocateUninitialisedByTemplate<SpecificClass>(szObjectName, oi, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Null(void)
{
	Ptr<SpecificClass> pObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::PointTo(SpecificClass* pcObject)
{
	Ptr<SpecificClass>	pObject;

	if (pcObject)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::PointToSetDirty(SpecificClass* pcObject)
{
	Ptr<SpecificClass>	pObject;

	if (pcObject)
	{
		pObject.AssignObject(pcObject);
		pcObject->SetDirty(true);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Malloc(void)
{
	bool			bResult;
	SpecificClass*	pcObject = AllocateUninitialisedByTemplate<SpecificClass>();
	if (pcObject)
	{
		bResult = AddIntoMemoryWithIndex(pcObject);
		if (bResult)
		{
			return PointToSetDirty(pcObject);
		}
		else
		{
			pcObject->KillInternal(false);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Malloc(char* szObjectName)
{
	bool			bResult;
	SpecificClass*	pcObject = AllocateUninitialisedByTemplate<SpecificClass>(szObjectName);
	if (pcObject)
	{
		bResult = AddIntoMemoryWithNameAndIndex(pcObject);
		if (bResult)
		{
			return PointToSetDirty(pcObject);
		}
		else
		{
			pcObject->KillInternal(false);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Get(OIndex oi)
{
	SpecificClass* pcObject = pcObject = (SpecificClass*)mcMemory.Get(oi);
	return PointTo<SpecificClass>(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Get(char* szObjectName)
{
	SpecificClass* pcObject = (SpecificClass*)mcMemory.Get(szObjectName);
	return PointTo<SpecificClass>(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
void CObjects::AddConstructor(void)
{
	SpecificClass*	pcClass;

	pcClass = mpcUnknownsAllocatingFrom->GetConstructor<SpecificClass>();
	if (!pcClass)
	{
		mpcUnknownsAllocatingFrom->AddConstructor<SpecificClass>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
CObjectSource* CObjects::AddSource(CAbstractFile* pcFile, char* szFilename)
{
	return mcSource.AddSource<SpecificClass>(pcFile, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> OMalloc(Args ... args)
{
	Ptr<SpecificClass> pObject = gcObjects.Malloc<SpecificClass>();
	if (pObject.IsNotNull())
	{
		pObject->Init(args...);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMalloc(char* szObjectName, Args ... args)
{
	Ptr<SpecificClass> pObject = gcObjects.Malloc<SpecificClass>(szObjectName);
	if (pObject.IsNotNull())
	{
		pObject->Init(args...);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMalloc(const char* szObjectName, Args ... args)
{
	Ptr<SpecificClass> pObject = gcObjects.Malloc<SpecificClass>((char*)szObjectName);
	if (pObject.IsNotNull())
	{
		pObject->Init(args...);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass>
Ptr<SpecificClass> OMallocNoI()
{
	return gcObjects.Malloc<SpecificClass>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass>
Ptr<SpecificClass> ONMallocNoI(char* szObjectName)
{
	return gcObjects.Malloc<SpecificClass>(szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMallocNoI(const char* szObjectName, Args ... args)
{
	return gcObjects.Malloc<SpecificClass>((char*)szObjectName);
}

#endif // __OBJECTS_H__

