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
#ifndef __C_P_P_TOKENS_H__
#define __C_P_P_TOKENS_H__


//Punctuators [lex.operators] and keywords [lex.key] of C++14, and the Microsoft and GNU keywords that the Windows
//headers use.  The alternative tokens (and, bitor, ...) and digraphs (<:, %>, ...) are the punctuators they stand for.
//The order must match gasCPPReservedWords in CppTokeniser.cpp.
enum ECppTokens
{
	CT_OpenRoundBracket,
	CT_CloseRoundBracket,
	CT_OpenCurlyBracket,
	CT_CloseCurlyBracket,
	CT_OpenSquareBracket,
	CT_CloseSquareBracket,
	CT_AssignmentLeftShift,
	CT_AssignmentRightShift,
	CT_Increment,
	CT_Decrement,
	CT_EqualTo,
	CT_NotEqualTo,
	CT_GreaterThanEqualTo,
	CT_LessThanEqualTo,
	CT_LogicalOr,
	CT_LogicalAnd,
	CT_AssignmentAdd,
	CT_AssignmentSubtract,
	CT_AssignmentMultiply,
	CT_AssignmentDivide,
	CT_AssignmentModulus,
	CT_AssignmentBitwiseAnd,
	CT_AssignmentBitwiseOr,
	CT_AssignmentBitwiseXor,
	CT_LeftShift,
	CT_RightShift,
	CT_Add,
	CT_Subtract,
	CT_Multiply,
	CT_Divide,
	CT_Modulus,
	CT_Assignment,
	CT_LogicalNot,
	CT_BitwiseAnd,
	CT_BitwiseOr,
	CT_BitwiseXor,
	CT_LessThan,
	CT_GreaterThan,
	CT_BitwiseNot,
	CT_ScopeResolution,		//::
	CT_QuestionMark,
	CT_Colon,
	CT_SemiColon,
	CT_MemberByPointer,		//->
	CT_Member,				//.
	CT_MemberByPointerFunctionPointerIndirection,  //->*
	CT_MemberFunctionPointerIndirection,  //.*
	CT_Elipsis,
	CT_Comma,
	CT_Hash,				//Only in skipped Microsoft __pragma and malformed code.
	CT_HashHash,

	CT_alignas,
	CT_alignof,
	CT_asm,
	CT_auto,
	CT_bool,
	CT_break,
	CT_case,
	CT_catch,
	CT_char,
	CT_char16_t,
	CT_char32_t,
	CT_class,
	CT_const,
	CT_constexpr,
	CT_const_cast,
	CT_continue,
	CT_decltype,
	CT_default,
	CT_delete,
	CT_do,
	CT_double,
	CT_dynamic_cast,
	CT_else,
	CT_enum,
	CT_explicit,
	CT_export,
	CT_extern,
	CT_false,
	CT_float,
	CT_for,
	CT_friend,
	CT_goto,
	CT_if,
	CT_inline,
	CT_int,
	CT_long,
	CT_mutable,
	CT_namespace,
	CT_new,
	CT_noexcept,
	CT_nullptr,
	CT_operator,
	CT_private,
	CT_protected,
	CT_public,
	CT_register,
	CT_reinterpret_cast,
	CT_return,
	CT_short,
	CT_signed,
	CT_sizeof,
	CT_static,
	CT_static_assert,
	CT_static_cast,
	CT_struct,
	CT_switch,
	CT_template,
	CT_this,
	CT_thread_local,
	CT_throw,
	CT_true,
	CT_try,
	CT_typedef,
	CT_typeid,
	CT_typename,
	CT_union,
	CT_unsigned,
	CT_using,
	CT_virtual,
	CT_void,
	CT_volatile,
	CT_wchar_t,
	CT_while,

	//Microsoft.
	CT___declspec,
	CT___cdecl,
	CT___stdcall,
	CT___fastcall,
	CT___thiscall,
	CT___vectorcall,
	CT___int8,
	CT___int16,
	CT___int32,
	CT___int64,
	CT___w64,
	CT___ptr32,
	CT___ptr64,
	CT___unaligned,
	CT___restrict,
	CT___forceinline,
	CT___inline,
	CT___pragma,
	CT___uuidof,

	//GNU.
	CT___attribute__,
	CT___extension__,
	CT___restrict__,
	CT___inline__,
	CT___asm__,
	CT___volatile__,
	CT___const__,
	CT___signed__,
	CT___typeof__,
	CT___alignof__,
	CT___thread,

	NUM_CPP_TOKENS
};


#endif // !__C_P_P_TOKENS_H__

