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
	mcFiles.Init(3, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFileMap::Kill(void)
{
	CHeaderFile*	pcHeader;
	SMapIterator	sIter;
	BOOL			bResult;

	bResult = mcFiles.StartIteration(&sIter, NULL, NULL, (void**)&pcHeader, NULL);
	while (bResult)
	{
		pcHeader->Kill();
		bResult = mcFiles.Iterate(&sIter, NULL, NULL, (void**)&pcHeader, NULL);
	}

	mcFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::AddFile(char* szAbsoluteFileName, BOOL bSystem)
{
	CHeaderFile*		pcHeader;
	char*				szName;

	
	pcHeader = mcFiles.Get(szAbsoluteFileName);
	if (!pcHeader)
	{
		pcHeader = mcFiles.Put(szAbsoluteFileName);
		szName = mcFiles.GetKeyForData(pcHeader);
		
		New<CHeaderFile>(pcHeader);
		pcHeader->Init(szName, bSystem);
	}
	return pcHeader;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::FindFile(char* szAbsoluteFileName)
{
	return mcFiles.Get(szAbsoluteFileName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::StartIteration(SMapIterator* psIter)
{
	CHeaderFile*	pcHeader;
	BOOL			bResult;

	bResult = mcFiles.StartIteration(psIter, NULL, NULL, (void**)&pcHeader, NULL);
	if (bResult)
	{
		return pcHeader;
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
	CHeaderFile*	pcHeader;
	BOOL			bResult;

	bResult = mcFiles.Iterate(psIter, NULL, NULL, (void**)&pcHeader, NULL);
	if (bResult)
	{
		return pcHeader;
	}
	else
	{
		return NULL;
	}
}


