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


#define ROOT_NAME	"GraphRoot"

#define CLEAR_MEMORY_CHUNK_SIZE		16384

#define OMalloc(classtype)			(gcObjects.Add<classtype>())
#define ONMalloc(classtype, name)	(gcObjects.Add<classtype>(name))
#define ORoot()						(gcObjects.AddRoot())
#define ONull						(Null())
#define ONNull(classtype)			(Null<classtype>())


class CObjects
{
friend class CBaseObject;
friend class CObjectAllocator;
protected:
	CUnknowns*				mpcUnknownsAllocatingFrom;

	CNamedIndexedObjects	mcMemory;  //Objects (BaseObject*) allocated in Unknowns referenced by name and OIndex.  
	CNamedIndexedData		mcDatabase;  //Objects in the database also referenced by string and OIndex.  
	
	CObjectsSource			mcSource;

	CIndexGenerator			mcIndexGenerator;

public:
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, char* szWorkingDirectory);
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, CIndexedConfig* pcConfig);
						void					Kill(void);

						BOOL					Flush(BOOL bClearMemory, BOOL bClearCache);
						BOOL					Save(CBaseObject* pcObject);
						BOOL					ForceSave(CBaseObject* pcObject);
						BOOL					Close(void);

						CPointerObject			Get(OIndex oi);
						CPointerObject			Get(char* szObjectName);
	template<class M> 	CPointer<M>				Get(OIndex oi);
	template<class M>	CPointer<M>				Get(char* szObjectName);
						BOOL					Contains(char* szObjectName);

	template<class M>	CPointer<M>				Add(void);
	template<class M>	CPointer<M>				Add(char* szObjectName);

						CPointer<CRoot>			AddRoot(void);

						void					Remove(CArrayBaseObjectPtr* papcKilled);

						CPointerObject			Null(void);
	template<class M>	CPointer<M>				Null(void);

	template<class M>	void					AddConstructor(void);
						long long int			NumMemoryIndexes(void);
						int						NumMemoryNames(void);
						long long int			NumDatabaseObjects(void);
						int						NumDatabaseObjectsCached(void);
						int						NumDatabaseObjectsCached(int iSize);
						long long int			NumDatabaseNames(void);
						CIndexGenerator*		GetIndexGenerator(void);
						CNamedIndexedObjects*	GetMemory(void);

						CBaseObject*			GetInMemoryObject(OIndex oi);
						void					RemoveInKill(CBaseObject* pvObject);
						CPointerObject			Dehollow(OIndex oi);

						BOOL					Dename(CBaseObject* pvObject);
						BOOL					Deindex(CBaseObject* pvObject);

						OIndex					StartMemoryIteration(SIndexesIterator* psIter);
						OIndex					IterateMemory(SIndexesIterator* psIter);

						CPointerObject			GetNotInMemory(char* szObjectName);
						CPointerObject			GetIfInMemory(char* szObjectName);

protected:
						BOOL				AddWithID(CBaseObject* pvObject, OIndex oi);
						BOOL				AddWithIDAndName(CBaseObject* pvObject, char* szObjectName, OIndex oi);
	template<class M>	M*					Allocate(void);
						CBaseObject*		Allocate(char* szClassName);
						BOOL				ClearMemory(void);
						void				KillDontFreeObjects(CArrayBaseObjectPtr* papcObjectPts);
						void				FreeObjects(CArrayBaseObjectPtr* papcObjectPts);
						void				FixDistToRoot(CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
						CPointerObject		GetIfInMemory(OIndex oi);
						CPointerObject		GetNotInMemory(OIndex oi, BOOL bOverwriteExisting);
};


extern CObjects gcObjects;


void ObjectsInit(void);
void ObjectsInit(char* szWorkingDirectory);
void ObjectsInit(CIndexedConfig* pcConfig);
void ObjectsKill(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>	
M* CObjects::Allocate(void)
{
	M*	pcObject;

	pcObject = mpcUnknownsAllocatingFrom->AddUnsafe<M>();
	if (pcObject)
	{
		pcObject->CBaseObject::PreInit(this);
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
//Called by Macro 'OMalloc'
CPointer<M> CObjects::Add(void)
{
	M	m;

	if (!m.IsNamed())
	{
		CPointer<M>		pObject;
		M*				pvObject;

		pvObject = Allocate<M>();
		AddWithID(pvObject, mcIndexGenerator.PopIndex());

		//No PointTo because we don't know the embedding object until assignment.
		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		//Can't add a named object without a name
		return Null<M>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
//Called by Macro 'ONMalloc'.  Note the 'N'.
CPointer<M> CObjects::Add(char* szObjectName)
{
	M	m;

	if (m.IsNamed())
	{
		CPointer<M>		pObject;
		M*				pvObject;

		pvObject = Allocate<M>();
		AddWithIDAndName(pvObject, szObjectName, mcIndexGenerator.PopIndex());

		//No PointTo because we don't know the embedding object until assignment.
		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		//Can't add an unnamed object with a name.
		return Null<M>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CPointer<M> CObjects::Null(void)
{
	CPointer<M>		pObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CPointer<M> CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		CPointer<M>		pObject;

		pObject.mpcObject = pvObject;
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
CPointer<M> CObjects::Get(char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(szObjectName);
	if (pvObject)
	{
		CPointer<M>		pObject;

		pObject.mpcObject = pvObject;
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


#endif // __OBJECTS_H__

