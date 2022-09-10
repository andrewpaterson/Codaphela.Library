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
#ifndef __ARRAY_COMMON_OBJECT_H__
#define __ARRAY_COMMON_OBJECT_H__
#include "Collection.h"
#include "ArrayCommonUnknown.h"


class CArrayCommonUnknown;
class CArrayCommonObject : public CCollection
{
CONSTRUCTABLE(CArrayCommonObject);
DESTRUCTABLE(CArrayCommonObject);
protected:
	CArrayCommonUnknown		mcArray;
	bool					mbSubRoot;

public:
	Ptr<CArrayCommonObject>	Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder);
	void					Class(void);

	bool					Add(CPointer& pObject);
	void					AddAll(CArrayCommonObject* pcArray);
	void					Set(int iIndex, CPointer& pObject);

	bool					Remove(CPointer& pObject);
	bool					Remove(CBaseObject* pcObject);
	void					RemoveAll(void);
	void					Clear(void);

	int						NumElements(void);
	int						NonNullElements(void);
	int						Size(void);
	int						Length(void);
	bool					IsEmpty(void);

	void					MakeSubRoot(void);
	bool					IsSubRoot(void);
	void					SetPointerTosExpectedDistToRoot(int iDistToRoot);

	int						NumPointerTos(void);
	int						BaseNumPointerTos(void);
	void					GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	void					BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	bool					ContainsPointerTo(CEmbeddedObject* pcEmbedded);
	void					RemoveAllPointerTosDontKill(void);
	void					RemoveAllPointerTos(void);
	void					CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	CBaseObject*			UnsafeGet(int iIndex);
	bool					UnsafeRemove(CBaseObject* pcObject);
	CEmbeddedObject*		GetEmbeddedObject(uint16 iIndex);

	void					BaseValidatePointerTos(void);
	void					ValidatePointerTos(void);
	void					ValidateConsistency(void);

	void					TouchAll(void);
	void					KillAll(void);

protected:
	void	FreePointers(void) override;
	void	Free(void);
	void	RemovePointerTo(CEmbeddedObject* pcTo);
	int		RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
	void	SetPointedTosDistToRoot(int iDistToRoot);

	void	UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
};


#endif // !__ARRAY_COMMON_OBJECT_H__

