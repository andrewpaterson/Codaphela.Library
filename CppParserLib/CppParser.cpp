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
void CCPPParser::Init(void)
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
void CCPPParser::Kill(void)
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
void CCPPParser::Expected(char* szString)
{
	//EngineOutput("Expected: ");
	//EngineOutput(szString);
	//EngineOutput("\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::DebugPush(char* szMethod)
{
	maszCallStack.Add(szMethod);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::DebugPop(CCPPReturn bSuccessful)
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
void CCPPParser::BuildAST(char* szSourze)
{
	CCPPReturn	cReturn;

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
		sz.Append(mcParser.Line() + 1);
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
CCPPReturn CCPPParser::BlockBody(void)
{
	PARSE_PUSH("BlockBody");

	CCPPReturn	cReturn;

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
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassBody(void)
{
	PARSE_PUSH("ClassBody");

	CCPPReturn	cReturn;
	bool		bDefn;

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
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::NamespaceBody(void)
{
	PARSE_PUSH("NamespaceBody");

	CCPPReturn	cReturn;
	bool		bDefn;

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
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassDecl(void)
{
	PARSE_PUSH("ClassDecl");

	CCPPReturn	cReturn;
	TRISTATE	tResult;
	EClassType	eClassType;

	cReturn = ClassSpec(&eClassType);
	if (cReturn.IsTrue())
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			PARSE_POP(CCPPReturn::NotFound());
		}

		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCPPReturn::NotFound());
		}

		cReturn = ClassDefn();
		PARSE_POP(cReturn);
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassDefn(void)
{
	PARSE_PUSH("ClassDefn");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

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
			PARSE_POP(CCPPReturn::True());
		}
		else 
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassInheritance(void)
{
	PARSE_PUSH("ClassInheritance");

	CCPPReturn			cReturn;
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
				PARSE_POP(CCPPReturn::True());
			}
			else
			{
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
	}
	PARSE_POP(CCPPReturn::InternalError());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::AccessSpecifier(EAccessSpecifiers* peSimpleType)
{
	PARSE_PUSH("AccessSpecifier");

	TRISTATE		tResult;
	size			i;
	char*			szAccess;

	for (i = 0; i < mszAccessSpecifiers.NumElements(); i++)
	{
		szAccess = mszAccessSpecifiers.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szAccess);
		if (tResult == TRITRUE)
		{
			*peSimpleType = (EAccessSpecifiers)i;
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassAccess(void)
{
	PARSE_PUSH("ClassAccess");

	CCPPReturn			cReturn;
	EAccessSpecifiers	eSpec;
	TRISTATE			tResult;

	cReturn = AccessSpecifier(&eSpec);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(':');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected(":");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else 
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EnumDecl(void)
{
	PARSE_PUSH("EnumDecl");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	cReturn = EnumSpec();
	if (cReturn.IsTrue())
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			Expected("Name");
			PARSE_POP(CCPPReturn::NotFound());
		}

		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		cReturn = EnumDefn();
		PARSE_POP(cReturn);
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::FunctionPointerDecl(bool bAllowsName)
{
	PARSE_PUSH("FunctionPointerDecl");

	CCPPReturn		cReturn;
	TRISTATE		tResult;
	ESimpleTypes	eSpec;
	int				iPointers;

	mcParser.PushPosition();

	cReturn = Type(&eSpec, &iPointers, false);
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
					cReturn = FunctionParametersDecl(false);
					if (cReturn.IsTrue())
					{
						mcParser.PassPosition();
						PARSE_POP(CCPPReturn::True());
					}
				}
			}
		}
	}
	mcParser.PopPosition();
	PARSE_POP(CCPPReturn::NotFound());
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::VariablesDecl(void)
{
	PARSE_PUSH("VariablesDecl");

	CCPPReturn			cReturn;
	TRISTATE			tResult;

	cReturn = FunctionPointerDecl(true);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected(";");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}

	cReturn = ListDecl();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ListDecl(void)
{
	PARSE_PUSH("ListDecl");

	CCPPReturn			cReturn;
	TRISTATE			tResult;
	ESimpleTypes		eSpec;
	int					iPointers;

	mcParser.PushPosition();

	cReturn = Type(&eSpec, &iPointers, false);
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
					PARSE_POP(CCPPReturn::NotFound());
				}
				else if (tResult == TRIERROR)
				{
					mcParser.PopPosition();
					PARSE_POP(CCPPReturn::NotFound());
				}
			}

			tResult = mcParser.GetExactCharacter(';');
			if (tResult == TRITRUE)
			{
				mcParser.PassPosition();
				PARSE_POP(CCPPReturn::True());
			}
			else if (tResult == TRIERROR)
			{
				mcParser.PopPosition();
				PARSE_POP(CCPPReturn::NotFound());
			}

			tResult = mcParser.GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				mcParser.PopPosition();
				PARSE_POP(CCPPReturn::NotFound());
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
CCPPReturn CCPPParser::SingleVariableDeclaration(void)
{
	PARSE_PUSH("SingleVariableDeclaration");

	CCPPReturn	cReturn;
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
					PARSE_POP(CCPPReturn::NotFound());
				}

				tResult = mcParser.GetExactCharacter(')');
				if (tResult == TRITRUE)
				{
					PARSE_POP(CCPPReturn::True());
				}
				else
				{
					Expected(")");
					PARSE_POP(CCPPReturn::NotFound());
				}
			}
			else
			{
				PARSE_POP(CCPPReturn::True());
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
CCPPReturn CCPPParser::DeclInitialisers(void)
{
	PARSE_PUSH("DeclInitialisers");

	TRISTATE	tResult;
	CCPPReturn	cReturn;
	int			iCount;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = Aggregate(&iCount);

		tResult = mcParser.GetExactCharacter('}');
		if (tResult != TRITRUE)
		{
			Expected("}");
			PARSE_POP(CCPPReturn::NotFound());
		}
		else
		{
			PARSE_POP(CCPPReturn::True());
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
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::DeclInitialisersContstructor(void)
{
	PARSE_PUSH("DeclInitialisersContstructor");

	CCPPReturn	cReturn;
	int			iCount;

	cReturn = Aggregate(&iCount);
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Aggregate(int* piExpressions)
{
	PARSE_PUSH("Aggregate");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

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
				PARSE_POP(CCPPReturn::NotFound());
			}
			break;
		}
	}
	PARSE_POP(CCPPReturn::True());	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Name(void)
{
	PARSE_PUSH("Name");

	TRISTATE	tReturn;
	size		iLength;
	char*		sz;
	bool		bResult;

	mcParser.PushPosition();
	bResult = ReservedWord();
	if (bResult)
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
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
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ArrayDecl(void)
{
	PARSE_PUSH("ArrayDecl");

	CCPPReturn	cReturn;
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
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ArrayBody(void)
{
	PARSE_PUSH("ArrayBody");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactCharacter('[');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(']');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			cReturn = Expression();
			if (cReturn.IsTrue())
			{
				tResult = mcParser.GetExactCharacter(']');
				if (tResult == TRITRUE)
				{
					PARSE_POP(CCPPReturn::True());
				}
				else
				{
					Expected("]");
					PARSE_POP(CCPPReturn::NotFound());
				}
			}
			Expected("Expression");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ArrayOperator(void)
{
	PARSE_PUSH("ArrayBody");

	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacter('[');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(']');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected("]");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Expressable(void)
{
	PARSE_PUSH("Expressable");

	CCPPReturn			cReturn;
	ESimpleOperators	eOp;
	ESimpleTypes		eType;
	TRISTATE			tResult;

	cReturn = Constant();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}
	
	cReturn = SimpleOperator(&eOp);
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	//Could be a type cast, a bracketted expression or a function call.
	cReturn = Parentheses();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = ArrayAccess();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = SimpleType(&eType);
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	tResult = mcParser.GetExactIdentifier("this");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCPPReturn::True());
	}

	tResult = mcParser.GetExactCharacter(',');
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = Name();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}


	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Expression(void)
{
	PARSE_PUSH("Expression");

	CCPPReturn			cReturn;
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
		PARSE_POP(CCPPReturn::NotFound());
	}
	else
	{
		PARSE_POP(CCPPReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ArrayAccess(void)
{
	PARSE_PUSH("ArrayAccess");

	CCPPReturn	cReturn;
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
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Reference(void)
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
			PARSE_POP(CCPPReturn::NotFound());
		}
		tResult = mcParser.GetExactCharacter('&');
		if (tResult == TRITRUE)
		{
			iRef++;
			continue;
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
		break;
	}
	if (iRef+iDeref > 0)
	{
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Constant(void)
{
	PARSE_PUSH("Constant");

	CCPPReturn	cReturn;
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
		tResult = mcParser.GetExactCharacter('f', false);
		tResult = mcParser.GetExactCharacter('F', false);
		PARSE_POP(CCPPReturn::True());
	}

	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::DoubleConst(void)
{
	PARSE_PUSH("DoubleConst");

	double		f;
	TRISTATE		tResult;
	uint16			iTemp;
	int64			iExponent;

	tResult = mcParser.GetFloat(&f);
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCaseInsensitiveCharacter('e', false);
		if (tResult == TRITRUE)
		{
			tResult = mcParser.GetInteger(&iExponent, &iTemp, false);
			if (tResult == TRITRUE)
			{
				PARSE_POP(CCPPReturn::True());
			}
			else
			{
				Expected("Integer");
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
		else
		{
			PARSE_POP(CCPPReturn::True());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::BooleanConst(void)
{
	PARSE_PUSH("BooleanConst");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("true");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCPPReturn::True());
	}
	else if (tResult == TRIFALSE)
	{
		tResult = mcParser.GetExactIdentifier("false");
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::CharConst(void)
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
			PARSE_POP(CCPPReturn::NotFound());
		}
		tResult = mcParser.GetExactCharacter('\'');
		if (tResult == TRIERROR)
		{
			Expected("'");
			PARSE_POP(CCPPReturn::NotFound());
		}
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::StringConst(void)
{
	PARSE_PUSH("StringConst");

	TRISTATE	tResult;
	size		iLength;
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
		PARSE_POP(CCPPReturn::True());
	}
	else if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
	else
	{
		if (mcParser.meError == TPE_SyntaxError)
		{
			Expected("\"");
		}
		PARSE_POP(CCPPReturn::NotFound());
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::DecimalInteger(void)
{
	PARSE_PUSH("DecimalInteger");

	TRISTATE				tResult;
	uint64  ulli;
	int16					iSign;
	uint16					iNumDigits;

	mcParser.PushPosition();

	tResult  = mcParser.GetDigits(&ulli, &iSign, &iNumDigits, true, false);
	if (tResult == TRITRUE)
	{
		//Make sure there are no decimals.
		if (mcParser.mszParserPos[0] == '.')
		{
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
		}

		mcParser.PassPosition();
		PARSE_POP(CCPPReturn::True());
	}
	mcParser.PopPosition();
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::IntegerConst(void)
{
	PARSE_PUSH("IntegerConst");

	uint64	ulliNumber;
	uint16					iNumDigits;
	TRISTATE				tResult;
	CCPPReturn				cReturn;

	cReturn = DecimalInteger();
	if (cReturn.IsTrue())
	{
		IntegerConstType();
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		tResult = mcParser.GetHexadecimal(&ulliNumber, &iNumDigits);
		if (tResult == TRITRUE)
		{
			IntegerConstType();
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			tResult = mcParser.GetOctal(&ulliNumber, &iNumDigits);
			if (tResult == TRITRUE)
			{
				IntegerConstType();
				PARSE_POP(CCPPReturn::True());
			}
			else
			{
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::IntegerConstType(void)
{
	PARSE_PUSH("IntegerConst");

	int i = 0;

	TRISTATE tResult;
	
	tResult = mcParser.GetExactCaseInsensitiveCharacter('U', false);
	tResult = mcParser.GetExactCaseInsensitiveCharacter('L', false);
	tResult = mcParser.GetExactCaseInsensitiveCharacter('L', false);

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Arguments(void)
{
	PARSE_PUSH("Arguments");

	TRISTATE	tResult;
	CCPPReturn	cReturn;
	int			iCount;

	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			Expected(")");
			PARSE_POP(CCPPReturn::NotFound());
		}
		
		cReturn = Aggregate(&iCount);
		if (cReturn.IsTrue())
		{
			tResult = mcParser.GetExactCharacter(')');
			if (tResult == TRITRUE)
			{
				PARSE_POP(CCPPReturn::True());
			}
			else if (tResult == TRIERROR)
			{
				Expected(")");
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
		else
		{
			Expected("Aggregate");
			PARSE_POP(CCPPReturn::NotFound());
		}
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Operator(ESimpleOperators* peOp, bool bEmptyTypeCast)
{
	PARSE_PUSH("Operator");

	CCPPReturn			cReturn;

	cReturn = SimpleOperator(peOp);
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		cReturn = TypeCast(bEmptyTypeCast);
		if (cReturn.IsTrue())
		{
			*peOp = SO_TypeCast;
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			cReturn = ArrayOperator();
			if (cReturn.IsTrue())
			{
				*peOp = SO_Array;
				PARSE_POP(CCPPReturn::True());
			}
		}
	}
	*peOp = SO_invalid;
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Parentheses(void)
{
	PARSE_PUSH("Parentheses");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			PARSE_POP(CCPPReturn::True());
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
					PARSE_POP(CCPPReturn::True());
				}
			}
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TemplateParameters(void)
{
	PARSE_PUSH("TemplateParameters");

	CCPPReturn			cReturn;
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
			cReturn = Type(&eSimpleType, &iPointerCount, false);
			tResult = mcParser.GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				break;
			}
		}
		tResult = mcParser.GetExactCharacter('>');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected(">");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::FunctionParametersDecl(bool bLooksLikeConstructor)
{
	PARSE_PUSH("FunctionParametersDecl");

	CCPPReturn			cReturn;
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
			PARSE_POP(CCPPReturn::NotFound());
		}

		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected(")");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::FunctionDecl(bool* pbDefn)
{
	PARSE_PUSH("FunctionDecl");

	CCPPReturn			cType;
	CCPPReturn			cName;
	CCPPReturn			cReturn;
	TRISTATE			tResult;
	ESimpleTypes		eSpec;
	int					iPointers;
	bool				bLooksLikeConstructor;
	int					iNameCount;

	mcParser.PushPosition();

	*pbDefn = false;

	//Thankfully it's not possible to return a function pointer declaration.
	cType = Type(&eSpec, &iPointers, true);
	cName = CompoundName(&iNameCount, false);

	bLooksLikeConstructor = false;
	if (cName.IsNotFound() && cType.IsTrue() && (eSpec == ST_notsimple))
	{
		//Remember that in this case the type has swallowed the name
		bLooksLikeConstructor = true;
	}

	cReturn = FunctionParametersDecl(bLooksLikeConstructor);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			*pbDefn = false;
			mcParser.PassPosition();
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
		}
		else 
		{
			*pbDefn = true;
			mcParser.PassPosition();
			PARSE_POP(CCPPReturn::True());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());

	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassSpec(EClassType* pcClassType)
{
	PARSE_PUSH("ClassSpec");

	TRISTATE	tResult;

	*pcClassType = CT_Invalid;
	tResult = mcParser.GetExactIdentifier("class");
	if (tResult == TRITRUE)
	{
		*pcClassType = CT_Class;
		PARSE_POP(CCPPReturn::True());
	}
	else if (tResult == TRIFALSE)
	{
		tResult = mcParser.GetExactIdentifier("struct");
		if (tResult == TRITRUE)
		{
			*pcClassType = CT_Struct;
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			tResult = mcParser.GetExactIdentifier("union");
			if (tResult == TRITRUE)
			{
				*pcClassType = CT_Union;
				PARSE_POP(CCPPReturn::True());
			}
			else
			{
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::SimpleType(ESimpleTypes* peSimpleType)
{
	PARSE_PUSH("SimpleType");

	TRISTATE		tResult;
	size			i;
	char*			szSimpleSpec;

	for (i = 0; i < mszSimpleTypes.NumElements(); i++)
	{
		szSimpleSpec = mszSimpleTypes.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szSimpleSpec);
		if (tResult == TRITRUE)
		{
			*peSimpleType = (ESimpleTypes)i;
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::SimpleModifier(ESimpleModifiers* peSimpleModifier)
{
	PARSE_PUSH("SimpleModifier");

	TRISTATE		tResult;
	size			i;
	char*			szSimpleSpec;

	for (i = 0; i < mszSimpleModifiers.NumElements(); i++)
	{
		szSimpleSpec = mszSimpleModifiers.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szSimpleSpec);
		if (tResult == TRITRUE)
		{
			*peSimpleModifier = (ESimpleModifiers)i;
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::SimpleOperator(ESimpleOperators* peOp)
{
	PARSE_PUSH("SimpleOperator");

	TRISTATE		tResult;
	size			i;
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
			PARSE_POP(CCPPReturn::True());
		}
		else if (tResult == TRIERROR)
		{
			*peOp = SO_invalid;
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
		}
	}

	*peOp = SO_invalid;
	mcParser.PopPosition();
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TypeCast(bool bEmpty)
{
	PARSE_PUSH("TypeCast");

	TRISTATE			tResult;
	CCPPReturn			cReturn;
	
	mcParser.PushPosition();

	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		if (!bEmpty)
		{
			cReturn = TypeOrFunctionPointer(false);
			if (cReturn.IsNotFound())
			{
				Expected("Type");
				mcParser.PopPosition();
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
			
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected(")");
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TypeOrFunctionPointer(bool bAllowsName)
{
	PARSE_PUSH("TypeOrFunctionPointer");

	CCPPReturn			cReturn;
	ESimpleTypes		eType;
	int					iPointerCount;

	mcParser.PushPosition();

	cReturn = FunctionPointerDecl(bAllowsName);
	if (cReturn.IsTrue())
	{
		mcParser.PassPosition();
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = Type(&eType, &iPointerCount, false);
	if (cReturn.IsTrue())
	{
		if (bAllowsName)
		{
			cReturn = Name();
		}
		mcParser.PassPosition();
		PARSE_POP(CCPPReturn::True());
	}

	mcParser.PopPosition();
	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Type(ESimpleTypes* peSimpleType, int* piPointerCount, bool bAllowDesctructor)
{
	PARSE_PUSH("Type");

	CCPPReturn			cReturn;
	int					iNameElements;
	int					iSimpleTypes;
	int					iSimpleModifiers;
	ESimpleModifiers	eSimpleModifier;
	bool				bRef;

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
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = CompoundName(&iNameElements, bAllowDesctructor);
	if (cReturn.IsTrue())
	{
		PointersOrReference(piPointerCount, &bRef);	
		*peSimpleType = ST_notsimple;
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		*peSimpleType = ST_invalid;
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Pointers(int* piPointerCount)
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
			tResult = mcParser.GetExactCharacter('*', false);
			if (tResult != TRITRUE)
			{
				break;
			}
			(*piPointerCount)++;
		}
		PARSE_POP(CCPPReturn::True());
	}
	else if (tResult == TRIERROR)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::PointersOrReference(int* piPointerCount, bool* pbReference)
{
	PARSE_PUSH("PointersOrReference");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactCharacter('&');
	*piPointerCount = 0;
	*pbReference = false;
	if (tResult == TRITRUE)
	{
		*pbReference = true;
		PARSE_POP(CCPPReturn::True());
	}
	else if (tResult == TRIERROR)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Pointers(piPointerCount);
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClosingCurlyColon(void)
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
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected(";");
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Parameter(void)
{
	PARSE_PUSH("Parameter");

	TRISTATE		tResult;
	CCPPReturn		cReturn;

	tResult = mcParser.GetExactCharacterSequence("...");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = TypeOrFunctionPointer(true);
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter('=');
		if (tResult == TRITRUE)
		{
			cReturn = Expression();
			if (cReturn.IsNotFound())
			{
				Expected("Expression");
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Parameters(int* piParameterCount)
{
	PARSE_PUSH("Parameters");

	TRISTATE		tResult;
	CCPPReturn		cReturn;

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
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::FunctionDefn(void)
{
	PARSE_PUSH("FunctionDefn");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = FunctionBody();

		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected("}");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::FunctionBody(void)
{
	PARSE_PUSH("FunctionBody");

	CCPPReturn	cReturn;

	cReturn = BlockBody();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Statement(void)
{
	PARSE_PUSH("Statement");

	CCPPReturn	cReturn;

	cReturn = ReturnStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = ContinueStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = BreakStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = ForStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}


	cReturn = ElseStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = IfStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = WhileStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = DoStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = BlockStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	cReturn = ExpressionStatement();
	if (cReturn.IsTrue())
	{
		PARSE_POP(CCPPReturn::True());
	}

	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ExpressionStatement(void)
{
	PARSE_PUSH("ExpressionStatement");

	CCPPReturn	cReturn;

	cReturn = Expression();
	if (cReturn.IsTrue())
	{
		cReturn = EndStatement();
		PARSE_POP(cReturn);
	}

	PARSE_POP(CCPPReturn::NotFound());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ForStatement(void)
{
	PARSE_PUSH("ForStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;
	int			iExpressions;

	tResult = mcParser.GetExactIdentifier("for");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Expression();
	if (cReturn.IsTrue())
	{
		tResult = mcParser.GetExactCharacter(';');
		if (tResult != TRITRUE)
		{
			Expected(";");
			PARSE_POP(CCPPReturn::NotFound());
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
				PARSE_POP(CCPPReturn::NotFound());
			}	
		}
	}

	cReturn = Aggregate(&iExpressions);
	//Doesn't matter what cReturn is.

	tResult = mcParser.GetExactCharacter(';');
	if (tResult != TRITRUE)
	{
		Expected(";");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Aggregate(&iExpressions);
	//Doesn't matter what cReturn is.

	tResult = mcParser.GetExactCharacter(')');
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCPPReturn::NotFound());
	}

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::IfStatement(void)
{
	PARSE_PUSH("IfStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("if");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Expression();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(')');
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCPPReturn::NotFound());
	}


	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::DoStatement(void)
{
	PARSE_PUSH("DoStatement");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("do");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::WhileStatement(void)
{
	PARSE_PUSH("WhileStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("while");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Expression();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(')');	
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCPPReturn::NotFound());
	}

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ReturnStatement(void)
{
	PARSE_PUSH("ReturnStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("return");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Expression();
	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EndStatement(void)
{
	PARSE_PUSH("EndStatement");

	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacter(';');
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCPPReturn::True());
	}
	else 
	{
		Expected(";");
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::BlockStatement(void)
{
	PARSE_PUSH("BlockStatement");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = BlockBody();
		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			Expected("}");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ReservedWord(void)
{
	TRISTATE		tResult;
	size			i;
	char*			szReservedWord;

	for (i = 0; i < mszReservedWords.NumElements(); i++)
	{
		szReservedWord = mszReservedWords.Get(i)->Text();
		tResult = mcParser.GetExactIdentifier(szReservedWord);
		if (tResult == TRITRUE)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EnumSpec(void)
{
	PARSE_PUSH("EnumSpec");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("enum");
	if (tResult == TRITRUE)
	{
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EnumDefn(void)
{
	PARSE_PUSH("EnumDefn");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = EnumBody();
		cReturn = ClosingCurlyColon();
		if (cReturn.IsTrue())
		{
			PARSE_POP(CCPPReturn::True());
		}
		else 
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		cReturn = EnumValue();
		if (cReturn.IsNotFound())
		{
			Expected("Enum Value");
			PARSE_POP(CCPPReturn::NotFound());
		}
		PARSE_POP(CCPPReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EnumBody(void)
{
	PARSE_PUSH("EnumBody");

	CCPPReturn	cReturn;
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
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
		else
		{
			break;
		}
	}

	//Even an empty body is OK.
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EnumValue(void)
{
	PARSE_PUSH("EnumValue");

	CCPPReturn	cReturn;
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
				PARSE_POP(CCPPReturn::True());
			}
			else 
			{
				Expected("Expression");
				PARSE_POP(CCPPReturn::NotFound());
			}
		}
		else if (tResult == TRIERROR)
		{
			PARSE_POP(CCPPReturn::NotFound());	
		}
		else
		{
			PARSE_POP(CCPPReturn::True());
		}
	}
	else
	{
		Expected("Name");
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::CompoundName(int* piNameCount, bool bAllowDesctructor)
{
	PARSE_PUSH("CompoundName");

	TRISTATE			tDoubleColon;
	CCPPReturn			cName;
	TRISTATE			tDestructor;
	int					iOldCount;
	CCPPReturn			cTemplate;
	CCPPReturn			cReturn;
	TRISTATE			tOperator;
	ESimpleOperators	eOp;
	ESimpleTypes		eCastType;
	int					iPointers;

	mcParser.PushPosition();

	(*piNameCount) =0;
	iOldCount = 0;
	for (;;)
	{
		tDoubleColon = mcParser.GetExactCharacterSequence("::*");
		if (tDoubleColon == TRIERROR)
		{
			mcParser.PopPosition();
			PARSE_POP(CCPPReturn::NotFound());
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
			PARSE_POP(CCPPReturn::True());
		}

		tOperator = mcParser.GetExactIdentifier("operator");
		if (tOperator == TRITRUE)
		{
			cReturn = Type(&eCastType, &iPointers, false);
			cReturn = Operator(&eOp, true);
			if (cReturn.IsNotFound())
			{
				Expected("Operator");
				mcParser.PopPosition();
				PARSE_POP(CCPPReturn::NotFound());
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
					PARSE_POP(CCPPReturn::NotFound());
				}
				else
				{
					mcParser.PassPosition();
					PARSE_POP(CCPPReturn::True());
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
CCPPReturn CCPPParser::BreakStatement(void)
{
	PARSE_PUSH("BreakStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("break");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ContinueStatement(void)
{
	PARSE_PUSH("ContinueStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("continue");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ElseStatement(void)
{
	PARSE_PUSH("ElseStatement");

	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("else");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::SwitchStatement(void)
{
	PARSE_PUSH("SwitchStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("switch");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('(');
	if (tResult != TRITRUE)
	{
		Expected("(");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = ExpressionStatement();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(')');
	if (tResult != TRITRUE)
	{
		Expected(")");
		PARSE_POP(CCPPReturn::NotFound());
	}

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::CaseStatement(void)
{
	PARSE_PUSH("CaseStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("case");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
	cReturn = ExpressionStatement();
	if (cReturn.IsNotFound())
	{
		Expected("Expression");
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(':');
	if (tResult != TRITRUE)
	{
		Expected(":");
		PARSE_POP(CCPPReturn::NotFound());
	}
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::GotoStatement(void)
{
	PARSE_PUSH("GotoStatement");

	CCPPReturn	cReturn;
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("goto");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Name();
	if (cReturn.IsNotFound())
	{
		Expected("Name");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TypedefDecl(void)
{
	PARSE_PUSH("TypedefDecl");

	CCPPReturn			cReturn;
	TRISTATE			tResult;

	tResult = mcParser.GetExactIdentifier("typedef");
	if (tResult == TRIFALSE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = TypeOrFunctionPointer(true);
	if (cReturn.IsNotFound())
	{
		Expected("Type");
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = EndStatement();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::LabelStatement(void)
{
	PARSE_PUSH("LabelStatement");

	CCPPReturn	cReturn;
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
		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		Expected(":");
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TemplateDecl(void)
{
	PARSE_PUSH("TemplateDecl");

	CCPPReturn			cReturn;

	cReturn = Template();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TemplateCompilation(void)
{
	PARSE_PUSH("Template");

	CCPPReturn			cReturn;
	TRISTATE			tResult;

	mcParser.PushPosition();

	tResult = mcParser.GetExactIdentifier("template");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactIdentifier("class");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = TypeOrFunctionPointer(true);
	if (cReturn.IsNotFound())
	{
		Expected("Type");
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter(';');
	if (tResult != TRITRUE)
	{
		Expected(";");
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	mcParser.PassPosition();
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Template(void)
{
	PARSE_PUSH("Template");

	CCPPReturn			cReturn;
	TRISTATE			tResult;

	mcParser.PushPosition();

	tResult = mcParser.GetExactIdentifier("template");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('<');
	if (tResult != TRITRUE)
	{
		Expected("<");
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = TemplateBody();
	if (cReturn.IsNotFound())
	{
		Expected("TemplateBody");
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('>');
	if (tResult != TRITRUE)
	{
		Expected(">");
		mcParser.PopPosition();
		PARSE_POP(CCPPReturn::NotFound());
	}

	mcParser.PassPosition();
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TemplateBody(void)
{
	PARSE_PUSH("TemplateBody");

	CCPPReturn	cReturn;
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
	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::ClassParam(void)
{
	PARSE_PUSH("ClassParam");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactIdentifier("class");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Name();

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TypenameParam(void)
{
	PARSE_PUSH("TypenameParam");

	TRISTATE	tResult;
	CCPPReturn	cReturn;

	tResult = mcParser.GetExactIdentifier("typename");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Name();

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::TemplateParam(void)
{
	PARSE_PUSH("TemplateParam");

	CCPPReturn			cReturn;
	TRISTATE			tResult;

	cReturn = Template();
	if (cReturn.IsNotFound())
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactIdentifier("class");
	if (tResult == TRITRUE)
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			Expected("Name");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}

	PARSE_POP(CCPPReturn::True());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::DeclaratorParam(void)
{
	PARSE_PUSH("DeclaratorParam");

	CCPPReturn			cReturn;

	cReturn = SingleVariableDeclaration();
	PARSE_POP(cReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::NamespaceDecl(void)
{
	PARSE_PUSH("NamespaceDecl");

	CCPPReturn			cReturn;
	TRISTATE			tResult;

	tResult = mcParser.GetExactIdentifier("namespace");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	cReturn = Name();
	if (cReturn.IsNotFound())
	{
		Expected("Name");
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactCharacter('=');
	if (tResult == TRITRUE)
	{
		cReturn = Name();
		if (cReturn.IsNotFound())
		{
			Expected("Name");
			PARSE_POP(CCPPReturn::NotFound());
		}
		
		tResult = mcParser.GetExactCharacter(';');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else
		{
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else if (tResult == TRIERROR)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
	else
	{
		cReturn = NamespaceDefn();
		if (cReturn.IsNotFound())
		{
			Expected("Namespace Block");
			PARSE_POP(CCPPReturn::NotFound());
		}

		PARSE_POP(CCPPReturn::True());
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::NamespaceDefn(void)
{
	PARSE_PUSH("NamespaceDefn");

	TRISTATE	tResult;
	CCPPReturn	cReturn;


	tResult = mcParser.GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		cReturn = NamespaceBody();
		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRITRUE)
		{
			PARSE_POP(CCPPReturn::True());
		}
		else 
		{
			Expected("}");
			PARSE_POP(CCPPReturn::NotFound());
		}
	}
	else
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::Using(void)
{
	PARSE_PUSH("Using");

	CCPPReturn			cReturn;
	TRISTATE			tResult;
	int					iCount;

	tResult = mcParser.GetExactIdentifier("using");
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}

	tResult = mcParser.GetExactIdentifier("namespace");
	if (tResult == TRIERROR)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
	else if (tResult == TRITRUE)
	{
		cReturn = CompoundName(&iCount, false);
		if (cReturn.IsNotFound())
		{
			Expected("Compound Name");
			PARSE_POP(CCPPReturn::NotFound());
		}

		PARSE_POP(CCPPReturn::True());
	}
	else
	{
		cReturn = CompoundName(&iCount, false);
		if (cReturn.IsNotFound())
		{
			Expected("Compound Name");
			PARSE_POP(CCPPReturn::NotFound());
		}

		PARSE_POP(CCPPReturn::True());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPReturn CCPPParser::EmptyDecl(void)
{
	PARSE_PUSH("EmptyDecl");

	TRISTATE			tResult;

	tResult = mcParser.GetExactCharacter(';');
	if (tResult != TRITRUE)
	{
		PARSE_POP(CCPPReturn::NotFound());
	}
	PARSE_POP(CCPPReturn::True());
}

