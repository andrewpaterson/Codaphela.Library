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

#include "BaseLib/FNVHash.h"
#include "NameSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNameSet::Init(void)
{
	mcNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNameSet::Kill(void)
{
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// FNV-1a: what a name is known by in the preprocessor's header cache
// (not the set's own hash).
//
//////////////////////////////////////////////////////////////////////////
uint64 CNameSet::Hash(const char* szName, size uiLength)
{
	return FNVHash64(szName, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNameSet::Contains(const char* szName, size uiLength)
{
	SHashText	sName;

	sName.Init(szName, uiLength);
	return mcNames.HasKey(&sName);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the name (its text must last as long as the set).  False if it was
// in already.
//
//////////////////////////////////////////////////////////////////////////
bool CNameSet::Add(const char* szName, size uiLength)
{
	SHashText	sName;

	sName.Init(szName, uiLength);
	return mcNames.Add(&sName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNameSet::NumNames(void) const
{
	return mcNames.NumElements();
}

