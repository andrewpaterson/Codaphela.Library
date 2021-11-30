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
#ifndef __P_P_TOKENS_H__
#define __P_P_TOKENS_H__
#include "BaseLib/MemoryStackExtended.h"
#include "PPBlock.h"
#include "PPReplacement.h"
#include "PPWhiteSpace.h"
#include "PPHashes.h"
#include "PPText.h"
#include "PPHolder.h"


class CPPTokens
{
protected:
	CMemoryStackExtended	mcStack;

	void				Init(void);
	void				Kill(void);

public:
	CPPHashes*			AddHashes(void);
	CPPReplacement*		AddReplacement(void);
	CPPText*			AddText(void);
	CPPWhiteSpace*		AddWhiteSpace(void);

	CPPBlock*			AddBlock(void);
	CPPHolder*			AddHolder(void);
	CPPLine*			AddLine(void);
};


#endif // !__P_P_TOKENS_H__


