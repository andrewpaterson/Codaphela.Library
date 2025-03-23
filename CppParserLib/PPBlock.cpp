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
#include "PPBlock.h"
#include "PPBlockSet.h"
#include "TokenHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::Init(size iLine, size iColumn, char* szFileName)
{
	CPPTokenListHolder::Init(iLine, iColumn, szFileName);
	msNext.Init(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::Kill(void)
{
	CPPTokenListHolder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPBlock::IsBlock(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPBlock*	pcCast;
	CPPToken*	pcToken;
	size			i;
	size			iNumTokens;

	if (pcSource->IsBlock())
	{
		pcCast = (CPPBlock*)pcSource;
		Init(pcCast->miLine, pcCast->miColumn, pcCast->ShortFileName());

		iNumTokens = pcCast->mcTokens.NumTokens();
		for (i = 0; i < iNumTokens; i++)
		{
			pcToken = DuplicatePPToken(pcCast->GetTokenList()->Get(i), pcTokens);
			mcTokens.Add(pcToken);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPBlock::Equals(CPPToken* pcOther)
{
	if (pcOther->IsBlock())
	{
		if (CPPTokenListHolder::Equals(pcOther))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::SetNext(size iTokenIndex, size iBlockIndex)
{
	msNext.Init(iTokenIndex, iBlockIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlock::DumpPointersAsChars(CArrayIntAndPointer* papc)
{
	size		i;
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

