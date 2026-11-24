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
#include "SourceFile.h"
#include "CTIdentifier.h"
#include "CPPUnit.h"
#include "CPPConstants.h"


//The unit's classes: finding and making them, laying them out, their members and what making or destroying one needs.


//////////////////////////////////////////////////////////////////////////
//
// Whether a class is complete where pcNode is: its definition is before
// pcNode, or pcNode is in it, or in a class it is defined in (whose member
// function bodies see the class complete).  A class is laid out when it is
// first needed whole, wherever that is; this says whether the program may
// need it there.  What a template instance or a template's code uses is
// complete where the instance is made, so is taken to be.  [basic.types] 5,
// [class.mem] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsDefinedBefore(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*	pcSpecifier;
	CCPPSyntax*					pcIn;

	pcSpecifier = pcClass->mpcSpecifier;
	if (!pcSpecifier || !pcNode || (pcNode->miFirstToken < 0) || (pcSpecifier->miFirstToken < 0) || pcClass->mpcTemplate || pcClass->mpcBindings || mcContext.GetBindings())
	{
		return true;
	}
	if (pcSpecifier->miFirstToken < pcNode->miFirstToken)
	{
		return true;
	}
	for (pcIn = pcNode->mpcParent; pcIn; pcIn = pcIn->mpcParent)
	{
		if ((pcIn->GetKind() == CPPS_ClassSpecifier) && (pcIn->miFirstToken <= pcSpecifier->miFirstToken) && (pcIn->miLastToken >= pcSpecifier->miLastToken))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's definition (mpcSpecifier), if it has been seen: its symbol's,
// or for a member class of a class template that is defined outside it
// (template<class T> struct Outer<T>::Inner { ... };) that definition, whose
// template parameters are bound to the arguments of the instance the class
// is a member of.  [temp.mem.class] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::FindSpecifier(CCPPClass* pcClass)
{
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;
	CCPPSyntaxClassSpecifier*		pcSpecifier;

	if (pcClass->mpcSpecifier)
	{
		return true;
	}
	if (pcClass->mpcSymbol && pcClass->mpcSymbol->mpcDeclaration && (pcClass->mpcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		pcClass->mpcSpecifier = (CCPPSyntaxClassSpecifier*)pcClass->mpcSymbol->mpcDeclaration;
		return true;
	}
	pcDeclaration = NULL;
	pcSpecifier = (pcClass->mpcEnclosing && pcClass->mpcSymbol) ? mcInstantiator.MemberClassDefinition(pcClass->mpcSymbol, &pcDeclaration) : NULL;
	if (!pcSpecifier)
	{
		return false;
	}
	pcClass->mpcSpecifier = pcSpecifier;
	pcClass->mpcBindings = mcInstantiator.RenameBindings(pcClass->mpcEnclosing, pcDeclaration);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class the first time it is needed whole.
//
//////////////////////////////////////////////////////////////////////////
//
// Keeps a definition written outside its class template (masOutsideDefinitions)
// with the pieces of it its readers want: pcDefined is the function
// definition or the simple declaration it is, pcDeclaration the template
// declaration whose parameters name the class template's, and pcMember a
// member template's own, in it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AddOutsideDefinition(ECPPOutsideDefinition eKind, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxTemplateDeclaration* pcMember, CCPPSyntax* pcDefined)
{
	SCPPOutsideDefinition*	psDefinition;

	psDefinition = masOutsideDefinitions.Add();
	memset(psDefinition, 0, sizeof(SCPPOutsideDefinition));
	psDefinition->eKind = eKind;
	psDefinition->pcDeclaration = pcDeclaration;
	psDefinition->pcMember = pcMember;
	if (pcDefined->GetKind() == CPPS_FunctionDefinition)
	{
		psDefinition->pcFunction = (CCPPSyntaxFunctionDefinition*)pcDefined;
		psDefinition->pcName = psDefinition->pcFunction->mpcDeclarator->GetName();
		return;
	}
	psDefinition->pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDefined;
	switch (eKind)
	{
	case CPPOD_MemberClass:
		psDefinition->pcSpecifier = (CCPPSyntaxClassSpecifier*)psDefinition->pcSimple->mpcSpecifiers->mpcType;
		break;
	case CPPOD_StaticMember:
		psDefinition->pcInit = (CCPPSyntaxInitDeclarator*)psDefinition->pcSimple->macpDeclarators.GetPtr(0);
		psDefinition->pcName = psDefinition->pcInit->mpcDeclarator->GetName();
		break;
	default:
		psDefinition->pcName = mcInstantiator.MemberVariableTemplateName(psDefinition->pcSimple);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// template<> int S<char>::f() { ... }: S<char>'s f is this, not the one
// its class template has.  Kept, and given to the instance it is of when
// it is laid out, or now if it has been.  [temp.expl.spec] 16
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::MemberSpecialisation(CCPPSyntaxFunctionDefinition* pcDefinition)
{
	CCPPClass*	pcClass;
	size		i;

	AddOutsideDefinition(CPPOD_MemberSpecialisation, NULL, NULL, pcDefinition);
	for (i = 0; i < mcClasses.GetClassList()->NumElements(); i++)
	{
		pcClass = mcClasses.GetClassList()->GetPtr(i);
		if (pcClass->mpcTemplate && pcClass->IsComplete())
		{
			SpecialiseMember(pcClass, pcDefinition);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The explicit specialisations of members that are of the instance.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SpecialiseMembers(CCPPClass* pcClass)
{
	size	i;

	for (i = 0; i < masOutsideDefinitions.NumElements(); i++)
	{
		if (masOutsideDefinitions.Get(i)->eKind == CPPOD_MemberSpecialisation)
		{
			SpecialiseMember(pcClass, masOutsideDefinitions.Get(i)->pcFunction);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name's qualifier, S<char>, names the class template instance
// pcClass: its class template, with arguments that make the same instance.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::QualifierIsInstance(CCPPSyntaxNameComponent* pcQualifier, CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPTemplate*	pcOwner;
	CChars			szArguments;
	bool			bSame;

	pcOwner = pcQualifier ? mcInstantiator.GetComponentTemplate(pcQualifier) : NULL;
	if (!pcOwner || !pcClass->mpcTemplate || (pcOwner != pcClass->mpcTemplate->mpcPrimary) || !pcQualifier->mbTemplateArguments)
	{
		return false;
	}
	szArguments.Init();
	mcDiagnostics.BeginQuiet();
	bSame = mcInstantiator.BindArguments(pcOwner->mpcDeclaration, &pcQualifier->macpTemplateArguments, NULL, &szArguments, pcNode, NULL) && szArguments.Equals(&pcClass->mszArguments);
	mcDiagnostics.EndQuiet();
	szArguments.Kill();
	return bSame;
}


//////////////////////////////////////////////////////////////////////////
//
// If the explicit specialisation is of a member of the instance (its
// class template, and arguments that make the same instance), the member
// is defined by it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SpecialiseMember(CCPPClass* pcClass, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	CCPPFunction*	pcFunction;

	if (!QualifierIsInstance(pcDefinition->mpcDeclarator->GetName()->GetQualifier(), pcClass, pcDefinition))
	{
		return;
	}

	mcContext.EnterBindings(pcClass->mpcBindings);
	mcDiagnostics.BeginQuiet();
	pcFunction = mcFunctionDeclarer.Declare(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, pcClass, CPPDA_Definition);
	mcDiagnostics.EndQuiet();
	mcContext.LeaveBindings();
	if (pcFunction && (pcDefinition->meBody == CPPFB_Body))
	{
		pcFunction->mpcDefinition = pcDefinition;
		pcFunction->mpcBindings = pcClass->mpcBindings;
		pcFunction->mbInline = true;
		if (pcFunction->mbCalled)
		{
			RequireFunction(pcFunction);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of a class template gets its function defined outside the template
// (pcMember).  Its template parameters are new names for the class's.  A function
// called before its definition was seen is generated now, as C++ instantiates it
// wherever in the unit its definition is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DefineOutsideMember(CCPPClass* pcClass, CCPPTemplate* pcMember)
{
	CCPPTemplateArguments*			pcMemberBindings;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPFunction*					pcFunction;

	pcMemberBindings = mcInstantiator.RenameBindings(pcClass, pcMember->mpcDeclaration);
	pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember->mpcDefinition;
	mcContext.EnterBindings(pcMemberBindings);
	mcDiagnostics.BeginQuiet();
	pcFunction = mcFunctionDeclarer.Declare(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, pcClass, CPPDA_Definition);
	mcDiagnostics.EndQuiet();
	mcContext.LeaveBindings();
	if (pcFunction && (pcDefinition->meBody == CPPFB_Body) && !pcFunction->mpcDefinition)
	{
		pcFunction->mpcDefinition = pcDefinition;
		pcFunction->mpcBindings = pcMemberBindings;
		pcFunction->mbInline = true;
		if (pcFunction->mbCalled)
		{
			RequireFunction(pcFunction);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcMember, a function defined outside a class template, is the
// class's: of the template (or the partial specialisation) the class is an
// instance of, or of the member class of a class template (template<class
// T> int Outer<T>::Inner::Size() { ... }, which has no owner: its symbol is
// the member class's) that the class is, in one instance.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsOutsideMemberOf(CCPPTemplate* pcMember, CCPPClass* pcClass)
{
	if (!pcMember->mbMember || pcMember->mbMemberTemplate)
	{
		return false;
	}
	if (pcClass->mpcTemplate)
	{
		return pcMember->mpcOwner == pcClass->mpcTemplate;
	}
	return pcClass->mpcEnclosing && !pcMember->mpcOwner && (pcMember->mpcSymbol == pcClass->mpcSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// A function of a class template defined outside it (pcMember) after some of its
// instances were laid out (a CChars used whole by a header Chars.cpp includes
// before CCharsBase's functions): they get it too.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DefineOutsideMembers(CCPPTemplate* pcMember)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; i < mcClasses.GetClassList()->NumElements(); i++)
	{
		pcClass = mcClasses.GetClassList()->GetPtr(i);
		if (IsOutsideMemberOf(pcMember, pcClass) && pcClass->IsComplete())
		{
			mcContext.EnterBindings(pcClass->mpcBindings);
			DefineOutsideMember(pcClass, pcMember);
			mcContext.LeaveBindings();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The identifier of a name component, or -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPUnit::GetComponentName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;

	if (!pcComponent || (pcComponent->miName < 0))
	{
		return -1;
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken->IsIdentifier())
	{
		return -1;
	}
	return ((CCTIdentifier*)pcToken)->miIdentifier;
}
