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
	int				i;
	CHeaderFile*	pcHeaderFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		mcFiles.GetAtIndex(i, NULL, &pcHeaderFile);
		pcHeaderFile->Kill();
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
	CChars*				pcName;
	CHeaderFile			cHeader;

	pcHeader = mcFiles.Put(szAbsoluteFileName);
	if (pcHeader)
	{
		pcName = mcFiles.PrivateGetKeyForData(pcHeader);
		
		cHeader.Init(pcName->Text(), bSystem);
		memcpy(pcHeader, &cHeader, sizeof(CHeaderFile));  //Initialise virtual function table.

		pcHeader->macBlockSets.mpcFile = pcHeader;
	}
	else
	{
		pcHeader = mcFiles.GetWithKey(szAbsoluteFileName);
	}
	return pcHeader;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::FindFile(char* szAbsoluteFileName)
{
	CChars		szFake;

	szFake.Fake(szAbsoluteFileName);
	return mcFiles.GetWithKey(&szFake);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CHeaderFileMap::NumFiles(void)
{
	return mcFiles.mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::GetFile(int iIndex)
{
	CChars*			psz;
	CHeaderFile*	pcHeaderFile;

	mcFiles.GetAtIndex(iIndex, &psz, &pcHeaderFile);

	return pcHeaderFile;
}

