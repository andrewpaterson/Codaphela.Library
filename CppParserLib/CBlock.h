/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __C_BLOCK_H__
#define __C_BLOCK_H__
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTypedPointer.h"
#include "DefineMap.h"
#include "CBlockToken.h"
#include "NamedDefine.h"


class CCBlockSet;
class CCBlock : public CPPAbstractHolder
{
public:
	CCBlockSet*				mpcBlockSet;  //The blockset this block is in.
	CMemoryStackExtended*	mpcStack;  //From CFile (from CBlockSet).
	SCTokenBlock			msNext;

	TOKEN_CONSTRUCT(CCBlock);
	void 	Init(int iLine, int iColumn);
	void 	Init(CCBlockSet* pcBlockSet, int iLine, int iColumn);
	void 	Kill(void);
	BOOL 	IsBlock(void);
	void	Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack);
	void	SetNext(int	iTokenIndex, int iBlockIndex);
	void*	Allocate(int iSize);
	BOOL	Equals(CPPToken* pcOther);
	void	Dump(CArrayTypedPointer* papc);
	void	DumpTokens(void);
};


typedef CArrayTemplate<CCBlock*>	CArrayPtrCBlocks;


#endif //__C_BLOCK_H__

