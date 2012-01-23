/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Meta.h"


CMeta*	gpcCurrentMeta = NULL;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeta::Init(void)
{
	mcDefinitions.Init(10);
	gpcCurrentMeta = this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeta::Kill(void)
{
	CDefinition*	pcDefinition;

	for (int i = 0; i < mcDefinitions.NumElements(); i++)
	{
		pcDefinition = mcDefinitions.Get(i);
		pcDefinition->Kill();
	}

	mcDefinitions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefinition* CMeta::AddDefintion(void)
{
	return mcDefinitions.Add();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeta::NumDefinitions(void)
{
	return mcDefinitions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefinition* CMeta::GetDefinition(int i)
{
	return mcDefinitions.Get(i);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefinition* CMeta::GetEntryDefinition(void)
{
	return mcDefinitions.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeta::Dump(void)
{
	CDefinition*	pcDefinition;

	for (int i = 0; i < NumDefinitions(); i++)
	{
		pcDefinition = GetDefinition(i);
		pcDefinition->Dump();
	}
}

