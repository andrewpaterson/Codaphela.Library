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
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CTString.h"
#include "CPPUnit.h"
#include "CPPAttributes.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit's diagnostics, asked for once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPAttributes::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	miSaysDeprecated = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPAttributes::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the declarator has __attribute__((szName)) or ((__szName__)):
// after it, after a function's parameters (int f(int) __attribute__((noinline));),
// where it is among the qualifiers (const, volatile) or after the exception
// specification, or before the declaration among its specifiers
// (__attribute__((noinline)) static int f(int), as BaseLib's NO_INLINE).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAttributes::HasAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName)
{
	return FindAttribute(pcDeclarator, szName) != -1;
}


//////////////////////////////////////////////////////////////////////////
//
// HasAttribute's attribute: the token that names it, or -1 if the
// declarator has none of the name.
//
//////////////////////////////////////////////////////////////////////////
int32 CCPPAttributes::FindAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName)
{
	CCPPSyntaxFunctionSuffix*	pcFunction;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntax*					pcDeclaration;
	int32						iAttribute;

	if (!pcDeclarator)
	{
		return -1;
	}
	pcFunction = pcDeclarator->GetFunctionSuffix();
	iAttribute = FindAttribute(&pcDeclarator->macpAttributes, szName);
	if ((iAttribute == -1) && pcFunction)
	{
		iAttribute = FindAttribute(&pcFunction->macpQualifiers, szName);
		iAttribute = (iAttribute != -1) ? iAttribute : FindAttribute(&pcFunction->macpAttributes, szName);
	}
	if (iAttribute != -1)
	{
		return iAttribute;
	}

	//The declaration the declarator is in (through its init-declarator).
	pcSpecifiers = NULL;
	for (pcDeclaration = pcDeclarator->mpcParent; pcDeclaration && !pcSpecifiers; pcDeclaration = pcDeclaration->mpcParent)
	{
		if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
		{
			pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers;
		}
		else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
		{
			pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcSpecifiers;
		}
		else if ((pcDeclaration->GetKind() != CPPS_InitDeclarator) && (pcDeclaration->GetKind() != CPPS_Declarator))
		{
			break;
		}
	}
	return pcSpecifiers ? FindAttribute(&pcSpecifiers->macpSpecifiers, szName) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The token that names the attribute szName, or __szName__, in the list
// (what else is in it, qualifiers, isn't looked at), or -1 if none does.
//
//////////////////////////////////////////////////////////////////////////
int32 CCPPAttributes::FindAttribute(CArraySyntax* papcAttributes, const char* szName)
{
	size		i;
	int32		iToken;
	CCPPSyntax*	pcAttribute;
	CCPPToken*	pcToken;
	char*		szIdentifier;
	int			iLength;
	size		uiName;

	uiName = strlen(szName);
	for (i = 0; i < papcAttributes->NumElements(); i++)
	{
		pcAttribute = papcAttributes->GetPtr(i);
		for (iToken = pcAttribute->miFirstToken; (pcAttribute->GetKind() == CPPS_Attribute) && (iToken <= pcAttribute->miLastToken); iToken++)
		{
			pcToken = mpcUnit->GetTokeniser()->Get(iToken);
			if (pcToken->IsIdentifier())
			{
				szIdentifier = mpcUnit->GetTokeniser()->GetIdentifier(((CCTIdentifier*)pcToken)->miIdentifier, &iLength);
				if ((((size)iLength == uiName) && (memcmp(szIdentifier, szName, uiName) == 0)) ||
					(((size)iLength == uiName + 4) && (memcmp(szIdentifier, "__", 2) == 0) && (memcmp(szIdentifier + 2, szName, uiName) == 0) && (memcmp(szIdentifier + 2 + uiName, "__", 2) == 0)))
				{
					return iToken;
				}
			}
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// __attribute__((used)) (or __used__) on a declarator: the global is kept even if
// nothing uses it (its constructor may register it somewhere).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAttributes::HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator)
{
	return HasAttribute(pcDeclarator, "used");
}


//////////////////////////////////////////////////////////////////////////
//
// The token of the [[deprecated]] the symbol's declaration is marked with
// (a variable, a typedef, a member, a class, an enumeration, an
// enumerator), or -1.  [dcl.attr.deprecated] 2
//
//////////////////////////////////////////////////////////////////////////
int32 CCPPAttributes::DeprecatedAttribute(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*	pcDeclaration;

	pcDeclaration = pcSymbol->mpcDeclaration;
	if (!pcDeclaration)
	{
		return -1;
	}
	switch (pcDeclaration->GetKind())
	{
	case CPPS_InitDeclarator:
		return FindAttribute(((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator, "deprecated");
	case CPPS_ClassSpecifier:
		return FindAttribute(&((CCPPSyntaxClassSpecifier*)pcDeclaration)->macpAttributes, "deprecated");
	case CPPS_ElaboratedTypeSpecifier:
		return FindAttribute(&((CCPPSyntaxElaboratedTypeSpecifier*)pcDeclaration)->macpAttributes, "deprecated");
	case CPPS_EnumSpecifier:
		return FindAttribute(&((CCPPSyntaxEnumSpecifier*)pcDeclaration)->macpAttributes, "deprecated");
	case CPPS_Enumerator:
		return FindAttribute(&((CCPPSyntaxEnumerator*)pcDeclaration)->macpAttributes, "deprecated");
	default:
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the source says deprecated anywhere: where it doesn't, which is
// nearly everywhere, no name is asked whether it is (WarnDeprecatedSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPAttributes::SaysDeprecated(void)
{
	CCPPToken*	pcToken;
	int64		lliDeprecated;
	int64		lliUnderscored;
	size		i;

	if (miSaysDeprecated == -1)
	{
		miSaysDeprecated = 0;
		lliDeprecated = mpcUnit->GetTokeniser()->GetIdentifierID((char*)"deprecated");
		lliUnderscored = mpcUnit->GetTokeniser()->GetIdentifierID((char*)"__deprecated__");
		for (i = 0; (miSaysDeprecated == 0) && (i < mpcUnit->GetTokeniser()->NumTokens()); i++)
		{
			pcToken = mpcUnit->GetTokeniser()->Get((int32)i);
			if (pcToken->IsIdentifier() && ((((CCTIdentifier*)pcToken)->miIdentifier == lliDeprecated) || (((CCTIdentifier*)pcToken)->miIdentifier == lliUnderscored)))
			{
				miSaysDeprecated = 1;
			}
		}
	}
	return miSaysDeprecated == 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The warning that what pcNode names (szWhat: a function, a variable ...)
// is deprecated, with what its attribute says of it, if it says anything:
// [[deprecated("use G")]].  iAttribute: the attribute's token.
// [dcl.attr.deprecated] 1, 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPAttributes::WarnDeprecated(CCPPSyntax* pcNode, const char* szWhat, int32 iAttribute)
{
	CChars		szText;
	CCPPToken*	pcOpen;
	CCTString*	pcString;

	szText.Init("The ");
	szText.Append(szWhat);
	szText.Append(" is deprecated");
	pcOpen = ((size)iAttribute + 2 < mpcUnit->GetTokeniser()->NumTokens()) ? mpcUnit->GetTokeniser()->Get(iAttribute + 1) : NULL;
	if (pcOpen && pcOpen->IsReservedWord() && (((CCTReservedWord*)pcOpen)->GetToken() == CT_OpenRoundBracket) && mpcUnit->GetTokeniser()->Get(iAttribute + 2)->IsString())
	{
		pcString = (CCTString*)mpcUnit->GetTokeniser()->Get(iAttribute + 2);
		szText.Append(": ");
		szText.Append(pcString->mszStart + 1, (size)(pcString->mszEnd - pcString->mszStart - 1));
	}
	mpcDiagnostics->Warning(pcNode, szText.Text());
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// WarnDeprecated for a name that isn't a function's: a variable, a member,
// an enumerator, or a type (a class, an enumeration, a typedef) whose
// declaration is marked [[deprecated]], where it is named outside that
// declaration.  [dcl.attr.deprecated] 2, 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPAttributes::WarnDeprecatedSymbol(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	const char*	szWhat;
	int32		iAttribute;

	if (!pcSymbol || !pcNode || !SaysDeprecated())
	{
		return;
	}
	switch (pcSymbol->meKind)
	{
	case CPPSK_Variable:	szWhat = "variable"; break;
	case CPPSK_Field:		szWhat = "member"; break;
	case CPPSK_Enumerator:	szWhat = "enumerator"; break;
	case CPPSK_Class:		szWhat = "class"; break;
	case CPPSK_Enum:		szWhat = "enumeration"; break;
	case CPPSK_Typedef:		szWhat = "type"; break;
	default:				return;
	}
	iAttribute = DeprecatedAttribute(pcSymbol);
	if ((iAttribute == -1) || ((pcNode->miFirstToken >= pcSymbol->mpcDeclaration->miFirstToken) && (pcNode->miFirstToken <= pcSymbol->mpcDeclaration->miLastToken)))
	{
		return;
	}
	WarnDeprecated(pcNode, szWhat, iAttribute);
}
