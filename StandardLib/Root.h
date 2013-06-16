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
#include "Set.h"
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
	CPointer<CSet>	mpObjects;
	CObjects*		mpcObjectsAllocatingFrom;

protected:
						CRoot();
						void			Init(void);
						void			Init(CObjects* pcObjectsAllocatingFrom);

public:
						void			KillData(void);

						void			Add(CPointerObject pObject);
						void			Remove(CPointerObject pObject);
						void			RemoveAll(void);
						BOOL			IsRoot(void);
						BOOL			IsSubRoot(void);

						BOOL			Save(CObjectSerialiser* pcFile);
						BOOL			Load(CObjectDeserialiser* pcFile);

						CPointerObject	Get(char* szObjectName);
	template<class M>	CPointer<M>		Get(char* szObjectName);
						CPointer<CSet>	GetAll(void);

						CSet*			TestGetSet(void);
						BOOL			IsSetHollow(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
//Called by Macro 'OMalloc'
CPointer<M> CRoot::Get(char* szObjectName)
{
	CPointerObject	pObject;
	CPointer<M>		pM;

	pObject = Get(szObjectName);

	pM = pObject;
	return pM;
}


#endif // __ROOT_H__

