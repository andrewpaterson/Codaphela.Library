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
#include "HeaderFileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFileMap::Init(void)
{
	mcFileMap.Init(TRUE, FALSE);
	mcFileList.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFileMap::Kill(void)
{
	CHeaderFile*	pcHeader;

	pcHeader = mcFileList.GetHead();
	while (pcHeader)
	{
		pcHeader->KillBlockSets();
		pcHeader = mcFileList.GetNext(pcHeader);
	}

	pcHeader = mcFileList.GetHead();
	while (pcHeader)
	{
		pcHeader->Kill();
		pcHeader = mcFileList.GetNext(pcHeader);
	}

	mcFileMap.Kill();
	mcFileList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::AddFile(char* szAbsoluteFileName, BOOL bSystem)
{
	CHeaderFile**		ppcHeader;
	CHeaderFile*		pcHeader;
	BOOL				bResult;
	
	ppcHeader = mcFileMap.Get(szAbsoluteFileName);
	if (!ppcHeader)
	{
		pcHeader = mcFileList.Add();
		if (pcHeader)
		{
			bResult = mcFileMap.Put(szAbsoluteFileName, &pcHeader);
			if (bResult)
			{
				New<CHeaderFile>(pcHeader);
				pcHeader->Init(szAbsoluteFileName, bSystem);
				return pcHeader;
			}
			else
			{
				mcFileList.Remove(pcHeader);
			}
		}
		return NULL;
	}
	else
	{
		return *ppcHeader;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::GetFile(char* szAbsoluteFileName)
{
	CHeaderFile**	pcHeader;

	pcHeader = mcFileMap.Get(szAbsoluteFileName);
	if (pcHeader)
	{
		return *pcHeader;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::StartIteration(SMapIterator* psIter)
{
	CHeaderFile**	ppcHeader;
	BOOL			bResult;

	bResult = mcFileMap.StartIteration(psIter, NULL, NULL, (void**)&ppcHeader, NULL);
	if (bResult && ppcHeader != NULL)
	{
		return *ppcHeader;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::Iterate(SMapIterator* psIter)
{
	CHeaderFile**	ppcHeader;
	BOOL			bResult;

	bResult = mcFileMap.Iterate(psIter, NULL, NULL, (void**)&ppcHeader, NULL);
	if (bResult && ppcHeader != NULL)
	{
		return *ppcHeader;
	}
	else
	{
		return NULL;
	}
}

