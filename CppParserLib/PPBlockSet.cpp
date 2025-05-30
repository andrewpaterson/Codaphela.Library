/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "PPDirective.h"
#include "SourceFile.h"
#include "PPBlockSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::Init(int iLine, int iBlock, bool bTextBlocks)
{
	mcUnprocessedTokens.Init();
	mapcBlocks.Init();
	miColumn = 0;
	miLine = iLine;
	miBlock = iBlock;
	mbTextBlocks = bTextBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::Kill(void)
{
	int			i;
	CPPBlock*	pcBlock;
	int			iNumElements;

	iNumElements = mapcBlocks.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcBlock = *mapcBlocks.Get(i);
		pcBlock->Unuse();
	}

	mapcBlocks.Kill();
	mcUnprocessedTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::AddBlock(CPPBlock* pcBlock)
{
	pcBlock->Use();
	mapcBlocks.Add(&pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlock* CPPBlockSet::GetMatchingBlock(CPPBlock* pcOtherBlock)
{
	int			i;
	CPPBlock* pcBlock;
	bool		bResult;
	int			iNumElements;

	iNumElements = mapcBlocks.NumElements();
	for (i = iNumElements - 1; i >= 0; i--)
	{
		pcBlock = *mapcBlocks.Get(i);
		bResult = pcOtherBlock->Equals(pcBlock);
		if (bResult)
		{
			return pcBlock;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPBlockSet::IsLastToken(int iToken)
{
	CPPToken*	pcToken;
	int			i;
	int			iNumTokens;

	iNumTokens = mcUnprocessedTokens.NumTokens();
	if (iNumTokens == iToken)
	{
		return true;
	}

	for (i = iNumTokens - 1; i >= iToken; i--)
	{
		pcToken = mcUnprocessedTokens.Get(i);
		if (!pcToken->IsEmpty())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::Print(CChars* psz)
{
	int				iNumElements;
	int				i;
	CPPBlock*		pcBlock;

	if (mbTextBlocks)
	{
		iNumElements = mapcBlocks.NumElements();
		for (i = 0; i < iNumElements; i++)
		{
			pcBlock = *mapcBlocks.Get(i);
			pcBlock->Print(psz);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::Dump(void)
{
	CChars			sz;
			
	sz.Init();
	Print(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::DumpBlockSet(void)
{
	CChars			sz;
	int				iLast;

	iLast = 0;
	sz.Init();
	if (mbTextBlocks)
	{
		sz.Append("/* ------- text ");
		sz.Append(miLine + 1);
		sz.Append(" ------- */\n");
	}
	else
	{
		sz.Append("/* ---- #directive ");
		sz.Append(miLine + 1);
		sz.Append(" ---- */\n");
	}
	mcUnprocessedTokens.Print(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPBlockSet::IsDirective(void)
{
	return !mbTextBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTokenList* CPPBlockSet::GetUnprocssedTokens(void)
{
	return &mcUnprocessedTokens;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPBlockSet::Line(void)
{
	return miLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPBlockSet::Column(void)
{
	return miColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPBlockSet::Block(void)
{
	return miBlock;
}

