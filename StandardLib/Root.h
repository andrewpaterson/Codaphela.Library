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
#ifndef __ROOT_H__
#define __ROOT_H__
#include "SetObject.h"
#include "Object.h"
#include "Pointer.h"


class CObjects;
class CRoot : public CObject
{
friend class CObjects;
friend class CUnknowns;
CONSTRUCTABLE(CRoot);
DESTRUCTABLE(CRoot);
protected:
	Ptr<CSetObject>		mpObjects;
	CObjects*			mpcObjectsAllocatingFrom;

public:
						CRoot();

						Ptr<CRoot>		Init(void);
						void			Class(void);
						void			Free(void);

						void			Add(CPointer& pObject);
						bool			Remove(CPointer& pObject);
						bool			Remove(CBaseObject* pcObject);
						void			RemoveAll(void);
						bool			IsRoot(void);
						bool			IsSubRoot(void);

						bool			Load(CObjectReader* pcFile) override;

						CPointer		Get(int iIndex);
	template<class M>	Ptr<M>			Get(int iIndex);
						CPointer		Get(char* szObjectName);
	template<class M>	Ptr<M>			Get(char* szObjectName);
						Ptr<CSetObject>	GetAll(void);
						int				NumObjects(void);

						void			CreateSet(void);
						void			NullifySet(void);
						CSetObject*		TestGetSet(void);
						bool			IsSetHollow(void);

						void			Add(CPointer& pO1, CPointer& pO2);
						void			Add(CPointer& pO1, CPointer& pO2, CPointer& pO3);

						void			TouchAll(void);
						void			KillAll(void);
protected:
						void			KillInternal(bool bHeapFromChanged, bool bValidateNotEmbedded) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CRoot::Get(int iIndex)
{
	Ptr<M> pM = Get(iIndex);
	return pM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CRoot::Get(char* szObjectName)
{
	Ptr<M> pM = Get(szObjectName);
	return pM;
}


#endif // __ROOT_H__

