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
#include "PPDirective.h"
#include "CFile.h"
#include "PPBlockSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::Init(CCFile* pcFile, int iLine, int iBlock, BOOL bTextBlocks)
{
	mcRawTokens.Init();
	mapcBlocks.Init();
	mpcFileTokens = pcFile->GetTokens();
	mpcFile = pcFile;
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

	for (i = 0; i < mapcBlocks.NumElements(); i++)
	{
		pcBlock = *mapcBlocks.Get(i);
		pcBlock->Kill();
	}

	mapcBlocks.Kill();
	mcRawTokens.Kill();
	mpcFileTokens = NULL;
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlock* CPPBlockSet::GetMatchingBlock(CPPBlock* pcOtherBlock)
{
	int				i;
	CPPBlock*		pcBlock;
	BOOL			bResult;

	for (i = mapcBlocks.NumElements()-1; i >= 0; i--)
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
CPPBlock* CPPBlockSet::CreateBlock(void)
{
	CPPBlock*	pcBlock;

	pcBlock = CPPBlock::Construct(mpcFileTokens->GetStack()->Add(sizeof(CPPBlock)));
	pcBlock->Init(this, miLine, miColumn);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlock* CPPBlockSet::AddBlock(void)
{
	CPPBlock*	pcBlock;

	pcBlock = CreateBlock();
	mapcBlocks.Add(&pcBlock);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPBlockSet::AddBlock(CPPBlock* pcBlock)
{
	if (pcBlock->mpcBlockSet == this)
	{
		mapcBlocks.Add(&pcBlock);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPBlockSet::IsLastToken(int iToken)
{
	CPPToken*	pcToken;
	int			i;

	if (mcRawTokens.mcArray.NumElements() == iToken)
	{
		return TRUE;
	}

	for (i = mcRawTokens.mcArray.NumElements()-1; i >= iToken; i--)
	{
		pcToken = *mcRawTokens.mcArray.Get(i);
		if (!pcToken->IsEmpty())
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSet::DumpRawTokens(void)
{
	CChars			sz;
	int				iLast;

	iLast = 0;
	sz.Init();
	if (mbTextBlocks)
	{
		sz.Append("/* ------- text ");
		sz.Append(miLine);
		sz.Append(" ------- */\n");
	}
	else
	{
		sz.Append("/* ---- #directive ");
		sz.Append(miLine);
		sz.Append(" ---- */\n");
	}
	mcRawTokens.Print(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPBlockSet::IsDirective(void)
{
	return !mbTextBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMemoryStackExtended* CPPBlockSet::GetStack(void)
{
	return mpcFileTokens->GetStack();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTokenHolder* CPPBlockSet::GetTokenHolder(void)
{
	return &mcRawTokens;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPBlockSet::GetFileName(void)
{
	return mpcFile->ShortName();
}

