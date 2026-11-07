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
#ifndef __DEPENDENT_READ_OBJECT_H__
#define __DEPENDENT_READ_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "ObjectHeader.h"


#define DEPENDENT_READ_OBJECT_FLAG_READ		0x01
#define DEPENDENT_READ_OBJECT_FLAG_EXISTED	0x02


class CBaseObject;
class CDependentReadObject : public CObjectIdentifier
{
protected:
	uint16	muiFlags;

public:
	void	Init(CObjectIdentifier* pcObjectIdentifier);
	void	Kill(void);

	char*	GetName(void);
	OIndex	GetOldIndex(void);
	void	SetRead(void);
	void	SetExisting(void);
	bool	IsRead(void);
	bool	ExistsInObjects(void);
};




typedef CArrayTemplate<CDependentReadObject> CArrayDependentReadObject;


#endif // __DEPENDENT_READ_OBJECT_H__

