/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "SpecialOperator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperator::Init(char* pcName, EPreprocessorSpecialOperator eType, int64 lliID)
{
	mszName.Init(pcName);
	meType = eType;
	mlliID = lliID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperator::Kill(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSpecialOperator::Equals(CSpecialOperator* pcOther)
{
	if (pcOther)
	{
		return meType == pcOther->meType;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSpecialOperator::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPreprocessorSpecialOperator CSpecialOperator::GetType(void)
{
	return meType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpecialOperator::Dump(void)
{
	CChars	sz;

	sz.Init();
	sz.Append(mszName);
	sz.Append(" -> ");
	
	switch (meType)
	{
	case PPSO_Defined:
		sz.Append("PPSO_Defined");
		break;
	case PPSO_HasAttribute:
		sz.Append("PPSO_HasAttribute");
		break;
	case PPSO_HasCppAttribute:
		sz.Append("PPSO_HasCppAttribute");
		break;
	case PPSO_HasBuiltIn:
		sz.Append("PPSO_HasBuiltIn");
		break;
	case PPSO_HasInclude:
		sz.Append("PPSO_HasInclude");
		break;
	default:
		sz.Append("Unknown");
		break;
	}
	sz.DumpKill();
}

