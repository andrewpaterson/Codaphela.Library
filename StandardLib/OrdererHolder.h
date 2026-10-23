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
#ifndef __ORDERER_HOLDER_H__
#define __ORDERER_HOLDER_H__
#include "BaseLib/ArrayChars.h"
#include "Object.h"
#include "Pointer.h"


//An object being ordered by COrderer with the names it fulfils and the names it requires.
class COrdererHolder : public CObject
{
CONSTRUCTABLE(COrdererHolder);
DESTRUCTABLE(COrdererHolder);
protected:
	Ptr<CBaseObject>	mpObject;
	CArrayChars			maszFulfilments;
	CArrayChars			maszRequirements;

public:
	void				Init(void);
	void				Init(Ptr<CBaseObject> pObject);
	void				Free(void) override;
	void				Class(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				AddFulfilment(const char* szName);
	void				AddRequirement(const char* szName);

	Ptr<CBaseObject>	GetObject(void);
	CArrayChars*		GetFulfilments(void);
	CArrayChars*		GetRequirements(void);

protected:
	bool				SaveNames(CObjectWriter* pcFile, CArrayChars* paszNames);
	bool				LoadNames(CObjectReader* pcFile, CArrayChars* paszNames);
};


#endif // __ORDERER_HOLDER_H__

