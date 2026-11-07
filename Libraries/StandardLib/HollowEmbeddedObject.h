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
#ifndef __HOLLOW_EMBEDDED_OBJECT_H__
#define __HOLLOW_EMBEDDED_OBJECT_H__
#include "EmbeddedObject.h"


class CObjects;
class CHollowObject;
class CHollowEmbeddedObject : public CEmbeddedObject
{
CONSTRUCTABLE(CHollowEmbeddedObject)
public:
	void				FreeInternal() override;
	void				DoneKill(bool bAllocatedInObjects) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	int					GetDistToRoot(void) override;
	bool				SetDistToRoot(int iDistToRoot) override;
	size				NumEmbedded(void) override;
	CEmbeddedObject*	GetEmbeddedObject(size iIndex) override;
	bool				IsHollow(void) override;
	CBaseObject*		Dehollow(void) override;
	size				GetEmbeddedIndex(void);
	CHollowObject*		GetHollowObject(void);
	CObjects*			GetObjectsThisIn(void) override;
	void				ValidateConsistency(void) override;
	bool				IsInitialised(void) override;
	bool				IsDirty(void) override;
	void				SetDirty(bool bDirty) override;
	bool				IsRoot(void) override;
	CClass*				GetClass(void) override;
	bool				HasNes(void) override;
	bool				HasNesPointerToThis(CEmbeddedObject* pcBaseObject) override;

protected:
};


#endif // __HOLLOW_EMBEDDED_OBJECT_H__

