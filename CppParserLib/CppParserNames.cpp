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
bool CCPPParser::IsNameStart(void)
{
	return IsIdentifier() || Is(CT_ScopeResolution) || Is(CT_operator) || (Is(CT_BitwiseNot) && IsIdentifierAhead(1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxName* CCPPParser::ParseName(CCPPSyntax* pcParent, bool bDeclaration, CCPPScope** ppcQualifier)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPScope*					pcScope;
	CCPPSymbol*					pcSymbol;
	bool						bQualified;
	bool						bDependent;
	bool						bTemplateKeyword;
	bool						bTemplate;
	int64						lliName;
	SCPPParserPosition			sPosition;
	int							iSavedFailed;
	bool						bArguments;
	bool						bMember;

	//id-expression: [::] nested-name-specifier unqualified-id, looked up as it is read.  [basic.lookup.qual]
	bMember = mbMemberName;
	mbMemberName = false;
	pcName = mpcMemory->CreateName(pcParent);
	pcName->miFirstToken = CurrentToken();
	pcScope = NULL;
	bQualified = false;
	bDependent = false;
	if (Accept(CT_ScopeResolution))
	{
		pcName->mbGlobal = true;
		pcScope = mpcSymbols->GetGlobalScope();
		bQualified = true;
	}

	for (;;)
	{
		bTemplateKeyword = false;
		if (bQualified && Accept(CT_template))
		{
			bTemplateKeyword = true;
		}

		if (IsIdentifier())
		{
			pcComponent = mpcMemory->CreateNameComponent(pcName);
			pcComponent->miFirstToken = CurrentToken();
			pcComponent->meKind = CPPNC_Identifier;
			pcComponent->mbTemplateKeyword = bTemplateKeyword;
			pcComponent->miName = CurrentToken();
			lliName = GetIdentifier();

			if (bDependent || (bMember && !bQualified && !IsAhead(1, CT_ScopeResolution)))
			{
				//A member name after . or -> is a member of a type the parser doesn't know, unless it qualifies a name,
				//as in p->CBase::f().
				pcSymbol = NULL;
			}
			else if (pcScope)
			{
				pcSymbol = pcScope->FindMember(lliName);
			}
			else
			{
				pcSymbol = mpcScope->Find(lliName);
			}
			pcComponent->mpcSymbol = pcSymbol;
			Resolve(CurrentToken(), pcSymbol, pcSymbol ? CPPTR_Reference : CPPTR_Unresolved);
			Next();

			if (Is(CT_LessThan))
			{
				bTemplate = bTemplateKeyword || (pcSymbol && pcSymbol->HasTemplate());
				if (!bTemplate && !pcSymbol && !bDeclaration)
				{
					//An unknown name, as a member of a dependent or unknown type, is a template if what follows the
					//'<' is a template argument list followed by '(' or '::'.
					sPosition = Mark();
					BeginTentative(&iSavedFailed);
					bArguments = ParseTemplateArguments(pcComponent);
					bTemplate = EndTentative(iSavedFailed) && bArguments && (Is(CT_OpenRoundBracket) || Is(CT_ScopeResolution));
					Rewind(sPosition);
					pcComponent->macpTemplateArguments.Kill();
					pcComponent->macpTemplateArguments.Init();
					pcComponent->mbTemplateArguments = false;
				}

				if (bTemplate)
				{
					if (!ParseTemplateArguments(pcComponent))
					{
						return NULL;
					}
				}
			}
		}
		else if (Is(CT_BitwiseNot) && IsIdentifierAhead(1))
		{
			//A destructor: ~X.
			pcComponent = mpcMemory->CreateNameComponent(pcName);
			pcComponent->miFirstToken = CurrentToken();
			pcComponent->meKind = CPPNC_Destructor;
			Next();
			pcComponent->miName = CurrentToken();
			lliName = GetIdentifier();
			pcSymbol = bDependent ? NULL : (pcScope ? pcScope->FindMember(lliName) : mpcScope->Find(lliName));
			pcComponent->mpcSymbol = pcSymbol;
			Resolve(CurrentToken(), pcSymbol, pcSymbol ? CPPTR_Reference : CPPTR_Unresolved);
			Next();
			if (Is(CT_LessThan) && pcSymbol && pcSymbol->HasTemplate())
			{
				if (!ParseTemplateArguments(pcComponent))
				{
					return NULL;
				}
			}
		}
		else if (Is(CT_operator))
		{
			pcComponent = ParseOperatorName(pcName);
			if (!pcComponent)
			{
				return NULL;
			}
			pcComponent->mbTemplateKeyword = bTemplateKeyword;
			pcSymbol = NULL;
		}
		else
		{
			Error("Expected a name.");
			return NULL;
		}

		pcComponent->miLastToken = PreviousToken();
		pcName->macpComponents.Add(pcComponent);

		//X::* is a pointer to member, not part of the name.
		if ((pcComponent->meKind == CPPNC_Identifier) && Is(CT_ScopeResolution) && !IsAhead(1, CT_Multiply))
		{
			Next();
			bQualified = true;
			if (bDependent || !pcSymbol || !pcSymbol->mpcMembers)
			{
				//A template parameter, an unknown name or an incomplete class: what follows can't be looked up.
				bDependent = true;
				pcScope = NULL;
			}
			else
			{
				pcScope = pcSymbol->mpcMembers;
			}
			continue;
		}
		break;
	}

	pcName->mpcSymbol = pcComponent->mpcSymbol;
	if (ppcQualifier)
	{
		*ppcQualifier = bQualified ? pcScope : NULL;
	}
	pcName->miLastToken = PreviousToken();
	return pcName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxNameComponent* CCPPParser::ParseOperatorName(CCPPSyntax* pcParent)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPToken*					pcToken;

	//operator+, operator(), operator new[] and conversion functions, operator const char*.  [over.oper] [class.conv.fct]
	pcComponent = mpcMemory->CreateNameComponent(pcParent);
	pcComponent->miFirstToken = CurrentToken();
	Expect(CT_operator);

	pcToken = Peek(0);
	if (!pcToken)
	{
		Error("Expected an operator.");
		return NULL;
	}

	if (pcToken->Is(CT_new) || pcToken->Is(CT_delete))
	{
		pcComponent->meKind = CPPNC_Operator;
		pcComponent->miName = CurrentToken();
		Next();
		if (Is(CT_OpenSquareBracket) && IsAhead(1, CT_CloseSquareBracket))
		{
			Next();
			Next();
		}
	}
	else if ((pcToken->Is(CT_OpenRoundBracket) && IsAhead(1, CT_CloseRoundBracket)) || (pcToken->Is(CT_OpenSquareBracket) && IsAhead(1, CT_CloseSquareBracket)))
	{
		pcComponent->meKind = CPPNC_Operator;
		pcComponent->miName = CurrentToken();
		Next();
		Next();
	}
	else if (pcToken->IsReservedWord() && (((CCTReservedWord*)pcToken)->GetToken() < CT_Hash) && !pcToken->Is(CT_OpenRoundBracket) && !pcToken->Is(CT_OpenSquareBracket) && !pcToken->Is(CT_OpenCurlyBracket) && !pcToken->Is(CT_CloseCurlyBracket) && !pcToken->Is(CT_SemiColon) && !pcToken->Is(CT_ScopeResolution) && !pcToken->Is(CT_Member) && !pcToken->Is(CT_QuestionMark) && !pcToken->Is(CT_Colon))
	{
		pcComponent->meKind = CPPNC_Operator;
		pcComponent->miName = CurrentToken();
		Next();
	}
	else
	{
		pcComponent->meKind = CPPNC_Conversion;
		pcComponent->mpcConversionType = ParseConversionTypeId(pcComponent);
		if (!pcComponent->mpcConversionType)
		{
			return NULL;
		}
	}
	pcComponent->miLastToken = PreviousToken();
	return pcComponent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseTemplateArguments(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPSyntax*							pcArgument;
	CCPPSyntaxPackExpansionExpression*	pcPack;
	bool								bNoGreater;

	//< template-argument-list >  The first '>' that isn't nested ends the list.  [temp.names]
	if (!Expect(CT_LessThan))
	{
		return false;
	}
	pcComponent->mbTemplateArguments = true;
	bNoGreater = mbNoGreater;
	mbNoGreater = true;

	while (!IsCloseAngle())
	{
		pcArgument = ParseTemplateArgument(pcComponent);
		if (!pcArgument)
		{
			mbNoGreater = bNoGreater;
			return false;
		}

		if (Is(CT_Elipsis))
		{
			if (pcArgument->GetKind() == CPPS_TypeId)
			{
				((CCPPSyntaxTypeId*)pcArgument)->mbPack = true;
			}
			else
			{
				pcPack = mpcMemory->CreatePackExpansionExpression(pcComponent);
				pcPack->mpcPattern = pcArgument;
				pcArgument->mpcParent = pcPack;
				pcPack->SetTokens(pcArgument->miFirstToken, CurrentToken());
				pcArgument = pcPack;
			}
			Next();
		}
		pcComponent->macpTemplateArguments.Add(pcArgument);

		if (!Accept(CT_Comma))
		{
			break;
		}
	}

	mbNoGreater = bNoGreater;
	if (!AcceptCloseAngle())
	{
		return Error("Expected '>'.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseTemplateArgument(CCPPSyntax* pcParent)
{
	SCPPParserPosition	sPosition;
	CCPPSyntaxTypeId*	pcType;
	int					iSavedFailed;
	bool				bType;

	//A template argument that could be a type-id is a type-id.  [temp.arg] 2
	if (IsTypeIdStart())
	{
		sPosition = Mark();
		BeginTentative(&iSavedFailed);
		pcType = ParseTypeId(pcParent);
		bType = EndTentative(iSavedFailed) && pcType && (Is(CT_Comma) || IsCloseAngle() || Is(CT_Elipsis));
		Rewind(sPosition);
		if (bType)
		{
			return ParseTypeId(pcParent);
		}
	}
	return ParseConditionalExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPParser::LookupNameAhead(int* piLength)
{
	SCPPParserPosition	sPosition;
	CCPPSyntaxName*		pcName;
	int					iSavedFailed;
	CCPPSymbol*			pcSymbol;

	//The symbol the name that is next refers to.  Nothing is consumed.
	sPosition = Mark();
	BeginTentative(&iSavedFailed);
	pcName = ParseName(NULL, false, NULL);
	EndTentative(iSavedFailed);
	pcSymbol = pcName ? pcName->mpcSymbol : NULL;
	if (piLength)
	{
		*piLength = CurrentToken() - sPosition.iToken;
	}
	Rewind(sPosition);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsTypeNameAhead(void)
{
	CCPPSymbol*	pcSymbol;

	if (!IsIdentifier() && !Is(CT_ScopeResolution))
	{
		return false;
	}
	pcSymbol = LookupNameAhead(NULL);
	return pcSymbol && pcSymbol->IsType();
}

