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
#include "CBlock.h"
#include "CBlockSet.h"
#include "TokenHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::Init(int iLine, int iColumn)
{
	CPPAbstractHolder::Init(iLine, iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::Init(CCBlockSet* pcBlockSet, int iLine, int iColumn)
{
	Init(iLine, iColumn);
	mpcBlockSet = pcBlockSet;
	mpcStack = pcBlockSet->mpcStack;
	msNext.Init(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::Kill(void)
{
	CPPAbstractHolder::Kill();
	mpcBlockSet = NULL;
	mpcStack = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCBlock::IsBlock(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack)
{
	CCBlock*	pcCast;
	CPPToken*	pcToken;
	int			i;

	if (pcSource->IsBlock())
	{
		pcCast = (CCBlock*)pcSource;
		Init(pcCast->miLine, pcCast->miColumn);

		for (i = 0; i < pcCast->mcTokens.mcArray.NumElements(); i++)
		{
			pcToken = DuplicatePPToken(*pcCast->mcTokens.mcArray.Get(i), pcStack);
			mcTokens.Add(&pcToken);
		}

		mpcBlockSet = pcCast->mpcBlockSet;
		mpcStack = pcCast->mpcStack;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCBlock::Allocate(int iSize)
{
	return mpcStack->Add(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCBlock::Equals(CPPToken* pcOther)
{
	if (pcOther->IsBlock())
	{
		if (CPPAbstractHolder::Equals(pcOther))
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::SetNext(int iTokenIndex, int iBlockIndex)
{
	msNext.Init(iTokenIndex, iBlockIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::Dump(CArrayIntAndPointer* papc)
{
	int		i;
	char*	sz;
	CChars	sz2;

	sz2.Init();
	for (i = 0; i < papc->NumElements(); i++)
	{
		sz = (char*)papc->GetPtr(i);
		sz2.Append(sz);
		sz2.AppendNewLine();
	}
	sz2.Dump();
	sz2.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCBlock::DumpTokens(void)
{
	CPPAbstractHolder::Dump();
}

