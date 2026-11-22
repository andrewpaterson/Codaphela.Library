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
#ifndef __C_P_P_PUNCTUATOR_LIST_H__
#define __C_P_P_PUNCTUATOR_LIST_H__


//The punctuators, each X(spelling, token, what kind of operator it is, operation): first one spelling of each token, in
//ECppTokens order (how the token prints, CppTokeniser.cpp), then the alternative spellings [lex.digraph], which are
//X(spelling, token).  The preprocessor finds them in its text (Punctuators.cpp) and the tokeniser makes their tokens.  The
//kind is CPPOF_ASSIGNMENT for = and the compound assignments and CPPOF_COMPARISON for the six comparisons (IsCPPAssignment,
//IsCPPComparison); the operation is the op of op= (CPPCompoundOperation), and the token itself for the rest.
#define CPP_PUNCTUATORS(X) \
	X("(", CT_OpenRoundBracket, 0, CT_OpenRoundBracket) \
	X(")", CT_CloseRoundBracket, 0, CT_CloseRoundBracket) \
	X("{", CT_OpenCurlyBracket, 0, CT_OpenCurlyBracket) \
	X("}", CT_CloseCurlyBracket, 0, CT_CloseCurlyBracket) \
	X("[", CT_OpenSquareBracket, 0, CT_OpenSquareBracket) \
	X("]", CT_CloseSquareBracket, 0, CT_CloseSquareBracket) \
	X("<<=", CT_AssignmentLeftShift, CPPOF_ASSIGNMENT, CT_LeftShift) \
	X(">>=", CT_AssignmentRightShift, CPPOF_ASSIGNMENT, CT_RightShift) \
	X("++", CT_Increment, 0, CT_Increment) \
	X("--", CT_Decrement, 0, CT_Decrement) \
	X("==", CT_EqualTo, CPPOF_COMPARISON, CT_EqualTo) \
	X("!=", CT_NotEqualTo, CPPOF_COMPARISON, CT_NotEqualTo) \
	X(">=", CT_GreaterThanEqualTo, CPPOF_COMPARISON, CT_GreaterThanEqualTo) \
	X("<=", CT_LessThanEqualTo, CPPOF_COMPARISON, CT_LessThanEqualTo) \
	X("||", CT_LogicalOr, 0, CT_LogicalOr) \
	X("&&", CT_LogicalAnd, 0, CT_LogicalAnd) \
	X("+=", CT_AssignmentAdd, CPPOF_ASSIGNMENT, CT_Add) \
	X("-=", CT_AssignmentSubtract, CPPOF_ASSIGNMENT, CT_Subtract) \
	X("*=", CT_AssignmentMultiply, CPPOF_ASSIGNMENT, CT_Multiply) \
	X("/=", CT_AssignmentDivide, CPPOF_ASSIGNMENT, CT_Divide) \
	X("%=", CT_AssignmentModulus, CPPOF_ASSIGNMENT, CT_Modulus) \
	X("&=", CT_AssignmentBitwiseAnd, CPPOF_ASSIGNMENT, CT_BitwiseAnd) \
	X("|=", CT_AssignmentBitwiseOr, CPPOF_ASSIGNMENT, CT_BitwiseOr) \
	X("^=", CT_AssignmentBitwiseXor, CPPOF_ASSIGNMENT, CT_BitwiseXor) \
	X("<<", CT_LeftShift, 0, CT_LeftShift) \
	X(">>", CT_RightShift, 0, CT_RightShift) \
	X("+", CT_Add, 0, CT_Add) \
	X("-", CT_Subtract, 0, CT_Subtract) \
	X("*", CT_Multiply, 0, CT_Multiply) \
	X("/", CT_Divide, 0, CT_Divide) \
	X("%", CT_Modulus, 0, CT_Modulus) \
	X("=", CT_Assignment, CPPOF_ASSIGNMENT, CT_Assignment) \
	X("!", CT_LogicalNot, 0, CT_LogicalNot) \
	X("&", CT_BitwiseAnd, 0, CT_BitwiseAnd) \
	X("|", CT_BitwiseOr, 0, CT_BitwiseOr) \
	X("^", CT_BitwiseXor, 0, CT_BitwiseXor) \
	X("<", CT_LessThan, CPPOF_COMPARISON, CT_LessThan) \
	X(">", CT_GreaterThan, CPPOF_COMPARISON, CT_GreaterThan) \
	X("~", CT_BitwiseNot, 0, CT_BitwiseNot) \
	X("::", CT_ScopeResolution, 0, CT_ScopeResolution) \
	X("?", CT_QuestionMark, 0, CT_QuestionMark) \
	X(":", CT_Colon, 0, CT_Colon) \
	X(";", CT_SemiColon, 0, CT_SemiColon) \
	X("->", CT_MemberByPointer, 0, CT_MemberByPointer) \
	X(".", CT_Member, 0, CT_Member) \
	X("->*", CT_MemberByPointerFunctionPointerIndirection, 0, CT_MemberByPointerFunctionPointerIndirection) \
	X(".*", CT_MemberFunctionPointerIndirection, 0, CT_MemberFunctionPointerIndirection) \
	X("...", CT_Elipsis, 0, CT_Elipsis) \
	X(",", CT_Comma, 0, CT_Comma) \
	X("#", CT_Hash, 0, CT_Hash) \
	X("##", CT_HashHash, 0, CT_HashHash)


#define CPP_ALTERNATIVE_PUNCTUATORS(X) \
	X("<:", CT_OpenSquareBracket) \
	X(":>", CT_CloseSquareBracket) \
	X("<%", CT_OpenCurlyBracket) \
	X("%>", CT_CloseCurlyBracket) \
	X("%:", CT_Hash) \
	X("%:%:", CT_HashHash)


#endif // __C_P_P_PUNCTUATOR_LIST_H__
