/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include <stdlib.h>
#include "BaseLib/PointerFunctions.h"
#include "CPPHeaderIndex.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Init(void)
{
	mcDeclarations.Init(true);
	mapsDeclarations.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHeaderIndex::Kill(void)
{
	size					i;
	SCPPHeaderDeclaration*	psDeclaration;

	for (i = 0; i < mapsDeclarations.NumElements(); i++)
	{
		psDeclaration = mapsDeclarations.GetPtr(i);
		psDeclaration->aszNames.Kill();
		psDeclaration->aiKinds.Kill();
		psDeclaration->szClass.Kill();
		SafeFree(psDeclaration);
	}
	mapsDeclarations.Kill();
	mcDeclarations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPHeaderDeclaration* CCPPHeaderIndex::Get(uint64 uiKey)
{
	SCPPHeaderDeclaration**	ppsDeclaration;

	ppsDeclaration = mcDeclarations.Get((int64)uiKey);
	return ppsDeclaration ? *ppsDeclaration : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A new declaration starting at uiKey, after any already starting there.
//
//////////////////////////////////////////////////////////////////////////
SCPPHeaderDeclaration* CCPPHeaderIndex::Add(uint64 uiKey)
{
	SCPPHeaderDeclaration*	psDeclaration;
	SCPPHeaderDeclaration*	psLast;

	psDeclaration = (SCPPHeaderDeclaration*)malloc(sizeof(SCPPHeaderDeclaration));
	psDeclaration->uiTokens = 0;
	psDeclaration->uiHash = 0;
	psDeclaration->bLazy = false;
	psDeclaration->aszNames.Init();
	psDeclaration->aiKinds.Init();
	psDeclaration->szClass.Init();
	psDeclaration->psNext = NULL;
	mapsDeclarations.Add(psDeclaration);
	psLast = Get(uiKey);
	if (!psLast)
	{
		mcDeclarations.Put((int64)uiKey, &psDeclaration);
		return psDeclaration;
	}
	while (psLast->psNext)
	{
		psLast = psLast->psNext;
	}
	psLast->psNext = psDeclaration;
	return psDeclaration;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPHeaderIndex::NumDeclarations(void)
{
	return mapsDeclarations.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CCPPHeaderIndex::Key(const char* szFile, int iLine, int iColumn)
{
	uint64		uiHash;
	const char*	sz;

	uiHash = 1469598103934665603ULL;
	for (sz = szFile; *sz; sz++)
	{
		uiHash = (uiHash ^ (uint8)*sz) * 1099511628211ULL;
	}
	uiHash = (uiHash ^ (uint32)iLine) * 1099511628211ULL;
	uiHash = (uiHash ^ (uint32)iColumn) * 1099511628211ULL;
	return uiHash;
}
