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
CCPPSyntaxExpression* CCPPParser::ParseExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression*		pcLeft;
	CCPPSyntaxExpression*		pcRight;
	CCPPSyntaxBinaryExpression*	pcComma;

	//expression , assignment-expression  [expr.comma]
	pcLeft = ParseAssignmentExpression(pcParent);
	if (!pcLeft)
	{
		return NULL;
	}

	while (Is(CT_Comma))
	{
		Next();
		pcRight = ParseAssignmentExpression(pcParent);
		if (!pcRight)
		{
			return NULL;
		}
		pcComma = mpcMemory->CreateBinaryExpression(pcParent);
		pcComma->meOperator = CT_Comma;
		pcComma->mpcLeft = pcLeft;
		pcComma->mpcRight = pcRight;
		pcLeft->mpcParent = pcComma;
		pcRight->mpcParent = pcComma;
		pcComma->SetTokens(pcLeft->miFirstToken, PreviousToken());
		pcLeft = pcComma;
	}
	return pcLeft;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseAssignmentExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression*		pcLeft;
	CCPPSyntax*					pcRight;
	CCPPSyntaxBinaryExpression*	pcAssignment;
	CCPPSyntaxThrowExpression*	pcThrow;
	ECppTokens					eOperator;

	//conditional-expression, logical-or-expression assignment-operator initializer-clause or throw-expression.
	//[expr.ass] [except.throw]
	if (Is(CT_throw))
	{
		pcThrow = mpcMemory->CreateThrowExpression(pcParent);
		pcThrow->miFirstToken = CurrentToken();
		Next();
		if (!Is(CT_SemiColon) && !Is(CT_CloseRoundBracket) && !Is(CT_Comma) && !Is(CT_Colon))
		{
			pcThrow->mpcOperand = ParseAssignmentExpression(pcThrow);
			if (!pcThrow->mpcOperand)
			{
				return NULL;
			}
		}
		pcThrow->miLastToken = PreviousToken();
		return pcThrow;
	}

	pcLeft = ParseConditionalExpression(pcParent);
	if (!pcLeft)
	{
		return NULL;
	}

	if (Is(CT_Assignment) || Is(CT_AssignmentAdd) || Is(CT_AssignmentSubtract) || Is(CT_AssignmentMultiply) || Is(CT_AssignmentDivide) || Is(CT_AssignmentModulus) || Is(CT_AssignmentBitwiseAnd) || Is(CT_AssignmentBitwiseOr) || Is(CT_AssignmentBitwiseXor) || Is(CT_AssignmentLeftShift) || (Is(CT_AssignmentRightShift) && !mbNoGreater))
	{
		eOperator = ((CCTReservedWord*)Peek(0))->GetToken();
		Next();
		pcRight = ParseInitializerClause(pcParent);
		if (!pcRight)
		{
			return NULL;
		}
		pcAssignment = mpcMemory->CreateBinaryExpression(pcParent);
		pcAssignment->meOperator = eOperator;
		pcAssignment->mpcLeft = pcLeft;
		pcAssignment->mpcRight = pcRight;
		pcLeft->mpcParent = pcAssignment;
		pcRight->mpcParent = pcAssignment;
		pcAssignment->SetTokens(pcLeft->miFirstToken, PreviousToken());
		return pcAssignment;
	}
	return pcLeft;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseConditionalExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression*				pcCondition;
	CCPPSyntaxConditionalExpression*	pcConditional;
	bool								bNoGreater;

	//logical-or-expression ? expression : assignment-expression  [expr.cond]
	pcCondition = ParseBinaryExpression(pcParent, 3);
	if (!pcCondition)
	{
		return NULL;
	}

	if (!Is(CT_QuestionMark))
	{
		return pcCondition;
	}
	Next();

	pcConditional = mpcMemory->CreateConditionalExpression(pcParent);
	pcConditional->mpcCondition = pcCondition;
	pcCondition->mpcParent = pcConditional;
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	pcConditional->mpcTrue = ParseExpression(pcConditional);
	mbNoGreater = bNoGreater;
	if (!pcConditional->mpcTrue || !Expect(CT_Colon))
	{
		return NULL;
	}
	pcConditional->mpcFalse = ParseAssignmentExpression(pcConditional);
	if (!pcConditional->mpcFalse)
	{
		return NULL;
	}
	pcConditional->SetTokens(pcCondition->miFirstToken, PreviousToken());
	return pcConditional;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseConstantExpression(CCPPSyntax* pcParent)
{
	return ParseConditionalExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPParser::GetBinaryPrecedence(void)
{
	CCPPToken*	pcToken;

	//Higher binds tighter.  In a template argument list '>' ends the argument.  [expr] [temp.names] 3
	pcToken = Peek(0);
	if (!pcToken || !pcToken->IsReservedWord())
	{
		return 0;
	}

	switch (((CCTReservedWord*)pcToken)->GetToken())
	{
	case CT_MemberFunctionPointerIndirection:
	case CT_MemberByPointerFunctionPointerIndirection:
		return 13;
	case CT_Multiply:
	case CT_Divide:
	case CT_Modulus:
		return 12;
	case CT_Add:
	case CT_Subtract:
		return 11;
	case CT_LeftShift:
		return 10;
	case CT_RightShift:
		return (mbNoGreater || mbHalf) ? 0 : 10;
	case CT_LessThan:
	case CT_LessThanEqualTo:
	case CT_GreaterThanEqualTo:
		return 9;
	case CT_GreaterThan:
		return mbNoGreater ? 0 : 9;
	case CT_EqualTo:
	case CT_NotEqualTo:
		return 8;
	case CT_BitwiseAnd:
		return 7;
	case CT_BitwiseXor:
		return 6;
	case CT_BitwiseOr:
		return 5;
	case CT_LogicalAnd:
		return 4;
	case CT_LogicalOr:
		return 3;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseBinaryExpression(CCPPSyntax* pcParent, int iMinPrecedence)
{
	CCPPSyntaxExpression*		pcLeft;
	CCPPSyntaxExpression*		pcRight;
	CCPPSyntaxBinaryExpression*	pcBinary;
	ECppTokens					eOperator;
	int							iPrecedence;

	//Precedence climbing over the left associative binary operators.
	pcLeft = ParseCastExpression(pcParent);
	if (!pcLeft)
	{
		return NULL;
	}

	for (;;)
	{
		iPrecedence = GetBinaryPrecedence();
		if ((iPrecedence == 0) || (iPrecedence < iMinPrecedence))
		{
			return pcLeft;
		}

		eOperator = ((CCTReservedWord*)Peek(0))->GetToken();
		Next();
		pcRight = ParseBinaryExpression(pcParent, iPrecedence + 1);
		if (!pcRight)
		{
			return NULL;
		}

		pcBinary = mpcMemory->CreateBinaryExpression(pcParent);
		pcBinary->meOperator = eOperator;
		pcBinary->mpcLeft = pcLeft;
		pcBinary->mpcRight = pcRight;
		pcLeft->mpcParent = pcBinary;
		pcRight->mpcParent = pcBinary;
		pcBinary->SetTokens(pcLeft->miFirstToken, PreviousToken());
		pcLeft = pcBinary;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsCastAhead(void)
{
	SCPPParserPosition	sPosition;
	int					iSavedFailed;
	CCPPSyntaxTypeId*	pcType;
	bool				bCast;

	//( type-id ) cast-expression.  The current token is '('.  [expr.cast]
	sPosition = Mark();
	Next();
	if (!IsTypeIdStart())
	{
		Rewind(sPosition);
		return false;
	}

	BeginTentative(&iSavedFailed);
	pcType = ParseTypeId(NULL);
	bCast = EndTentative(iSavedFailed) && pcType && Is(CT_CloseRoundBracket);
	Rewind(sPosition);
	return bCast;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseCastExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxCastExpression*	pcCast;
	bool						bNoGreater;

	if (Is(CT_OpenRoundBracket) && IsCastAhead())
	{
		pcCast = mpcMemory->CreateCastExpression(pcParent);
		pcCast->miFirstToken = CurrentToken();
		Next();
		bNoGreater = mbNoGreater;
		mbNoGreater = false;
		pcCast->mpcType = ParseTypeId(pcCast);
		mbNoGreater = bNoGreater;
		if (!pcCast->mpcType || !Expect(CT_CloseRoundBracket))
		{
			return NULL;
		}

		if (Is(CT_OpenCurlyBracket))
		{
			Error("Compound literals are not C++.");
			return NULL;
		}
		pcCast->mpcOperand = ParseCastExpression(pcCast);
		if (!pcCast->mpcOperand)
		{
			return NULL;
		}
		pcCast->miLastToken = PreviousToken();
		return pcCast;
	}
	return ParseUnaryExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseUnaryExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxNoexceptExpression*	pcNoexcept;
	CCPPToken*						pcToken;
	ECppTokens						eToken;

	//[expr.unary]
	pcToken = Peek(0);
	if (!pcToken)
	{
		Error("Expected an expression.");
		return NULL;
	}

	if (pcToken->IsReservedWord())
	{
		eToken = ((CCTReservedWord*)pcToken)->GetToken();
		switch (eToken)
		{
		case CT_Increment:
		case CT_Decrement:
		case CT_Multiply:
		case CT_BitwiseAnd:
		case CT_Add:
		case CT_Subtract:
		case CT_LogicalNot:
		case CT_BitwiseNot:
			pcUnary = mpcMemory->CreateUnaryExpression(pcParent);
			pcUnary->miFirstToken = CurrentToken();
			pcUnary->meOperator = eToken;
			Next();
			pcUnary->mpcOperand = ParseCastExpression(pcUnary);
			if (!pcUnary->mpcOperand)
			{
				return NULL;
			}
			pcUnary->miLastToken = PreviousToken();
			return pcUnary;

		case CT_sizeof:
		case CT_alignof:
		case CT___alignof__:
			return ParseSizeofExpression(pcParent);

		case CT_noexcept:
			pcNoexcept = mpcMemory->CreateNoexceptExpression(pcParent);
			pcNoexcept->miFirstToken = CurrentToken();
			Next();
			if (!Expect(CT_OpenRoundBracket))
			{
				return NULL;
			}
			pcNoexcept->mpcOperand = ParseExpression(pcNoexcept);
			if (!pcNoexcept->mpcOperand || !Expect(CT_CloseRoundBracket))
			{
				return NULL;
			}
			pcNoexcept->miLastToken = PreviousToken();
			return pcNoexcept;

		case CT_new:
			return ParseNewExpression(pcParent);

		case CT_delete:
			return ParseDeleteExpression(pcParent);

		case CT___extension__:
			Next();
			return ParseCastExpression(pcParent);

		case CT_ScopeResolution:
			if (IsAhead(1, CT_new))
			{
				return ParseNewExpression(pcParent);
			}
			if (IsAhead(1, CT_delete))
			{
				return ParseDeleteExpression(pcParent);
			}
			break;

		default:
			break;
		}
	}
	return ParsePostfixExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParsePostfixExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression*			pcExpression;
	CCPPSyntaxSubscriptExpression*	pcSubscript;
	CCPPSyntaxCallExpression*		pcCall;
	CCPPSyntaxMemberExpression*		pcMember;
	CCPPSyntaxUnaryExpression*		pcUnary;
	bool							bNoGreater;

	//[expr.post]
	pcExpression = ParsePrimaryExpression(pcParent);
	if (!pcExpression)
	{
		return NULL;
	}

	for (;;)
	{
		if (Is(CT_OpenSquareBracket))
		{
			pcSubscript = mpcMemory->CreateSubscriptExpression(pcParent);
			pcSubscript->miFirstToken = pcExpression->miFirstToken;
			pcSubscript->mpcArray = pcExpression;
			pcExpression->mpcParent = pcSubscript;
			Next();
			bNoGreater = mbNoGreater;
			mbNoGreater = false;
			pcSubscript->mpcIndex = Is(CT_OpenCurlyBracket) ? (CCPPSyntax*)ParseBracedInitList(pcSubscript) : (CCPPSyntax*)ParseExpression(pcSubscript);
			mbNoGreater = bNoGreater;
			if (!pcSubscript->mpcIndex || !Expect(CT_CloseSquareBracket))
			{
				return NULL;
			}
			pcSubscript->miLastToken = PreviousToken();
			pcExpression = pcSubscript;
		}
		else if (Is(CT_OpenRoundBracket))
		{
			pcCall = mpcMemory->CreateCallExpression(pcParent);
			pcCall->miFirstToken = pcExpression->miFirstToken;
			pcCall->mpcCallee = pcExpression;
			pcExpression->mpcParent = pcCall;
			Next();
			bNoGreater = mbNoGreater;
			mbNoGreater = false;
			if (!ParseExpressionList(&pcCall->macpArguments, pcCall, CT_CloseRoundBracket))
			{
				mbNoGreater = bNoGreater;
				return NULL;
			}
			mbNoGreater = bNoGreater;
			if (!Expect(CT_CloseRoundBracket))
			{
				return NULL;
			}
			pcCall->miLastToken = PreviousToken();
			pcExpression = pcCall;
		}
		else if (Is(CT_Member) || Is(CT_MemberByPointer))
		{
			pcMember = mpcMemory->CreateMemberExpression(pcParent);
			pcMember->miFirstToken = pcExpression->miFirstToken;
			pcMember->mpcObject = pcExpression;
			pcMember->mbArrow = Is(CT_MemberByPointer);
			pcExpression->mpcParent = pcMember;
			Next();
			mbMemberName = true;
			pcMember->mpcName = ParseName(pcMember, false, NULL);
			mbMemberName = false;
			if (!pcMember->mpcName)
			{
				return NULL;
			}
			pcMember->miLastToken = PreviousToken();
			pcExpression = pcMember;
		}
		else if (Is(CT_Increment) || Is(CT_Decrement))
		{
			pcUnary = mpcMemory->CreateUnaryExpression(pcParent);
			pcUnary->miFirstToken = pcExpression->miFirstToken;
			pcUnary->meOperator = ((CCTReservedWord*)Peek(0))->GetToken();
			pcUnary->mbPostfix = true;
			pcUnary->mpcOperand = pcExpression;
			pcExpression->mpcParent = pcUnary;
			Next();
			pcUnary->miLastToken = PreviousToken();
			pcExpression = pcUnary;
		}
		else
		{
			return pcExpression;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxTypeId* CCPPParser::CreateSimpleTypeId(CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeId*				pcType;
	CCPPSyntaxKeywordSpecifier*		pcKeyword;
	ECppTokens						eToken;

	//The simple type specifiers of int(x) or unsigned long(x).  [expr.type.conv]
	pcType = mpcMemory->CreateTypeId(pcParent);
	pcType->miFirstToken = CurrentToken();
	pcType->mpcSpecifiers = mpcMemory->CreateDeclSpecifiers(pcType);
	pcType->mpcSpecifiers->miFirstToken = CurrentToken();
	while (Peek(0) && Peek(0)->IsReservedWord() && IsSimpleTypeKeyword(((CCTReservedWord*)Peek(0))->GetToken()))
	{
		eToken = ((CCTReservedWord*)Peek(0))->GetToken();
		pcKeyword = mpcMemory->CreateKeywordSpecifier(pcType->mpcSpecifiers);
		pcKeyword->meKeyword = eToken;
		pcKeyword->SetTokens(CurrentToken(), CurrentToken());
		pcType->mpcSpecifiers->macpSpecifiers.Add(pcKeyword);
		if (eToken == CT_long)
		{
			pcType->mpcSpecifiers->miLongs++;
		}
		else
		{
			pcType->mpcSpecifiers->muiTypeKeywords |= CPPTK_INT;
		}
		Next();
	}
	pcType->mpcSpecifiers->miLastToken = PreviousToken();
	pcType->miLastToken = PreviousToken();
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseConstructExpression(CCPPSyntax* pcParent, CCPPSyntaxTypeId* pcType)
{
	CCPPSyntaxConstructExpression*	pcConstruct;
	ECPPInitializer					eKind;

	//T(x) or T{x}.  [expr.type.conv]
	pcConstruct = mpcMemory->CreateConstructExpression(pcParent);
	pcConstruct->miFirstToken = pcType->miFirstToken;
	pcConstruct->mpcType = pcType;
	pcType->mpcParent = pcConstruct;
	pcConstruct->mpcInitializer = ParseInitializer(pcConstruct, &eKind);
	if (!pcConstruct->mpcInitializer)
	{
		return NULL;
	}
	pcConstruct->miLastToken = PreviousToken();
	return pcConstruct;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParsePrimaryExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxLiteralExpression*		pcLiteral;
	CCPPSyntaxThisExpression*			pcThis;
	CCPPSyntaxParenthesisedExpression*	pcParentheses;
	CCPPSyntaxIdExpression*				pcId;
	CCPPSyntaxTypeId*					pcType;
	CCPPSyntaxTypeidExpression*			pcTypeid;
	CCPPSyntaxNamedTypeSpecifier*		pcNamed;
	CCPPSyntaxName*						pcName;
	CCPPToken*							pcToken;
	ECppTokens							eToken;
	bool								bNoGreater;

	//[expr.prim]
	pcToken = Peek(0);
	if (!pcToken)
	{
		Error("Expected an expression.");
		return NULL;
	}

	if (IsStringPart())
	{
		return ParseStringExpression(pcParent);
	}

	if (pcToken->IsNumber() || pcToken->IsChar() || pcToken->IsBool() || pcToken->Is(CT_nullptr))
	{
		pcLiteral = mpcMemory->CreateLiteralExpression(pcParent);
		pcLiteral->miLiteral = CurrentToken();
		pcLiteral->SetTokens(CurrentToken(), CurrentToken());
		Next();
		return pcLiteral;
	}

	if (pcToken->IsReservedWord())
	{
		eToken = ((CCTReservedWord*)pcToken)->GetToken();
		switch (eToken)
		{
		case CT_this:
			pcThis = mpcMemory->CreateThisExpression(pcParent);
			pcThis->SetTokens(CurrentToken(), CurrentToken());
			Next();
			return pcThis;

		case CT_OpenRoundBracket:
			pcParentheses = mpcMemory->CreateParenthesisedExpression(pcParent);
			pcParentheses->miFirstToken = CurrentToken();
			Next();
			bNoGreater = mbNoGreater;
			mbNoGreater = false;
			pcParentheses->mpcExpression = ParseExpression(pcParentheses);
			mbNoGreater = bNoGreater;
			if (!pcParentheses->mpcExpression || !Expect(CT_CloseRoundBracket))
			{
				return NULL;
			}
			pcParentheses->miLastToken = PreviousToken();
			return pcParentheses;

		case CT_OpenSquareBracket:
			Error("Lambda expressions are not supported.");
			return NULL;

		case CT_static_cast:
		case CT_dynamic_cast:
		case CT_reinterpret_cast:
		case CT_const_cast:
			return ParseNamedCastExpression(pcParent);

		case CT_typeid:
			pcTypeid = mpcMemory->CreateTypeidExpression(pcParent);
			pcTypeid->miFirstToken = CurrentToken();
			Next();
			if (!Expect(CT_OpenRoundBracket))
			{
				return NULL;
			}
			if (IsTypeIdStart() && IsTypeIdAhead(CT_CloseRoundBracket))
			{
				pcTypeid->mpcType = ParseTypeId(pcTypeid);
			}
			else
			{
				pcTypeid->mpcOperand = ParseExpression(pcTypeid);
			}
			if ((!pcTypeid->mpcType && !pcTypeid->mpcOperand) || !Expect(CT_CloseRoundBracket))
			{
				return NULL;
			}
			pcTypeid->miLastToken = PreviousToken();
			return pcTypeid;

		case CT_typename:
			pcType = ParseTypeId(pcParent);
			if (!pcType)
			{
				return NULL;
			}
			return ParseConstructExpression(pcParent, pcType);

		case CT_ScopeResolution:
		case CT_operator:
		case CT_BitwiseNot:
			break;

		default:
			if (IsSimpleTypeKeyword(eToken))
			{
				pcType = CreateSimpleTypeId(pcParent);
				if (!Is(CT_OpenRoundBracket) && !Is(CT_OpenCurlyBracket))
				{
					Error("Expected '(' or '{' after the type.");
					return NULL;
				}
				return ParseConstructExpression(pcParent, pcType);
			}
			Error("Expected an expression.");
			return NULL;
		}
	}

	if (pcToken->IsIdentifier() && IsTypeTraitName())
	{
		return ParseTypeTraitExpression(pcParent);
	}

	if (IsNameStart())
	{
		pcId = mpcMemory->CreateIdExpression(pcParent);
		pcId->miFirstToken = CurrentToken();
		pcName = ParseName(pcId, false, NULL);
		if (!pcName)
		{
			return NULL;
		}

		if (pcName->mpcSymbol && pcName->mpcSymbol->IsType() && (Is(CT_OpenRoundBracket) || Is(CT_OpenCurlyBracket)))
		{
			//A type name followed by ( or { makes an object.
			pcType = mpcMemory->CreateTypeId(pcParent);
			pcType->miFirstToken = pcId->miFirstToken;
			pcType->mpcSpecifiers = mpcMemory->CreateDeclSpecifiers(pcType);
			pcNamed = mpcMemory->CreateNamedTypeSpecifier(pcType->mpcSpecifiers);
			pcNamed->mpcName = pcName;
			pcName->mpcParent = pcNamed;
			pcNamed->SetTokens(pcId->miFirstToken, PreviousToken());
			pcType->mpcSpecifiers->mpcType = pcNamed;
			pcType->mpcSpecifiers->macpSpecifiers.Add(pcNamed);
			pcType->mpcSpecifiers->SetTokens(pcId->miFirstToken, PreviousToken());
			pcType->miLastToken = PreviousToken();
			return ParseConstructExpression(pcParent, pcType);
		}

		pcId->mpcName = pcName;
		pcId->miLastToken = PreviousToken();
		return pcId;
	}

	Error("Expected an expression.");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseStringExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxStringExpression*	pcString;

	//Adjacent string literals are one.  [lex.string] 13
	pcString = mpcMemory->CreateStringExpression(pcParent);
	pcString->miFirstToken = CurrentToken();
	if (!IsStringPart())
	{
		Error("Expected a string.");
		return NULL;
	}
	while (IsStringPart())
	{
		Next();
	}
	pcString->miLastToken = PreviousToken();
	return pcString;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsStringPart(void)
{
	CCPPToken*	pcToken;
	int64		lliName;

	//MSVC's __FUNCTION__, __FUNCSIG__ and __FUNCDNAME__ are string literals, as in __FUNCTION__ ".Java".
	pcToken = Peek(0);
	if (!pcToken)
	{
		return false;
	}
	if (pcToken->IsString())
	{
		return true;
	}
	lliName = GetIdentifier();
	return (lliName != -1) && ((lliName == mlliFunction) || (lliName == mlliFuncsig) || (lliName == mlliFuncdname));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsTypeTraitName(void)
{
	int		iLength;
	char*	szName;

	//Compiler intrinsics that take types, such as __is_trivially_constructible(T) and __builtin_offsetof(T, m).
	if (!IsAhead(1, CT_OpenRoundBracket))
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(GetIdentifier(), &iLength);
	if (!szName || (iLength < 5) || (szName[0] != '_') || (szName[1] != '_'))
	{
		return false;
	}
	if ((strncmp(szName, "__is_", 5) == 0) || (strncmp(szName, "__has_", 6) == 0) || (strcmp(szName, "__underlying_type") == 0) || (strcmp(szName, "__builtin_va_arg") == 0) || (strcmp(szName, "__builtin_offsetof") == 0) || (strcmp(szName, "__builtin_types_compatible_p") == 0))
	{
		return mpcScope->Find(GetIdentifier()) == NULL;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseTypeTraitExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeTraitExpression*	pcTrait;
	CCPPSyntax*						pcArgument;
	SCPPParserPosition				sPosition;
	int								iSavedFailed;
	bool							bType;
	CCPPSyntaxTypeId*				pcType;

	pcTrait = mpcMemory->CreateTypeTraitExpression(pcParent);
	pcTrait->miFirstToken = CurrentToken();
	pcTrait->miName = CurrentToken();
	Next();
	Expect(CT_OpenRoundBracket);
	while (!Is(CT_CloseRoundBracket))
	{
		//Each argument is a type if it can be one.
		bType = false;
		if (IsTypeIdStart())
		{
			sPosition = Mark();
			BeginTentative(&iSavedFailed);
			pcType = ParseTypeId(pcTrait);
			bType = EndTentative(iSavedFailed) && pcType && (Is(CT_Comma) || Is(CT_CloseRoundBracket));
			Rewind(sPosition);
		}
		pcArgument = bType ? (CCPPSyntax*)ParseTypeId(pcTrait) : (CCPPSyntax*)ParseAssignmentExpression(pcTrait);
		if (!pcArgument)
		{
			return NULL;
		}
		pcTrait->macpArguments.Add(pcArgument);
		if (!Accept(CT_Comma))
		{
			break;
		}
	}
	if (!Expect(CT_CloseRoundBracket))
	{
		return NULL;
	}
	pcTrait->miLastToken = PreviousToken();
	return pcTrait;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseNamedCastExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxNamedCastExpression*	pcCast;
	bool							bNoGreater;

	//static_cast<T>(e) and the others.  [expr.static.cast]
	pcCast = mpcMemory->CreateNamedCastExpression(pcParent);
	pcCast->miFirstToken = CurrentToken();
	pcCast->meCast = ((CCTReservedWord*)Peek(0))->GetToken();
	Next();
	if (!Expect(CT_LessThan))
	{
		return NULL;
	}
	bNoGreater = mbNoGreater;
	mbNoGreater = true;
	pcCast->mpcType = ParseTypeId(pcCast);
	mbNoGreater = bNoGreater;
	if (!pcCast->mpcType)
	{
		return NULL;
	}
	if (!AcceptCloseAngle())
	{
		Error("Expected '>'.");
		return NULL;
	}
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	pcCast->mpcOperand = ParseExpression(pcCast);
	mbNoGreater = bNoGreater;
	if (!pcCast->mpcOperand || !Expect(CT_CloseRoundBracket))
	{
		return NULL;
	}
	pcCast->miLastToken = PreviousToken();
	return pcCast;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseSizeofExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxSizeofExpression*	pcSizeof;
	SCPPParserPosition			sPosition;
	bool						bType;
	bool						bNoGreater;

	//sizeof unary-expression, sizeof ( type-id ), sizeof...( pack ) and alignof ( type-id ).  [expr.sizeof]
	pcSizeof = mpcMemory->CreateSizeofExpression(pcParent);
	pcSizeof->miFirstToken = CurrentToken();
	pcSizeof->meKeyword = ((CCTReservedWord*)Peek(0))->GetToken();
	Next();

	if (Accept(CT_Elipsis))
	{
		pcSizeof->mbPack = true;
		if (!Expect(CT_OpenRoundBracket) || !IsIdentifier())
		{
			return NULL;
		}
		pcSizeof->miPack = CurrentToken();
		Next();
		if (!Expect(CT_CloseRoundBracket))
		{
			return NULL;
		}
		pcSizeof->miLastToken = PreviousToken();
		return pcSizeof;
	}

	bType = false;
	if (Is(CT_OpenRoundBracket))
	{
		sPosition = Mark();
		Next();
		bType = IsTypeIdStart() && IsTypeIdAhead(CT_CloseRoundBracket);
		if (bType)
		{
			bNoGreater = mbNoGreater;
			mbNoGreater = false;
			pcSizeof->mpcType = ParseTypeId(pcSizeof);
			mbNoGreater = bNoGreater;
			if (!pcSizeof->mpcType || !Is(CT_CloseRoundBracket))
			{
				Rewind(sPosition);
				bType = false;
			}
			else
			{
				Next();
			}
		}
		else
		{
			Rewind(sPosition);
		}
	}

	if (!bType)
	{
		pcSizeof->mpcType = NULL;
		pcSizeof->mpcOperand = ParseUnaryExpression(pcSizeof);
		if (!pcSizeof->mpcOperand)
		{
			return NULL;
		}
	}
	pcSizeof->miLastToken = PreviousToken();
	return pcSizeof;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseNewExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxNewExpression*		pcNew;
	CCPPSyntaxPointerOperator*		pcPointer;
	CCPPSyntaxExpression*			pcSize;
	CCPPSyntaxDeclarator*			pcDeclarator;
	SCPPParserPosition				sPosition;
	ECPPInitializer					eKind;
	bool							bParenthesisedType;
	bool							bNoGreater;

	//[::] new ( placement ) new-type-id new-initializer or new ( type-id ).  [expr.new]
	pcNew = mpcMemory->CreateNewExpression(pcParent);
	pcNew->miFirstToken = CurrentToken();
	pcNew->mbGlobal = Accept(CT_ScopeResolution);
	Expect(CT_new);
	bNoGreater = mbNoGreater;
	mbNoGreater = false;

	if (Is(CT_OpenRoundBracket))
	{
		//new (T) is a parenthesised type unless a type follows it, as in new (pvMemory) T.
		sPosition = Mark();
		Next();
		bParenthesisedType = IsTypeIdStart() && IsTypeIdAhead(CT_CloseRoundBracket);
		if (bParenthesisedType)
		{
			ParseTypeId(NULL);
			bParenthesisedType = Is(CT_CloseRoundBracket);
			if (bParenthesisedType)
			{
				Next();
				bParenthesisedType = !IsTypeIdStart();
			}
		}
		Rewind(sPosition);

		if (bParenthesisedType)
		{
			Next();
			pcNew->mbParenthesisedType = true;
			pcNew->mpcType = ParseTypeId(pcNew);
			if (!pcNew->mpcType || !Expect(CT_CloseRoundBracket))
			{
				mbNoGreater = bNoGreater;
				return NULL;
			}
		}
		else
		{
			Next();
			pcNew->mbPlacement = true;
			if (!ParseExpressionList(&pcNew->macpPlacement, pcNew, CT_CloseRoundBracket) || !Expect(CT_CloseRoundBracket))
			{
				mbNoGreater = bNoGreater;
				return NULL;
			}
		}
	}

	if (!pcNew->mpcType)
	{
		if (Accept(CT_OpenRoundBracket))
		{
			pcNew->mbParenthesisedType = true;
			pcNew->mpcType = ParseTypeId(pcNew);
			if (!pcNew->mpcType || !Expect(CT_CloseRoundBracket))
			{
				mbNoGreater = bNoGreater;
				return NULL;
			}
		}
		else
		{
			//A new-type-id: type specifiers and pointer operators.  [ size ] follows.
			pcNew->mpcType = mpcMemory->CreateTypeId(pcNew);
			pcNew->mpcType->miFirstToken = CurrentToken();
			pcNew->mpcType->mpcSpecifiers = ParseDeclSpecifiers(pcNew->mpcType, CPPDC_TypeId);
			if (!pcNew->mpcType->mpcSpecifiers || (pcNew->mpcType->mpcSpecifiers->macpSpecifiers.NumElements() == 0))
			{
				mbNoGreater = bNoGreater;
				Error("Expected a type after new.");
				return NULL;
			}
			pcDeclarator = mpcMemory->CreateDeclarator(pcNew->mpcType);
			pcDeclarator->miFirstToken = CurrentToken();
			while (Is(CT_Multiply) || IsMemberPointerAhead())
			{
				pcPointer = ParsePointerOperator(pcDeclarator);
				if (!pcPointer)
				{
					mbNoGreater = bNoGreater;
					return NULL;
				}
				pcDeclarator->macpPrefix.Add(pcPointer);
			}
			pcDeclarator->miLastToken = PreviousToken();
			pcNew->mpcType->mpcDeclarator = pcDeclarator;
			pcNew->mpcType->miLastToken = PreviousToken();

			while (Is(CT_OpenSquareBracket))
			{
				Next();
				pcSize = ParseExpression(pcNew);
				if (!pcSize || !Expect(CT_CloseSquareBracket))
				{
					mbNoGreater = bNoGreater;
					return NULL;
				}
				pcNew->macpArraySizes.Add(pcSize);
			}
		}
	}

	if (Is(CT_OpenRoundBracket) || Is(CT_OpenCurlyBracket))
	{
		pcNew->mpcInitializer = ParseInitializer(pcNew, &eKind);
		if (!pcNew->mpcInitializer)
		{
			mbNoGreater = bNoGreater;
			return NULL;
		}
	}
	mbNoGreater = bNoGreater;
	pcNew->miLastToken = PreviousToken();
	return pcNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExpression* CCPPParser::ParseDeleteExpression(CCPPSyntax* pcParent)
{
	CCPPSyntaxDeleteExpression*	pcDelete;

	//[::] delete cast-expression and [::] delete [] cast-expression.  [expr.delete]
	pcDelete = mpcMemory->CreateDeleteExpression(pcParent);
	pcDelete->miFirstToken = CurrentToken();
	pcDelete->mbGlobal = Accept(CT_ScopeResolution);
	Expect(CT_delete);
	if (Is(CT_OpenSquareBracket) && IsAhead(1, CT_CloseSquareBracket))
	{
		pcDelete->mbArray = true;
		Next();
		Next();
	}
	pcDelete->mpcOperand = ParseCastExpression(pcDelete);
	if (!pcDelete->mpcOperand)
	{
		return NULL;
	}
	pcDelete->miLastToken = PreviousToken();
	return pcDelete;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxInitializerList* CCPPParser::ParseBracedInitList(CCPPSyntax* pcParent)
{
	CCPPSyntaxInitializerList*			pcList;
	CCPPSyntax*							pcElement;
	CCPPSyntaxPackExpansionExpression*	pcPack;
	bool								bNoGreater;

	//{ initializer-clause , ... }  [dcl.init.list]
	pcList = mpcMemory->CreateInitializerList(pcParent);
	pcList->miFirstToken = CurrentToken();
	if (!Expect(CT_OpenCurlyBracket))
	{
		return NULL;
	}
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	while (!Is(CT_CloseCurlyBracket))
	{
		pcElement = ParseInitializerClause(pcList);
		if (!pcElement)
		{
			mbNoGreater = bNoGreater;
			return NULL;
		}
		if (Is(CT_Elipsis))
		{
			pcPack = mpcMemory->CreatePackExpansionExpression(pcList);
			pcPack->mpcPattern = pcElement;
			pcElement->mpcParent = pcPack;
			pcPack->SetTokens(pcElement->miFirstToken, CurrentToken());
			Next();
			pcElement = pcPack;
		}
		pcList->macpElements.Add(pcElement);
		if (!Accept(CT_Comma))
		{
			break;
		}
		if (Is(CT_CloseCurlyBracket))
		{
			pcList->mbTrailingComma = true;
		}
	}
	mbNoGreater = bNoGreater;
	if (!Expect(CT_CloseCurlyBracket))
	{
		return NULL;
	}
	pcList->miLastToken = PreviousToken();
	return pcList;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseInitializerClause(CCPPSyntax* pcParent)
{
	//assignment-expression or braced-init-list.  [dcl.init]
	if (Is(CT_OpenCurlyBracket))
	{
		return ParseBracedInitList(pcParent);
	}
	return ParseAssignmentExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseExpressionList(CArraySyntax* papcExpressions, CCPPSyntax* pcParent, ECppTokens eClose)
{
	CCPPSyntax*							pcExpression;
	CCPPSyntaxPackExpansionExpression*	pcPack;

	//initializer-clause , ...  The closing token is not consumed.  [expr.post]
	while (!Is(eClose))
	{
		pcExpression = ParseInitializerClause(pcParent);
		if (!pcExpression)
		{
			return false;
		}
		if (Is(CT_Elipsis))
		{
			pcPack = mpcMemory->CreatePackExpansionExpression(pcParent);
			pcPack->mpcPattern = pcExpression;
			pcExpression->mpcParent = pcPack;
			pcPack->SetTokens(pcExpression->miFirstToken, CurrentToken());
			Next();
			pcExpression = pcPack;
		}
		papcExpressions->Add(pcExpression);
		if (!Accept(CT_Comma))
		{
			break;
		}
	}
	return true;
}

