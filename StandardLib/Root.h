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
#include "NamedObject.h"
#include "Pointer.h"


class CObjects;
class CRoot : public CNamedObject
{
friend class CObjects;
friend class CUnknowns;
friend class CConstructorUnknown;
BASE_FUNCTIONS(CRoot);
protected:
	Ptr<CSetObject>	mpObjects;
	CObjects*		mpcObjectsAllocatingFrom;

protected:
						CRoot();
						Ptr<CRoot>		Init(void);

public:
						void			Kill(void);
						void			Class(void);
						void			KillData(void);

						void			Add(CPointer& pObject);
						BOOL			Remove(CPointer& pObject);
						BOOL			Remove(CBaseObject* pcObject);
						void			RemoveAll(void);
						BOOL			IsRoot(void);
						BOOL			IsSubRoot(void);

						void			KillAll(void);

						BOOL			Save(CObjectSerialiser* pcFile);
						BOOL			Load(CObjectDeserialiser* pcFile);

						CPointer		Get(int iIndex);
	template<class M>	Ptr<M>			Get(int iIndex);
						CPointer		Get(char* szObjectName);
	template<class M>	Ptr<M>			Get(char* szObjectName);
						Ptr<CSetObject>	GetAll(void);
						int				NumObjects(void);

						void			CreateSet(void);
						void			NullifySet(void);
						CSetObject*		TestGetSet(void);
						BOOL			IsSetHollow(void);
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

