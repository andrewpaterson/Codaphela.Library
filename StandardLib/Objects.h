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
#include "CoreLib/Files.h"
#include "NamedIndexedData.h"
#include "NamedIndexedObjects.h"
#include "Unknowns.h"
#include "Pointer.h"


#define OMalloc(classtype)			(gcObjects.Add<classtype>());
#define ONMalloc(classtype, name)	(gcObjects.Add<classtype>(name));


class CObjects
{
protected:
	CUnknowns*				mpcUnknownsAllocatingFrom;

	CNamedIndexedObjects	mcMemory;  //Objects (BaseObject*) allocated in Unkonws referenced by name and OIndex.  
	CNamedIndexedData		mcDatabase;  //Objects in the database also referenced by string and OIndex.  
	CFiles					mcFiles;  //Objects on the file system (including pack files) in .DRG files referenced only by name.

	OIndex					moiNext;

public:
						void			Init(CUnknowns* pcUnknownsAllocatingFrom, char* szWorkingDirectory);
						void			Kill(void);


						CPointerObject	Get(OIndex oi);
						CPointerObject	Get(char* szName);
	template<class M> 	CPointer<M>		Get(OIndex oi);
	template<class M>	CPointer<M>		Get(char* szName);

	template<class M>	CPointer<M>		Add(void);
	template<class M>	CPointer<M>		Add(char* szName);

						CPointerObject	Null(void);
	template<class M>	CPointer<M>		Null(void);

protected:
						void			StepNextObjectID(void);
						void			AddWithID(CBaseObject* pvObject);
						void			AddWithIDAndName(CBaseObject* pvObject, char* szName);
	template<class M>	M*				Allocate(void);
};						


extern CObjects gcObjects;


void ObjectsInit(char* szWorkingDirectory);
void ObjectsKill(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>	
M* CObjects::Allocate(void)
{
	return mpcUnknownsAllocatingFrom->AddUnsafe<M>();
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
		AddWithID(pvObject);

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
CPointer<M> CObjects::Add(char* szName)
{
	M	m;

	if (m.IsNamed())
	{
		CPointer<M>		pObject;
		M*				pvObject;

		pvObject = Allocate<M>();
		AddWithIDAndName(pvObject, szName);

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
CPointer<M> CObjects::Get(char* szName)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(szName);
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


#endif // __OBJECTS_H__

