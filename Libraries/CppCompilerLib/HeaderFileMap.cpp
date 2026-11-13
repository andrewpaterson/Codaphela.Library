/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "HeaderFileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFileMap::Init(void)
{
	mcFileMap.Init(true, false);
	mcFileList.Init();
	mcMutex.Init();
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
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The header at szAbsoluteFileName, added if it isn't here yet.  Called
// while units are compiled, the caller holds the lock (Lock).
//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::AddFile(char* szAbsoluteFileName, bool bSystem)
{
	CHeaderFile**		ppcHeader;
	CHeaderFile*		pcHeader;
	bool				bResult;
	
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
	CHeaderFile**	ppcHeader;
	CHeaderFile*	pcHeader;

	mcMutex.LockShared();
	ppcHeader = mcFileMap.Get(szAbsoluteFileName);
	pcHeader = ppcHeader ? *ppcHeader : NULL;
	mcMutex.UnlockShared();
	return pcHeader;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFileMap::StartIteration(SMapIterator* psIter)
{
	CHeaderFile**	ppcHeader;
	bool			bResult;

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
	bool			bResult;

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


//////////////////////////////////////////////////////////////////////////
//
// Held while a header is added as units are compiled (AddFile).
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFileMap::Lock(void)
{
	mcMutex.Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFileMap::Unlock(void)
{
	mcMutex.Unlock();
}

