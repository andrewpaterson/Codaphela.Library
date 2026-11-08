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
#ifndef __SERIALISED_OBJECT_H__
#define __SERIALISED_OBJECT_H__
#include "BaseLib/IndexedGeneral.h"
#include "ObjectFileGeneral.h"


//The serialised name maps onto the output of FileWriter.
//This means it's length is strlen+1.
struct SSerialisedName
{
	int32	iLength;
	char	sz[4];
};


#pragma pack(push, 1)
class CSerialisedObject
{
protected:
	uint32				miLength;  //Four bytes in 32 and 64 bit builds; this class maps onto the written file data.
	int32				miMagic;
	int32				meType;  //See ObjectFileGeneral.h

	OIndex				moi;
	uint16				muiSize;

	SSerialisedName		msName;

public:
	bool	IsNamed(void);
	bool	IsIndexed(void);
	bool	IsVoid(void);

	char*	GetName(void);
	OIndex	GetIndex(void);
	lsize	GetLength(void);
	bool	SetLength(size iLength);
};
#pragma pack(pop)


#endif // __SERIALISED_OBJECT_H__

