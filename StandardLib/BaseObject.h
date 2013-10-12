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
#include "CoreLib/IndexedGeneral.h"
#include "EmbeddedObject.h"


//Tested for root is only valid whilst the scene graph is calling CanFindRoot.  It stops the graph from walking already tested objects.
#define OBJECT_FLAGS_TESTED_FOR_ROOT		0x02

//Invalidated is set when the object on the file system is changed and must be reloaded.  This objects does not use it.
#define OBJECT_FLAGS_INVALIDATED			0x04

//Dirty must be manually set when an object needs to be written from memory to indexed data.  Objects are - by default always dirty.
#define OBJECT_FLAGS_DIRTY					0x08

//Debug flag marking whether or not an object has had kill called on it.  An object that is killed should be removed from Memory so an object with this flag set is broken.
#define OBJECT_FLAGS_KILLED					0x10

//Debug flag marking whether or not an object has had it's graph dumped yet.
#define OBJECT_FLAGS_DUMPED					0x20

//This object cannot be reached and is marked for killing.
#define OBJECT_FLAGS_UNREACHABLE			0x40

//How man embedded objects are in the object.  If you have more than 255 then you need your head smacked.
#define OBJECT_FLAGS_NUM_EMBEDDED			0x0000FF00


class CObjectDeserialiser;
class CObjectSerialiser;
class CObjects;
class CObjectRemapFrom;
class CBaseObject : public CEmbeddedObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CArrayCommonObject;
friend class CObject;
friend class CObjects;
friend class CObjectRemapFrom;

BASE_FUNCTIONS(CBaseObject);
protected:
	CObjects*	mpcObjectsThisIn;
	int			miDistToRoot;
	OIndex		moi;
	int			miFlags;

public:
								CBaseObject();
	virtual	void				PreInit(CObjects* pcObjects);
			void				PreInit(void);
	virtual	void				Class(void);

			void				Kill(void);

	virtual void				KillDontFree(void);
	virtual void				KillData(void) =0;

			OIndex				GetOI(void);
			void				SetObjectID(OIndex oi);
			void				ClearIndex(void);

	virtual BOOL				IsRoot(void);
	virtual BOOL				IsSubRoot(void);
			BOOL				IsUnknown(void);
			BOOL				IsHollow(void);
	virtual BOOL				IsCollection(void) =0;
	virtual BOOL				IsObject(void) =0;
	virtual BOOL				IsNamed(void);
			BOOL				IsInvalidated(void);
			BOOL				IsDirty(void);

	virtual char*				GetName(void);
	virtual void				SetName(char* szName);
			int					SerialisedSize(void);

	virtual BOOL				IsEmbeddedDirty(void);
			int					GetNumEmbedded(void);

	virtual void				SetDistToRoot(int iDistToRoot) =0;
			void				SetDirty(void);
			int					DistToRoot(void);
			BOOL				TestedForRoot(void);
	virtual void				GetTos(CArrayEmbeddedObjectPtr* papcTos) =0;
	virtual int					NumTos(void) =0;
	virtual void				RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged) =0;
			void				AddHeapFrom(CBaseObject* pcFrom);
			void				FixDistToRoot(void);
			void				RemoveHeapFrom(CBaseObject* pcFrom);
	virtual int					RecurseNumHeapFroms(void);
	virtual int					RecurseNumTotalFroms(void);
	virtual int					RecurseNumStackFroms(void);
			CObjects*			GetObjects(void);

			CEmbeddedObject* 	TestGetTo(int iToIndex);
			int					TestGetNumEmbeddedFromFlags(void);
	virtual void				RecurseSetFlagEmbedded(int iFlag, int iFlagValue);
	virtual void				RecurseSetDistToRoot(int iDistToRoot);
			BOOL				TestCanFindRoot(void);
	
protected:
	virtual void			KillToPointers(void) =0;
	virtual void			Free(void);
			int				RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) =0;
			BOOL			RemoveToFrom(CEmbeddedObject* pcPointedTo, CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
			void			PotentiallySetDistToRoot(CBaseObject* pcTos, int iExpectedDistToRoot);
	virtual BOOL			CanFindRoot(void);
			CBaseObject*	ClearDistToSubRoot(void);
			void			CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled);
	virtual void			CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled) =0;
			void			ContainerCollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled);
			void			CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled, CBaseObject* pcPointedTo);
			void			MarkThisForKilling(CArrayBaseObjectPtr* papcKilled);
			void			KillCollected(CArrayBaseObjectPtr* papcKilled);
			int				KillThisGraph(void);
			BOOL			IsBaseObject(void);
			int				GetNumEmbeddedFromFlags(void);
			void			SetFlagNumEmbedded(int iNumEmbedded);
			void			SetFlagEmbedded(int iFlag, int iFlagValue);
			void			TryKill(BOOL bStackPointerRemoved);
			void			ClearStackPointersTo(void);
			BOOL			IsMarkedUnreachable(void);
};


#endif // __BASE_OBJECT_H__

