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
#ifndef __ABSTRACT_SYNTAX_TYPES_H__
#define __ABSTRACT_SYNTAX_TYPES_H__


enum AbstractSyntaxTypes
{
	EAST_TemplateDecl,
	EAST_ListDecl,
	EAST_NoStatement,
	EAST_SClass, //Specifier: Class
	EAST_NoSpecifier,
	EAST_Name,
	EAST_NoName,
	EAST_NoBaseClass,
	EAST_ClassType,
	EAST_TypenameParam,
	EAST_NoType,
	EAST_TemplateParam,
	EAST_ClassParam,
	EAST_DeclParam,
	EAST_NoTemplate,
	EAST_SInt,
	EAST_SChar,
	EAST_VarDecl,
	EAST_SimpleType,
	EAST_Local,
	EAST_Array,
	EAST_CharConst,
	EAST_Expression,
	EAST_NoExpression,
	EAST_IntConst,
	EAST_BinaryExpr,
	EAST_InitDecl,
	EAST_FunctionDecl,
	EAST_UnaryExpr,
	EAST_SVoid,
	EAST_AssignExpr,
	EAST_ExprStmt,
	EAST_CallExpr,
	EAST_ParenthesisExpr,
	EAST_CompoundStmt


};


#endif // __ABSTRACT_SYNTAX_TYPES_H__

