/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __SIMPLE_OPERATOR_H__
#define __SIMPLE_OPERATOR_H__


enum ESimpleOperators
{
	SO_AssignmentLeftShift,
	SO_AssignmentRightShift,
	SO_Increment,
	SO_Decrement,
	SO_EqualTo,
	SO_NotEqualTo,
	SO_GreaterThanEqualTo,
	SO_LessThanEqualTo,
	SO_LogicalOr,
	SO_LogicalAnd,
	SO_AssignmentAdd,
	SO_AssignmentSubtract,
	SO_AssignmentMultiply,
	SO_AssignmentDivide,
	SO_AssignmentModulus,
	SO_AssignmentBitwiseAnd,
	SO_AssignmentBitwiseOr,
	SO_AssignmentBitwiseXor,
	SO_LeftShift,
	SO_RightShift,
	SO_Add,
	SO_Subtract,
	SO_Multiply,
	SO_Divide,
	SO_Modulus,
	SO_Assignment,
	SO_LogicalNot,
	SO_BitwiseAnd,
	SO_BitwiseOr,
	SO_BitwiseXor,
	SO_LessThan,
	SO_GreaterThan,
	SO_BitwiseNot,
	SO_TypeID,
	SO_ConstCast,
	SO_DynamicCast,
	SO_ReinterpretCast,
	SO_StaticCast,
	SO_SizeOf,
	SO_New,
	SO_Delete,
	SO_Throw,
	SO_PointerToMember,  //::*  WTF?
	SO_ScopeResolution,
	SO_TernaryQuestionMark,
	SO_TernaryColon,

	SO_MemberByPointer,  //->
	SO_Member,  //.
	SO_MemberByPointerFunctionPointerIndirection,  //->*
	SO_MemberFunctionPointerIndirection,  //.*

	NUM_SIMPLE_OPERATORS,
	SO_invalid,
	SO_TypeCast,
	SO_Array,
};


#endif // !__SIMPLE_OPERATOR_H__

