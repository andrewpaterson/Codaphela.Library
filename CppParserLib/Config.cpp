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
#include "Config.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConfig::Init(char* szName)
{
	mszName.Init(szName);
	maszDefines.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConfig::Kill(void)
{
	maszDefines.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConfig::AddDefine(char* szDefine)
{
	maszDefines.Add(szDefine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConfig::AddDefines(char* szDefines)
{
	CChars			szTemp;
	CArrayChars		aszTemp;
	size			i;
	CChars*			pszDefine;

	szTemp.Init(szDefines);

	aszTemp.Init();
	szTemp.Split(&aszTemp, ';');

	for (i = 0; i < aszTemp.NumElements(); i++)
	{
		pszDefine = aszTemp.Get(i);
		pszDefine->StripWhitespace();
		AddDefine(pszDefine->Text());
	}

	szTemp.Kill();
	aszTemp.Kill();
}

