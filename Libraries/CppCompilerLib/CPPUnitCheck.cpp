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
#include "CPPSyntaxTree.h"
#include "CPPSemanticPass.h"
#include "CPPUnit.h"


//////////////////////////////////////////////////////////////////////////
//
// Works out the types of the unit's expressions without generating any
// code, for pcPass, which is told what names resolved to.  Every class the
// unit defines is completed and every function it defines is checked
// (inline ones, member functions defined in their classes and template
// instances too), each statement on its own: one that can't be typed is
// an error and the others are still checked.  Returns false if anything
// couldn't be (GetErrors).
//
// A declaration is only wrong to the compiler when what it declares is
// used, so a type nothing declares in one that nothing uses ("int
// Use(CLater* pc);" in a class, "CNever* Make(void);") was no error: a
// check says so at the end (ReportUndeclaredTypes), as a C++ compiler
// would where the name is written.
//
// With bRemember the pass is also told the type of each expression and
// what changes the place of one (Typed, PlaceTyped and Changed), for a
// check that is kept (CCPPSemanticPass::Check).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Check(CCPPSemanticPass* pcPass, bool bRemember)
{
	CCPPSyntaxTranslationUnit*	pcRoot;

	mpcSemanticPass = pcPass;
	mbRemember = bRemember;
	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
		CheckClasses(&pcRoot->macpDeclarations);
	}
	do
	{
		if (mbRemember)
		{
			CheckTemplatesAsThemselves();
		}
	}
	while (CheckDefinitions());
	ReportClassErrors();
	ReportUndeclaredTypes();
	CheckContext();
	mpcSemanticPass = NULL;
	mbRemember = false;
	return mcErrors.NumErrors() == 0;
}


