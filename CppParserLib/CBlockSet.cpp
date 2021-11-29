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
#include "CBlockSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlockSet::Init(CCFile* pcFile, int iLine, int iBlock, BOOL bTextBlocks)
{
	mcRawTokens.Init();
	mapcBlocks.Init();
	mpcStack = &pcFile->mcStack;
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
void CCBlockSet::Kill(void)
{
	int			i;
	CCBlock*	pcBlock;

	for (i = 0; i < mapcBlocks.NumElements(); i++)
	{
		pcBlock = *mapcBlocks.Get(i);
		pcBlock->Kill();
	}

	mapcBlocks.Kill();
	mcRawTokens.Kill();
	mpcStack = NULL;
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCBlock* CCBlockSet::GetMatchingBlock(CCBlock* pcOtherBlock)
{
	int				i;
	CCBlock*		pcBlock;
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
CCBlock* CCBlockSet::CreateBlock(void)
{
	CCBlock*	pcBlock;

	pcBlock = CCBlock::Construct(mpcStack->Add(sizeof(CCBlock)));
	pcBlock->Init(this, miLine, miColumn);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCBlock* CCBlockSet::AddBlock(void)
{
	CCBlock*	pcBlock;

	pcBlock = CreateBlock();
	mapcBlocks.Add(&pcBlock);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCBlockSet::AddBlock(CCBlock* pcBlock)
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
BOOL CCBlockSet::IsLastToken(int iToken)
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
void CCBlockSet::DumpRawTokens(void)
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
	mcRawTokens.Append(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCBlockSet::IsDirective(void)
{
	return !mbTextBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMemoryStackExtended* CCBlockSet::GetStack(void)
{
	return mpcStack;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTokenHolder* CCBlockSet::GetTokenHolder(void)
{
	return &mcRawTokens;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCBlockSet::Line(void)
{
	return miLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCBlockSet::Column(void)
{
	return miColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCBlockSet::Block(void)
{
	return miBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCBlockSet::GetFileName(void)
{
	return mpcFile->ShortName();
}

