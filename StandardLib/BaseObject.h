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


class CObjectDeserialiser;
class CObjectSerialiser;
class CBaseObject : public CUnknown
{
template<class M>
friend class CPointer;
friend class CPointerObject;
friend class CObjectGraphDeserialiser;
friend class CArrayCommonObject;
friend class CObject;
friend class CArray;

BASE_FUNCTIONS(CBaseObject);
protected:
	int									miDistToRoot;
	OIndex								moi;
	CArrayEmbedded<CBaseObject*, 6>		mapFroms;  //Objects that 'this' is pointed from.  
	int									miFlags;

public:
							CBaseObject();
			void			Kill(void);

	virtual BOOL			Save(CObjectSerialiser* pcFile) =0;
	virtual BOOL			Load(CObjectDeserialiser* pcFile) =0;

			OIndex			GetOI(void);
			void			SetObjectID(OIndex oi);

			BOOL			HasFroms(void);

	virtual BOOL			IsRoot(void);
	virtual BOOL			IsSubRoot(void);
			BOOL			IsUnknown(void);
	virtual BOOL			IsHollow(void);
	virtual BOOL			IsCollection(void) =0;
	virtual BOOL			IsObject(void) =0;
	virtual BOOL			IsNamed(void);
			BOOL			IsInvalidated(void);

	virtual char*			GetName(void);

	virtual void			SetDistToRoot(int iDistToRoot) =0;
			int				DistToRoot(void);
			BOOL			TestedForRoot(void);
	virtual void			GetTos(CArrayBaseObjectPtr* papcTos) =0;
			int				NumFroms(void);
	virtual int				NumTos(void) =0;
			CBaseObject* 	TestGetTo(int iToIndex);
			CBaseObject* 	TestGetFrom(int iFromIndex);

protected:
			CBaseObject*	GetFrom(int iFrom);
	virtual void			RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged) =0;
			BOOL			RemoveToFrom(CBaseObject* pcPointedTo, CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
			void			AddFrom(CBaseObject* pcFrom);
			void			RemoveFrom(CBaseObject* pcFrom);
			void			RemoveEmbeddedFrom(CBaseObject* pcFrom);
			void			PotentiallySetDistToRoot(CBaseObject* pcTos, int iExpectedDistToRoot);
			void			FixDistToRoot(void);
			void			FixDistToRoot(CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
			BOOL			CanFindRoot(void);
			CBaseObject*	ClearDistToSubRoot(void);
	virtual void			CollectedThoseToBeKilled(CArrayBaseObjectPtr* papcKilled) =0;
			void			MarkForKilling(CArrayBaseObjectPtr* papcKilled);
			void			KillCollected(CArrayBaseObjectPtr* papcKilled);
			void			KillThisGraph(void);
};


#endif // __BASE_OBJECT_H__

