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
#include "CoreLib/IndexedGeneral.h"
#include "NamedIndexedData.h"
#include "NamedIndexedObjects.h"
#include "Unknowns.h"
#include "Null.h"
#include "ObjectsSource.h"
#include "Root.h"
#include "IndexGenerator.h"
#include "Pointer.h"
#include "DistCalculator.h"


#define ROOT_NAME	"GraphRoot"

#define CLEAR_MEMORY_CHUNK_SIZE		16384

#define OMalloc(classtype)			(gcObjects.Add<classtype>())
#define ONMalloc(classtype, name)	(gcObjects.Add<classtype>(name))
#define ORoot()						(gcObjects.AddRoot())
#define ONull						(Null())

//#define ONRef(name)  gcObjects.AllocateNamedHollow();  //TODO: Sometimes you need to refer to an object that is already in the database without allocating a new one.

#define  LOG_OBJECT_ALLOCATION(pcObject) LogObjectAllocation(pcObject, __ENGINE_PRETTY_FUNCTION__)
#define  LOG_OBJECT_DESTRUCTION(pcObject) LogObjectDestruction(pcObject, __ENGINE_PRETTY_FUNCTION__)

class CHollowObject;
class CNamedHollowObject;
class CObjects
{
friend class CBaseObject;
friend class CObjectAllocator;
protected:
	BOOL					mbInitialised;
	CUnknowns*				mpcUnknownsAllocatingFrom;

	CNamedIndexedObjects	mcMemory;  //Objects (BaseObject*) allocated in Unknowns referenced by name and OIndex.  
	CNamedIndexedData		mcDatabase;  //Objects in the database also referenced by string and OIndex.  
	
	CObjectsSource			mcSource;  //An object found on disk will be 'cooked' converter and then placed in memory.

	CIndexGenerator			mcIndexGenerator;
	BOOL					mbDatabase;

	CStackPointers*			mpcStackPointers;

	CDistCalculator			mcDistCalculator;

public:
												CObjects();
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, char* szWorkingDirectory);
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, CIndexedConfig* pcConfig);
						void					Kill(void);
						void					DumpIndex(void);
						void					DumpNames(void);
						void					DumpGraph(void);
						void					ValidateEmpty(void);
						void					ValidateObjectsConsistency(void);

	template<class M>	void					AddConstructor(void);
	template<class M>	CObjectSource*			AddSource(CAbstractFile* pcFile, char* szFileName);

						BOOL					Flush(BOOL bClearMemory, BOOL bClearCache);
						BOOL					Save(CBaseObject* pcObject);
						BOOL					ForceSave(CBaseObject* pcObject);
						BOOL					Close(void);

						CPointer				Get(OIndex oi);
						CPointer				Get(char* szObjectName);
	template<class M> 	Ptr<M>					Get(OIndex oi);
	template<class M>	Ptr<M>					Get(char* szObjectName);
						BOOL					Contains(char* szObjectName);

	template<class M>	Ptr<M>					Add(void);
	template<class M>	Ptr<M>					Add(char* szObjectName);

						Ptr<CRoot>				AddRoot(void);
						Ptr<CRoot>				GetRoot(void);

						BOOL					Remove(CArrayBlockObjectPtr* papcKilled);

						CPointer				Null(void);
	template<class M>	Ptr<M>					Null(void);

						long long int			NumMemoryIndexes(void);
						int						NumMemoryNames(void);
						long long int			NumDatabaseObjects(void);
						int						NumDatabaseObjectsCached(void);
						int						NumDatabaseObjectsCached(int iSize);
						long long int			NumDatabaseNames(void);
						CIndexGenerator*		GetIndexGenerator(void);
						CNamedIndexedObjects*	GetMemory(void);

						void					RemoveInKill(CBaseObject* pvObject);
						CBaseObject*			Dehollow(OIndex oi);
						CBaseObject*			Dehollow(char* szObjectName);

						BOOL					Dename(CBaseObject* pvObject);
						BOOL					Deindex(CBaseObject* pvObject);

						CStackPointers*			GetStackPointers(void);

						OIndex					StartMemoryIteration(SIndexesIterator* psIter);
						OIndex					IterateMemory(SIndexesIterator* psIter);
						CPointer				TestGetFromMemory(OIndex oi);
						CPointer				TestGetFromMemory(char* szName);
