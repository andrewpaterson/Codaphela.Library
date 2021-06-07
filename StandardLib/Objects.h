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
#include "CoreLib/DataConnection.h"
#include "CoreLib/IndexedGeneral.h"
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
#define  LOG_OBJECT_DESTRUCTION(pcObject) LogObjectDestruction(pcObject, __ENGINE_PRETTY_FUNCTION__)


class CHollowObject;
class CNamedHollowObject;
class CObjects
{
friend class CBaseObject;
friend class CInternalObjectDeserialiser;
protected:
	BOOL					mbInitialised;
	CUnknowns*				mpcUnknownsAllocatingFrom;

	CNamedIndexedObjects	mcMemory;			//Objects (BaseObject*) allocated in Unknowns referenced by name and OIndex.  
	CObjectsSource			mcSource;			//An object found on disk will be deserialised and then placed in memory.
	CClasses				mcClasses;

	CDataConnection*		mpcDataConnection;
	CSequenceConnection*	mpcSequenceConnection;

	CStackPointers*			mpcStackPointers;

	CDistCalculator			mcDistCalculator;

public:
												CObjects();
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
						void					Kill(void);

	template<class M>	void					AddConstructor(void);
	template<class M>	CObjectSource*			AddSource(CAbstractFile* pcFile, char* szFileName);

						BOOL					Flush(void);
						BOOL					ForceSave(CBaseObject* pcObject);

						CPointer				Get(OIndex oi);
						CPointer				Get(char* szObjectName);
	template<class M> 	Ptr<M>					Get(OIndex oi);
	template<class M>	Ptr<M>					Get(char* szObjectName);
						BOOL					Contains(char* szObjectName);
						BOOL					Contains(OIndex oi);

	template<class M> 	Ptr<M>					Malloc(void);
	template<class M>	Ptr<M>					Malloc(char* szObjectName);

						Ptr<CRoot>				Root(void);

						BOOL					Remove(CArrayBlockObjectPtr* papcKilled);

						CPointer				Null(void);
	template<class M>	Ptr<M>					Null(void);

						int64					NumMemoryIndexes(void);
						int						NumMemoryNames(void);
						int64					NumDatabaseIndexes(void);
						int						NumDatabaseNames(void);

						CSequenceConnection*	GetIndexGenerator(void);
						CNamedIndexedObjects*	GetMemory(void);
						BOOL					EvictInMemory(void);

						CBaseObject*			Dehollow(OIndex oi);
						CBaseObject*			Dehollow(char* szObjectName);

						BOOL					Replace(CBaseObject* pvNewObject, char* szExistingName, OIndex oiNew);
						BOOL					Dename(CBaseObject* pvObject);
						BOOL					Deindex(CBaseObject* pvObject);

						CStackPointers*			GetStackPointers(void);
						CClasses*				GetClasses(void);

						OIndex					StartMemoryIteration(SIndexesIterator* psIter);
						OIndex					IterateMemory(SIndexesIterator* psIter);
						CPointer				TestGetFromMemory(OIndex oi);
						CPointer				TestGetFromMemory(char* szName);

						void					DumpIndex(void);
						void					DumpNames(void);
						void					DumpGraph(void);
						void					ValidateEmpty(void);
						void					ValidateObjectsConsistency(void);

protected:
						Ptr<CRoot>				GetRoot(void);
						BOOL					HasRoot(void);

						BOOL					AddObjectIntoMemoryWithIndex(CBaseObject* pvObject, OIndex oi);
						BOOL					AddObjectIntoMemoryWithIndexAndName(CBaseObject* pvObject, char* szObjectName, OIndex oi);
	template<class M> 	Ptr<M>					PointTo(M* pcObject);
	template<class M> 	Ptr<M>					PointToSetDirty(M* pcObject);
	template<class M>	M*						AllocateUninitialisedByTemplate(int iAdditionalBytes);
						CBaseObject*			AllocateUninitialisedByClassName(char* szClassName);
						BOOL					ValidateCanAllocate(char* szClassName);
						BOOL					ValidateCanAllocate(void);
						CBaseObject*			GetFromMemory(OIndex oi);
						CBaseObject*			GetFromMemory(char* szObjectName);
						CBaseObject*			GetFromDatabase(OIndex oi);
						CBaseObject*			GetFromDatabase(char* szObjectName);
						CBaseObject*			GetFromSources(char* szObjectName);
						OIndex					GetNextIndex(void);
						
