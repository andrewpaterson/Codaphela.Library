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
#include "CTIdentifier.h"
#include "CppParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclarator* CCPPParser::ParseDeclarator(CCPPSyntax* pcParent, ECPPDeclaratorKind eKind, ECPPDeclarationContext eContext)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntaxKeywordSpecifier*	pcKeyword;
	CCPPSyntaxAttribute*		pcAttribute;
	CCPPSyntaxFunctionSuffix*	pcFunction;
	CCPPSyntaxArraySuffix*		pcArray;
	CCPPScope*					pcQualifier;
	bool						bFunctionAllowed;
	CCPPSyntaxName*				pcName;

	//[dcl.decl]
	pcDeclarator = mpcMemory->Create<CCPPSyntaxDeclarator>(pcParent);
	pcDeclarator->miFirstToken = CurrentToken();
	pcQualifier = NULL;

	for (;;)
	{
		if (IsAttributeStart())
		{
			pcAttribute = ParseAttribute(pcDeclarator);
			if (!pcAttribute)
			{
				return NULL;
			}
			pcDeclarator->macpPrefix.Add(pcAttribute);
		}
		else if (Is(CT___cdecl) || Is(CT___stdcall) || Is(CT___fastcall) || Is(CT___thiscall) || Is(CT___vectorcall))
		{
			//Calling conventions, as in void (__stdcall *pf)(void).
			pcKeyword = mpcMemory->Create<CCPPSyntaxKeywordSpecifier>(pcDeclarator);
			pcKeyword->meKeyword = ((CCTReservedWord*)Peek(0))->GetToken();
			pcKeyword->SetTokens(CurrentToken(), CurrentToken());
			pcDeclarator->macpPrefix.Add(pcKeyword);
			Next();
		}
		else if (IsPointerOperatorStart())
		{
			pcPointer = ParsePointerOperator(pcDeclarator);
			if (!pcPointer)
			{
				return NULL;
			}
			pcDeclarator->macpPrefix.Add(pcPointer);
		}
		else
		{
			break;
		}
	}

	if (Accept(CT_Elipsis))
	{
		//A parameter pack, as in Args... args.  [temp.variadic]
		pcDeclarator->mbPack = true;
	}

	if (Is(CT_OpenRoundBracket) && IsNestedDeclarator(eKind))
	{
		Next();
		pcDeclarator->mpcNested = ParseDeclarator(pcDeclarator, eKind, eContext);
		if (!pcDeclarator->mpcNested || !Expect(CT_CloseRoundBracket))
		{
			return NULL;
		}
	}
	else if ((eKind != CPPDK_Abstract) && IsNameStart())
	{
		pcDeclarator->mpcName = ParseName(pcDeclarator, true, &pcQualifier);
		if (!pcDeclarator->mpcName)
		{
			return NULL;
		}
	}
	else if (eKind == CPPDK_Named)
	{
		Error("Expected a declarator.");
		return NULL;
	}

	//A '(' after a simple name in a block or namespace is a function declarator only if it holds parameters; otherwise
	//it starts an initialiser, as in CChars sz(szOther).  [dcl.ambig.res]
	pcName = pcDeclarator->mpcName;
	for (;;)
	{
		if (Is(CT_OpenRoundBracket))
		{
			bFunctionAllowed = true;
			if (pcName && pcName->IsSimple() && (eKind == CPPDK_Named) && (eContext != CPPDC_Class) && (pcDeclarator->macpSuffixes.NumElements() == 0))
			{
				bFunctionAllowed = IsParameterClause();
			}
			else if ((eKind != CPPDK_Named) && !pcDeclarator->mpcNested && !pcName && (pcDeclarator->macpSuffixes.NumElements() == 0) && (eContext != CPPDC_Parameter) && (eContext != CPPDC_TypeId))
			{
				bFunctionAllowed = IsParameterClause();
			}

			if (!bFunctionAllowed)
			{
				break;
			}

			pcFunction = ParseFunctionSuffix(pcDeclarator, pcQualifier);
			if (!pcFunction)
			{
				return NULL;
			}
			pcDeclarator->macpSuffixes.Add(pcFunction);
		}
		else if (Is(CT_OpenSquareBracket) && !IsAhead(1, CT_OpenSquareBracket))
		{
			pcArray = ParseArraySuffix(pcDeclarator);
			if (!pcArray)
			{
				return NULL;
			}
			pcDeclarator->macpSuffixes.Add(pcArray);
		}
		else
		{
			break;
		}
	}

	ParseAttributes(&pcDeclarator->macpAttributes, pcDeclarator);
	pcDeclarator->miLastToken = PreviousToken();
	return pcDeclarator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsPointerOperatorStart(void)
{
	return Is(CT_Multiply) || Is(CT_BitwiseAnd) || Is(CT_LogicalAnd) || IsMemberPointerAhead();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsMemberPointerAhead(void)
{
	int		i;
	int		iDepth;

	//X::* or A::B<C>::*  [dcl.mptr]
	i = 0;
	if (IsAhead(0, CT_ScopeResolution))
	{
		i++;
	}
	for (;;)
	{
		if (!IsIdentifierAhead(i))
		{
			return false;
		}
		i++;
		if (IsAhead(i, CT_LessThan))
		{
			iDepth = 0;
			for (;;)
			{
				if (!Peek(i))
				{
					return false;
				}
				if (IsAhead(i, CT_LessThan))
				{
					iDepth++;
				}
				else if (IsAhead(i, CT_GreaterThan))
				{
					iDepth--;
				}
				else if (IsAhead(i, CT_RightShift))
				{
					iDepth -= 2;
				}
				else if (IsAhead(i, CT_SemiColon) || IsAhead(i, CT_OpenCurlyBracket))
				{
					return false;
				}
				i++;
				if (iDepth <= 0)
				{
					break;
				}
			}
		}
		if (!IsAhead(i, CT_ScopeResolution))
		{
			return false;
		}
		i++;
		if (IsAhead(i, CT_Multiply))
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxPointerOperator* CCPPParser::ParsePointerOperator(CCPPSyntax* pcParent)
{
	CCPPSyntaxPointerOperator*	pcPointer;

	//[dcl.ptr] [dcl.ref] [dcl.mptr]
	pcPointer = mpcMemory->Create<CCPPSyntaxPointerOperator>(pcParent);
	pcPointer->miFirstToken = CurrentToken();
	if (Accept(CT_Multiply))
	{
		pcPointer->meKind = CPPP_Pointer;
	}
	else if (Accept(CT_BitwiseAnd))
	{
		pcPointer->meKind = CPPP_Reference;
	}
	else if (Accept(CT_LogicalAnd))
	{
		pcPointer->meKind = CPPP_RValueReference;
	}
	else
	{
		pcPointer->meKind = CPPP_MemberPointer;
		pcPointer->mpcClass = ParseName(pcPointer, false, NULL);
		if (!pcPointer->mpcClass || !Expect(CT_ScopeResolution) || !Expect(CT_Multiply))
		{
			return NULL;
		}
	}

	if ((pcPointer->meKind == CPPP_Pointer) || (pcPointer->meKind == CPPP_MemberPointer))
	{
		ParseQualifiers(&pcPointer->macpQualifiers, pcPointer, false);
	}
	pcPointer->miLastToken = PreviousToken();
	return pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::ParseQualifiers(CArraySyntax* papcQualifiers, CCPPSyntax* pcParent, bool bRefQualifiers)
{
	CCPPSyntaxKeywordSpecifier*	pcKeyword;
	CCPPSyntaxAttribute*		pcAttribute;
	ECppTokens					eToken;

	//cv-qualifiers, Microsoft and g++ qualifiers and, after a function's parameters, ref-qualifiers.
	for (;;)
	{
		if (IsAttributeStart() && !Is(CT_OpenSquareBracket))
		{
			pcAttribute = ParseAttribute(pcParent);
			if (!pcAttribute)
			{
				return;
			}
			papcQualifiers->Add(pcAttribute);
			continue;
		}

		if (!Peek(0) || !Peek(0)->IsReservedWord())
		{
			return;
		}
		eToken = ((CCTReservedWord*)Peek(0))->GetToken();
		if ((eToken == CT_const) || (eToken == CT_volatile) || (eToken == CT___const__) || (eToken == CT___volatile__) || (eToken == CT___restrict) || (eToken == CT___restrict__) || (eToken == CT___unaligned) || (eToken == CT___ptr32) || (eToken == CT___ptr64) || (eToken == CT___w64))
		{
		}
		else if (bRefQualifiers && ((eToken == CT_BitwiseAnd) || (eToken == CT_LogicalAnd)))
		{
		}
		else
		{
			return;
		}

		pcKeyword = mpcMemory->Create<CCPPSyntaxKeywordSpecifier>(pcParent);
		pcKeyword->meKeyword = eToken;
		pcKeyword->SetTokens(CurrentToken(), CurrentToken());
		papcQualifiers->Add(pcKeyword);
		Next();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsNestedDeclarator(ECPPDeclaratorKind eKind)
{
	CCPPToken*			pcNext;
	SCPPParserPosition	sPosition;
	bool				bMemberPointer;
	bool				bType;

	//After '(': a nested declarator, as in (*pf)(int), or the parameters of a function type, as in void (int).
	pcNext = Peek(1);
	if (!pcNext)
	{
		return false;
	}

	if (pcNext->Is(CT_Multiply) || pcNext->Is(CT_BitwiseAnd) || pcNext->Is(CT_LogicalAnd) || pcNext->Is(CT___cdecl) || pcNext->Is(CT___stdcall) || pcNext->Is(CT___fastcall) || pcNext->Is(CT___thiscall) || pcNext->Is(CT___vectorcall) || pcNext->Is(CT___attribute__) || pcNext->Is(CT___declspec) || pcNext->Is(CT_OpenRoundBracket))
	{
		return true;
	}

	if (pcNext->Is(CT_ScopeResolution) || pcNext->IsIdentifier())
	{
		sPosition = Mark();
		Next();
		bMemberPointer = IsMemberPointerAhead();
		bType = IsTypeNameAhead();
		Rewind(sPosition);

		if (bMemberPointer || (eKind == CPPDK_Named))
		{
			return true;
		}
		if (eKind == CPPDK_Abstract)
		{
			return false;
		}

		//A parameter may be named in parentheses, but a type name starts the parameters of a function type.
		return !bType;
	}

	if (pcNext->Is(CT_operator) || pcNext->Is(CT_BitwiseNot) || pcNext->Is(CT_Elipsis))
	{
		return (eKind == CPPDK_Named) || pcNext->Is(CT_operator);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsParameterClause(void)
{
	CCPPToken*			pcNext;
	ECppTokens			eToken;
	bool				bType;
	SCPPParserPosition	sPosition;

	//After '(': true if parameters follow.  An empty list makes a function: T f(); declares a function.
	pcNext = Peek(1);
	if (!pcNext)
	{
		return false;
	}

	if (pcNext->Is(CT_CloseRoundBracket) || pcNext->Is(CT_Elipsis))
	{
		return true;
	}

	if (pcNext->IsReservedWord())
	{
		eToken = ((CCTReservedWord*)pcNext)->GetToken();
		if (IsTypeStartKeyword(eToken) || IsDeclSpecifierKeyword(eToken) || (eToken == CT___attribute__) || (eToken == CT___declspec))
		{
			return true;
		}
		return false;
	}

	if (pcNext->IsIdentifier() || pcNext->Is(CT_ScopeResolution))
	{
		sPosition = Mark();
		Next();
		bType = IsTypeNameAhead();
		if (!bType && IsSkimming() && IsIdentifierAhead(0) && (IsIdentifierAhead(1) || IsAhead(1, CT_Multiply) || IsAhead(1, CT_BitwiseAnd) || IsAhead(1, CT_Comma) || IsAhead(1, CT_CloseRoundBracket)))
		{
			//An unknown name in a skimmed header is probably a type.
			bType = true;
		}
		Rewind(sPosition);
		return bType;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxFunctionSuffix* CCPPParser::ParseFunctionSuffix(CCPPSyntax* pcParent, CCPPScope* pcQualifierScope)
{
	CCPPSyntaxFunctionSuffix*		pcFunction;
	CCPPSyntaxParameter*			pcParameter;
	CCPPSyntaxKeywordSpecifier*		pcVirt;
	CCPPScope*						pcScope;
	bool							bDeclaringTemplate;
	bool							bNoGreater;
	int64							lliName;

	//( parameter-declaration-clause ) cv ref-qualifier exception-specification -> trailing-return-type virt-specifiers
	//[dcl.fct]
	pcFunction = mpcMemory->Create<CCPPSyntaxFunctionSuffix>(pcParent);
	pcFunction->miFirstToken = CurrentToken();
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}

	pcScope = PushScope(CPPSC_Function, NULL);
	pcScope->mpcQualifier = pcQualifierScope;
	pcFunction->mpcScope = pcScope;
	bDeclaringTemplate = mbDeclaringTemplate;
	mbDeclaringTemplate = false;
	bNoGreater = mbNoGreater;
	mbNoGreater = false;

	while (!Is(CT_CloseRoundBracket))
	{
		if (Accept(CT_Elipsis))
		{
			pcFunction->mbVariadic = true;
			break;
		}

		pcParameter = ParseParameter(pcFunction);
		if (!pcParameter)
		{
			PopScope(pcScope);
			mbDeclaringTemplate = bDeclaringTemplate;
			mbNoGreater = bNoGreater;
			return NULL;
		}
		pcFunction->macpParameters.Add(pcParameter);

		if (Accept(CT_Comma))
		{
			if (Is(CT_Elipsis))
			{
				pcFunction->mbVariadicComma = true;
			}
			continue;
		}
		if (Accept(CT_Elipsis))
		{
			pcFunction->mbVariadic = true;
		}
		break;
	}

	PopScope(pcScope);
	mbDeclaringTemplate = bDeclaringTemplate;
	mbNoGreater = bNoGreater;
	if (!Expect(CT_CloseRoundBracket))
	{
		return NULL;
	}

	ParseQualifiers(&pcFunction->macpQualifiers, pcFunction, true);

	if (Is(CT_noexcept) || Is(CT_throw))
	{
		pcFunction->mpcExceptionSpecification = ParseExceptionSpecification(pcFunction);
		if (!pcFunction->mpcExceptionSpecification)
		{
			return NULL;
		}
	}

	ParseAttributes(&pcFunction->macpAttributes, pcFunction);

	if (Accept(CT_MemberByPointer))
	{
		pcFunction->mpcTrailingReturn = ParseTypeId(pcFunction);
		if (!pcFunction->mpcTrailingReturn)
		{
			return NULL;
		}
	}

	//override and final are identifiers with a special meaning here.  [class.mem]
	for (;;)
	{
		lliName = GetIdentifier();
		if ((lliName == -1) || ((lliName != mpcTokeniser->mlliOverride) && (lliName != mpcTokeniser->mlliFinal)))
		{
			break;
		}
		pcVirt = mpcMemory->Create<CCPPSyntaxKeywordSpecifier>(pcFunction);
		pcVirt->SetTokens(CurrentToken(), CurrentToken());
		pcFunction->macpVirtSpecifiers.Add(pcVirt);
		Next();
	}

	pcFunction->miLastToken = PreviousToken();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExceptionSpecification* CCPPParser::ParseExceptionSpecification(CCPPSyntax* pcParent)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPSyntaxTypeId*					pcType;

	//noexcept, noexcept(e) and the dynamic throw(T, U).  [except.spec]
	pcSpecification = mpcMemory->Create<CCPPSyntaxExceptionSpecification>(pcParent);
	pcSpecification->miFirstToken = CurrentToken();
	if (Accept(CT_noexcept))
	{
		pcSpecification->mbNoexcept = true;
		if (Accept(CT_OpenRoundBracket))
		{
			pcSpecification->mbParentheses = true;
			pcSpecification->mpcCondition = ParseConstantExpression(pcSpecification);
			if (!pcSpecification->mpcCondition || !Expect(CT_CloseRoundBracket))
			{
				return NULL;
			}
		}
	}
	else
	{
		Expect(CT_throw);
		if (!Expect(CT_OpenRoundBracket))
		{
			return NULL;
		}
		while (!Is(CT_CloseRoundBracket))
		{
			if (Is(CT_Elipsis))
			{
				//Microsoft's throw(...).
				Next();
				break;
			}
			pcType = ParseTypeId(pcSpecification);
			if (!pcType)
			{
				return NULL;
			}
			pcSpecification->macpTypes.Add(pcType);
			if (!Accept(CT_Comma))
			{
				break;
			}
		}
		if (!Expect(CT_CloseRoundBracket))
		{
			return NULL;
		}
	}
	pcSpecification->miLastToken = PreviousToken();
	return pcSpecification;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxParameter* CCPPParser::ParseParameter(CCPPSyntax* pcParent)
{
	CCPPSyntaxParameter*	pcParameter;

	//[dcl.fct] parameter-declaration
	pcParameter = mpcMemory->Create<CCPPSyntaxParameter>(pcParent);
	pcParameter->miFirstToken = CurrentToken();
	pcParameter->mpcSpecifiers = ParseDeclSpecifiers(pcParameter, CPPDC_Parameter);
	if (!pcParameter->mpcSpecifiers)
	{
		return NULL;
	}
	if (pcParameter->mpcSpecifiers->macpSpecifiers.NumElements() == 0)
	{
		Error("Expected a parameter.");
		return NULL;
	}

	pcParameter->mpcDeclarator = ParseDeclarator(pcParameter, CPPDK_Either, CPPDC_Parameter);
	if (!pcParameter->mpcDeclarator)
	{
		return NULL;
	}
	pcParameter->mpcSymbol = DeclareDeclarator(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter, CPPDC_Parameter);

	if (Accept(CT_Assignment))
	{
		pcParameter->mpcDefault = ParseInitializerClause(pcParameter);
		if (!pcParameter->mpcDefault)
		{
			return NULL;
		}
	}
	pcParameter->miLastToken = PreviousToken();
	return pcParameter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxArraySuffix* CCPPParser::ParseArraySuffix(CCPPSyntax* pcParent)
{
	CCPPSyntaxArraySuffix*	pcArray;
	bool					bNoGreater;

	//[ constant-expression ]  [dcl.array]
	pcArray = mpcMemory->Create<CCPPSyntaxArraySuffix>(pcParent);
	pcArray->miFirstToken = CurrentToken();
	Expect(CT_OpenSquareBracket);
	if (!Is(CT_CloseSquareBracket))
	{
		bNoGreater = mbNoGreater;
		mbNoGreater = false;
		pcArray->mpcSize = ParseConstantExpression(pcArray);
		mbNoGreater = bNoGreater;
		if (!pcArray->mpcSize)
		{
			return NULL;
		}
	}
	if (!Expect(CT_CloseSquareBracket))
	{
		return NULL;
	}
	pcArray->miLastToken = PreviousToken();
	return pcArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxTypeId* CCPPParser::ParseTypeId(CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeId*	pcType;

	//type-specifier-seq abstract-declarator  [dcl.name]
	pcType = mpcMemory->Create<CCPPSyntaxTypeId>(pcParent);
	pcType->miFirstToken = CurrentToken();
	pcType->mpcSpecifiers = ParseDeclSpecifiers(pcType, CPPDC_TypeId);
	if (!pcType->mpcSpecifiers)
	{
		return NULL;
	}
	if (!pcType->mpcSpecifiers->mpcType && (pcType->mpcSpecifiers->muiTypeKeywords == 0) && (pcType->mpcSpecifiers->miLongs == 0))
	{
		Error("Expected a type.");
		return NULL;
	}

	pcType->mpcDeclarator = ParseDeclarator(pcType, CPPDK_Abstract, CPPDC_TypeId);
	if (!pcType->mpcDeclarator)
	{
		return NULL;
	}
	pcType->miLastToken = PreviousToken();
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxTypeId* CCPPParser::ParseConversionTypeId(CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeId*			pcType;
	CCPPSyntaxPointerOperator*	pcPointer;

	//The type of operator T: type specifiers and pointer operators only, so operator int() parses.  [class.conv.fct]
	pcType = mpcMemory->Create<CCPPSyntaxTypeId>(pcParent);
	pcType->miFirstToken = CurrentToken();
	pcType->mpcSpecifiers = ParseDeclSpecifiers(pcType, CPPDC_TypeId);
	if (!pcType->mpcSpecifiers)
	{
		return NULL;
	}
	if (!pcType->mpcSpecifiers->mpcType && (pcType->mpcSpecifiers->muiTypeKeywords == 0) && (pcType->mpcSpecifiers->miLongs == 0))
	{
		//operator needs an operator or a conversion type: int operator ; has neither.
		if (pcType->mpcSpecifiers->macpSpecifiers.NumElements() == 0)
		{
			Error("Expected an operator.");
		}
		else
		{
			Error("Expected a type.");
		}
		return NULL;
	}
	pcType->mpcDeclarator = mpcMemory->Create<CCPPSyntaxDeclarator>(pcType);
	pcType->mpcDeclarator->miFirstToken = CurrentToken();
	while (Is(CT_Multiply) || Is(CT_BitwiseAnd) || Is(CT_LogicalAnd))
	{
		pcPointer = ParsePointerOperator(pcType->mpcDeclarator);
		if (!pcPointer)
		{
			return NULL;
		}
		pcType->mpcDeclarator->macpPrefix.Add(pcPointer);
	}
	pcType->mpcDeclarator->miLastToken = PreviousToken();
	pcType->miLastToken = PreviousToken();
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsTypeIdStart(void)
{
	CCPPToken*	pcToken;
	ECppTokens	eToken;

	pcToken = Peek(0);
	if (!pcToken)
	{
		return false;
	}

	if (pcToken->IsReservedWord())
	{
		eToken = ((CCTReservedWord*)pcToken)->GetToken();
		return IsTypeStartKeyword(eToken) || (eToken == CT_const) || (eToken == CT_volatile) || (eToken == CT___const__) || (eToken == CT___volatile__);
	}

	if (pcToken->IsIdentifier() || pcToken->Is(CT_ScopeResolution))
	{
		return IsTypeNameAhead();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsTypeIdAhead(ECppTokens eFollowing)
{
	SCPPTentative		sTentative;
	CCPPSyntaxTypeId*	pcType;

	//True if a type-id followed by eFollowing is next, as in sizeof(T) but not sizeof(T() + 1).  Nothing is consumed.
	BeginTentative(&sTentative);
	pcType = ParseTypeId(NULL);
	return EndTentative(&sTentative, (pcType != NULL) && Is(eFollowing));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseTypeIdOrExpression(CCPPSyntax* pcParent, bool bConstant)
{
	//( type-id | expression ) after the '(' of decltype, __typeof__ or alignas: a type-id if one fills the parentheses,
	//otherwise an expression, a constant expression if bConstant.
	if (IsTypeIdStart() && IsTypeIdAhead(CT_CloseRoundBracket))
	{
		return ParseTypeId(pcParent);
	}
	if (bConstant)
	{
		return ParseConstantExpression(pcParent);
	}
	return ParseExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseInitializer(CCPPSyntax* pcParent, ECPPInitializer* peKind)
{
	CCPPSyntaxParenthesisedInitializer*		pcParentheses;
	bool									bNoGreater;

	//= initializer-clause, ( expression-list ) or braced-init-list.  [dcl.init]
	if (Accept(CT_Assignment))
	{
		if (Accept(CT_default))
		{
			*peKind = CPPI_Default;
			return NULL;
		}
		if (Accept(CT_delete))
		{
			*peKind = CPPI_Delete;
			return NULL;
		}
		*peKind = CPPI_Equals;
		return ParseInitializerClause(pcParent);
	}

	if (Is(CT_OpenRoundBracket))
	{
		*peKind = CPPI_Parentheses;
		pcParentheses = mpcMemory->Create<CCPPSyntaxParenthesisedInitializer>(pcParent);
		pcParentheses->miFirstToken = CurrentToken();
		Next();
		bNoGreater = mbNoGreater;
		mbNoGreater = false;
		if (!ParseExpressionList(&pcParentheses->macpElements, pcParentheses, CT_CloseRoundBracket))
		{
			mbNoGreater = bNoGreater;
			return NULL;
		}
		mbNoGreater = bNoGreater;
		if (!Expect(CT_CloseRoundBracket))
		{
			return NULL;
		}
		pcParentheses->miLastToken = PreviousToken();
		return pcParentheses;
	}

	*peKind = CPPI_Braces;
	return ParseBracedInitList(pcParent);
}

