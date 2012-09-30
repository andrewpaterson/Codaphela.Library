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
#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "BaseObject.h"


class CObject : public CBaseObject
{
template<class M>
friend class CPointer;
friend class CPointerObject;
friend class CObjectGraphDeserialiser;

BASE_FUNCTIONS(CObject);
protected:
	CArrayEmbedded<CPointerObject*, 5>	mapPointers;  //Pointers in this object.  

public:
					CObject();
	void			Kill(void);
	int				NumTos(void);
	BOOL			IsCollection(void);
	BOOL			IsObject(void);
	void			SetDistToRoot(int iDistToRoot);
	CPointerObject* Pointer(CPointerObject* pcPointer);
	
protected:
	void			KillToPointers(void);
	void			GetTos(CArrayBaseObjectPtr* papcTos);
	void			RemoveTo(CBaseObject* pcTo);
	void			RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
	void			CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled);
	int				RemapTos(CBaseObject* pcOld, CBaseObject* pcNew);
	void			Free(void);
};


#endif // __OBJECT_H__