						void					KillDontFreeObjects(CArrayBlockObjectPtr* papcObjectPts);
						void					FreeObjects(CArrayBlockObjectPtr* papcObjectPts);
						void					FreeObject(CBaseObject* pvObject);

						void					RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcObject, int iLevel, BOOL bEmbedded);
						void					ValidateSceneGraph(void);
						void					ValidateIndexedObjects(void);
						void					ClearValidationFlags(void);
						void					RecurseValidateSceneGraph(CBaseObject* pcBaseObject);
						CNamedHollowObject*		AllocateNamedHollow(uint16 iNumEmbedded);
						CHollowObject*			AllocateHollow(uint16 iNumEmbedded);
						void					AppenedHollowEmbeddedObjects(CBaseObject* pcHollow, uint16 iNumEmbedded, void* pvEmbedded) ;
						void					PrintMemory(CChars* psz);


protected:  //Above is protected also:
						template<class M>	M* AllocateNewByTemplate(void);
						template<class M>	M* AllocateNewByTemplate(char* szObjectName);
public:
						CBaseObject*			AllocateNew(char* szClassName);
						CBaseObject*			AllocateExistingNamed(char* szClassName, char* szObjectName);  //This mean overwrite an existing object with a new object (with the same name).
						CBaseObject*			AllocateForDeserialisation(char* szClassName, OIndex oiForced);
						CBaseObject*			AllocateNewNamed(char* szClassName, char* szObjectName);
						CBaseObject*			AllocateExistingHollow(OIndex oiForced, uint16 iNumEmbedded);
						CBaseObject*			AllocateExistingHollowFromMemoryOrMaybeANewNamedHollow(char* szObjectName, uint16 iNumEmbedded);
						CBaseObject*			AllocateExistingHollowFromMemoryOrMaybeANewNamedHollow(char* szObjectName, OIndex oiForced, uint16 iNumEmbedded);

protected:
						CBaseObject*			ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced);
						CBaseObject*			ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, OIndex oiForced);
						CBaseObject*			AllocateForExistingInDatabaseWithExplicitIdentifiers(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting);
};


extern CObjects	gcObjects;
extern BOOL		gbObjects;

void ObjectsInit(void);
void ObjectsInit(CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
void ObjectsInit(CUnknowns* pcUnknowns, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
void ObjectsKill(void);
BOOL ObjectsValidate(void);
BOOL ObjectsFlush(void);

void LogObjectAllocation(CBaseObject* pcObject, char* szMethod);
void LogObjectDestruction(CBaseObject* pcObject, char* szMethod);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(int iAdditionalBytes)
{
	SpecificClass*	pcObject;
	BOOL			bResult;

	bResult = ValidateCanAllocate();
	if (!bResult)
	{
		return NULL;
	}

	pcObject = mpcUnknownsAllocatingFrom->Add<SpecificClass>(iAdditionalBytes);
	if (pcObject)
	{
		pcObject->Allocate(this);
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateNewByTemplate(void)
{
	SpecificClass*	pvObject;
	OIndex			oi;
	BOOL			bResult;

	pvObject = AllocateUninitialisedByTemplate<SpecificClass>(0);
	if (!pvObject)
	{
		return pvObject;
	}

	oi = GetIndexGenerator()->GetNext();
	bResult = AddObjectIntoMemoryWithIndex(pvObject, oi);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
		return NULL;
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateNewByTemplate(char* szObjectName)
{
	SpecificClass*	pvObject;
	BOOL			bResult;
	OIndex			oi;

	pvObject = AllocateUninitialisedByTemplate<SpecificClass>(0);

	oi = GetNextIndex();
	bResult = AddObjectIntoMemoryWithIndexAndName(pvObject, szObjectName, oi);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
		return NULL;
	}

	return pvObject;
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
		pcObject->SetDirty(TRUE);
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
	SpecificClass* pcObject = AllocateNewByTemplate<SpecificClass>();
	return PointToSetDirty(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Malloc(char* szObjectName)
{
	SpecificClass* pcObject = AllocateNewByTemplate<SpecificClass>(szObjectName);
	return PointToSetDirty(pcObject);
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
	mpcUnknownsAllocatingFrom->AddConstructor<SpecificClass>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
CObjectSource* CObjects::AddSource(CAbstractFile* pcFile, char* szFileName)
{
	return mcSource.AddSource<SpecificClass>(pcFile, szFileName);
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


#endif // __OBJECTS_H__

