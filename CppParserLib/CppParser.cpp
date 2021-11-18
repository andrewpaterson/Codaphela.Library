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
#include "BaseLib/TextParser.h"
#include "CppParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCppParser::Init(void)
{
	mcAST.Init();
	mcParser.Init();

	//Remember to keep this list synchronised with ESimpleTypes
	mszSimpleTypes.Init();
	mszSimpleTypes.Add("int");
	mszSimpleTypes.Add("long");
	mszSimpleTypes.Add("short");
	mszSimpleTypes.Add("char");
	mszSimpleTypes.Add("double");
	mszSimpleTypes.Add("float");
	mszSimpleTypes.Add("bool");
	mszSimpleTypes.Add("unsigned");
	mszSimpleTypes.Add("signed");
	mszSimpleTypes.Add("void");

	if (mszSimpleTypes.NumElements() != NUM_SIMPLE_SPECIFIERS)
	{
		gcUserError.Set("Simple specifiers is out of synch!");
	}

	mszSimpleModifiers.Init();
	mszSimpleModifiers.Add("const");
	mszSimpleModifiers.Add("volatile");
	mszSimpleModifiers.Add("static");
	mszSimpleModifiers.Add("extern");
	mszSimpleModifiers.Add("inline");
	mszSimpleModifiers.Add("register");

	if (mszSimpleModifiers.NumElements() != NUM_SIMPLE_MODIFIERS)
	{
		gcUserError.Set("Simple specifiers is out of synch!");
	}

	mszAccessSpecifiers.Init();
	mszAccessSpecifiers.Add("public");
	mszAccessSpecifiers.Add("protected");
	mszAccessSpecifiers.Add("private");

	mszSimpleOperators.Init();
	mszSimpleOperators.Add("<<=");
	mszSimpleOperators.Add(">>=");
	mszSimpleOperators.Add("++");
	mszSimpleOperators.Add("--");
	mszSimpleOperators.Add("==");
	mszSimpleOperators.Add("!=");
	mszSimpleOperators.Add(">=");
	mszSimpleOperators.Add("<=");
	mszSimpleOperators.Add("||");
	mszSimpleOperators.Add("&&");
	mszSimpleOperators.Add("+=");
	mszSimpleOperators.Add("-=");
	mszSimpleOperators.Add("*=");
	mszSimpleOperators.Add("/=");
	mszSimpleOperators.Add("%=");
	mszSimpleOperators.Add("&=");
	mszSimpleOperators.Add("|=");
	mszSimpleOperators.Add("^=");
	mszSimpleOperators.Add("<<");
	mszSimpleOperators.Add(">>");
	mszSimpleOperators.Add("+");
	mszSimpleOperators.Add("-");
	mszSimpleOperators.Add("*");
	mszSimpleOperators.Add("/");
	mszSimpleOperators.Add("%");
	mszSimpleOperators.Add("=");
	mszSimpleOperators.Add("!");
	mszSimpleOperators.Add("&");
	mszSimpleOperators.Add("|");
	mszSimpleOperators.Add("^");
	mszSimpleOperators.Add(">");
	mszSimpleOperators.Add("<");
	mszSimpleOperators.Add("~");
	mszSimpleOperators.Add("typeid");
	mszSimpleOperators.Add("const_cast");
	mszSimpleOperators.Add("dynamic_cast");
	mszSimpleOperators.Add("reinterpret_cast");
	mszSimpleOperators.Add("static_cast");
	mszSimpleOperators.Add("sizeof");
	mszSimpleOperators.Add("new");
	mszSimpleOperators.Add("delete");
	mszSimpleOperators.Add("throw");
	mszSimpleOperators.Add("::*");
	mszSimpleOperators.Add("::");
	mszSimpleOperators.Add("?");
	mszSimpleOperators.Add(":");
	mszSimpleOperators.Add("->");
	mszSimpleOperators.Add(".");
	mszSimpleOperators.Add("->*");
	mszSimpleOperators.Add(".*");

	if (mszSimpleOperators.NumElements() != NUM_SIMPLE_OPERATORS)
	{
		gcUserError.Set("Simple operators is out of synch!");
	}

/*

	Left-to-Right
q	::			Scope resolution
z	++ --		Postfix increment and decrement
q	()			Function call
q	[]			Array subscripting
q	.			Element selection by reference
q	->			Element selection through pointer
z	typeid		Run-time type information
z	const_cast
z	dynamic_cast
z	reinterpret_cast
z	static_cast

	Right-to-Left
z	++ --		Prefix increment and decrement
z	+ -			Unary plus and minus
z	! ~			Logical NOT and bitwise NOT
q	(type)		Type cast
z	*			Indirection (dereference)
z	&			Address-of (reference)
z	sizeof		Size-of
z	new			Dynamic memory allocation
z	delete		Dynamic memory deallocation

	Left-to-Right
q	.* ->* 		Pointer to member
z	* / % 		Multiplication, division, and modulus
z	+ - 		Addition and subtraction
z	<< >> 		Bitwise left shift and right shift
z	< <=		Relational less than etc..
z	> >= 		Relational greater than etc...
z	== != 		Relational equals and not equals
z	& 			Bitwise AND
z	^ 			Bitwise XOR
z	| 			Bitwise OR
z	&& 			Logical AND
z	|| 			Logical OR

	Right-to-Left
A	c?t:f 		Ternary conditional 
z	=			Direct assignment
z	+= -=		Assignment by sum and difference
z	*= /= %=	Assignment by product, dividend, and remainder
z	<<= >>=		Assignment by bitwise left shift and right shift
z	&= ^= |=	Assignment by bitwise AND, XOR, and OR
z	throw 		Throw operator
	
	Left-to-Right
A	, 			Comma.  This operator can just fuck-off already!

*/

	mszReservedWords.Init();
	mszReservedWords.Add("constauto");
	mszReservedWords.Add("double");
	mszReservedWords.Add("float");
	mszReservedWords.Add("int");
	mszReservedWords.Add("short");
	mszReservedWords.Add("struct");
	mszReservedWords.Add("unsigned");
	mszReservedWords.Add("break");
	mszReservedWords.Add("continue");
	mszReservedWords.Add("else");
	mszReservedWords.Add("for");
	mszReservedWords.Add("long");
	mszReservedWords.Add("signed");
	mszReservedWords.Add("switch");
	mszReservedWords.Add("void");
	mszReservedWords.Add("case");
	mszReservedWords.Add("default");
	mszReservedWords.Add("enum");
	mszReservedWords.Add("goto");
	mszReservedWords.Add("register");
	mszReservedWords.Add("sizeof");
	mszReservedWords.Add("typedef");
	mszReservedWords.Add("volatile");
	mszReservedWords.Add("char");
	mszReservedWords.Add("do");
	mszReservedWords.Add("extern");
	mszReservedWords.Add("if");
	mszReservedWords.Add("return");
	mszReservedWords.Add("static");
	mszReservedWords.Add("union");
	mszReservedWords.Add("while");
	mszReservedWords.Add("asm");
	mszReservedWords.Add("dynamic_cast");
	mszReservedWords.Add("namespace");
	mszReservedWords.Add("reinterpret_cast");
	mszReservedWords.Add("try");
	mszReservedWords.Add("bool");
	mszReservedWords.Add("explicit");
	mszReservedWords.Add("new");
	mszReservedWords.Add("static_cast");
	mszReservedWords.Add("typeid");
	mszReservedWords.Add("catch");
	mszReservedWords.Add("false");
	mszReservedWords.Add("operator");
	mszReservedWords.Add("template");
	mszReservedWords.Add("typename");
	mszReservedWords.Add("class");
	mszReservedWords.Add("friend");
	mszReservedWords.Add("private");
	mszReservedWords.Add("this");
	mszReservedWords.Add("using");
	mszReservedWords.Add("const_cast");
	mszReservedWords.Add("inline");
	mszReservedWords.Add("public");
	mszReservedWords.Add("throw");
	mszReservedWords.Add("virtual");
	mszReservedWords.Add("delete");
	mszReservedWords.Add("mutable");
	mszReservedWords.Add("protected");
	mszReservedWords.Add("true");
	mszReservedWords.Add("wchar_t");
	mszReservedWords.Add("and");
	mszReservedWords.Add("bitand");
	mszReservedWords.Add("compl");
	mszReservedWords.Add("not_eq");
	mszReservedWords.Add("or_eq");
	mszReservedWords.Add("xor_eq");
	mszReservedWords.Add("and_eq");
	mszReservedWords.Add("bitor");
	mszReservedWords.Add("not");
	mszReservedWords.Add("or");
	mszReservedWords.Add("xor");
	
	maszCallStack.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCppParser::Kill(void)
{
	maszCallStack.Kill();
	mszReservedWords.Kill();
	mszSimpleOperators.Kill();
	mszAccessSpecifiers.Kill();
	mszSimpleModifiers.Kill();
	mszSimpleTypes.Kill();
	mcParser.Kill();
	mcAST.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCppParser::Expected(char* szString)
{
	//EngineOutput("Expected: ");
	//EngineOutput(szString);
	//EngineOutput("\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCppParser::DebugPush(char* szMethod)
{
	maszCallStack.Add(szMethod);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCppParser::DebugPop(CCppReturn bSuccessful)
{
	//CChars*		szMethod;
	int			iTail;

	iTail = maszCallStack.NumElements()-1;
	//szMethod = maszCallStack.Get(iTail);
	//if (bSuccessful.IsTrue())
	//{
	//	EngineOutput(szMethod->Text());
	//	EngineOutput("\n");
	//}
	maszCallStack.Remove(iTail);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCppParser::BuildAST(char* szSourze)
{
	CCppReturn	cReturn;

	mcParser.Kill();
	mcParser.Init(szSourze);

	cReturn = NamespaceBody();
	
	mcParser.SkipWhiteSpace();
	if (mcParser.mbOutsideText)
	{
		CChars::Dump("Parsed Succesfully!\n");
	}
	else
	{
		CChars	sz;
		char*	szPos;
		char*	szEnd;
		char*	szStart;

		sz.Init("Parsing Broke (");
		sz.Append(mcParser.Line()+1);
		sz.Append("): ");
		szPos = mcParser.mszParserPos;
		mcParser.FindStartOfLine();
		szStart = mcParser.mszParserPos;
		mcParser.FindEndOfLine();
		szEnd = mcParser.mszParserPos;
		sz.AppendSubString(szPos, szEnd);
		sz.Append("\n");
		sz.AppendSubString(szStart, szEnd);
		sz.Append("\n");
		sz.Dump();
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::BlockBody(void)
{
	PARSE_PUSH("BlockBody");

	CCppReturn	cReturn;

	for (;;)
	{
		cReturn = ClassDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = EnumDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TypedefDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = Using();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = EmptyDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = VariablesDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = Statement();
		if (cReturn.IsTrue())
		{
			continue;
		}
		break;
	}
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassBody(void)
{
	PARSE_PUSH("ClassBody");

	CCppReturn	cReturn;
	BOOL		bDefn;

	for (;;)
	{
		cReturn = ClassAccess();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = ClassDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = EnumDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TypedefDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = Using();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = EmptyDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TemplateDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = VariablesDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = FunctionDecl(&bDefn);
		if (cReturn.IsTrue() && !bDefn)
		{
			continue;
		}
		else if (cReturn.IsTrue() && bDefn)
		{
			cReturn = FunctionDefn();
			if (cReturn.IsTrue())
			{
				continue;
			}
		}
		break;
	}
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::NamespaceBody(void)
{
	PARSE_PUSH("NamespaceBody");

	CCppReturn	cReturn;
	BOOL		bDefn;

	for (;;)
	{
		cReturn = ClassDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = EnumDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TypedefDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = Using();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = EmptyDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = NamespaceDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TemplateDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TemplateCompilation();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = VariablesDecl();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = FunctionDecl(&bDefn);
		if (cReturn.IsTrue() && !bDefn)
		{
			continue;
		}
		else if (cReturn.IsTrue() && bDefn)
		{
			cReturn = FunctionDefn();
			if (cReturn.IsTrue())
			{
				continue;
			}
		}

		break;
	}
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassDecl(void)
{
	PARSE_PUSH("ClassDecl");

	CCppReturn	cReturn;
	TRISTATE	tResult;
	EClassType	eClassType;

	cReturn = ClassSpec(&eClassType);
	if (cReturn.IsTrue())
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			PARSE_POP(CCppReturn::NotFound());
		}

		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());
		}

		cReturn = ClassDefn();
		PARSE_POP(cReturn);
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassDefn(void)
{
	PARSE_PUSH("ClassDefn");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactCharacter(':');
	if (tResult == TRITRUE)
	{
		cReturn = ClassInheritance();
	}

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = ClassBody();
		cReturn = ClosingCurlyColon();
		if (cReturn.IsTrue())
		{
			PARSE_POP(CCppReturn::True());
		}
		else 
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassInheritance(void)
{
	PARSE_PUSH("ClassInheritance");

	CCppReturn			cReturn;
	EAccessSpecifiers	eSpec;
	int					iCount;

	iCount = 0;
	for (;;)
	{
		cReturn = AccessSpecifier(&eSpec);
		cReturn = Name();
		if (cReturn.IsTrue())
		{
			iCount++;
			cReturn = TemplateParameters();
			continue;
		}
		else
		{
			if (iCount >0)
			{
				PARSE_POP(CCppReturn::True());
			}
			else
			{
				PARSE_POP(CCppReturn::NotFound());
			}
		}
	}
	PARSE_POP(CCppReturn::InternalError());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::AccessSpecifier(EAccessSpecifiers* peSimpleType)
{
	PARSE_PUSH("AccessSpecifier");

	TRISTATE		tResult;
	int				i;
	char*			szAccess;

	for (i = 0; i < mszAccessSpecifiers.NumElements(); i++)
	{
		szAccess = mszAccessSpecifiers.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szAccess);
		if (tResult == TRITRUE)
		{
			*peSimpleType = (EAccessSpecifiers)i;
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassAccess(void)
{
	PARSE_PUSH("ClassAccess");

	CCppReturn			cReturn;
	EAccessSpecifiers	eSpec;
	TRISTATE			tResult;

	cReturn = AccessSpecifier(&eSpec);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(':');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected(":");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else 
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EnumDecl(void)
{
	PARSE_PUSH("EnumDecl");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	cReturn = EnumSpec();
	if (cReturn.IsTrue())
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			Expected("Name");
			PARSE_POP(CCppReturn::NotFound());
		}

		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		cReturn = EnumDefn();
		PARSE_POP(cReturn);
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::FunctionPointerDecl(BOOL bAllowsName)
{
	PARSE_PUSH("FunctionPointerDecl");

	CCppReturn		cReturn;
	TRISTATE		tResult;
	ESimpleTypes	eSpec;
	int				iPointers;

	mcParser.PushPosition();

	cReturn = Type(&eSpec, &iPointers, FALSE);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter('(');
		if (tResult == TRITRUE)
		{
			tResult = mcParser.GetExactCharacter('*');
			if (tResult == TRITRUE)
			{
				if (bAllowsName)
				{
					cReturn = Name();
				}
				tResult = mcParser.GetExactCharacter(')');
				if (tResult == TRITRUE)
				{
					cReturn = FunctionParametersDecl(FALSE);
					if (cReturn.IsTrue())
					{
						mcParser.PassPosition();
						PARSE_POP(CCppReturn::True());
					}
				}
			}
		}
	}
	mcParser.PopPosition();
	PARSE_POP(CCppReturn::NotFound());
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::VariablesDecl(void)
{
	PARSE_PUSH("VariablesDecl");

	CCppReturn			cReturn;
	TRISTATE			tResult;

	cReturn = FunctionPointerDecl(TRUE);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected(";");
			PARSE_POP(CCppReturn::NotFound());
		}
	}

	cReturn = ListDecl();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ListDecl(void)
{
	PARSE_PUSH("ListDecl");

	CCppReturn			cReturn;
	TRISTATE			tResult;
	ESimpleTypes		eSpec;
	int					iPointers;

	mcParser.PushPosition();

	cReturn = Type(&eSpec, &iPointers, FALSE);
	if (cReturn.IsTrue())
	{
		for (;;)
		{
			cReturn = SingleVariableDeclaration();
			if (cReturn.IsNotFound())
			{
				mcParser.PopPosition();
				PARSE_POP(cReturn);
			}

			//This is a hack!
			{
				tResult = mcParser.GetExactCharacter('{');
				if (tResult == TRITRUE)
				{
					//Whoops, was a funcion definition.
					mcParser.PopPosition();
					PARSE_POP(CCppReturn::NotFound());
				}
				else if (tResult == TRIERROR)
				{
					mcParser.PopPosition();
					PARSE_POP(CCppReturn::NotFound());
				}
			}

			tResult = mcParser.GetExactCharacter(';');
			if (tResult == TRITRUE)
			{
				mcParser.PassPosition();
				PARSE_POP(CCppReturn::True());
			}
			else if (tResult == TRIERROR)
			{
				mcParser.PopPosition();
				PARSE_POP(CCppReturn::NotFound());
			}

			tResult = mcParser.GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				mcParser.PopPosition();
				PARSE_POP(CCppReturn::NotFound());
			}
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(cReturn);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::SingleVariableDeclaration(void)
{
	PARSE_PUSH("SingleVariableDeclaration");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	cReturn = Name();
	if (cReturn.IsTrue())
	{
		cReturn = ArrayDecl();

		tResult = mcParser.GetExactCharacter('=');
		if (tResult == TRITRUE)
		{
			cReturn = DeclInitialisers();
			PARSE_POP(cReturn);
		}
		else
		{
			//This is either a constructor call or a function definition or a function declaration.  Sucks yes.
			tResult = mcParser.GetExactCharacter('(');
			if (tResult == TRITRUE)
			{
				cReturn = DeclInitialisersContstructor();

				if (cReturn.IsNotFound())
				{
					//This deals with the constructor case
					PARSE_POP(CCppReturn::NotFound());
				}

				tResult = mcParser.GetExactCharacter(')');
				if (tResult == TRITRUE)
				{
					PARSE_POP(CCppReturn::True());
				}
				else
				{
					Expected(")");
					PARSE_POP(CCppReturn::NotFound());
				}
			}
			else
			{
				PARSE_POP(CCppReturn::True());
			}
		}
	}
	else
	{
		PARSE_POP(cReturn);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::DeclInitialisers(void)
{
	PARSE_PUSH("DeclInitialisers");

	TRISTATE	tResult;
	CCppReturn	cReturn;
	int			iCount;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = Aggregate(&iCount);

		tResult = mcParser.GetExactCharacter('}');
		if (tResult != TRITRUE)
		{
			Expected("}");
			PARSE_POP(CCppReturn::NotFound());
		}
		else
		{
			PARSE_POP(CCppReturn::True());
		}
	}
	else
	{
		cReturn = Expression();
		if (cReturn.IsTrue())
		{
			PARSE_POP(cReturn);
		}
		else
		{
			Expected("Expression");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::DeclInitialisersContstructor(void)
{
	PARSE_PUSH("DeclInitialisersContstructor");

	CCppReturn	cReturn;
	int			iCount;

	cReturn = Aggregate(&iCount);
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Aggregate(int* piExpressions)
{
	PARSE_PUSH("Aggregate");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	*piExpressions = 0;
	for (;;)
	{
		cReturn = Expression();
		if (cReturn.IsTrue())
		{
			(*piExpressions)++;
			tResult = mcParser.GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				break;
			}
		}
		else
		{
			if (*piExpressions == 0)
			{
				PARSE_POP(CCppReturn::NotFound());
			}
			break;
		}
	}
	PARSE_POP(CCppReturn::True());	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Name(void)
{
	PARSE_PUSH("Name");

	TRISTATE	tReturn;
	int			iLength;
	char*		sz;
	BOOL		bResult;

	mcParser.PushPosition();
	bResult = ReservedWord();
	if (bResult)
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}
	mcParser.PassPosition();

	tReturn = mcParser.GetIdentifier(NULL, &iLength);
	if (tReturn == TRITRUE)
	{
		if (iLength < MAX_IDENTIFIER)
		{
			mcParser.GetIdentifier(mszScratchPad);
		}
		else
		{
			sz = (char*)malloc(iLength+1);
			mcParser.GetIdentifier(sz);
			free(sz);
		}
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ArrayDecl(void)
{
	PARSE_PUSH("ArrayDecl");

	CCppReturn	cReturn;
	int			iCount;

	iCount = 0;
	for (;;)
	{
		cReturn = ArrayBody();
		if (cReturn.IsTrue())
		{
			iCount++;
		}
		else
		{
			break;
		}
	}

	if (iCount > 0)
	{
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ArrayBody(void)
{
	PARSE_PUSH("ArrayBody");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactCharacter('[');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(']');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			cReturn = Expression();
			if (cReturn.IsTrue())
			{
				tResult = mcParser.GetExactCharacter(']');
				if (tResult == TRITRUE)
				{
					PARSE_POP(CCppReturn::True());
				}
				else
				{
					Expected("]");
					PARSE_POP(CCppReturn::NotFound());
				}
			}
			Expected("Expression");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ArrayOperator(void)
{
	PARSE_PUSH("ArrayBody");

	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacter('[');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(']');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected("]");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Expressable(void)
{
	PARSE_PUSH("Expressable");

	CCppReturn			cReturn;
	ESimpleOperators	eOp;
	ESimpleTypes		eType;
	TRISTATE			tResult;

	cReturn = Constant();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}
	
	cReturn = SimpleOperator(&eOp);
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	//Could be a type cast, a bracketted expression or a function call.
	cReturn = Parentheses();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = ArrayAccess();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = SimpleType(&eType);
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	tResult = mcParser.GetExactIdentifier("this");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCppReturn::True());
	}

	tResult = mcParser.GetExactCharacter(',');
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = Name();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}


	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Expression(void)
{
	PARSE_PUSH("Expression");

	CCppReturn			cReturn;
	int					iCount;

	for (iCount = 0;; iCount++)
	{
		cReturn = Expressable();
		if (cReturn.IsNotFound())
		{
			break;
		}
	}

	if (iCount == 0)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	else
	{
		PARSE_POP(CCppReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ArrayAccess(void)
{
	PARSE_PUSH("ArrayAccess");

	CCppReturn	cReturn;
	int			iCount;

	iCount = 0;
	for (;;)
	{
		cReturn = ArrayBody();
		if (cReturn.IsTrue())
		{
			iCount++;
		}
		else if (cReturn.IsNotFound())
		{
			break;
		}
	}

	if (iCount > 0)
	{
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Reference(void)
{
	PARSE_PUSH("Reference");

	TRISTATE	tResult;
	int			iRef;
	int			iDeref;

	iRef = 0;
	iDeref = 0;
	for (;;)
	{
		tResult = mcParser.GetExactCharacter('*');
		if (tResult == TRITRUE)
		{
			iDeref++;
			continue;
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());
		}
		tResult = mcParser.GetExactCharacter('&');
		if (tResult == TRITRUE)
		{
			iRef++;
			continue;
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());
		}
		break;
	}
	if (iRef+iDeref > 0)
	{
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Constant(void)
{
	PARSE_PUSH("Constant");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	cReturn = BooleanConst();
	if (cReturn.IsTrue())
	{
		PARSE_POP(cReturn);
	}

	cReturn = CharConst();
	if (cReturn.IsTrue())
	{
		PARSE_POP(cReturn);
	}

	cReturn = StringConst();
	if (cReturn.IsTrue())
	{
		PARSE_POP(cReturn);
	}

	cReturn = IntegerConst();
	if (cReturn.IsTrue())
	{
		PARSE_POP(cReturn);
	}

	//Float and Double.  *Must* come after Integer
	cReturn = DoubleConst();
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter('f', FALSE);
		tResult = mcParser.GetExactCharacter('F', FALSE);
		PARSE_POP(CCppReturn::True());
	}

	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::DoubleConst(void)
{
	PARSE_PUSH("DoubleConst");

	double		f;
	TRISTATE		tResult;
	int				iTemp;
	int				iExponent;

	tResult = mcParser.GetFloat(&f);
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCaseInsensitiveCharacter('e', FALSE);
		if (tResult == TRITRUE)
		{
			tResult = mcParser.GetInteger(&iExponent, &iTemp, FALSE);
			if (tResult == TRITRUE)
			{
				PARSE_POP(CCppReturn::True());
			}
			else
			{
				Expected("Integer");
				PARSE_POP(CCppReturn::NotFound());
			}
		}
		else
		{
			PARSE_POP(CCppReturn::True());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::BooleanConst(void)
{
	PARSE_PUSH("BooleanConst");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("true");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCppReturn::True());
	}
	else if (tResult == TRIFALSE)
	{
		tResult = mcParser.GetExactIdentifier("false");
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::CharConst(void)
{
	PARSE_PUSH("CharConst");

	TRISTATE	tResult;
	char		c;

	tResult = mcParser.GetExactCharacter('\'');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetCharacter(&c);
		if (tResult == TRIERROR)
		{
			Expected("Character");
			PARSE_POP(CCppReturn::NotFound());
		}
		tResult = mcParser.GetExactCharacter('\'');
		if (tResult == TRIERROR)
		{
			Expected("'");
			PARSE_POP(CCppReturn::NotFound());
		}
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::StringConst(void)
{
	PARSE_PUSH("StringConst");

	TRISTATE	tResult;
	int			iLength;
	char*		sz;

	tResult = mcParser.GetQuotedCharacterSequence('"', '"', NULL, &iLength);
	if (tResult == TRITRUE)
	{
		if (iLength < MAX_IDENTIFIER)
		{
			mcParser.GetQuotedCharacterSequence('"', '"', mszScratchPad);
		}
		else
		{
			sz = (char*)malloc(iLength+1);
			mcParser.GetQuotedCharacterSequence('"', '"', sz);
			free(sz);
		}
		PARSE_POP(CCppReturn::True());
	}
	else if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	else
	{
		if (mcParser.meError == TPE_SyntaxError)
		{
			Expected("\"");
		}
		PARSE_POP(CCppReturn::NotFound());
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::DecimalInteger(void)
{
	PARSE_PUSH("DecimalInteger");

	TRISTATE				tResult;
	unsigned long long int  ulli;
	int						iSign;
	int						iNumDigits;

	mcParser.PushPosition();

	tResult  = mcParser.GetDigits(&ulli, &iSign, &iNumDigits, TRUE, FALSE);
	if (tResult == TRITRUE)
	{
		//Make sure there are no decimals.
		if (mcParser.mszParserPos[0] == '.')
		{
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}

		mcParser.PassPosition();
		PARSE_POP(CCppReturn::True());
	}
	mcParser.PopPosition();
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::IntegerConst(void)
{
	PARSE_PUSH("IntegerConst");

	unsigned long long int	ulliNumber;
	int						iNumDigits;
	TRISTATE				tResult;
	CCppReturn				cReturn;

	cReturn = DecimalInteger();
	if (cReturn.IsTrue())
	{
		IntegerConstType();
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		tResult = mcParser.GetHexadecimal(&ulliNumber, &iNumDigits);
		if (tResult == TRITRUE)
		{
			IntegerConstType();
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			tResult = mcParser.GetOctal(&ulliNumber, &iNumDigits);
			if (tResult == TRITRUE)
			{
				IntegerConstType();
				PARSE_POP(CCppReturn::True());
			}
			else
			{
				PARSE_POP(CCppReturn::NotFound());
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::IntegerConstType(void)
{
	PARSE_PUSH("IntegerConst");

	int i = 0;

	TRISTATE tResult;
	
	tResult = mcParser.GetExactCaseInsensitiveCharacter('U', FALSE);
	tResult = mcParser.GetExactCaseInsensitiveCharacter('L', FALSE);
	tResult = mcParser.GetExactCaseInsensitiveCharacter('L', FALSE);

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Arguments(void)
{
	PARSE_PUSH("Arguments");

	TRISTATE	tResult;
	CCppReturn	cReturn;
	int			iCount;

	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			Expected(")");
			PARSE_POP(CCppReturn::NotFound());
		}
		
		cReturn = Aggregate(&iCount);
		if (cReturn.IsTrue())
		{
			tResult = mcParser.GetExactCharacter(')');
			if (tResult == TRITRUE)
			{
				PARSE_POP(CCppReturn::True());
			}
			else if (tResult == TRIERROR)
			{
				Expected(")");
				PARSE_POP(CCppReturn::NotFound());
			}
		}
		else
		{
			Expected("Aggregate");
			PARSE_POP(CCppReturn::NotFound());
		}
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Operator(ESimpleOperators* peOp, BOOL bEmptyTypeCast)
{
	PARSE_PUSH("Operator");

	CCppReturn			cReturn;

	cReturn = SimpleOperator(peOp);
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		cReturn = TypeCast(bEmptyTypeCast);
		if (cReturn.IsTrue())
		{
			*peOp = SO_TypeCast;
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			cReturn = ArrayOperator();
			if (cReturn.IsTrue())
			{
				*peOp = SO_Array;
				PARSE_POP(CCppReturn::True());
			}
		}
	}
	*peOp = SO_invalid;
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Parentheses(void)
{
	PARSE_PUSH("Parentheses");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			cReturn = Expression();
			if (cReturn.IsTrue())
			{
				tResult = mcParser.GetExactCharacter(')');
				if (tResult == TRITRUE)
				{
					mcParser.PassPosition();
					PARSE_POP(CCppReturn::True());
				}
			}
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TemplateParameters(void)
{
	PARSE_PUSH("TemplateParameters");

	CCppReturn			cReturn;
	TRISTATE			tResult;
	ESimpleTypes		eSimpleType;
	int					iPointerCount;


	//This is either a constructor call or a function definition or a function declaration.  Sucks yes.
	tResult = mcParser.GetExactCharacter('<');
	if (tResult == TRITRUE)
	{
		for (;;)
		{
			//I bet it's possibe to use a function pointer declaration here too...
			cReturn = Type(&eSimpleType, &iPointerCount, FALSE);
			tResult = mcParser.GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				break;
			}
		}
		tResult = mcParser.GetExactCharacter('>');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected(">");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::FunctionParametersDecl(BOOL bLooksLikeConstructor)
{
	PARSE_PUSH("FunctionParametersDecl");

	CCppReturn			cReturn;
	TRISTATE			tResult;
	int					iParameterCount;

	//This is either a constructor call or a function definition or a function declaration.  Sucks yes.
	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		cReturn = Parameters(&iParameterCount);

		//Parameters can never fail so this is pointless.
		if (cReturn.IsNotFound())
		{
			PARSE_POP(CCppReturn::NotFound());
		}

		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected(")");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::FunctionDecl(BOOL* pbDefn)
{
	PARSE_PUSH("FunctionDecl");

	CCppReturn			cType;
	CCppReturn			cName;
	CCppReturn			cReturn;
	TRISTATE			tResult;
	ESimpleTypes		eSpec;
	int					iPointers;
	BOOL				bLooksLikeConstructor;
	int					iNameCount;

	mcParser.PushPosition();

	*pbDefn = FALSE;

	//Thankfully it's not possible to return a function pointer declaration.
	cType = Type(&eSpec, &iPointers, TRUE);
	cName = CompoundName(&iNameCount, FALSE);

	bLooksLikeConstructor = FALSE;
	if (cName.IsNotFound() && cType.IsTrue() && (eSpec == ST_notsimple))
	{
		//Remember that in this case the type has swallowed the name
		bLooksLikeConstructor = TRUE;
	}

	cReturn = FunctionParametersDecl(bLooksLikeConstructor);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			*pbDefn = FALSE;
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}
		else 
		{
			*pbDefn = TRUE;
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());

	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassSpec(EClassType* pcClassType)
{
	PARSE_PUSH("ClassSpec");

	TRISTATE	tResult;

	*pcClassType = CT_Invalid;
	tResult = mcParser.GetExactIdentifier("class");
	if (tResult == TRITRUE)
	{
		*pcClassType = CT_Class;
		PARSE_POP(CCppReturn::True());
	}
	else if (tResult == TRIFALSE)
	{
		tResult = mcParser.GetExactIdentifier("struct");
		if (tResult == TRITRUE)
		{
			*pcClassType = CT_Struct;
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			tResult = mcParser.GetExactIdentifier("union");
			if (tResult == TRITRUE)
			{
				*pcClassType = CT_Union;
				PARSE_POP(CCppReturn::True());
			}
			else
			{
				PARSE_POP(CCppReturn::NotFound());
			}
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::SimpleType(ESimpleTypes* peSimpleType)
{
	PARSE_PUSH("SimpleType");

	TRISTATE		tResult;
	int				i;
	char*			szSimpleSpec;

	for (i = 0; i < mszSimpleTypes.NumElements(); i++)
	{
		szSimpleSpec = mszSimpleTypes.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szSimpleSpec);
		if (tResult == TRITRUE)
		{
			*peSimpleType = (ESimpleTypes)i;
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::SimpleModifier(ESimpleModifiers* peSimpleModifier)
{
	PARSE_PUSH("SimpleModifier");

	TRISTATE		tResult;
	int				i;
	char*			szSimpleSpec;

	for (i = 0; i < mszSimpleModifiers.NumElements(); i++)
	{
		szSimpleSpec = mszSimpleModifiers.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szSimpleSpec);
		if (tResult == TRITRUE)
		{
			*peSimpleModifier = (ESimpleModifiers)i;
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::SimpleOperator(ESimpleOperators* peOp)
{
	PARSE_PUSH("SimpleOperator");

	TRISTATE		tResult;
	int				i;
	char*			szSimpleOp;

	mcParser.PushPosition();

	for (i = 0; i < mszSimpleOperators.NumElements(); i++)
	{
		szSimpleOp = mszSimpleOperators.Get(i)->Text();
		tResult = mcParser.GetExactCharacterSequence(szSimpleOp);
		if (tResult == TRITRUE)
		{
			*peOp = (ESimpleOperators)i;
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			*peOp = SO_invalid;
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}
	}

	*peOp = SO_invalid;
	mcParser.PopPosition();
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TypeCast(BOOL bEmpty)
{
	PARSE_PUSH("TypeCast");

	TRISTATE			tResult;
	CCppReturn			cReturn;
	
	mcParser.PushPosition();

	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		if (!bEmpty)
		{
			cReturn = TypeOrFunctionPointer(FALSE);
			if (cReturn.IsNotFound())
			{
				Expected("Type");
				mcParser.PopPosition();
				PARSE_POP(CCppReturn::NotFound());
			}
		}
			
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected(")");
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TypeOrFunctionPointer(BOOL bAllowsName)
{
	PARSE_PUSH("TypeOrFunctionPointer");

	CCppReturn			cReturn;
	ESimpleTypes		eType;
	int					iPointerCount;

	mcParser.PushPosition();

	cReturn = FunctionPointerDecl(bAllowsName);
	if (cReturn.IsTrue())
	{
		mcParser.PassPosition();
		PARSE_POP(CCppReturn::True());
	}

	cReturn = Type(&eType, &iPointerCount, FALSE);
	if (cReturn.IsTrue())
	{
		if (bAllowsName)
		{
			cReturn = Name();
		}
		mcParser.PassPosition();
		PARSE_POP(CCppReturn::True());
	}

	mcParser.PopPosition();
	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Type(ESimpleTypes* peSimpleType, int* piPointerCount, BOOL bAllowDesctructor)
{
	PARSE_PUSH("Type");

	CCppReturn			cReturn;
	int					iNameElements;
	int					iSimpleTypes;
	int					iSimpleModifiers;
	ESimpleModifiers	eSimpleModifier;
	BOOL				bRef;

	iSimpleModifiers = 0;
	for (;;)
	{
		cReturn = SimpleModifier(&eSimpleModifier);
		if (cReturn.IsTrue())
		{
			iSimpleModifiers++;
		}
		else
		{
			break;
		}
	}

	iSimpleTypes = 0;
	for (;;)
	{
		cReturn = SimpleType(peSimpleType);
		if (cReturn.IsTrue())
		{
			iSimpleTypes++;
		}
		else
		{
			break;
		}
	}

	if (iSimpleTypes > 0)
	{
		if (iSimpleTypes > 1)
		{
			*peSimpleType = ST_compoundSimple;
		}

		Pointers(piPointerCount);
		PARSE_POP(CCppReturn::True());
	}

	cReturn = CompoundName(&iNameElements, bAllowDesctructor);
	if (cReturn.IsTrue())
	{
		PointersOrReference(piPointerCount, &bRef);	
		*peSimpleType = ST_notsimple;
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		*peSimpleType = ST_invalid;
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Pointers(int* piPointerCount)
{
	PARSE_PUSH("Pointers");

	TRISTATE	tResult;

	*piPointerCount = 0;
	tResult = mcParser.GetExactCharacter('*');
	if (tResult == TRITRUE)
	{
		(*piPointerCount)++;
		for (;;)
		{
			tResult = mcParser.GetExactCharacter('*', FALSE);
			if (tResult != TRITRUE)
			{
				break;
			}
			(*piPointerCount)++;
		}
		PARSE_POP(CCppReturn::True());
	}
	else if (tResult == TRIERROR)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::PointersOrReference(int* piPointerCount, BOOL* pbReference)
{
	PARSE_PUSH("PointersOrReference");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactCharacter('&');
	*piPointerCount = 0;
	*pbReference = FALSE;
	if (tResult == TRITRUE)
	{
		*pbReference = TRUE;
		PARSE_POP(CCppReturn::True());
	}
	else if (tResult == TRIERROR)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Pointers(piPointerCount);
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClosingCurlyColon(void)
{
	PARSE_PUSH("ClosingCurlyColon");

	TRISTATE	tResult;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('}');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected(";");
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Parameter(void)
{
	PARSE_PUSH("Parameter");

	TRISTATE		tResult;
	CCppReturn		cReturn;

	tResult = mcParser.GetExactCharacterSequence("...");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = TypeOrFunctionPointer(TRUE);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter('=');
		if (tResult == TRITRUE)
		{
			cReturn = Expression();
			if (cReturn.IsNotFound())
			{
				Expected("Expression");
				PARSE_POP(CCppReturn::NotFound());
			}
		}
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Parameters(int* piParameterCount)
{
	PARSE_PUSH("Parameters");

	TRISTATE		tResult;
	CCppReturn		cReturn;

	*piParameterCount = 0;
	for (;;)
	{
		cReturn = Parameter();
		if (cReturn.IsTrue())
		{
			tResult = mcParser.GetExactCharacter(',');
			if (tResult == TRITRUE)
			{
				(*piParameterCount)++;
				continue;
			}
			else
				break;
		}

		break;
	}
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::FunctionDefn(void)
{
	PARSE_PUSH("FunctionDefn");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = FunctionBody();

		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected("}");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::FunctionBody(void)
{
	PARSE_PUSH("FunctionBody");

	CCppReturn	cReturn;

	cReturn = BlockBody();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Statement(void)
{
	PARSE_PUSH("Statement");

	CCppReturn	cReturn;

	cReturn = ReturnStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = ContinueStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = BreakStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = ForStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}


	cReturn = ElseStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = IfStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = WhileStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = DoStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = BlockStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	cReturn = ExpressionStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCppReturn::True());
	}

	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ExpressionStatement(void)
{
	PARSE_PUSH("ExpressionStatement");

	CCppReturn	cReturn;

	cReturn = Expression();
	if (cReturn.IsTrue())
	{
		cReturn = EndStatement();
		PARSE_POP(cReturn);
	}

	PARSE_POP(CCppReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ForStatement(void)
{
	PARSE_PUSH("ForStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;
	int			iExpressions;

	tResult = mcParser.GetExactIdentifier("for");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Expression();
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult != TRITRUE)
		{
			Expected(";");
			PARSE_POP(CCppReturn::NotFound());
		}	
	}
	else
	{
		cReturn = ListDecl();
		if (cReturn.IsNotFound())
		{
			tResult = mcParser.GetExactCharacter(';');
			if (tResult != TRITRUE)
			{
				Expected(";");
				PARSE_POP(CCppReturn::NotFound());
			}	
		}
	}

	cReturn = Aggregate(&iExpressions);
	//Doesn't matter what cReturn is.

	tResult = mcParser.GetExactCharacter(';');
	if (tResult != TRITRUE)
	{
		Expected(";");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Aggregate(&iExpressions);
	//Doesn't matter what cReturn is.

	tResult = mcParser.GetExactCharacter(')');
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCppReturn::NotFound());
	}

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::IfStatement(void)
{
	PARSE_PUSH("IfStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("if");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Expression();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(')');
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCppReturn::NotFound());
	}


	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::DoStatement(void)
{
	PARSE_PUSH("DoStatement");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("do");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::WhileStatement(void)
{
	PARSE_PUSH("WhileStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("while");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Expression();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(')');	
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCppReturn::NotFound());
	}

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ReturnStatement(void)
{
	PARSE_PUSH("ReturnStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("return");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Expression();
	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EndStatement(void)
{
	PARSE_PUSH("EndStatement");

	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacter(';');
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCppReturn::True());
	}
	else 
	{
		Expected(";");
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::BlockStatement(void)
{
	PARSE_PUSH("BlockStatement");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = BlockBody();
		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			Expected("}");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCppParser::ReservedWord(void)
{
	TRISTATE		tResult;
	int				i;
	char*			szReservedWord;

	for (i = 0; i < mszReservedWords.NumElements(); i++)
	{
		szReservedWord = mszReservedWords.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szReservedWord);
		if (tResult == TRITRUE)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EnumSpec(void)
{
	PARSE_PUSH("EnumSpec");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("enum");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EnumDefn(void)
{
	PARSE_PUSH("EnumDefn");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = EnumBody();
		cReturn = ClosingCurlyColon();
		if (cReturn.IsTrue())
		{
			PARSE_POP(CCppReturn::True());
		}
		else 
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		cReturn = EnumValue();
		if (cReturn.IsNotFound())
		{
			Expected("Enum Value");
			PARSE_POP(CCppReturn::NotFound());
		}
		PARSE_POP(CCppReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EnumBody(void)
{
	PARSE_PUSH("EnumBody");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	for (;;)
	{
		cReturn = EnumValue();
		if (cReturn.IsTrue())
		{
			tResult = mcParser.GetExactCharacter(',');
			if (tResult == TRIFALSE)
			{
				break;
			}
			else if (tResult == TRIERROR)
			{
				PARSE_POP(CCppReturn::NotFound());
			}
		}
		else
		{
			break;
		}
	}

	//Even an empty body is OK.
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EnumValue(void)
{
	PARSE_PUSH("EnumValue");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	cReturn = Name();
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter('=');
		if (tResult == TRITRUE)
		{
			cReturn = Expression();
			if (cReturn.IsTrue())
			{
				PARSE_POP(CCppReturn::True());
			}
			else 
			{
				Expected("Expression");
				PARSE_POP(CCppReturn::NotFound());
			}
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCppReturn::NotFound());	
		}
		else
		{
			PARSE_POP(CCppReturn::True());
		}
	}
	else
	{
		Expected("Name");
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::CompoundName(int* piNameCount, BOOL bAllowDesctructor)
{
	PARSE_PUSH("CompoundName");

	TRISTATE			tDoubleColon;
	CCppReturn			cName;
	TRISTATE			tDestructor;
	int					iOldCount;
	CCppReturn			cTemplate;
	CCppReturn			cReturn;
	TRISTATE			tOperator;
	ESimpleOperators	eOp;
	ESimpleTypes		eCastType;
	int					iPointers;

	mcParser.PushPosition();

	(*piNameCount) = 0;
	iOldCount = 0;
	for (;;)
	{
		tDoubleColon = mcParser.GetExactCharacterSequence("::*");
		if (tDoubleColon == TRIERROR)
		{
			mcParser.PopPosition();
			PARSE_POP(CCppReturn::NotFound());
		}

		if (tDoubleColon == TRITRUE)
		{
			(*piNameCount)++;
		}
		else
		{
			tDoubleColon = mcParser.GetExactCharacterSequence("::");
			if (tDoubleColon == TRITRUE)
			{
				(*piNameCount)++;
			}
		}

		if (bAllowDesctructor)
		{
			tDestructor = mcParser.GetExactCharacter('~');
			if (tDestructor == TRITRUE)
			{
				(*piNameCount)++;
			}
		}

		//Make sure that we don't get "Text Text".  Must always be seperated eg: "Text::Text".
		if ((iOldCount > 0) && (iOldCount == *piNameCount))
		{
			mcParser.PassPosition();
			PARSE_POP(CCppReturn::True());
		}

		tOperator = mcParser.GetExactIdentifier("operator");
		if (tOperator == TRITRUE)
		{
			cReturn = Type(&eCastType, &iPointers, FALSE);
			cReturn = Operator(&eOp, TRUE);
			if (cReturn.IsNotFound())
			{
				Expected("Operator");
				mcParser.PopPosition();
				PARSE_POP(CCppReturn::NotFound());
			}
			(*piNameCount)++;
		}
		else
		{
			cName = Name();
			if (cName.IsNotFound())
			{
				if (*piNameCount == 0)
				{
					mcParser.PopPosition();
					PARSE_POP(CCppReturn::NotFound());
				}
				else
				{
					mcParser.PassPosition();
					PARSE_POP(CCppReturn::True());
				}
			}
			else if (cName.IsTrue())
			{
				(*piNameCount)++;
				cTemplate = TemplateParameters();
			}
		}

		iOldCount = (*piNameCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::BreakStatement(void)
{
	PARSE_PUSH("BreakStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("break");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ContinueStatement(void)
{
	PARSE_PUSH("ContinueStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("continue");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ElseStatement(void)
{
	PARSE_PUSH("ElseStatement");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("else");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::SwitchStatement(void)
{
	PARSE_PUSH("SwitchStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("switch");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = ExpressionStatement();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(')');
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCppReturn::NotFound());
	}

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::CaseStatement(void)
{
	PARSE_PUSH("CaseStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("case");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	cReturn = ExpressionStatement();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(':');
	if (tResult != TRITRUE)
	{
		Expected(":");
		PARSE_POP(CCppReturn::NotFound());
	}
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::GotoStatement(void)
{
	PARSE_PUSH("GotoStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("goto");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Name();
	if (cReturn.IsNotFound())
	{
		Expected("Name");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TypedefDecl(void)
{
	PARSE_PUSH("TypedefDecl");

	CCppReturn			cReturn;
	TRISTATE			tResult;

	tResult = mcParser.GetExactIdentifier("typedef");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = TypeOrFunctionPointer(TRUE);
	if (cReturn.IsNotFound())
	{
		Expected("Type");
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::LabelStatement(void)
{
	PARSE_PUSH("LabelStatement");

	CCppReturn	cReturn;
	TRISTATE	tResult;

	mcParser.PushPosition();
	cReturn = Name();
	if (cReturn.IsNotFound())
	{
		mcParser.PopPosition();
		PARSE_POP(cReturn);
	}

	tResult = mcParser.GetExactCharacter(':');
	if (tResult == TRITRUE)
	{
		mcParser.PassPosition();
		PARSE_POP(CCppReturn::True());
	}
	else
	{
		Expected(":");
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TemplateDecl(void)
{
	PARSE_PUSH("TemplateDecl");

	CCppReturn			cReturn;

	cReturn = Template();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TemplateCompilation(void)
{
	PARSE_PUSH("Template");

	CCppReturn			cReturn;
	TRISTATE			tResult;

	mcParser.PushPosition();

	tResult = mcParser.GetExactIdentifier("template");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactIdentifier("class");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = TypeOrFunctionPointer(TRUE);
	if (cReturn.IsNotFound())
	{
		Expected("Type");
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(';');
	if (tResult != TRITRUE)
	{
		Expected(";");
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	mcParser.PassPosition();
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Template(void)
{
	PARSE_PUSH("Template");

	CCppReturn			cReturn;
	TRISTATE			tResult;

	mcParser.PushPosition();

	tResult = mcParser.GetExactIdentifier("template");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('<');
	if (tResult != TRITRUE)
	{
		Expected("<");
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = TemplateBody();
	if (cReturn.IsNotFound())
	{
		Expected("TemplateBody");
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('>');
	if (tResult != TRITRUE)
	{
		Expected(">");
		mcParser.PopPosition();
		PARSE_POP(CCppReturn::NotFound());
	}

	mcParser.PassPosition();
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TemplateBody(void)
{
	PARSE_PUSH("TemplateBody");

	CCppReturn	cReturn;
	TRISTATE	tResult;
	int			iCount;

	for (iCount = 0;; iCount++)
	{
		if (iCount > 0)
		{
			tResult = mcParser.GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				break;
			}
		}


		cReturn = ClassParam();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TypenameParam();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = TemplateParam();
		if (cReturn.IsTrue())
		{
			continue;
		}

		cReturn = DeclaratorParam();
		if (cReturn.IsTrue())
		{
			continue;
		}

		break;
	}
	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::ClassParam(void)
{
	PARSE_PUSH("ClassParam");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactIdentifier("class");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Name();

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TypenameParam(void)
{
	PARSE_PUSH("TypenameParam");

	TRISTATE	tResult;
	CCppReturn	cReturn;

	tResult = mcParser.GetExactIdentifier("typename");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Name();

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::TemplateParam(void)
{
	PARSE_PUSH("TemplateParam");

	CCppReturn			cReturn;
	TRISTATE			tResult;

	cReturn = Template();
	if (cReturn.IsNotFound())
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactIdentifier("class");
	if (tResult == TRITRUE)
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			Expected("Name");
			PARSE_POP(CCppReturn::NotFound());
		}
	}

	PARSE_POP(CCppReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::DeclaratorParam(void)
{
	PARSE_PUSH("DeclaratorParam");

	CCppReturn			cReturn;

	cReturn = SingleVariableDeclaration();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::NamespaceDecl(void)
{
	PARSE_PUSH("NamespaceDecl");

	CCppReturn			cReturn;
	TRISTATE			tResult;

	tResult = mcParser.GetExactIdentifier("namespace");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	cReturn = Name();
	if (cReturn.IsNotFound())
	{
		Expected("Name");
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('=');
	if (tResult == TRITRUE)
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			Expected("Name");
			PARSE_POP(CCppReturn::NotFound());
		}
		
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else
		{
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else if (tResult == TRIERROR)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	else
	{
		cReturn = NamespaceDefn();
		if (cReturn.IsNotFound())
		{
			Expected("Namespace Block");
			PARSE_POP(CCppReturn::NotFound());
		}

		PARSE_POP(CCppReturn::True());
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::NamespaceDefn(void)
{
	PARSE_PUSH("NamespaceDefn");

	TRISTATE	tResult;
	CCppReturn	cReturn;


	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = NamespaceBody();
		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCppReturn::True());
		}
		else 
		{
			Expected("}");
			PARSE_POP(CCppReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCppReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::Using(void)
{
	PARSE_PUSH("Using");

	CCppReturn			cReturn;
	TRISTATE			tResult;
	int					iCount;

	tResult = mcParser.GetExactIdentifier("using");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}

	tResult = mcParser.GetExactIdentifier("namespace");
	if (tResult == TRIERROR)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	else if (tResult == TRITRUE)
	{
		cReturn = CompoundName(&iCount, FALSE);
		if (cReturn.IsNotFound())
		{
			Expected("Compound Name");
			PARSE_POP(CCppReturn::NotFound());
		}

		PARSE_POP(CCppReturn::True());
	}
	else
	{
		cReturn = CompoundName(&iCount, FALSE);
		if (cReturn.IsNotFound())
		{
			Expected("Compound Name");
			PARSE_POP(CCppReturn::NotFound());
		}

		PARSE_POP(CCppReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCppReturn CCppParser::EmptyDecl(void)
{
	PARSE_PUSH("EmptyDecl");

	TRISTATE			tResult;

	tResult = mcParser.GetExactCharacter(';');
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCppReturn::NotFound());
	}
	PARSE_POP(CCppReturn::True());
}

