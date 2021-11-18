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
	CT_PointerToMember,  //::*
	CT_ScopeResolution, //::
	CT_QuestionMark,
	CT_Colon,
	CT_SemiColon,
	CT_MemberByPointer,  //->
	CT_Member,  //.
	CT_MemberByPointerFunctionPointerIndirection,  //->*
	CT_MemberFunctionPointerIndirection,  //.*
	CT_Elipsis,
	CT_Comma,
	CT_constauto,
	CT_double,
	CT_float,
	CT_int,
	CT_short,
	CT_struct,
	CT_unsigned,
	CT_break,
	CT_continue,
	CT_else,
	CT_for,
	CT_long,
	CT_signed,
	CT_switch,
	CT_void,
	CT_case,
	CT_default,
	CT_enum,
	CT_goto,
	CT_register,
	CT_sizeof,
	CT_typedef,
	CT_volatile,
	CT_char,
	CT_do,
	CT_extern,
	CT_if,
	CT_return,
	CT_static,
	CT_union,
	CT_while,
	CT_asm,
	CT_dynamic_cast,
	CT_namespace,
	CT_reinterpret_cast,
	CT_try,
	CT_bool,
	CT_explicit,
	CT_new,
	CT_static_cast,
	CT_typeid,
	CT_catch,
	CT_false,
	CT_operator,
	CT_template,
	CT_typename,
	CT_class,
	CT_friend,
	CT_private,
	CT_this,
	CT_using,
	CT_const_cast,
	CT_inline,
	CT_public,
	CT_throw,
	CT_virtual,
	CT_delete,
	CT_mutable,
	CT_protected,
	CT_true,
	CT_wchar_t,
	CT_and,
	CT_bitand,
	CT_compl,
	CT_not_eq,
	CT_or_eq,
	CT_xor_eq,
	CT_and_eq,
	CT_bitor,
	CT_not,
	CT_or,
	CT_xor,
	NUM_CPP_TOKENS
};


#endif // !__C_P_P_TOKENS_H__

