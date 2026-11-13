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
#include "CPPSyntaxTree.h"
#include "CPPSemanticPass.h"
#include "CPP65Generator.h"


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
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Check(CCPPSemanticPass* pcPass)
{
	CCPPSyntaxTranslationUnit*	pcRoot;

	mpcSemanticPass = pcPass;
	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
		CheckClasses(&pcRoot->macpDeclarations);
	}
	while (CheckDefinitions())
	{
	}
	ReportClassErrors();
	mpcSemanticPass = NULL;
	return mcErrors.NumErrors() == 0;
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
bool CCPP65Generator::CheckDefinitions(void)
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
void CCPP65Generator::CheckClasses(CArraySyntax* papcDeclarations)
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
void CCPP65Generator::CheckFunction(CCPPFunction* pcFunction)
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
void CCPP65Generator::CheckNode(CCPPSyntax* pcNode)
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


//Checks each of a node's children (CCPP65Generator::CheckChildren).
class CCPP65CheckChildren : public CCPPSyntaxChildVisitor
{
public:
	CCPP65Generator*	mpcGenerator;

	void	Visit(CCPPSyntax* pcChild) override	{ mpcGenerator->CheckNode(pcChild); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::CheckChildren(CCPPSyntax* pcNode)
{
	CCPP65CheckChildren	cChildren;

	cChildren.mpcGenerator = this;
	pcNode->VisitFields(&cChildren);
}


//////////////////////////////////////////////////////////////////////////
//
// An expression is typed as a whole.  If it can't be, it is an error and
// its parts are typed on their own (quietly), so the names in them are
// still resolved: in f(a.Get(), x) with no f that fits, a.Get() still is.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::CheckExpression(CCPPSyntax* pcExpression)
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
void CCPP65Generator::CheckLocal(CCPPSyntaxInitDeclarator* pcInit)
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
bool CCPP65Generator::IsCheckedExpression(CCPPSyntax* pcNode)
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
void CCPP65Generator::Resolved(CCPPSyntaxName* pcName, CCPPFunction* pcFunction)
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
void CCPP65Generator::Resolved(CCPPSyntaxName* pcName, CCPPSymbol* pcSymbol)
{
	if (!mpcSemanticPass || !pcName || !pcName->GetLast())
	{
		return;
	}
	mpcSemanticPass->Resolved(pcName->GetLast()->miName, pcSymbol, NULL);
}

