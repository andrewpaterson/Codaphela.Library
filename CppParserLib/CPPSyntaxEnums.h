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
#ifndef __C_P_P_SYNTAX_ENUMS_H__
#define __C_P_P_SYNTAX_ENUMS_H__


enum ECPPFunctionBody
{
	CPPFB_Body,
	CPPFB_Default,		//= default;
	CPPFB_Delete,		//= delete;
	CPPFB_Skipped,		//Not parsed: the tokens are kept.
};


enum ECPPInitializer
{
	CPPI_None,
	CPPI_Equals,		//= x
	CPPI_Parentheses,	//(x)
	CPPI_Braces,		//{x}
	CPPI_Default,		//= default
	CPPI_Delete,		//= delete
};


enum ECPPAccess
{
	CPPA_None,
	CPPA_Public,
	CPPA_Protected,
	CPPA_Private,
};


enum ECPPPointer
{
	CPPP_Pointer,
	CPPP_Reference,
	CPPP_RValueReference,
	CPPP_MemberPointer,
};


enum ECPPNameComponent
{
	CPPNC_Identifier,
	CPPNC_Operator,		//operator+
	CPPNC_Conversion,	//operator int
	CPPNC_Destructor,	//~X
};


//CCPPSyntaxDeclSpecifiers::muiFlags.  [dcl.spec]
#define CPPDS_TYPEDEF			0x00000001
#define CPPDS_FRIEND			0x00000002
#define CPPDS_CONSTEXPR			0x00000004
#define CPPDS_STATIC			0x00000008
#define CPPDS_EXTERN			0x00000010
#define CPPDS_REGISTER			0x00000020
#define CPPDS_MUTABLE			0x00000040
#define CPPDS_THREAD_LOCAL		0x00000080
#define CPPDS_INLINE			0x00000100
#define CPPDS_VIRTUAL			0x00000200
#define CPPDS_EXPLICIT			0x00000400
#define CPPDS_CONST				0x00000800
#define CPPDS_VOLATILE			0x00001000

//CCPPSyntaxDeclSpecifiers::muiTypeKeywords.  [dcl.type.simple]
#define CPPTK_VOID				0x00000001
#define CPPTK_BOOL				0x00000002
#define CPPTK_CHAR				0x00000004
#define CPPTK_WCHAR_T			0x00000008
#define CPPTK_CHAR16_T			0x00000010
#define CPPTK_CHAR32_T			0x00000020
#define CPPTK_SHORT				0x00000040
#define CPPTK_INT				0x00000080
#define CPPTK_SIGNED			0x00000100
#define CPPTK_UNSIGNED			0x00000200
#define CPPTK_FLOAT				0x00000400
#define CPPTK_DOUBLE			0x00000800
#define CPPTK_AUTO				0x00001000
#define CPPTK_INT8				0x00002000
#define CPPTK_INT16				0x00004000
#define CPPTK_INT32				0x00008000
#define CPPTK_INT64				0x00010000


#endif // __C_P_P_SYNTAX_ENUMS_H__

