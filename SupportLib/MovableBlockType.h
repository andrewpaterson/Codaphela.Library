#ifndef __MOVABLE_BLOCK_TYPE_H__
#define __MOVABLE_BLOCK_TYPE_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ClassDefines.h"
#include "MovableBlock.h"


class CMovableBlockType : public CObject
{
CONSTRUCTABLE(CMovableBlockType);
protected:
	CArrayMovableBlock		maBlocksOfType;  //These are ALL the blocks of this type, e.g. all the image cel blocks 
	CCharsImmutable			mszTypeName;

public:
	void 				Init(char* szTypeName);
	void 				Free(void);
	void				Class(void);

	bool				Load(CObjectReader* pcFile);
	bool				Save(CObjectWriter* pcFile);

	bool 				Is(char* szTypeName);
	char*				GetName(void);

	size				NumBlocks(void);
	void				AddBlock(Ptr<CMovableBlock> pcTile);

	Ptr<CMovableBlock>	GetBlock(size iIndex);
	Ptr<CMovableBlock>	GetNullBlock(void);
};


typedef CArray<CMovableBlockType>	CArrayMovableBlockType;


#endif // __MOVABLE_BLOCK_TYPE_H__

