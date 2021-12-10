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
#include "TranslationUnit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STULog::Init(BOOL bLogInlucdes, BOOL bLogBlocks)
{
	szIncludesLog.Init();
	szBlocksLog.Init();
	this->bLogInlucdes = bLogInlucdes;
	this->bLogBlocks = bLogBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STULog::Kill(void)
{
	szBlocksLog.Kill();
	szIncludesLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTranslationUnit::Init(char* szFullName, CLibrary* pcLibrary, BOOL bLogIncludes, BOOL bLogBlocks)
{
	CSourceFile::Init(szFullName);
	mpcLibrary = pcLibrary;
	mpcProcessed = NULL;

	if (bLogBlocks || bLogIncludes)
	{
		mpcLogs = (STULog*)malloc(sizeof(STULog));
		mpcLogs->Init(bLogIncludes, bLogBlocks);
	}
	else
	{
		mpcLogs = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTranslationUnit::Kill(void)
{
	if (mpcProcessed)
	{
		mpcProcessed->Kill();
		mpcProcessed = NULL;
	}

	if (mpcLogs)
	{
		mpcLogs->Kill();
		free(mpcLogs);
	}
	
	CSourceFile::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTranslationUnit::IsTranslationUnit(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTranslationUnit::IsHeader(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTranslationUnit::LogBlocks(void)
{
	if (mpcLogs)
	{
		if (mpcLogs->bLogBlocks)
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
BOOL CTranslationUnit::LogInlucdes(void)
{
	if (mpcLogs)
	{
		if (mpcLogs->bLogInlucdes)
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
BOOL CTranslationUnit::IsPragmaOnced(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTranslationUnit::IsSystemFile(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STULog* CTranslationUnit::GetLogs(void)
{
	return mpcLogs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLibrary* CTranslationUnit::GetLibrary(void)
{
	return mpcLibrary;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPTokenList* CTranslationUnit::GetProcessedTokenList(void)
{
	return mpcProcessed->GetTokenList();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlock* CTranslationUnit::AddProcessedBlock(void)
{
	mpcProcessed = mcTokenMemory.AddBlock();
	mpcProcessed->Init(-1, -1, ShortName());
	return mpcProcessed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTranslationUnit::Print(CChars* psz)
{
	return mpcProcessed->Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTranslationUnit::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

