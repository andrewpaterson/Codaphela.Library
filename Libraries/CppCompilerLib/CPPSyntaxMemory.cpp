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
#include "BaseLib/Numbers.h"
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
// pcSyntaxes allocates the list of nodes (NULL for the system allocator),
// pcNodes the nodes (NULL for chunks of the memory's own).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxMemory::Init(CCPPSyntaxTree* pcTree, CMallocator* pcSyntaxes, CMallocator* pcNodes)
{
	mpcTree = pcTree;
	mcStack.Init(64 KB);
	mpcNodes = pcNodes;
	if (pcSyntaxes)
	{
		mapcSyntaxes.Init(pcSyntaxes);
	}
	else
	{
		mapcSyntaxes.Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxMemory::Kill(void)
{
	size		i;
	CCPPSyntax*	pcSyntax;

	for (i = 0; i < mapcSyntaxes.NumElements(); i++)
	{
		pcSyntax = mapcSyntaxes.GetPtr(i);
		pcSyntax->Kill();
		if (mpcNodes)
		{
			mpcNodes->Free(pcSyntax);
		}
	}
	mapcSyntaxes.Kill();
	mcStack.Kill();
	mpcNodes = NULL;
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxMemory::NumSyntaxes(void)
{
	return mapcSyntaxes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The nodes made since there were uiSyntaxes are killed (a tentative
// parse's).  Nodes in an arena are freed; the memory's own chunks keep
// theirs until Kill.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxMemory::Rollback(size uiSyntaxes)
{
	CCPPSyntax*	pcSyntax;

	while (mapcSyntaxes.NumElements() > uiSyntaxes)
	{
		pcSyntax = mapcSyntaxes.Pop();
		pcSyntax->Kill();
		if (mpcNodes)
		{
			mpcNodes->Free(pcSyntax);
		}
	}
}

