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
#ifndef __P_P_BLOCK_SET_H__
#define __P_P_BLOCK_SET_H__
#include "PPBlock.h"
#include "DefineMap.h"
#include "PPTokenMemory.h"


typedef CArrayTemplate<CPPBlock*>	CArrayPPBlocks;


class CPPBlockSet
{
protected:
	CPPTokenList		mcUnprocessedTokens;
	int					miLine;
	int					miColumn;
	int					miBlock;
	bool				mbTextBlocks;

	//If TextBlocks then the following are valid.  Otherwise this is a # directive block.
	CArrayPPBlocks		mapcBlocks;

public:
	void 				Init(int iLine, int iBlock, bool bTextBlocks);
	void 				Kill(void);
	CPPBlock*			GetMatchingBlock(CPPBlock* pcOtherBlock);
	void				AddBlock(CPPBlock* pcBlock);
	bool				IsLastToken(int iToken);
	bool				IsDirective(void);
	CPPTokenList*		GetUnprocssedTokens(void);
	int					Line(void);
	int					Column(void);
	int					Block(void);
	void				Print(CChars* psz);
	void				Dump(void);
	void				DumpBlockSet(void);
};


#endif // !__P_P_BLOCK_SET_H__

