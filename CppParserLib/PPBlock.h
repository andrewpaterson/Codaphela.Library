/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __P_P_BLOCK_H__
#define __P_P_BLOCK_H__
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayIntAndPointer.h"
#include "DefineMap.h"
#include "PPTokenBlockIndex.h"
#include "NamedDefine.h"


class CPPBlockSet;
class CPPBlock : public CPPAbstractHolder
{
protected:
	CPPBlockSet*			mpcBlockSet;  //The blockset this block is in.
	CPPTokens*				mpcFileTokens;  //From CFile (from CBlockSet).
	SPPTokenBlockIndex		msNext;

public:
	TOKEN_CONSTRUCT(CPPBlock);
	void 				Init(int iLine, int iColumn);
	void 				Init(CPPBlockSet* pcBlockSet, int iLine, int iColumn);
	void 				Kill(void);
	BOOL 				IsBlock(void);
	BOOL				IsForBlockSet(CPPBlockSet* pcBlockSet);
	void				Copy(CPPToken* pcSource, CPPTokens* pcTokens);
	void				SetNext(int	iTokenIndex, int iBlockIndex);
	BOOL				Equals(CPPToken* pcOther);
	SPPTokenBlockIndex	GetNextTokenBlock(void);
	void				DumpPointersAsChars(CArrayIntAndPointer* papc);
};


typedef CArrayTemplate<CPPBlock*>	CArrayPtrCBlocks;


#endif //__P_P_BLOCK_H__

