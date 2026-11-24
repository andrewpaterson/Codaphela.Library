/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include <stdlib.h>
#include <string.h>
#include "CPPHeaderIndex.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Init(void)
{
	mcFiles.Init();
	mcDeclarations.Init();
	mapsDeclarations.Init();
	mcMutex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Kill(void)
{
	size	i;

	for (i = 0; i < mapsDeclarations.NumElements(); i++)
	{
		Free(mapsDeclarations.GetPtr(i));
	}
	mapsDeclarations.Kill();
	mcDeclarations.Kill();
	mcFiles.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Lock(void)
{
	mcMutex.Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Unlock(void)
{
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPHeaderDeclaration* CCPPHeaderIndex::Get(const char* szFile, int iLine, int iColumn)
{
	SCPPHeaderPlace				sPlace;
	SCPPHeaderDeclaration**		ppsDeclaration;

	sPlace.uiFile = mcFiles.Find(szFile);
	if (sPlace.uiFile == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	sPlace.iLine = iLine;
	sPlace.iColumn = iColumn;
	ppsDeclaration = mcDeclarations.Get(&sPlace);
	return ppsDeclaration ? *ppsDeclaration : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A declaration to fill in and Publish.
//
//////////////////////////////////////////////////////////////////////////
SCPPHeaderDeclaration* CCPPHeaderIndex::New(void)
{
	SCPPHeaderDeclaration*	psDeclaration;

	psDeclaration = (SCPPHeaderDeclaration*)malloc(sizeof(SCPPHeaderDeclaration));
	psDeclaration->uiTokens = 0;
	psDeclaration->uiHash = 0;
	psDeclaration->bLazy = false;
	psDeclaration->aszNames.Init();
	psDeclaration->aiKinds.Init();
	psDeclaration->szClass.Init();
	psDeclaration->auiBlock.Init();
	psDeclaration->psNext = NULL;
	return psDeclaration;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds psDeclaration (from New, filled in) starting at line iLine and
// column iColumn of szFile, after any already starting there.  If another
// unit published the same one (the same tokens) first, psDeclaration goes.
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Publish(const char* szFile, int iLine, int iColumn, SCPPHeaderDeclaration* psDeclaration)
{
	SCPPHeaderPlace			sPlace;
	SCPPHeaderDeclaration*	psLast;

	mcMutex.Lock();
	psLast = Get(szFile, iLine, iColumn);
	if (!psLast)
	{
		sPlace.uiFile = mcFiles.FindOrAddIndex(szFile);
		sPlace.iLine = iLine;
		sPlace.iColumn = iColumn;
		mcDeclarations.Put(&sPlace, &psDeclaration);
	}
	else
	{
		for (;;)
		{
			if ((psLast->uiTokens == psDeclaration->uiTokens) && (psLast->uiHash == psDeclaration->uiHash))
			{
				mcMutex.Unlock();
				Free(psDeclaration);
				return;
			}
			if (!psLast->psNext)
			{
				break;
			}
			psLast = psLast->psNext;
		}
		psLast->psNext = psDeclaration;
	}
	mapsDeclarations.Add(psDeclaration);
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPHeaderIndex::NumDeclarations(void)
{
	size	uiDeclarations;

	mcMutex.Lock();
	uiDeclarations = mapsDeclarations.NumElements();
	mcMutex.Unlock();
	return uiDeclarations;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Free(SCPPHeaderDeclaration* psDeclaration)
{
	psDeclaration->aszNames.Kill();
	psDeclaration->aiKinds.Kill();
	psDeclaration->szClass.Kill();
	psDeclaration->auiBlock.Kill();
	free(psDeclaration);
}
