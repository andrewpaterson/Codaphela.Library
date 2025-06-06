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
#include "PPWhitespace.h"
#include "PPHashes.h"
#include "PPText.h"
#include "PPTextWithSource.h"
#include "PPTokenReplacementsHolder.h"
#include "PPDirective.h"
#include "PPConditional.h"
#include "PPInclude.h"


struct SPPTokenIterator
{
	int				iStack;
	int				iElement;
	void*			pvStackData;
};


class CPPTokenMemory
{
protected:
	CMemoryStackExtended	mcStack;

public:
	void							Init(void);
	void							Kill(void);

	void							Mark(CStackMarkExtended* psMark);
	void							Rollback(CStackMarkExtended* psMark);

	CPPHashes*						AddHashes(void);
	CPPReplacement*					AddReplacement(void);
	CPPText*						AddText(void);
	CPPTextWithSource*				AddTextWithSource(void);
	CPPWhitespace*					AddWhitespace(void);

	CPPBlock*						AddBlock(void);
	CPPTokenReplacementsHolder*		AddTokenReplacementsHolder(void);
	CPPLine*						AddLine(void);
	CPPDirective*					AddDirective(void);

	CPPConditional*					AddConditional(void);
	CPPInclude*						AddInclude(void);

	CPPToken*						StartIteration(SPPTokenIterator* psIter);
	CPPToken*						Iterate(SPPTokenIterator* psIter);

	size_t							Size(void);
	char*							Print(CChars* psz);
	void							Dump(void);
};


#endif // !__P_P_TOKENS_H__

