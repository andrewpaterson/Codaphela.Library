/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __ORDERER_H__
#define __ORDERER_H__
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/MapStringInt.h"
#include "Array.h"
#include "OrdererHolder.h"


typedef CMapLongTemplate<size>	CMapLongSize;


//Orders objects so that each comes after the objects that fulfil the names it requires (ported from Logicim's
//Orderer).  Each object fulfils some names and requires others.  Objects are taken in the order they were added: on
//each pass every object whose requirements are all fulfilled by the objects already ordered is ordered, and the passes
//repeat until every object is ordered.  If a pass orders nothing the requirements can't be met: a name nothing fulfils,
//or objects that require each other.  Names are case sensitive.
class COrderer : public CObject
{
CONSTRUCTABLE(COrderer);
DESTRUCTABLE(COrderer);
protected:
	Ptr<CArray<COrdererHolder>>	mpHolders;
	CMapLongSize				mcHolderIndices;  //Object index to holder index, rebuilt when it doesn't match mpHolders (after a Load).

public:
	void					Init(void);
	void					Free(void) override;
	void					Class(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					Add(Ptr<CBaseObject> pObject);
	void					AddFulfilment(Ptr<CBaseObject> pObject, const char* szName);
	void					AddRequirement(Ptr<CBaseObject> pObject, const char* szName);

	bool					Order(Ptr<CArray<CBaseObject>> pOrdered);
	bool					Order(Ptr<CArray<CBaseObject>> pOrdered, Ptr<CArray<CBaseObject>> pUnordered);

	size					NumObjects(void);
	Ptr<COrdererHolder>		GetHolder(Ptr<CBaseObject> pObject);

protected:
	Ptr<COrdererHolder>		GetOrAddHolder(Ptr<CBaseObject> pObject);
	size					GetHolderIndex(Ptr<CBaseObject> pObject);
	void					IndexHolders(void);
	bool					AreRequirementsFulfilled(Ptr<COrdererHolder> pHolder, CMapStringInt* pcFulfilled);
};


#endif // __ORDERER_H__

