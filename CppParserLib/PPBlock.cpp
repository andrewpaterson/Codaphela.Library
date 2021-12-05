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
#include "PPBlock.h"
#include "PPBlockSet.h"
#include "TokenHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::Init(int iLine, int iColumn)
{
	CPPAbstractHolder::Init(iLine, iColumn);
	msNext.Init(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::Kill(void)
{
	CPPAbstractHolder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPBlock::IsBlock(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPBlock*	pcCast;
	CPPToken*	pcToken;
	int			i;
	int			iNumTokens;

	if (pcSource->IsBlock())
	{
		pcCast = (CPPBlock*)pcSource;
		Init(pcCast->miLine, pcCast->miColumn);

		iNumTokens = pcCast->mcTokens.NumTokens();
		for (i = 0; i < iNumTokens; i++)
		{
			pcToken = DuplicatePPToken(pcCast->GetTokens()->Get(i), pcTokens);
			mcTokens.Add(pcToken);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPBlock::Equals(CPPToken* pcOther)
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
void CPPBlock::SetNext(int iTokenIndex, int iBlockIndex)
{
	msNext.Init(iTokenIndex, iBlockIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::DumpPointersAsChars(CArrayIntAndPointer* papc)
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
SPPTokenBlockIndex CPPBlock::GetNextTokenBlock(void)
{
	return msNext;
}