protected:
						BOOL					AddWithID(CBaseObject* pvObject, OIndex oi);
						BOOL					AddWithIDAndName(CBaseObject* pvObject, char* szObjectName, OIndex oi);
	template<class M>	M*						Allocate(void);
	template<class M>	M*						Allocate(int iAdditionalBytes);
						CBaseObject*			Allocate(char* szClassName);
						BOOL					ValidateCanAllocate(char* szClassName);
						BOOL					ValidateCanAllocate(void);
						CBaseObject*			GetFromMemory(OIndex oi);
						CBaseObject*			GetFromMemory(char* szObjectName);
						CBaseObject*			GetFromDatabase(OIndex oi);
						CBaseObject*			GetFromDatabase(char* szObjectName);
						CBaseObject*			GetFromSources(char* szObjectName);
						BOOL					ClearMemory(void);
						void					KillDontFreeObjects(CArrayBlockObjectPtr* papcObjectPts);
						void					FreeObjects(CArrayBlockObjectPtr* papcObjectPts);
						void					RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcObject, int iLevel, BOOL bEmbedded);
						void					ValidateSceneGraph(void);
						void					ValidateIndexedObjects(void);
						void					ClearValidationFlags(void);
						void					RecurseValidateSceneGraph(CBaseObject* pcBaseObject);
						CNamedHollowObject*		AllocateNamedHollow(unsigned short iNumEmbedded);
						CHollowObject*			AllocateHollow(unsigned short iNumEmbedded);
						void					AppenedHollowEmbeddedObjects(CBaseObject* pcHollow, unsigned short iNumEmbedded, void* pvEmbedded) ;
						void					PrintMemory(CChars* psz);
};


extern CObjects gcObjects;

void ObjectsInit(void);
void ObjectsInit(char* szWorkingDirectory);
void ObjectsInit(CIndexedConfig* pcConfig);
void ObjectsKill(void);

void LogObjectAllocation(CBaseObject* pcObject, char* szMethod);
void LogObjectDestruction(CBaseObject* pcObject, char* szMethod);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>	
M* CObjects::Allocate(void)
{
	return Allocate<M>(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>	
M* CObjects::Allocate(int iAdditionalBytes)
{
	M*		pcObject;
	BOOL	bResult;

	bResult = ValidateCanAllocate();
	if (!bResult)
	{
		return NULL;
	}

	pcObject = mpcUnknownsAllocatingFrom->AddUnsafe<M>(iAdditionalBytes);
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
template<class M>
//Called by Macro 'OMalloc'
Ptr<M> CObjects::Add(void)
{
	Ptr<M>	pObject;
	M*		pvObject;

	pvObject = Allocate<M>();
	if (pvObject->IsNamed())
	{
		LOG_OBJECT_ALLOCATION(pvObject);
		AddWithIDAndName(pvObject, "", mcIndexGenerator.PopIndex());

		//No PointTo because we don't know the embedding object until assignment.
		pObject.AssignObject(pvObject);
		return pObject;
	}

	LOG_OBJECT_ALLOCATION(pvObject);
	AddWithID(pvObject, mcIndexGenerator.PopIndex());  //TODO: The OID should be assigned when saved, not when allocated.

	//No PointTo because we don't know the embedding object until assignment.
	pObject.AssignObject(pvObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
//Called by Macro 'ONMalloc'.  Note the 'N'.
Ptr<M> CObjects::Add(char* szObjectName)
{
	Ptr<M>	pObject;
	M*		pvObject;

	pvObject = Allocate<M>();
	if (!pvObject->IsNamed())
	{
		//Can't add an unnamed object with a name.
		mpcUnknownsAllocatingFrom->Remove(pvObject);
		return Null<M>();
	}

	LOG_OBJECT_ALLOCATION(pvObject);
	AddWithIDAndName(pvObject, szObjectName, mcIndexGenerator.PopIndex());

	//No PointTo because we don't know the embedding object until assignment.
	pObject.AssignObject(pvObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CObjects::Null(void)
{
	Ptr<M> pObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		Ptr<M>		pObject;

		pObject.AssignObject(pvObject);
		return pObject;
	}
	else
	{
		return Null<M>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CObjects::Get(char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(szObjectName);
	if (pvObject)
	{
		Ptr<M>		pObject;

		pObject.AssignObject(pvObject);
		return pObject;
	}
	else
	{
		return Null<M>();
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CObjects::AddConstructor(void)
{
	mpcUnknownsAllocatingFrom->AddConstructor<M>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CObjectSource* CObjects::AddSource(CAbstractFile* pcFile, char* szFileName)
{
	return mcSource.AddSource<M>(pcFile, szFileName);
}


#endif // __OBJECTS_H__

