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
#ifndef __C_P_P_PUNCTUATOR_LIST_H__
#define __C_P_P_PUNCTUATOR_LIST_H__


//The punctuators, each X(spelling, token): first one spelling of each token, in ECppTokens order (how the token prints,
//CppTokeniser.cpp), then the alternative spellings [lex.digraph].  The preprocessor finds them in its text (Punctuators.cpp)
//and the tokeniser makes their tokens.
#define CPP_PUNCTUATORS(X) \
	X("(", CT_OpenRoundBracket) \
	X(")", CT_CloseRoundBracket) \
	X("{", CT_OpenCurlyBracket) \
	X("}", CT_CloseCurlyBracket) \
	X("[", CT_OpenSquareBracket) \
	X("]", CT_CloseSquareBracket) \
	X("<<=", CT_AssignmentLeftShift) \
	X(">>=", CT_AssignmentRightShift) \
	X("++", CT_Increment) \
	X("--", CT_Decrement) \
	X("==", CT_EqualTo) \
	X("!=", CT_NotEqualTo) \
	X(">=", CT_GreaterThanEqualTo) \
	X("<=", CT_LessThanEqualTo) \
	X("||", CT_LogicalOr) \
	X("&&", CT_LogicalAnd) \
	X("+=", CT_AssignmentAdd) \
	X("-=", CT_AssignmentSubtract) \
	X("*=", CT_AssignmentMultiply) \
	X("/=", CT_AssignmentDivide) \
	X("%=", CT_AssignmentModulus) \
	X("&=", CT_AssignmentBitwiseAnd) \
	X("|=", CT_AssignmentBitwiseOr) \
	X("^=", CT_AssignmentBitwiseXor) \
	X("<<", CT_LeftShift) \
	X(">>", CT_RightShift) \
	X("+", CT_Add) \
	X("-", CT_Subtract) \
	X("*", CT_Multiply) \
	X("/", CT_Divide) \
	X("%", CT_Modulus) \
	X("=", CT_Assignment) \
	X("!", CT_LogicalNot) \
	X("&", CT_BitwiseAnd) \
	X("|", CT_BitwiseOr) \
	X("^", CT_BitwiseXor) \
	X("<", CT_LessThan) \
	X(">", CT_GreaterThan) \
	X("~", CT_BitwiseNot) \
	X("::", CT_ScopeResolution) \
	X("?", CT_QuestionMark) \
	X(":", CT_Colon) \
	X(";", CT_SemiColon) \
	X("->", CT_MemberByPointer) \
	X(".", CT_Member) \
	X("->*", CT_MemberByPointerFunctionPointerIndirection) \
	X(".*", CT_MemberFunctionPointerIndirection) \
	X("...", CT_Elipsis) \
	X(",", CT_Comma) \
	X("#", CT_Hash) \
	X("##", CT_HashHash)


#define CPP_ALTERNATIVE_PUNCTUATORS(X) \
	X("<:", CT_OpenSquareBracket) \
	X(":>", CT_CloseSquareBracket) \
	X("<%", CT_OpenCurlyBracket) \
	X("%>", CT_CloseCurlyBracket) \
	X("%:", CT_Hash) \
	X("%:%:", CT_HashHash)


#endif // __C_P_P_PUNCTUATOR_LIST_H__
