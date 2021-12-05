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
#ifndef __C_BLOCK_SET_H__
#define __C_BLOCK_SET_H__
#include "PPBlock.h"
#include "DefineMap.h"
#include "PPTokenMemory.h"


class CPPBlockSet
{
protected:
	CPPTokenHolder		mcRawTokens;
	CPPTokenMemory*		mpcFileTokens;
	int					miLine;
	int					miColumn;
	int					miBlock;
	BOOL				mbTextBlocks;

	//If TextBlocks then the following are valid.  Otherwise this is a # directive block.
	CArrayPtrCBlocks	mapcBlocks;

public:
	void 					Init(CPPTokenMemory* pcFileTokens, int iLine, int iBlock, BOOL bTextBlocks);
	void 					Kill(void);
	CPPBlock*				GetMatchingBlock(CPPBlock* pcOtherBlock);
	CPPBlock*				CreateBlock(void);
	CPPBlock*				AddBlock(void);
	BOOL					AddBlock(CPPBlock* pcBlock);
	BOOL					IsLastToken(int iToken);
	BOOL					IsDirective(void);
	CPPTokenMemory*			GetFileTokens(void);
	CPPTokenHolder*			GetRawTokensHolder(void);
	int						Line(void);
	int						Column(void);
	int						Block(void);
	void					Dump(void);
};


#endif // !__C_BLOCK_SET_H__

