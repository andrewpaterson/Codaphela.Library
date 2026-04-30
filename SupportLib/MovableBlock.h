#ifndef __MOVABLE_BLOCK_H__
#define __MOVABLE_BLOCK_H__
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
#include "BaseLib/CharsImmutable.h"
#include "StandardLib/Array.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Object.h"


class CMovableBlock : public CObject
{
CONSTRUCTABLE(CMovableBlock);
DESTRUCTABLE(CMovableBlock);
protected:
public:
			void	Init(void);
			void 	Free(void);
			void	Class(void);

			bool	Save(CObjectWriter* pcFile);
			bool	Load(CObjectReader* pcFile);

	virtual	void	MovableBlockAbstract(void) =0;
};


typedef CArray<CMovableBlock>	CArrayMovableBlock;


#endif // __MOVABLE_BLOCK_H__

