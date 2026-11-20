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
#include "CPPFunctionPacks.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionPacks::Init(void)
{
	masPacks.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionPacks::Kill(void)
{
	size	i;

	for (i = 0; i < masPacks.NumElements(); i++)
	{
		masPacks.Get(i)->apcElements.Kill();
	}
	masPacks.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionPacks::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The pack, added with no elements if it isn't one of them yet.
//
//////////////////////////////////////////////////////////////////////////
SCPPFunctionPack* CCPPFunctionPacks::Add(CCPPSymbol* pcPack)
{
	SCPPFunctionPack*	psPack;

	psPack = Find(pcPack);
	if (!psPack)
	{
		psPack = masPacks.Add();
		psPack->pcPack = pcPack;
		psPack->apcElements.Init();
	}
	return psPack;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds each of pcPacks' packs that isn't one of these yet, with its
// elements.
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionPacks::AddAll(CCPPFunctionPacks* pcPacks)
{
	SCPPFunctionPack*	psPack;
	size				i;

	for (i = 0; i < pcPacks->masPacks.NumElements(); i++)
	{
		psPack = pcPacks->masPacks.Get(i);
		if (!Find(psPack->pcPack))
		{
			Add(psPack->pcPack)->apcElements.Copy(&psPack->apcElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pack of the symbol, or NULL if it isn't one of them.
//
//////////////////////////////////////////////////////////////////////////
SCPPFunctionPack* CCPPFunctionPacks::Find(CCPPSymbol* pcPack)
{
	size	i;

	for (i = 0; i < masPacks.NumElements(); i++)
	{
		if (masPacks.Get(i)->pcPack == pcPack)
		{
			return masPacks.Get(i);
		}
	}
	return NULL;
}

