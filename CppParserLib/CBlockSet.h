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
#ifndef __C_BLOCK_SET_H__
#define __C_BLOCK_SET_H__
#include "CBlock.h"
#include "DefineMap.h"


class CCFile;
class CCBlockSet
{
public:
	CPPTokenHolder			mcRawTokens;
	CMemoryStackExtended*	mpcStack;  //From CFile
	CCFile*					mpcFile;  //The file this block is a part of
	int						miLine;
	int						miColumn;
	int						miBlock;
	BOOL					mbTextBlocks;

	//If TextBlocks then the following are valid.  Otherwise this is a # directive block.
	CArrayPtrCBlocks		mapcBlocks;

	void 		Init(CCFile* pcFile, int iLine, int iBlock, BOOL bTextBlocks);
	void 		Kill(void);
	CCBlock*	GetMatchingBlock(CCBlock* pcOtherBlock);
	CCBlock*	CreateBlock(void);
	CCBlock*	AddBlock(void);
	BOOL		AddBlock(CCBlock* pcBlock);
	BOOL		IsLastToken(int iToken);
	BOOL		IsDirective(void);
	void		DumpRawTokens(void);
};


#endif // !__C_BLOCK_SET_H__

