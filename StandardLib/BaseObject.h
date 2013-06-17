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
#ifndef __BASE_OBJECT_H__
#define __BASE_OBJECT_H__
#include "BaseLib/ArrayEmbedded.h"
#include "CoreLib/IndexedGeneral.h"
#include "Unknown.h"


class CBaseObject;
typedef CArrayTemplate<CBaseObject*>		CArrayBaseObjectPtr;
typedef CArrayEmbedded<CBaseObject*, 32>	CArrayEmbeddedBaseObjectPtr;


//Tested for root is only valid whilst the scene graph is calling CanFindRoot.  It stops the graph from walking already tested objects.
#define OBJECT_FLAGS_TESTED_FOR_ROOT	0x02

//Invalidated is set when the object on the file system is changed and must be reloaded.  This objects does not use it.
#define OBJECT_FLAGS_INVALIDATED		0x04

//Dirty must be manually set when an object needs to be written from memory to indexed data.  Objects are - by default always dirty.
#define OBJECT_FLAGS_DIRTY				0x08

//Debug flag marking whether or not an object has had kill called on it.  An object that is killed should be removed from Memory so an object with this flag set is broken.
#define OBJECT_FLAGS_KILLED				0x10


#define ROOT_DIST_TO_ROOT			 0
#define UNATTACHED_DIST_TO_ROOT		-1
#define CLEARED_DIST_TO_ROOT		-2


class CObjectDeserialiser;
class CObjectSerialiser;
class CObjects;
class CBaseObject : public CUnknown
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CArrayCommonObject;
friend class CObject;
friend class CObjects;

BASE_FUNCTIONS(CBaseObject);
protected:
	CObjects*							mpcObjectsThisIn;
	int									miDistToRoot;
	OIndex								moi;
	CArrayEmbedded<CBaseObject*, 6>		mapFroms;  //Objects that 'this' is pointed from.  
	int									miFlags;

public:
							CBaseObject();
			void			PreInit(CObjects* pcObjects);

			void			Kill(void);

			void			KillDontFree(void);
	virtual void			KillData(void) =0;

	virtual BOOL			Save(CObjectSerialiser* pcFile) =0;
	virtual BOOL			Load(CObjectDeserialiser* pcFile) =0;

			OIndex			GetOI(void);
			void			SetObjectID(OIndex oi);
			void			ClearIndex(void);

			BOOL			HasFroms(void);

	virtual BOOL			IsRoot(void);
	virtual BOOL			IsSubRoot(void);
			BOOL			IsUnknown(void);
	virtual BOOL			IsHollow(void);
	virtual BOOL			IsCollection(void) =0;
	virtual BOOL			IsObject(void) =0;
	virtual BOOL			IsNamed(void);
			BOOL			IsInvalidated(void);
			BOOL			IsDirty(void);

	virtual char*			GetName(void);
	virtual void			SetName(char* szName);
			int				SerialisedSize(void);

	virtual void			SetDistToRoot(int iDistToRoot) =0;
			void			SetDirty(void);
			int				DistToRoot(void);
			BOOL			TestedForRoot(void);
	virtual void			GetTos(CArrayBaseObjectPtr* papcTos) =0;
			int				NumFroms(void);
	virtual int				NumTos(void) =0;
			CBaseObject* 	TestGetTo(int iToIndex);
			CBaseObject* 	TestGetFrom(int iFromIndex);
	virtual void			RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged) =0;
			void			AddFrom(CBaseObject* pcFrom);
			void			FixDistToRoot(void);
	
protected:
	virtual void			KillToPointers(void) =0;
	virtual void			Free(void);
			CBaseObject*	GetFrom(int iFrom);
	virtual int				RemapTos(CBaseObject* pcOld, CBaseObject* pcNew) =0;
			BOOL			RemoveToFrom(CBaseObject* pcPointedTo, CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
			void			PrivateRemoveFrom(CBaseObject* pcFrom);
			void			RemoveFrom(CBaseObject* pcFrom);
			int				RemoveAllFroms(void);
	virtual void			RemoveTo(CBaseObject* pcTo) =0;
			void			CopyFroms(CBaseObject* pcSource);
			void			PotentiallySetDistToRoot(CBaseObject* pcTos, int iExpectedDistToRoot);
			BOOL			CanFindRoot(void);
			CBaseObject*	ClearDistToSubRoot(void);
	virtual void			CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled) =0;
			void			MarkForKilling(CArrayBaseObjectPtr* papcKilled);
			void			KillCollected(CArrayBaseObjectPtr* papcKilled);
			int				KillThisGraph(void);
};


#endif // __BASE_OBJECT_H__

