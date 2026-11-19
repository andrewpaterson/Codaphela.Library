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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpSpecifiers.Init(NodeArrayMallocator());
	muiFlags = 0;
	muiTypeKeywords = 0;
	miLongs = 0;
	mpcType = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::Kill(void)
{
	macpSpecifiers.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A simple type keyword counted in the specifiers: what the type the
// keywords name is worked out from (CCPPDeclarators).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::CountTypeKeyword(ECppTokens eToken)
{
	switch (eToken)
	{
	case CT_void:		muiTypeKeywords |= CPPTK_VOID;		break;
	case CT_bool:		muiTypeKeywords |= CPPTK_BOOL;		break;
	case CT_char:		muiTypeKeywords |= CPPTK_CHAR;		break;
	case CT_wchar_t:	muiTypeKeywords |= CPPTK_WCHAR_T;	break;
	case CT_char16_t:	muiTypeKeywords |= CPPTK_CHAR16_T;	break;
	case CT_char32_t:	muiTypeKeywords |= CPPTK_CHAR32_T;	break;
	case CT_short:		muiTypeKeywords |= CPPTK_SHORT;		break;
	case CT_int:		muiTypeKeywords |= CPPTK_INT;		break;
	case CT_long:		miLongs++;							break;
	case CT_signed:
	case CT___signed__:	muiTypeKeywords |= CPPTK_SIGNED;	break;
	case CT_unsigned:	muiTypeKeywords |= CPPTK_UNSIGNED;	break;
	case CT_float:		muiTypeKeywords |= CPPTK_FLOAT;		break;
	case CT_double:		muiTypeKeywords |= CPPTK_DOUBLE;	break;
	case CT_auto:		muiTypeKeywords |= CPPTK_AUTO;		break;
	case CT___int8:		muiTypeKeywords |= CPPTK_INT8;		break;
	case CT___int16:	muiTypeKeywords |= CPPTK_INT16;		break;
	case CT___int32:	muiTypeKeywords |= CPPTK_INT32;		break;
	case CT___int64:	muiTypeKeywords |= CPPTK_INT64;		break;
	default:													break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintList(pszDest, &macpSpecifiers, " ", iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpSpecifiers);
	pcFields->Integer(&muiFlags);
	pcFields->Integer(&muiTypeKeywords);
	pcFields->Integer(&miLongs);
	pcFields->Child(&mpcType);
}

