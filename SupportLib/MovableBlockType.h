/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __MOVABLE_BLOCK_TYPE_H__
#define __MOVABLE_BLOCK_TYPE_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/ArrayType.h"
#include "MovableBlock.h"


class CMovableBlockType : public CUnknown
{
CONSTRUCTABLE(CMovableBlockType);
protected:
	CArrayUnknown		macBlocksOfType;  //These are ALL the blocks of this type, e.g. all the image cel blocks 
	CCharsImmutable		mszTypeName;

public:
	void 			Init(char* szTypeName);
	void 			Kill(void);
	bool 			Is(char* szTypeName);
	CMovableBlock*	GetNull(void);
	void			AddTile(CMovableBlock* pcTile);
	CMovableBlock*	Get(size iIndex);
};


typedef CArrayType<CMovableBlockType>	CArrayMovableBlockType;


#endif // __MOVABLE_BLOCK_TYPE_H__