//Finds the types a unit's declarations name that nothing declares (CCPPUnit::ReportUndeclaredTypes).
class CCPPUndeclaredTypeFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*	mpcUnit;
	int			miTemplates;	//How many template declarations the node is in.

	void	Visit(CCPPSyntax* pcChild) override
	{
		bool	bTemplate;

		bTemplate = (pcChild->GetKind() == CPPS_TemplateDeclaration);
		miTemplates += bTemplate ? 1 : 0;
		if ((pcChild->GetKind() == CPPS_NamedTypeSpecifier) && (miTemplates == 0))
		{
			mpcUnit->UndeclaredType((CCPPSyntaxNamedTypeSpecifier*)pcChild);
		}
		pcChild->VisitFields(this);
		miTemplates -= bTemplate ? 1 : 0;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// An error for each type a declaration names that nothing declares,
// whether or not what it declares is used.  Not in a template (a name
// there may be one only an instance has), and not in the compiler's own
// headers, which are skimmed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportUndeclaredTypes(void)
{
	CCPPUndeclaredTypeFinder	cFinder;

	if (mpcTree->GetRoot())
	{
		cFinder.mpcUnit = this;
		cFinder.miTemplates = 0;
		mpcTree->GetRoot()->VisitFields(&cFinder);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A type named by one plain name that the parser found nothing for: an
// error at the name, unless there is one there already (what it declares
// was used).  A name with a scope before it is left: its scope may be one
// the parser can't look in.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::UndeclaredType(CCPPSyntaxNamedTypeSpecifier* pcNamed)
{
	SPreprocessedToken*	psSource;
	CCPPToken*			pcToken;
	size				i;
	int					iToken;

	if (!pcNamed->mpcName || pcNamed->mpcName->mpcSymbol || !pcNamed->mpcName->IsSimple())
	{
		return;
	}
	iToken = pcNamed->mpcName->miFirstToken;
	pcToken = (iToken >= 0) ? mpcTokeniser->Get((size)iToken) : NULL;
	psSource = pcToken ? mpcTokeniser->GetSource(pcToken) : NULL;
	if (!psSource || (psSource->pcFile && psSource->pcFile->IsSystemFile()))
	{
		return;
	}
	for (i = 0; i < mcErrors.NumErrors(); i++)
	{
		if (mcErrors.GetToken(i) == iToken)
		{
			return;
		}
	}
	ErrorAt(iToken, "This type is not declared");
}


//////////////////////////////////////////////////////////////////////////
//
// Checks the functions waiting to be generated, then every function with
// a definition not checked yet.  Checking can declare more (a template's
// instances, a class's members), so it is done until it returns false.
// A member of a class template's instance that nothing calls isn't
// instantiated in C++, so what is wrong in it isn't an error: it is
// checked quietly, only for its names.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckDefinitions(void)
{
	CCPPFunction*	pcFunction;
	size			i;
	bool			bChecked;

	bChecked = GenerateQueued();
	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		if (pcFunction->mpcDefinition && !pcFunction->mbGenerated && !pcFunction->mbImplicit)
		{
			if (pcFunction->mpcBindings)
			{
				BeginQuiet();
				GenerateFunction(pcFunction);
				EndQuiet();
			}
			else
			{
				GenerateFunction(pcFunction);
			}
			bChecked = true;
		}
	}
	return bChecked;
}


//////////////////////////////////////////////////////////////////////////
//
// Completes every class the declarations define (in namespaces and
// extern "C" too), so their member functions are declared and those
// defined in the class are checked, whether or not the unit uses them.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckClasses(CArraySyntax* papcDeclarations)
{
	size							i;
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxClassSpecifier*		pcClassSpecifier;
	CCPPSymbol*						pcSymbol;
	CCPPClass*					pcClass;

	for (i = 0; i < papcDeclarations->NumElements(); i++)
	{
		pcDeclaration = papcDeclarations->GetPtr(i);
		switch (pcDeclaration->GetKind())
		{
		case CPPS_SimpleDeclaration:
			pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
			if (!pcSimple->mpcSpecifiers || !pcSimple->mpcSpecifiers->mpcType || (pcSimple->mpcSpecifiers->mpcType->GetKind() != CPPS_ClassSpecifier))
			{
				break;
			}
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
			pcSymbol = pcClassSpecifier->mpcSymbol;
			if (pcSymbol && (pcSymbol->meKind == CPPSK_Class))
			{
				pcClass = GetClass(pcSymbol, pcClassSpecifier);
				CompleteClass(pcClass, pcClassSpecifier);
			}
			break;
		case CPPS_LinkageSpecification:
			CheckClasses(&((CCPPSyntaxLinkageSpecification*)pcDeclaration)->macpDeclarations);
			break;
		case CPPS_Namespace:
			CheckClasses(&((CCPPSyntaxNamespace*)pcDeclaration)->macpDeclarations);
			break;
		default:
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function's parameters and locals are given their types, then each
// of its member initialisers and statements is checked.  A local that
// isn't supported is an error; the rest of the function is still checked.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckFunction(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	size							i;

	pcDefinition = pcFunction->mpcDefinition;
	if (!pcDefinition || pcFunction->mbImplicit)
	{
		return;
	}

	BeginFunction(pcFunction);
	AddParameters(pcFunction);
	AllocateLocals(pcDefinition->mpcBody);
	for (i = 0; i < pcDefinition->macpMemberInitializers.NumElements(); i++)
	{
		CheckChildren(pcDefinition->macpMemberInitializers.GetPtr(i));
	}
	CheckNode(pcDefinition->mpcBody);
	EndFunction(pcFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// The expressions in a statement (or in anything else in a function):
// each is typed, and each local is initialised.  Classes and enums
// declared in a function aren't looked into.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckNode(CCPPSyntax* pcNode)
{
	if (!pcNode)
	{
		return;
	}
	if (IsCheckedExpression(pcNode))
	{
		CheckExpression(pcNode);
		return;
	}
	switch (pcNode->GetKind())
	{
	case CPPS_InitDeclarator:
		if (pcNode->mpcParent && (pcNode->mpcParent->GetKind() == CPPS_SimpleDeclaration))
		{
			CheckLocal((CCPPSyntaxInitDeclarator*)pcNode);
			return;
		}
		break;
	case CPPS_ClassSpecifier:
	case CPPS_EnumSpecifier:
	case CPPS_LazyDeclaration:
	case CPPS_SkippedStatement:
	case CPPS_SkippedDeclaration:
	case CPPS_StaticAssert:
		return;
	default:
		break;
	}
	CheckChildren(pcNode);
}


//Checks each of a node's children (CCPPUnit::CheckChildren).
class CCPP65CheckChildren : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*	mpcUnit;

	void	Visit(CCPPSyntax* pcChild) override	{ mpcUnit->CheckNode(pcChild); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckChildren(CCPPSyntax* pcNode)
{
	CCPP65CheckChildren	cChildren;

	cChildren.mpcUnit = this;
	pcNode->VisitFields(&cChildren);
}


//////////////////////////////////////////////////////////////////////////
//
// An expression is typed as a whole.  If it can't be, it is an error and
// its parts are typed on their own (quietly), so the names in them are
// still resolved: in f(a.Get(), x) with no f that fits, a.Get() still is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckExpression(CCPPSyntax* pcExpression)
{
	if (mcTypeChecker.Value(pcExpression))
	{
		return;
	}
	BeginQuiet();
	CheckChildren(pcExpression);
	EndQuiet();
}


//////////////////////////////////////////////////////////////////////////
//
// A local (or static local) variable's initialiser, as the IR front end
// initialises it.  If it can't be, its expressions are typed on their own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckLocal(CCPPSyntaxInitDeclarator* pcInit)
{
	CCPPVariable*	pcVariable;

	if (pcInit->mpcDeclarator->IsFunction())
	{
		DeclareBlockFunction(((CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent)->mpcSpecifiers, pcInit->mpcDeclarator);
		return;
	}
	pcVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
	if (pcVariable && mcTypeChecker.Initialise(pcVariable->mpsType, pcInit->mpcInitializer, pcInit))
	{
		return;
	}
	if (pcInit->mpcInitializer)
	{
		BeginQuiet();
		CheckNode(pcInit->mpcInitializer);
		EndQuiet();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The expressions the type checker types (not initialiser lists, which
// are only typed for what they initialise).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsCheckedExpression(CCPPSyntax* pcNode)
{
	ECPPSyntax	eKind;

	eKind = pcNode->GetKind();
	if ((eKind == CPPS_InitializerList) || (eKind == CPPS_ParenthesisedInitializer))
	{
		return false;
	}
	return (eKind >= CPPS_LiteralExpression) && (eKind <= CPPS_NoexceptExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found which function pcName calls (or names): the
// semantic pass is told, when checking.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Resolved(CCPPSyntaxName* pcName, CCPPFunction* pcFunction)
{
	CCPPSyntaxName*	pcDeclared;
	CCPPSymbol*		pcSymbol;

	if (!mpcSemanticPass || !pcName || !pcName->GetLast() || !pcFunction)
	{
		return;
	}
	pcSymbol = NULL;
	if (pcFunction->mpcDeclarator)
	{
		pcDeclared = pcFunction->mpcDeclarator->GetName();
		pcSymbol = pcDeclared ? pcDeclared->mpcSymbol : NULL;
	}
	if (!pcSymbol)
	{
		pcSymbol = pcName->mpcSymbol;
	}
	mpcSemanticPass->Resolved(pcName->GetLast()->miName, pcSymbol, pcFunction->mpcDeclarator);
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the member (a field or a static member) pcName
// names.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Resolved(CCPPSyntaxName* pcName, CCPPSymbol* pcSymbol)
{
	if (!mpcSemanticPass || !pcName || !pcName->GetLast())
	{
		return;
	}
	mpcSemanticPass->Resolved(pcName->GetLast()->miName, pcSymbol, NULL);
}



//////////////////////////////////////////////////////////////////////////
//
// A kept check has every template the unit declares as itself too
// (Refactorings.txt in the IDE, T3): a class template's instance with its
// parameters standing for themselves is completed, so its member
// functions are declared, those defined outside it with them; and a
// function template's, and a member template's of a class that isn't an
// instance (or is one as itself), is declared.  CheckDefinitions then
// checks their bodies, quietly: what is in a body that only a template
// argument would have (pv->Init(), a > b) can't be typed, which is no
// error, and the rest is typed in the template's own words, M* and
// CArray<M>&.  Completing a class can declare more templates (its member
// templates), so it is done before each round of definitions.
//
// An explicit specialisation has no parameters and is what it is; a
// partial specialisation is left (its instance as itself would need the
// pattern's arguments).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckTemplatesAsThemselves(void)
{
	CCPPTemplate*	pcTemplate;
	CCPPClass*		pcClass;
	size			i;

	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbAsItself)
		{
			continue;
		}
		pcTemplate->mbAsItself = true;
		if (!pcTemplate->mpcDeclaration || (pcTemplate->mpcDeclaration->macpParameters.NumElements() == 0) || (pcTemplate->mpcPrimary != pcTemplate))
		{
			continue;
		}

		if (pcTemplate->mpcDefinition && (pcTemplate->mpcDefinition->GetKind() == CPPS_ClassSpecifier))
		{
			BeginQuiet();
			pcClass = InstantiateClassAsItself(pcTemplate);
			if (pcClass)
			{
				CompleteClass(pcClass, pcTemplate->mpcDefinition);
			}
			EndQuiet();
		}
		else if (!pcTemplate->mbMember || (pcTemplate->mbMemberTemplate && pcTemplate->mpcClass && (!pcTemplate->mpcClass->mpcBindings || pcTemplate->mpcClass->mpcBindings->IsSelf())))
		{
			//A function template, or a member template in a class that isn't another instance.  A class template's
			//members defined outside it are declared with its instance (LayoutClass).
			BeginQuiet();
			InstantiateFunctionAsItself(pcTemplate);
			EndQuiet();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what is being checked is written as it is checked: it is not
// in a template, or the template's parameters stand for themselves.  In
// an instance with arguments an expression's type is the instance's
// (CFoo*, where the template says M*), which isn't the type of what is
// written.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsCheckedAsWritten(void)
{
	return !mcContext.GetBindings() || mcContext.GetBindings()->IsSelf();
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker typed pcExpression: a kept check's pass is told, if
// it is checked as written.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Typed(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	if (mbRemember && IsCheckedAsWritten())
	{
		mpcSemanticPass->Typed(pcExpression, psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the type of the place pcExpression is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	if (mbRemember && IsCheckedAsWritten())
	{
		mpcSemanticPass->PlaceTyped(pcExpression, psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the place pcExpression is can be changed there
// (CPPPC_): bound to a reference that isn't const, its address taken, or
// a member function that isn't const called on it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Changed(CCPPSyntax* pcExpression, uint8 uiChanges)
{
	if (mbRemember)
	{
		mpcSemanticPass->Changed(pcExpression, uiChanges);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function pcDeclarator declares or defines, NULL if the unit has
// none: not an instance of a template, which the template's declarator
// is shared by.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::FindDeclaredFunction(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPFunction*	pcFunction;
	size			i;

	for (i = 0; pcDeclarator && (i < mapcFunctions.NumElements()); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		if (pcFunction->mpcBindings || pcFunction->mpcTemplate)
		{
			continue;
		}
		if ((pcFunction->mpcDeclarator == pcDeclarator) || (pcFunction->mpcDefinition && (pcFunction->mpcDefinition->mpcDeclarator == pcDeclarator)))
		{
			return pcFunction;
		}
	}
	return NULL;
}

