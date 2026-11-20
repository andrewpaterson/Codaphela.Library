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
#include "CPPUnit.h"
#include "CPPSymbolKey.h"
#include <ctype.h>


//The generator's templates: declaring them, binding and deducing their arguments, and instantiating classes and functions.


//////////////////////////////////////////////////////////////////////////
//
// True if the declaration defines a static data member of a class
// template: its declarator's name is qualified by the class template.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStaticMemberDefinition(CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxName*				pcName;

	if ((pcSimple->macpDeclarators.NumElements() != 1) || (pcSimple->macpDeclarators.GetPtr(0)->GetKind() != CPPS_InitDeclarator))
	{
		return false;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	return pcName && !pcInit->mpcDeclarator->IsFunction() && pcName->IsQualifiedByClass() && pcName->GetQualifier() && pcName->GetQualifier()->mbTemplateArguments && GetComponentTemplate(pcName->GetQualifier());
}


//////////////////////////////////////////////////////////////////////////
//
// The data of templates' instances, defined when the unit's functions are
// written (as an initialiser that isn't a constant is a start up routine):
// class template instances' static members (DefineInstanceStatics) and
// variable templates' instances (DefineVariableInstances), until none is
// left, as one's may need more.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::DefineInstanceData(void)
{
	bool	bResult;
	bool	bDefined;

	bResult = true;
	do
	{
		bDefined = DefineInstanceStatics(&bResult);
		bDefined |= DefineVariableInstances(&bResult);
		if (bDefined)
		{
			GenerateQueued();
		}
	} while (bDefined);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Defines each class template instance's static data members that a
// template declaration outside the class defines and that aren't yet, as
// the instance's data (every unit that has the instance has it, and the
// link keeps one), with the template's parameters its arguments.  True if
// it defined one; *pbResult false after an error.  [temp.static]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::DefineInstanceStatics(bool* pbResult)
{
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcOwner;
	CCPPClass*						pcClass;
	size							i;
	size							j;
	bool							bDefined;

	bDefined = false;
	for (i = 0; i < mapcStaticMemberTemplates.NumElements(); i++)
	{
		pcDeclaration = (CCPPSyntaxTemplateDeclaration*)mapcStaticMemberTemplates.GetPtr(i);
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcDeclaration;
		pcName = ((CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0))->mpcDeclarator->GetName();
		pcOwner = GetComponentTemplate(pcName->GetQualifier());
		for (j = 0; j < mapcClasses.NumElements(); j++)
		{
			pcClass = mapcClasses.GetPtr(j);
			if (!pcClass->mpcTemplate || (pcClass->mpcTemplate != pcOwner) || !pcClass->IsComplete() || pcClass->mbExternInstance || IsInstanceStaticDefined(pcClass, pcName))
			{
				continue;
			}
			mcContext.EnterBindings(RenameBindings(pcClass, pcDeclaration));
			mpcDefining = OwnStatic(pcClass, pcName->mpcSymbol);
			*pbResult &= GlobalDeclaration(pcSimple);
			mpcDefining = NULL;
			mcContext.LeaveBindings();
			bDefined = true;
		}
	}
	return bDefined;
}


//////////////////////////////////////////////////////////////////////////
//
// Defines each variable template instance that isn't yet, as its own data
// (every unit that has the instance has it, and the link keeps one), with
// its template's parameters its arguments, or as its explicit
// specialisation.  True if it defined one; *pbResult false after an error.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::DefineVariableInstances(bool* pbResult)
{
	CCPPVariable*	pcVariable;
	size			i;
	bool			bDefined;

	bDefined = false;
	for (i = 0; i < mapcVariableInstances.NumElements(); i++)
	{
		pcVariable = mapcVariableInstances.GetPtr(i);
		if (pcVariable->mbDefined)
		{
			continue;
		}
		mcContext.EnterBindings(pcVariable->mpcBindings);
		mpcDefining = pcVariable;
		*pbResult &= GlobalDeclaration((CCPPSyntaxSimpleDeclaration*)pcVariable->mpcTemplate->mpcDefinition);
		mpcDefining = NULL;
		mcContext.LeaveBindings();
		pcVariable->mbDefined = true;
		bDefined = true;
	}
	return bDefined;
}


//////////////////////////////////////////////////////////////////////////
//
// The name a variable template's declaration declares (pi in
// template<class T> T pi = T(3);, size<char> in an explicit
// specialisation), or NULL if the declaration isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxName* CCPPUnit::VariableTemplateName(CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxName*				pcName;

	if ((pcSimple->macpDeclarators.NumElements() != 1) || (pcSimple->macpDeclarators.GetPtr(0)->GetKind() != CPPS_InitDeclarator))
	{
		return NULL;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	if (!pcName || pcInit->mpcDeclarator->IsFunction() || !pcName->mpcSymbol || (pcName->mpcSymbol->meKind != CPPSK_VariableTemplate) || (pcName->macpComponents.NumElements() != 1))
	{
		return NULL;
	}
	return pcName;
}


//////////////////////////////////////////////////////////////////////////
//
// The init-declarator a variable is declared by: its symbol's, or for a
// variable template's instance its template's (or explicit
// specialisation's), whose symbol all its instances share.  NULL if it
// has none.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxInitDeclarator* CCPPUnit::VariableDeclarator(CCPPVariable* pcVariable)
{
	CCPPSyntax*	pcDeclaration;

	if (pcVariable->mpcTemplate)
	{
		return (CCPPSyntaxInitDeclarator*)((CCPPSyntaxSimpleDeclaration*)pcVariable->mpcTemplate->mpcDefinition)->macpDeclarators.GetPtr(0);
	}
	pcDeclaration = pcVariable->mpcSymbol ? pcVariable->mpcSymbol->mpcDeclaration : NULL;
	return (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator)) ? (CCPPSyntaxInitDeclarator*)pcDeclaration : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable a variable template's name with its arguments (pi<int>)
// names: its instance for the arguments, made the first time it is named,
// of its explicit specialisation for them if it has one.  Defined with the
// unit's data (DefineVariableInstances).  NULL after an error.
// [temp.variable]
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::VariableInstance(CCPPSymbol* pcSymbol, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcTemplate;
	CCPPTemplate*				pcSource;
	CCPPTemplateArguments*		pcBindings;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPVariable*				pcVariable;
	SCPP65Type*					psType;
	CChars						szArguments;
	size						i;

	pcTemplate = GetTemplate(pcSymbol);
	if (!pcTemplate || !pcComponent || !pcComponent->mbTemplateArguments)
	{
		Error(pcNode, "A variable template is named with its template arguments");
		return NULL;
	}
	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, &pcComponent->macpTemplateArguments, NULL, &szArguments, pcNode);
	pcVariable = NULL;
	for (i = 0; pcBindings && !pcVariable && (i < mapcVariableInstances.NumElements()); i++)
	{
		if ((mapcVariableInstances.GetPtr(i)->mpcSymbol == pcSymbol) && mapcVariableInstances.GetPtr(i)->mszArguments.Equals(&szArguments))
		{
			pcVariable = mapcVariableInstances.GetPtr(i);
		}
	}
	if (!pcBindings || pcVariable)
	{
		szArguments.Kill();
		return pcVariable;
	}

	pcSource = FindVariableSpecialisation(pcTemplate, &szArguments, pcNode);
	pcBindings = pcSource ? NULL : pcBindings;
	pcSource = pcSource ? pcSource : pcTemplate;
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcSource->mpcDefinition;
	pcInit = (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0);
	mcContext.EnterBindings(pcBindings);
	psType = mcDeclarators.GetType(pcSimple->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	mcContext.LeaveBindings();
	if (psType)
	{
		pcVariable = AddVariable(&mapcGlobals, pcSymbol, psType, CPPVS_Global, 0);
		pcVariable->mpcTemplate = pcSource;
		pcVariable->mpcBindings = pcBindings;
		pcVariable->mszArguments.Append(&szArguments);
		mapcVariableInstances.Add(pcVariable);
	}
	szArguments.Kill();
	return pcVariable;
}


//////////////////////////////////////////////////////////////////////////
//
// The explicit specialisation of a variable template for the mangled
// arguments (template<> int size<char> = 9;), or NULL.  A specialisation's
// arguments are worked out once.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::FindVariableSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplate*	pcTemplate;
	size			i;

	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || !pcTemplate->mpcPattern)
		{
			continue;
		}
		if (!pcTemplate->mbArguments)
		{
			pcTemplate->mbArguments = true;
			BeginQuiet();
			if (!BindArguments(pcPrimary->mpcDeclaration, &pcTemplate->mpcPattern->macpTemplateArguments, NULL, &pcTemplate->mszArguments, pcNode))
			{
				pcTemplate->mszArguments.Clear();
			}
			EndQuiet();
		}
		if (!pcTemplate->mszArguments.Empty() && pcTemplate->mszArguments.Equals(pszArguments))
		{
			return pcTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the instance's static member the name is of is defined, or it
// has none of the name (it can't be).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsInstanceStaticDefined(CCPPClass* pcClass, CCPPSyntaxName* pcName)
{
	CCPPVariable*	pcVariable;

	pcVariable = OwnStatic(pcClass, pcName->mpcSymbol);
	return !pcVariable || pcVariable->mbDefined;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's own static data member of the symbol (each instance of a
// class template has one, all of the symbol), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::OwnStatic(CCPPClass* pcClass, CCPPSymbol* pcSymbol)
{
	CCPPVariable*	pcVariable;
	size			i;

	for (i = 0; i < mapcGlobals.NumElements(); i++)
	{
		pcVariable = mapcGlobals.GetPtr(i);
		if ((pcVariable->mpcClass == pcClass) && (pcVariable->mpcSymbol == pcSymbol))
		{
			return pcVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Records a template declaration: a class template, a function template, a member
// function of a class template defined outside it, a member template defined outside
// its class (template<class M> template<class N> ... for a class template), or an
// explicit or partial specialisation of a class or function template, or an
// explicit specialisation of a class template's member function
// (MemberSpecialisation).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration)
{
	CCPPTemplate*					pcTemplate;
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxTemplateDeclaration*	pcClassTemplate;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxName*					pcName;
	CCPPSyntaxNameComponent*		pcClassName;
	CCPPTemplate*					pcOwnerTemplate;
	bool							bExplicit;

	pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
	if (!pcDeclaration)
	{
		return true;
	}
	if (pcTemplateDeclaration->mbInstantiation)
	{
		return ExplicitInstantiation(pcDeclaration, pcTemplateDeclaration->mbExtern);
	}
	bExplicit = pcTemplateDeclaration->macpParameters.NumElements() == 0;

	//template<class M> template<class N> N SBox<M>::Mul(N n) { ... }
	pcClassTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_TemplateDeclaration) && !bExplicit)
	{
		pcClassTemplate = pcTemplateDeclaration;
		pcTemplateDeclaration = (CCPPSyntaxTemplateDeclaration*)pcDeclaration;
		pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
		if (pcTemplateDeclaration->mbInstantiation || (pcTemplateDeclaration->macpParameters.NumElements() == 0) || !pcDeclaration || (pcDeclaration->GetKind() != CPPS_FunctionDefinition))
		{
			return true;
		}
	}

	pcTemplate = NULL;
	if ((pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && !pcClassTemplate)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		if (pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			//template<class T> struct SAlign<T*> { ... } and template<> struct SAlign<char> { ... } specialise SAlign.
			pcSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
			pcClassName = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
			if (pcClassName && pcClassName->mbTemplateArguments)
			{
				Specialisation(pcSpecifier->mpcSymbol, pcTemplateDeclaration, pcSpecifier, pcClassName);
			}
			else if (!bExplicit)
			{
				DefineClassTemplate(pcSpecifier->mpcSymbol, pcTemplateDeclaration, pcSpecifier);
			}
		}
		else if (!bExplicit && pcSimple->mpcSpecifiers && pcSimple->mpcSpecifiers->mpcType && (pcSimple->mpcSpecifiers->mpcType->GetKind() == CPPS_ElaboratedTypeSpecifier) && (pcSimple->macpDeclarators.NumElements() == 0))
		{
			//template<class... T> struct Tuple; is a template its specialisations can specialise.
			DeclareClassTemplate((CCPPSyntaxElaboratedTypeSpecifier*)pcSimple->mpcSpecifiers->mpcType, pcTemplateDeclaration);
		}
		else if (!bExplicit && IsStaticMemberDefinition(pcSimple))
		{
			//template<class T> int S<T>::count = 3; is each instance's (DefineInstanceStatics).
			mapcStaticMemberTemplates.Add(pcTemplateDeclaration);
		}
		else if (VariableTemplateName(pcSimple))
		{
			//template<class T> T pi = T(3); and template<> int size<char> = 9;
			pcName = VariableTemplateName(pcSimple);
			if (bExplicit)
			{
				Specialisation(pcName->mpcSymbol, pcTemplateDeclaration, pcSimple, pcName->GetLast());
			}
			else
			{
				NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcSimple, false);
			}
		}
		else if (!bExplicit)
		{
			//template<class T> T Twice(T v);
			DeclareFunctionTemplate(pcTemplateDeclaration, pcSimple);
		}
	}
	else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcDeclaration;
		pcName = pcFunction->mpcDeclarator->GetName();
		if (pcName && pcName->IsQualifiedByClass() && !bExplicit)
		{
			//template<class M> M* CArrayTemplate<M>::Get(size iIndex) { ... }
			pcClassName = pcName->GetQualifier();
			pcOwnerTemplate = GetComponentTemplate(pcClassName);
			if (pcOwnerTemplate && pcClassTemplate)
			{
				pcTemplate = NewTemplate(pcOwnerTemplate->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mbMemberTemplate = true;
				pcTemplate->mpcClassDeclaration = pcClassTemplate;
			}
			else if (pcOwnerTemplate)
			{
				pcTemplate = NewTemplate(pcOwnerTemplate->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mpcOwner = OutsideMemberOwner(pcOwnerTemplate, pcClassName, pcTemplateDeclaration);
				DefineOutsideMembers(pcTemplate);
			}
			else if (pcClassName->mpcSymbol && (pcClassName->mpcSymbol->meKind == CPPSK_Class))
			{
				//template<class T> int CValue::Size(T* p) { ... }
				pcTemplate = NewTemplate(pcClassName->mpcSymbol, pcTemplateDeclaration, pcFunction, true);
				pcTemplate->mbMemberTemplate = true;
			}
			if (pcTemplate && pcTemplate->mbMemberTemplate)
			{
				pcTemplate->mlliName = GetFunctionName(pcName->GetLast());
			}
		}
		else if (pcName && pcName->IsQualifiedByClass() && bExplicit && !pcClassTemplate)
		{
			//template<> int S<char>::f() { ... }
			MemberSpecialisation(pcFunction);
		}
		else if (pcName && (pcName->macpComponents.NumElements() == 1) && bExplicit && !pcClassTemplate)
		{
			//template<> int Size<char>(char c) { ... } and template<> int Size(short s) { ... }
			Specialisation(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction, pcName->GetLast());
		}
		else if (pcName && !pcClassTemplate && !bExplicit && (pcName->mpcSymbol || (pcName->GetLast()->meKind == CPPNC_Operator) || (pcName->GetLast()->meKind == CPPNC_Literal)))
		{
			//An operator template (and a literal operator template) has no symbol; it is found by its operator (FindFreeOperator).
			pcTemplate = FindFunctionTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction->mpcDeclarator);
			if (pcTemplate && !pcTemplate->mpcDefinition)
			{
				DefineFunctionTemplate(pcTemplate, pcTemplateDeclaration, pcFunction);
			}
			else
			{
				NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, pcFunction, false);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// template<class T> T Twice(T v); declares a function template: its
// instances are declarations (what sizeof, decltype and noexcept ask of a
// call needs no more) until it is defined.  Declared again, or after its
// definition, it is the same template.  [temp.fct], [basic.def.odr] 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DeclareFunctionTemplate(CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxSimpleDeclaration* pcSimple)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxName*			pcName;
	CCPPTemplate*			pcTemplate;

	pcDeclarator = (pcSimple->macpDeclarators.NumElements() == 1) ? ((CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0))->mpcDeclarator : NULL;
	pcName = (pcDeclarator && pcDeclarator->GetFunctionSuffix()) ? pcDeclarator->GetName() : NULL;
	if (!pcName || (pcName->macpComponents.NumElements() != 1) || !pcName->mpcSymbol || (pcName->mpcSymbol->meKind != CPPSK_FunctionTemplate) ||
		(pcSimple->mpcSpecifiers && (pcSimple->mpcSpecifiers->muiFlags & (CPPDS_FRIEND | CPPDS_TYPEDEF))))
	{
		return;
	}
	if (FindFunctionTemplate(pcName->mpcSymbol, pcDeclaration, pcDeclarator))
	{
		return;
	}
	pcTemplate = NewTemplate(pcName->mpcSymbol, pcDeclaration, NULL, false);
	pcTemplate->mpcDeclarator = pcDeclarator;
	pcTemplate->mpcSpecifiers = pcSimple->mpcSpecifiers;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template declared before is defined: the template is the
// definition's from here on, whose parameters' names its body uses.  The
// instances made of its declaration were declarations only, which nothing
// that is evaluated could call: they are left behind, and the next use of
// one makes it from the definition.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DefineFunctionTemplate(CCPPTemplate* pcTemplate, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	pcTemplate->mpcDeclaration = pcDeclaration;
	pcTemplate->mpcDefinition = pcDefinition;
	pcTemplate->mpcDeclarator = NULL;
	pcTemplate->mpcSpecifiers = NULL;
	pcTemplate->mapcInstances.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The function template of the name pcSymbol is, in its scope, that has
// this signature (FunctionTemplateSignature): the one a declaration
// declares again, or a definition defines.  NULL if there is none, and for
// an operator template, which has no symbol.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::FindFunctionTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplate*	pcFound;
	CChars			szSignature;
	CChars			szOther;
	size			i;

	if (!pcSymbol)
	{
		return NULL;
	}
	szSignature.Init();
	FunctionTemplateSignature(pcDeclaration, pcDeclarator, &szSignature);
	pcFound = NULL;
	for (i = 0; !pcFound && (i < mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcSymbol || !pcTemplate->IsFunction() ||
			(pcTemplate->mpcSymbol->mlliName != pcSymbol->mlliName) || (pcTemplate->mpcSymbol->mpcScope != pcSymbol->mpcScope))
		{
			continue;
		}
		szOther.Init();
		FunctionTemplateSignature(pcTemplate->mpcDeclaration, pcTemplate->FunctionDeclarator(), &szOther);
		pcFound = szOther.Equals(&szSignature) ? pcTemplate : NULL;
		szOther.Kill();
	}
	szSignature.Kill();
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template's parameters as they are written, after the number of
// its template parameters and with each of those named by its place: what
// template<class T> T f(T) and template<class U> U f(U u) { ... } have the
// same, and another overload of f hasn't.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FunctionTemplateSignature(CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest)
{
	CCPPSymbolKey	cKey;
	CChars			szWritten;
	CCPPSyntax*		pcParameter;
	CCPPSymbol*		pcSymbol;
	char*			szText;
	char*			szName;
	size			uiStart;
	size			uiEnd;
	size			i;
	int				iLength;
	bool			bNamed;

	szWritten.Init();
	cKey.Init(mpcTokeniser, mpcTree);
	cKey.GetSignature(pcDeclarator->GetFunctionSuffix(), &szWritten);
	cKey.Kill();

	pszDest->Append((int32)pcDeclaration->macpParameters.NumElements());
	szText = szWritten.Text();
	for (uiStart = 0; uiStart < szWritten.Length(); uiStart = uiEnd)
	{
		//A word, or one character that isn't of one.
		for (uiEnd = uiStart; (uiEnd < szWritten.Length()) && (isalnum((unsigned char)szText[uiEnd]) || (szText[uiEnd] == '_')); uiEnd++)
		{
		}
		if (uiEnd == uiStart)
		{
			uiEnd++;
			pszDest->Append(szText[uiStart]);
			continue;
		}
		bNamed = false;
		for (i = 0; !bNamed && (i < pcDeclaration->macpParameters.NumElements()); i++)
		{
			pcParameter = pcDeclaration->macpParameters.GetPtr(i);
			pcSymbol = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((pcParameter->GetKind() == CPPS_Parameter) ? ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol : NULL);
			szName = pcSymbol ? mpcTokeniser->GetIdentifier(pcSymbol->mlliName, &iLength) : NULL;
			if (szName && ((size)iLength == uiEnd - uiStart) && (memcmp(szName, &szText[uiStart], iLength) == 0))
			{
				pszDest->Append('$');
				pszDest->Append((int32)i);
				bNamed = true;
			}
		}
		if (!bNamed)
		{
			pszDest->Append(&szText[uiStart], uiEnd - uiStart);
		}
	}
	szWritten.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// template<class... T> struct Tuple; declares a class template: one its
// specialisations can specialise before it is defined, or if it never is,
// as a variadic template is often written.  Declared again, it is the
// same template.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DeclareClassTemplate(CCPPSyntaxElaboratedTypeSpecifier* pcSpecifier, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = (pcSpecifier->mpcName && (pcSpecifier->mpcName->macpComponents.NumElements() == 1)) ? pcSpecifier->mpcName->mpcSymbol : NULL;
	if (pcSymbol && !GetTemplate(pcSymbol))
	{
		NewTemplate(pcSymbol, pcDeclaration, pcSpecifier, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A class template's definition: the template it declared before, if it
// was, now with the definition's parameters and body, or else a new one.
// Its instances made before (A<int>* p;) are of the definition, their
// arguments bound to its parameters.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DefineClassTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPTemplate*	pcTemplate;
	CCPPClass*		pcClass;
	size			i;

	pcTemplate = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	if (!pcTemplate || !pcTemplate->IsDeclaredClass())
	{
		NewTemplate(pcSymbol, pcDeclaration, pcSpecifier, false);
		return;
	}

	pcTemplate->mpcDeclaration = pcDeclaration;
	pcTemplate->mpcDefinition = pcSpecifier;
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if ((pcClass->mpcTemplate == pcTemplate) && !pcClass->mpcSpecifier)
		{
			pcClass->mpcSpecifier = pcSpecifier;
			pcClass->mpcBindings = RenameBindings(pcClass, pcDeclaration);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Records a specialisation of the template pcSymbol names, which must be declared
// before it.  pcPattern holds its template arguments (a function specialisation's
// may have none: they are deduced from its parameters).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern)
{
	CCPPTemplate*	pcPrimary;
	CCPPTemplate*	pcTemplate;
	int64			lliName;

	//template<> int Size(short s) names a new function; its template is found by name.
	pcPrimary = pcSymbol ? GetTemplate(pcSymbol) : NULL;
	lliName = pcPattern ? GetComponentName(pcPattern) : -1;
	if (!pcPrimary && (lliName != -1))
	{
		pcPrimary = GetTemplateByName(lliName, pcDefinition->GetKind(), ComponentLookup(pcPattern));
	}
	if (!pcPrimary || (pcPrimary->DefinitionKind() != pcDefinition->GetKind()))
	{
		return NULL;
	}
	pcTemplate = NewTemplate(pcPrimary->mpcSymbol, pcDeclaration, pcDefinition, false);
	pcTemplate->mpcPrimary = pcPrimary;
	pcTemplate->mpcPattern = pcPattern;
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template a member function defined outside it belongs to: the primary
// when the class's arguments are the declaration's parameters in order
// (CArrayTemplate<M>), else the partial specialisation with the same arguments
// written the same way (SAlign<T*>).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::OutsideMemberOwner(CCPPTemplate* pcPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPTemplate*		pcTemplate;
	CCPPSyntax*			pcArgument;
	CCPPSyntax*			pcParameter;
	CCPPSyntaxTypeId*	pcTypeId;
	CCPPSymbol*			pcSymbol;
	CChars				szClass;
	CChars				szPattern;
	size				i;
	bool				bPrimary;

	if (!pcClass->mbTemplateArguments)
	{
		return pcPrimary;
	}
	bPrimary = pcClass->macpTemplateArguments.NumElements() == pcDeclaration->macpParameters.NumElements();
	for (i = 0; bPrimary && (i < pcClass->macpTemplateArguments.NumElements()); i++)
	{
		pcArgument = pcClass->macpTemplateArguments.GetPtr(i);
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		pcSymbol = NULL;
		if (pcArgument->GetKind() == CPPS_TypeId)
		{
			pcTypeId = (CCPPSyntaxTypeId*)pcArgument;
			if (pcTypeId->mpcSpecifiers && pcTypeId->mpcSpecifiers->mpcType && (pcTypeId->mpcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier) && !(pcTypeId->mpcSpecifiers->muiFlags & CPPDS_CONST) && (!pcTypeId->mpcDeclarator || ((pcTypeId->mpcDeclarator->NumPointers() == 0) && (pcTypeId->mpcDeclarator->macpSuffixes.NumElements() == 0))))
			{
				pcSymbol = ((CCPPSyntaxNamedTypeSpecifier*)pcTypeId->mpcSpecifiers->mpcType)->mpcName->mpcSymbol;
			}
		}
		else if (pcArgument->GetKind() == CPPS_IdExpression)
		{
			pcSymbol = ((CCPPSyntaxIdExpression*)pcArgument)->mpcName->mpcSymbol;
		}
		bPrimary = pcSymbol && (pcSymbol == ((pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol));
	}
	if (bPrimary)
	{
		return pcPrimary;
	}

	szClass.Init();
	pcClass->PrettyPrint(&szClass, 0);
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || !pcTemplate->mpcPattern)
		{
			continue;
		}
		szPattern.Init();
		pcTemplate->mpcPattern->PrettyPrint(&szPattern, 0);
		if (szPattern.Equals(&szClass))
		{
			szPattern.Kill();
			szClass.Kill();
			return pcTemplate;
		}
		szPattern.Kill();
	}
	szClass.Kill();
	return pcPrimary;
}


//////////////////////////////////////////////////////////////////////////
//
// A new template, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember)
{
	CCPPTemplate*	pcTemplate;

	pcTemplate = NewMalloc<CCPPTemplate>();
	pcTemplate->Init(pcSymbol, pcDeclaration, pcDefinition, bMember, mcContext.GetNamespace());
	mapcTemplates.Add(pcTemplate);
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// template class Name<Arguments>; generates every member function of the instance
// that has a definition, and template int Max<int>(int, int); (or Max(int, int))
// the function template's instance.  extern template ... (bExtern) generates
// neither: the unit uses the instance another unit's explicit instantiation
// generates (IsInstantiatedElsewhere).  An explicit instantiation after an extern
// one in the same unit generates the instance.  [temp.explicit]  The functions
// are generated in the order they are defined, as a class's that isn't a
// template are, also those waiting already: what the inliner makes of them
// depends on it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ExplicitInstantiation(CCPPSyntax* pcDeclaration, bool bExtern)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxTypeSpecifier*		pcType;
	CCPPSyntaxName*					pcName;
	CCPPClass*					pcClass;
	CCPPFunction*					pcFunction;
	CArrayCPPFunctions				apcDefined;
	size							i;
	size							j;

	pcType = NULL;
	pcSimple = NULL;
	if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		pcType = pcSimple->mpcSpecifiers ? pcSimple->mpcSpecifiers->mpcType : NULL;
	}
	if (pcSimple && (pcSimple->macpDeclarators.NumElements() == 1))
	{
		return ExplicitFunctionInstantiation((CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0), bExtern);
	}
	if (!pcType || (pcSimple->macpDeclarators.NumElements() != 0) || ((pcType->GetKind() != CPPS_ElaboratedTypeSpecifier) && (pcType->GetKind() != CPPS_NamedTypeSpecifier)))
	{
		return Error(pcDeclaration, "Only explicit instantiations of class templates and function templates are supported by the compiler yet");
	}
	pcName = (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier) ? ((CCPPSyntaxElaboratedTypeSpecifier*)pcType)->mpcName : ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
	pcClass = GetNameClass(pcName, pcDeclaration);
	if (!pcClass)
	{
		return false;
	}
	if (bExtern)
	{
		//Its members are declared when it is completed, wherever that is.
		pcClass->mbExternInstance = true;
		return true;
	}
	pcClass->mbExternInstance = false;
	if (!CompleteClass(pcClass, pcDeclaration))
	{
		return false;
	}
	//The defined functions by where their definitions are (an insertion sort: it keeps the order of equals), then those
	//the compiler writes.
	apcDefined.Init();
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mpcDefinition)
		{
			for (j = apcDefined.NumElements(); (j > 0) && (apcDefined.GetPtr(j - 1)->mpcDefinition->miFirstToken > pcFunction->mpcDefinition->miFirstToken); j--)
			{
			}
			*apcDefined.InsertAt(j) = pcFunction;
		}
	}
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (!pcFunction->mpcDefinition && pcFunction->mbImplicit)
		{
			apcDefined.Add(pcFunction);
		}
	}
	//One already waiting (called before its definition was seen) waits again in its place in that order.
	for (i = 0; i < apcDefined.NumElements(); i++)
	{
		pcFunction = apcDefined.GetPtr(i);
		if (pcFunction->mbQueued && !pcFunction->mbGenerated)
		{
			for (j = muiQueueNext; j < mapcQueue.NumElements(); j++)
			{
				if (mapcQueue.GetPtr(j) == pcFunction)
				{
					mapcQueue.RemoveAt(j);
					break;
				}
			}
			pcFunction->mbQueued = false;
		}
		RequireFunction(pcFunction);
	}
	apcDefined.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// template int Max<int>(int a, int b); and template int Max(int, int); name the
// instance of the function template Max whose arguments are the explicit ones
// and those deduced from the parameters' types.  Generated unless bExtern.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ExplicitFunctionInstantiation(CCPPSyntaxInitDeclarator* pcInit, bool bExtern)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPTemplate*				pcTemplate;
	CCPPTemplateArguments*		pcBindings;
	CCPPFunction*				pcFunction;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CCPPDeduction				cDeduction;
	CChars						szArguments;
	int64						lliName;
	size						i;

	pcName = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetName() : NULL;
	pcSuffix = pcInit->mpcDeclarator ? pcInit->mpcDeclarator->GetFunctionSuffix() : NULL;
	if (!pcName || !pcSuffix || pcName->IsQualifiedByClass())
	{
		return Error(pcInit, "Only explicit instantiations of class templates and function templates are supported by the compiler yet");
	}
	pcComponent = pcName->GetLast();
	pcTemplate = pcName->mpcSymbol ? GetTemplate(pcName->mpcSymbol) : NULL;
	if (!pcTemplate || !pcTemplate->IsFunction())
	{
		lliName = GetComponentName(pcComponent);
		pcTemplate = (lliName != -1) ? GetTemplateByName(lliName, CPPS_FunctionDefinition, ComponentLookup(pcComponent)) : NULL;
	}
	if (!pcTemplate)
	{
		return Error(pcInit, "The explicit instantiation names no function template");
	}

	//The parameters' types are the deduced arguments' (as FindFunctionSpecialisation deduces them).
	mcContext.EnterBindings(NULL);
	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		apsParameters.Add((psType && psType->IsReference()) ? psType->psBase : psType);
	}
	cDeduction.Init(this, pcTemplate);
	cDeduction.Deduce(&apsParameters, NULL);
	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, cDeduction.GetTypes(), &szArguments, pcInit, cDeduction.GetValues());
	cDeduction.Kill();
	apsParameters.Kill();
	mcContext.LeaveBindings();

	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcInit) : NULL;
	szArguments.Kill();
	if (!pcFunction)
	{
		return false;
	}
	pcFunction->mbExternInstance = bExtern;
	if (!bExtern)
	{
		RequireFunction(pcFunction);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The function is generated by the unit with its explicit instantiation, not by
// this one: an instance extern template names (of a function template, or a member
// of a class template defined outside the class) that isn't inline, constexpr or
// written by the compiler.  [temp.explicit] 10: those still are generated where
// used.  The index's and the IDE's checks check it as ever.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsInstantiatedElsewhere(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;

	pcDefinition = pcFunction->mpcDefinition;
	if (mpcSemanticPass || !pcDefinition || pcFunction->mbImplicit)
	{
		return false;
	}
	if (pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)))
	{
		return false;
	}
	if (pcFunction->mbExternInstance)
	{
		return true;
	}

	//A member template's instance isn't the class's to instantiate.  A member defined in the class is inline.
	if (!pcFunction->mpcClass || !pcFunction->mpcClass->mbExternInstance || pcFunction->mpcTemplate)
	{
		return false;
	}
	pcName = pcDefinition->mpcDeclarator ? pcDefinition->mpcDeclarator->GetName() : NULL;
	return pcName && pcName->IsQualifiedByClass();
}


//////////////////////////////////////////////////////////////////////////
//
// The member class template of the symbol that pcClass has, or if it is
// NULL the class it is named in (MemberScopeClass: for a class template's
// member, its instance in use), the class laid out first, as it keeps its
// member templates then.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::MemberClassTemplate(CCPPSymbol* pcSymbol, CCPPClass* pcClass)
{
	CCPPTemplate*	pcTemplate;
	size			i;

	pcClass = pcClass ? pcClass : MemberScopeClass(pcSymbol);
	if (!pcClass || !CompleteClass(pcClass, NULL))
	{
		return NULL;
	}
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && (pcTemplate->mpcClass == pcClass) && (pcTemplate->mpcSymbol == pcSymbol))
		{
			return pcTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The template pcSymbol declares.  A function template declared before it is
// defined has a symbol for each declaration (chained by mpcNext); the template is
// recorded under the definition's.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::GetTemplate(CCPPSymbol* pcSymbol)
{
	CCPPTemplate*	pcTemplate;
	CCPPSymbol*		pcDeclaration;
	size			i;

	//A member class template is its class's: the class in use where it is named.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_ClassTemplate) && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Class))
	{
		return MemberClassTemplate(pcSymbol, NULL);
	}
	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if ((pcDeclaration != pcSymbol) && (pcDeclaration->meKind != pcSymbol->meKind))
		{
			continue;
		}
		for (i = 0; i < mapcTemplates.NumElements(); i++)
		{
			pcTemplate = mapcTemplates.GetPtr(i);
			if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && (pcTemplate->mpcSymbol == pcDeclaration))
			{
				return pcTemplate;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A primary template by name, of a class or a function (eDefinition): one
// visible from pcLookup, else any.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplate*	pcFound;
	size			i;

	pcFound = NULL;
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (!pcTemplate->mbMember && (pcTemplate->mpcPrimary == pcTemplate) && pcTemplate->mpcSymbol && (pcTemplate->mpcSymbol->mlliName == lliName) && (pcTemplate->DefinitionKind() == eDefinition))
		{
			if (pcTemplate->mpcSymbol->mpcScope->Encloses(pcLookup))
			{
				return pcTemplate;
			}
			pcFound = pcFound ? pcFound : pcTemplate;
		}
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template a name's component names.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent)
{
	int64	lliName;

	lliName = GetComponentName(pcComponent);
	return (lliName != -1) ? GetTemplateByName(lliName, CPPS_ClassSpecifier, ComponentLookup(pcComponent)) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Binds a template's parameters to its arguments, worked out with the current
// bindings.  Parameters without an argument take what was deduced (a type in
// papsDeduced, or for a non-type parameter its value in palliDeduced where
// papsDeduced has an entry) or else their defaults.  The arguments are mangled into
// pszArguments.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPUnit::BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced)
{
	CArrayCPPExpandedArguments	asArguments;
	CCPPTemplateArguments*		pcBindings;
	SCPPBinding*				psBinding;
	SCPPExpandedArgument*		psArgument;
	CCPPSyntax*					pcParameter;
	CCPPSyntaxTypeParameter*	pcType;
	CCPPSyntaxParameter*		pcValue;
	size						i;
	bool						bResult;

	asArguments.Init();
	bResult = !papcArguments || ExpandTemplateArguments(papcArguments, &asArguments);
	pcBindings = NewBindings();
	for (i = 0; bResult && (i < pcTemplate->macpParameters.NumElements()); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		psArgument = (i < asArguments.NumElements()) ? asArguments.Get(i) : NULL;
		psBinding = pcBindings->macBindings.Add();
		psBinding->psType = NULL;
		psBinding->lliValue = 0;
		psBinding->pcParameter = NULL;
		psBinding->papsPack = NULL;
		psBinding->palliPack = NULL;
		psBinding->bSelf = false;
		psBinding->pcArgument = NULL;
		psBinding->pcTemplate = NULL;

		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			psBinding->pcParameter = pcType->mpcSymbol;
			if (pcType->mbPack)
			{
				//A pack (the last parameter) takes the rest of the explicit arguments, or else the deduced types.
				bResult = BindTypePack(psBinding, &asArguments, i, papsDeduced, pszArguments);
				break;
			}
			if (pcType->mpcTemplate)
			{
				bResult = BindTemplate(psBinding, psArgument ? psArgument->pcArgument : pcType->mpcDefault, pcNode, pszArguments);
				if (!bResult)
				{
					break;
				}
				continue;
			}
			if (psArgument)
			{
				//const int is a different argument to int (its type is const).
				psBinding->psType = ExpandedArgumentType(psArgument);
			}
			else if (papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->psType = papsDeduced->GetPtr(i);
			}
			else if (pcType->mpcDefault)
			{
				//A default can use the parameters before it.
				pcBindings->mpcOuter = mcContext.GetBindings();
				mcContext.EnterBindings(pcBindings);
				psBinding->psType = mcDeclarators.GetTypeIdType(pcType->mpcDefault);
				mcContext.LeaveBindings();
				pcBindings->mpcOuter = NULL;
			}
			if (!psBinding->psType)
			{
				bResult = Error(psArgument ? psArgument->pcArgument : pcNode, "The template argument is not a type the compiler supports");
				break;
			}
			pszArguments->Append('_');
			mcMangler.AppendType(pszArguments, psBinding->psType);
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			pcValue = (CCPPSyntaxParameter*)pcParameter;
			psBinding->pcParameter = pcValue->mpcSymbol;
			if (pcValue->mpcDeclarator && pcValue->mpcDeclarator->mbPack)
			{
				bResult = BindValuePack(psBinding, &asArguments, i, pszArguments);
				break;
			}
			if (psArgument && IsEntityParameter(pcValue))
			{
				bResult = BindEntity(psBinding, psArgument, pszArguments);
				if (!bResult)
				{
					break;
				}
				continue;
			}
			if (psArgument)
			{
				bResult = ExpandedArgumentValue(psArgument, &psBinding->lliValue) && ValueArgumentFits(pcValue, psArgument, psBinding->lliValue, pcBindings);
			}
			else if (palliDeduced && papsDeduced && (i < papsDeduced->NumElements()) && papsDeduced->GetPtr(i))
			{
				psBinding->lliValue = palliDeduced->GetValue(i);
			}
			else if (!pcValue->mpcDefault)
			{
				bResult = Error(pcNode, "The template argument must be a constant");
			}
			else if (!Constant(pcValue->mpcDefault, &psBinding->lliValue))
			{
				bResult = ConstantError(pcNode, "The template argument must be a constant");
			}
			if (!bResult)
			{
				break;
			}
			pszArguments->Append("_L");
			AppendMangledValue(pszArguments, psBinding->lliValue);
		}
		else
		{
			bResult = Error(pcParameter, "This template parameter is not supported by the compiler yet");
		}
	}
	asArguments.Kill();
	return bResult ? pcBindings : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a template argument written for a parameter of an integer type
// is a value of that type, as a converted constant expression must be:
// Byte<300> for an unsigned char isn't, nor Count<-1> for an unsigned int.
// An error (and false) if not.  The parameter's type is worked out with the
// arguments before it (pcBindings); one that can't be yet isn't checked.
// [temp.arg.nontype] 1, [expr.const] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ValueArgumentFits(CCPPSyntaxParameter* pcParameter, SCPPExpandedArgument* psArgument, int64 lliValue, CCPPTemplateArguments* pcBindings)
{
	SCPP65Type*	psType;
	size		uiErrors;

	if (psArgument->bExpanded)
	{
		return true;
	}
	uiErrors = mcErrors.NumErrors();
	mcContext.EnterBindings(pcBindings);
	BeginQuiet();
	psType = mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
	EndQuiet();
	mcContext.LeaveBindings();
	mcErrors.Truncate(uiErrors);
	if (!psType || ConstantFits(psType, lliValue))
	{
		return true;
	}
	//Said again where the instance is named for good (CCPP65TypeChecker::ConstantValue): this may only be a try.
	mbValueArgumentRefused = true;
	return ReportOnce(psArgument->pcArgument, "The constant's value isn't one of the type it is converted to");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a constant's value is one of an integer type's (any is a bool's,
// and an enum's isn't asked).  [expr.const] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ConstantFits(SCPP65Type* psType, int64 lliValue)
{
	return !psType->IsInteger() || psType->pcEnum || (psType->eKind == CPP65TK_Bool) || (ConstantOfType(psType, lliValue) == lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a non-type template parameter is a pointer or a reference
// (template<int* P>, template<int (*F)(int)>, template<int& R>): its
// argument names an object or a function, not a value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsEntityParameter(CCPPSyntaxParameter* pcParameter)
{
	return pcParameter->mpcDeclarator && ((pcParameter->mpcDeclarator->NumPointers() > 0) || pcParameter->mpcDeclarator->mpcNested);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer or reference parameter takes its argument as it is written:
// &g or f (a pointer: the address of a global or a function) or g (a
// reference), which its name then stands for.  Mangled as _X, A for an
// address, the global's label or the function's name, E.  [temp.arg.nontype]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindEntity(SCPPBinding* psBinding, SCPPExpandedArgument* psArgument, CChars* pszArguments)
{
	CCPPSyntax*		pcEntity;
	CCPPSymbol*		pcSymbol;
	CCPPVariable*	pcVariable;
	bool			bAddress;

	pcEntity = psArgument->bExpanded ? NULL : SkipParentheses(psArgument->pcArgument);
	bAddress = pcEntity && (pcEntity->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcEntity)->meOperator == CT_BitwiseAnd);
	if (bAddress)
	{
		pcEntity = SkipParentheses(((CCPPSyntaxUnaryExpression*)pcEntity)->mpcOperand);
	}
	pcSymbol = (pcEntity && (pcEntity->GetKind() == CPPS_IdExpression)) ? ((CCPPSyntaxIdExpression*)pcEntity)->mpcName->mpcSymbol : NULL;
	pcVariable = pcSymbol ? GetVariable(pcSymbol) : NULL;
	if (!(pcVariable && (pcVariable->meStorage == CPPVS_Global)) && !(pcSymbol && (pcSymbol->meKind == CPPSK_Function)))
	{
		return Error(psArgument->pcArgument, "The template argument must name a global or a function");
	}
	psBinding->pcArgument = psArgument->pcArgument;
	pszArguments->Append(bAddress ? "_XA" : "_X");
	if (pcVariable)
	{
		mcMangler.AppendGlobalLabel(pszArguments, pcVariable);
	}
	else
	{
		mcMangler.AppendFunctionName(pszArguments, pcSymbol->mlliName);
	}
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A template template parameter takes the class template its argument (or
// default) names: Use<Box, long>'s C is Box.  Mangled as _Y, the template's
// scopes and name, E.  [temp.arg.template]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindTemplate(SCPPBinding* psBinding, CCPPSyntax* pcArgument, CCPPSyntax* pcNode, CChars* pszArguments)
{
	CCPPSyntaxTypeId*			pcTypeId;
	CCPPSyntaxTypeSpecifier*	pcType;
	CCPPSyntaxName*				pcName;
	char*						szName;
	int							iLength;

	pcTypeId = (pcArgument && (pcArgument->GetKind() == CPPS_TypeId)) ? (CCPPSyntaxTypeId*)pcArgument : NULL;
	pcType = (pcTypeId && pcTypeId->mpcSpecifiers) ? pcTypeId->mpcSpecifiers->mpcType : NULL;
	pcName = (pcType && (pcType->GetKind() == CPPS_NamedTypeSpecifier) && (!pcTypeId->mpcDeclarator || pcTypeId->mpcDeclarator->IsEmpty())) ? ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName : NULL;
	if (pcName && pcName->mpcSymbol && (pcName->mpcSymbol->meKind == CPPSK_TemplateTemplateParameter))
	{
		//A template template parameter passed on: what it is bound to.
		psBinding->pcTemplate = FindBinding(pcName->mpcSymbol) ? FindBinding(pcName->mpcSymbol)->pcTemplate : NULL;
	}
	else if (pcName && pcName->mpcSymbol && (pcName->mpcSymbol->meKind == CPPSK_ClassTemplate) && !pcName->GetLast()->mbTemplateArguments)
	{
		psBinding->pcTemplate = GetTemplate(pcName->mpcSymbol);
	}
	if (!psBinding->pcTemplate)
	{
		return Error(pcArgument ? pcArgument : pcNode, "The template argument must be a class template");
	}
	pszArguments->Append("_Y");
	mcMangler.AppendScopeNames(pszArguments, psBinding->pcTemplate->mpcSymbol->mpcScope);
	szName = mpcTokeniser->GetIdentifier(psBinding->pcTemplate->mpcSymbol->mlliName, &iLength);
	pszArguments->Append(szName, iLength);
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A type parameter pack takes the expanded arguments from uiFirst on, all
// types, or if there are none the deduced types from there.  Mangled as
// _J, its types, E.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindTypePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CArrayCPP65Types* papsDeduced, CChars* pszArguments)
{
	SCPP65Type*	psType;
	size		j;

	psBinding->papsPack = NewPack();
	for (j = uiFirst; j < pasArguments->NumElements(); j++)
	{
		psType = ExpandedArgumentType(pasArguments->Get(j));
		if (!psType)
		{
			return Error(pasArguments->Get(j)->pcArgument, "The template argument is not a type the compiler supports");
		}
		psBinding->papsPack->Add(psType);
	}
	for (j = uiFirst; (uiFirst >= pasArguments->NumElements()) && papsDeduced && (j < papsDeduced->NumElements()); j++)
	{
		if (papsDeduced->GetPtr(j))
		{
			psBinding->papsPack->Add(papsDeduced->GetPtr(j));
		}
	}
	pszArguments->Append("_J");
	for (j = 0; j < psBinding->papsPack->NumElements(); j++)
	{
		mcMangler.AppendType(pszArguments, psBinding->papsPack->GetPtr(j));
	}
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A non-type parameter pack takes the expanded arguments from uiFirst on,
// all constants.  Mangled as _J, L value E for each, E.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindValuePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CChars* pszArguments)
{
	int64	lliValue;
	size	j;

	psBinding->palliPack = NewValuePack();
	pszArguments->Append("_J");
	for (j = uiFirst; j < pasArguments->NumElements(); j++)
	{
		if (!ExpandedArgumentValue(pasArguments->Get(j), &lliValue))
		{
			return false;
		}
		psBinding->palliPack->Add(lliValue);
		pszArguments->Append('L');
		AppendMangledValue(pszArguments, lliValue);
		pszArguments->Append('E');
	}
	pszArguments->Append('E');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A template argument's value as it is mangled: n before a negative one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendMangledValue(CChars* pszArguments, int64 lliValue)
{
	pszArguments->Append((lliValue < 0) ? "n" : "");
	pszArguments->Append((lliValue < 0) ? -lliValue : lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a template parameter in what is being generated.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPUnit::FindBinding(CCPPSymbol* pcParameter)
{
	return mcContext.GetBindings() ? mcContext.GetBindings()->Find(pcParameter) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template for the arguments, made if it is new.  It is
// laid out when it is first needed whole, from the most specialised definition
// that matches the arguments.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::InstantiateClass(CCPPTemplate* pcTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CChars					szArguments;

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	pcClass = pcBindings ? InstanceOfClass(pcTemplate, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template for arguments that have been bound
// (pszArguments is them mangled), made if it is new.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::InstanceOfClass(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcSpecialised;
	CCPPTemplate*	pcSpecialisation;
	CCPPClass*	pcClass;
	size			i;

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary == pcTemplate) && pcClass->mszArguments.Equals(pszArguments))
		{
			return pcClass;
		}
	}

	pcSpecialised = NULL;
	pcSpecialisation = pcBindings->mbSelf ? NULL : FindClassSpecialisation(pcTemplate, pcBindings, &pcSpecialised, pcNode);
	if (pcSpecialisation)
	{
		pcTemplate = pcSpecialisation;
		pcBindings = pcSpecialised;
	}
	if (pcTemplate->mpcPrimary->mpcClass && !pcBindings->mpcOuter)
	{
		//A member class template's instance sees the template arguments of the class it is in.
		pcBindings->mpcOuter = pcTemplate->mpcPrimary->mpcClass->mpcBindings;
	}

	//An instance used before the template was defined (UndefinedInstance) becomes this instance.
	pcClass = NULL;
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		if (mapcClasses.GetPtr(i)->IsInstanceNamed(pcTemplate->mpcSymbol) && mapcClasses.GetPtr(i)->mszArguments.Equals(pszArguments))
		{
			pcClass = mapcClasses.GetPtr(i);
			pcClass->mpcSymbol = pcTemplate->mpcSymbol;
			pcClass->mszArguments.Kill();
			pcClass->mszArguments.Init();
			break;
		}
	}
	if (!pcClass)
	{
		pcClass = NewClass(pcTemplate->mpcSymbol, pcTemplate->mpcSymbol->mlliName);
	}
	pcClass->mpcTemplate = pcTemplate;
	pcClass->mpcSpecifier = pcTemplate->IsDeclaredClass() ? NULL : (CCPPSyntaxClassSpecifier*)pcTemplate->mpcDefinition;
	pcClass->mpcBindings = pcBindings;
	pcBindings->mpcClass = pcClass;
	pcClass->mszArguments.Append(pszArguments);
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class that stands for a template's type parameter where the
// template is checked as itself: one for each parameter, complete, with
// nothing in it, and written as the parameter's name (CCPPTypeWriter).
// A value of it can be declared, copied, pointed to and passed; what is
// asked of it that only an argument would have (a member, an operator, a
// conversion) isn't there, so an expression that needs one has no type.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::PlaceholderClass(CCPPSymbol* pcParameter)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (pcClass->mbPlaceholder && (pcClass->mpcSymbol == pcParameter))
		{
			return pcClass;
		}
	}
	pcClass = NewClass(pcParameter, pcParameter->mlliName);
	pcClass->mbPlaceholder = true;
	pcClass->mbEmpty = true;
	pcClass->meState = CPPCS_Complete;
	mcTypes.SetClassBytes(pcClass->mpsType, 1);
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A template's parameters bound to themselves: each type parameter to
// its placeholder class, each value parameter to a value that isn't
// known (it is 1 where one is needed), a pack to nothing.  NULL if the
// template has a parameter that can't be (a template for a parameter).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPUnit::BindAsItself(CCPPSyntaxTemplateDeclaration* pcDeclaration, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*		pcBindings;
	CArrayCPP65Types			apsTypes;
	CArrayInt64					alliValues;
	CCPPSyntax*					pcParameter;
	CCPPSyntaxTypeParameter*	pcType;
	size						i;
	bool						bBound;

	apsTypes.Init();
	alliValues.Init();
	bBound = true;
	for (i = 0; bBound && (i < pcDeclaration->macpParameters.NumElements()); i++)
	{
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			if (pcType->mbPack)
			{
				break;
			}
			bBound = pcType->mpcSymbol && !pcType->mpcTemplate;
			if (bBound)
			{
				apsTypes.Add(PlaceholderClass(pcType->mpcSymbol)->mpsType);
				alliValues.Add(0);
			}
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			if (((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator && ((CCPPSyntaxParameter*)pcParameter)->mpcDeclarator->mbPack)
			{
				break;
			}
			apsTypes.Add(&mcTypes.msInt);
			alliValues.Add(1);
		}
		else
		{
			bBound = false;
		}
	}

	pcBindings = NULL;
	if (bBound)
	{
		BeginQuiet();
		pcBindings = BindArguments(pcDeclaration, NULL, &apsTypes, pszArguments, pcNode, &alliValues);
		EndQuiet();
	}
	if (pcBindings)
	{
		pcBindings->mbSelf = true;
		for (i = 0; i < pcBindings->macBindings.NumElements(); i++)
		{
			pcBindings->macBindings.Get(i)->bSelf = true;
		}
	}
	alliValues.Kill();
	apsTypes.Kill();
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// A class template as itself: its instance with its parameters standing
// for themselves, which CArray<M> written in the template is too.  NULL
// if it can't be made.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::InstantiateClassAsItself(CCPPTemplate* pcTemplate)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CChars					szArguments;

	szArguments.Init();
	pcBindings = BindAsItself(pcTemplate->mpcDeclaration, &szArguments, pcTemplate->mpcDefinition);
	pcClass = pcBindings ? InstanceOfClass(pcTemplate, pcBindings, &szArguments, pcTemplate->mpcDefinition) : NULL;
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template as itself, a member template of a class too: its
// instance with its parameters standing for themselves.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::InstantiateFunctionAsItself(CCPPTemplate* pcTemplate)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CChars					szArguments;

	if (pcTemplate->mpcClass)
	{
		BeginQuiet();
		DefineMemberTemplate(pcTemplate, pcTemplate->mpcDeclaration);
		EndQuiet();
	}
	if (!pcTemplate->mpcDefinition || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
	{
		return NULL;
	}
	szArguments.Init();
	pcBindings = BindAsItself(pcTemplate->mpcDeclaration, &szArguments, pcTemplate->mpcDefinition);
	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcTemplate->mpcDefinition) : NULL;
	szArguments.Kill();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of a class template that has only been declared: an incomplete class (it can be pointed to) with the
// instance's name.  It becomes the instance if the template is defined later (InstantiateClass).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPSyntax*		pcDeclaration;
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*	pcClass;
	CChars			szArguments;
	size			i;

	for (pcDeclaration = pcSymbol->mpcDeclaration; pcDeclaration && (pcDeclaration->GetKind() != CPPS_TemplateDeclaration); pcDeclaration = pcDeclaration->mpcParent)
	{
	}
	if (!pcDeclaration)
	{
		return NULL;
	}

	szArguments.Init();
	pcBindings = BindArguments((CCPPSyntaxTemplateDeclaration*)pcDeclaration, papcArguments, NULL, &szArguments, pcNode);
	if (!pcBindings)
	{
		szArguments.Kill();
		return NULL;
	}
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (pcClass->IsInstanceNamed(pcSymbol) && pcClass->mszArguments.Equals(&szArguments))
		{
			szArguments.Kill();
			return pcClass;
		}
	}
	pcClass = NewClass(pcSymbol, pcSymbol->mlliName);
	pcClass->mszArguments.Append(&szArguments);
	szArguments.Kill();
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The most specialised specialisation of a class template that matches the
// arguments (psArguments), with its own parameters bound in *ppsBindings.  An
// explicit specialisation is chosen first; of the partial ones, the one whose
// arguments match more of the types' structure.  NULL if none matches.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::FindClassSpecialisation(CCPPTemplate* pcPrimary, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments** ppcBindings, CCPPSyntax* pcNode)
{
	CCPPSpecialisationMatch	cMatch;
	CCPPTemplate*			pcTemplate;
	CCPPTemplate*			pcBest;
	CCPPTemplateArguments*	pcDeduced;
	CCPPTemplateArguments*	pcBestDeduced;
	size					i;
	int						iScore;
	int						iBest;
	bool					bMatches;
	bool					bAmbiguous;

	pcBest = NULL;
	pcBestDeduced = NULL;
	iBest = -1;
	bAmbiguous = false;
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || !pcTemplate->mpcPattern)
		{
			continue;
		}

		cMatch.Init(this, pcTemplate);
		bMatches = cMatch.Match(pcArguments);
		pcDeduced = cMatch.GetDeduced();
		iScore = cMatch.GetScore();
		cMatch.Kill();
		if (!bMatches)
		{
			continue;
		}
		if (iScore > iBest)
		{
			pcBest = pcTemplate;
			pcBestDeduced = pcDeduced;
			iBest = iScore;
			bAmbiguous = false;
		}
		else if (iScore == iBest)
		{
			bAmbiguous = true;
		}
	}
	if (bAmbiguous)
	{
		//Kept (ClassError): an instance is made once, and often where errors are dropped.
		ClassError(pcNode ? pcNode->miFirstToken : -1, "More than one specialisation of the class template matches equally");
	}
	*ppcBindings = pcBestDeduced;
	return pcBest;
}


//////////////////////////////////////////////////////////////////////////
//
// New empty bindings, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPUnit::NewBindings(void)
{
	CCPPTemplateArguments*	pcBindings;

	pcBindings = NewMalloc<CCPPTemplateArguments>();
	pcBindings->Init();
	mapcAllBindings.Add(pcBindings);
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template for explicit arguments and the types of the
// call's arguments.  Template parameters without an explicit argument are deduced
// (DeduceArguments).  An explicit specialisation for the arguments is used instead
// of the template's definition.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::InstantiateFunction(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CArrayCPP65Types		apsArguments;
	CCPPDeduction			cDeduction;
	CChars					szArguments;
	size					i;

	if (pcTemplate->mpcClass && !DefineMemberTemplate(pcTemplate, pcNode))
	{
		return NULL;
	}

	apsArguments.Init();
	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		apsArguments.Add(TypeOf(papcArguments->GetPtr(i)));
	}
	cDeduction.Init(this, pcTemplate);
	cDeduction.Deduce(&apsArguments, papcArguments);
	apsArguments.Kill();

	szArguments.Init();
	pcBindings = BindArguments(pcTemplate->mpcDeclaration, papcExplicit, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues());
	cDeduction.Kill();
	pcFunction = pcBindings ? InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template for arguments that have been bound
// (pszArguments is them mangled), declared if it is new.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::InstanceOfFunction(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPTemplate*					pcSpecialisation;
	CCPPFunction*					pcFunction;
	CCPPSyntaxFunctionSuffix*		pcInClass;
	CCPPSyntaxParameter*			pcParameter;
	size							i;

	pcDefinition = (CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition;
	for (i = 0; i < pcTemplate->mapcInstances.NumElements(); i++)
	{
		pcFunction = pcTemplate->mapcInstances.GetPtr(i);
		if (pcFunction->mszArguments.Equals(pszArguments))
		{
			return pcFunction;
		}
	}

	pcSpecialisation = pcBindings->mbSelf ? NULL : FindFunctionSpecialisation(pcTemplate, pszArguments, pcNode);
	if (pcSpecialisation)
	{
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcSpecialisation->mpcDefinition;
	}

	//A member template's instance also sees its class's template arguments.
	if (pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = pcTemplate->mpcClassDeclaration ? RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
	}
	else
	{
		pcBindings->mpcOuter = mcContext.GetBindings();
	}
	//A member template only declared has instances only declared.
	pcDeclarator = pcDefinition ? pcDefinition->mpcDeclarator : pcTemplate->FunctionDeclarator();
	pcSpecifiers = pcDefinition ? pcDefinition->mpcSpecifiers : pcTemplate->FunctionSpecifiers();
	mcContext.EnterBindings(pcBindings);
	mcContext.EnterNamespace(pcTemplate->mpcNamespace);
	pcFunction = DeclareFunction(pcSpecifiers, pcDeclarator, pcDefinition != NULL, pcTemplate->mpcClass, false, pcTemplate, pszArguments);
	mcContext.LeaveNamespace();
	mcContext.LeaveBindings();
	if (!pcTemplate->mpcClass)
	{
		pcBindings->mpcOuter = NULL;
	}
	if (pcFunction)
	{
		//template<class T> int Exact(T) = delete;: each instance is deleted.  [dcl.fct.def.delete]
		pcFunction->mbDeleted = pcDefinition && (pcDefinition->meBody == CPPFB_Delete);
		pcFunction->mpcDefinition = pcFunction->mbDeleted ? NULL : pcDefinition;
		pcFunction->mpcBindings = pcBindings;
		pcFunction->mbInline = true;
		pcFunction->mbStatic |= pcTemplate->mbStatic;

		//A member template defined outside its class has its default arguments in the class.  [dcl.fct.default]
		pcInClass = (pcTemplate->mpcDeclarator && (pcTemplate->mpcDeclarator != pcDeclarator)) ? pcTemplate->mpcDeclarator->GetFunctionSuffix() : NULL;
		if (pcInClass && (pcInClass->macpParameters.NumElements() == pcFunction->mapcDefaults.NumElements()))
		{
			for (i = 0; i < pcInClass->macpParameters.NumElements(); i++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcInClass->macpParameters.GetPtr(i);
				if (!pcFunction->mapcDefaults.GetPtr(i) && pcParameter->mpcDefault)
				{
					pcFunction->mapcDefaults.SetPtr(i, pcParameter->mpcDefault);
				}
			}
		}
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The explicit specialisation of a function template for the mangled arguments,
// or NULL.  A specialisation's arguments are its explicit ones (Size<char>) and
// those deduced from its parameters' types (Size(short)), worked out once.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psType;
	CArrayCPP65Types			apsParameters;
	CCPPDeduction				cDeduction;
	size						i;
	size						j;

	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if ((pcTemplate->mpcPrimary != pcPrimary) || (pcTemplate == pcPrimary) || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		if (!pcTemplate->mbArguments)
		{
			pcTemplate->mbArguments = true;
			mcContext.EnterBindings(NULL);
			BeginQuiet();
			apsParameters.Init();
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
			for (j = 0; pcSuffix && (j < pcSuffix->macpParameters.NumElements()); j++)
			{
				pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(j);
				psType = mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
				apsParameters.Add((psType && psType->IsReference()) ? psType->psBase : psType);
			}
			cDeduction.Init(this, pcPrimary);
			cDeduction.Deduce(&apsParameters, NULL);
			if (!BindArguments(pcPrimary->mpcDeclaration, (pcTemplate->mpcPattern && pcTemplate->mpcPattern->mbTemplateArguments) ? &pcTemplate->mpcPattern->macpTemplateArguments : NULL, cDeduction.GetTypes(), &pcTemplate->mszArguments, pcNode, cDeduction.GetValues()))
			{
				pcTemplate->mszArguments.Clear();
			}
			cDeduction.Kill();
			apsParameters.Kill();
			EndQuiet();
			mcContext.LeaveBindings();
		}
		if (!pcTemplate->mszArguments.Empty() && pcTemplate->mszArguments.Equals(pszArguments))
		{
			return pcTemplate;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A class template instance's bindings under the parameter names of a declaration
// outside the class (template<class N> ... CArrayTemplate<N>::...).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplateArguments* CCPPUnit::RenameBindings(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPSyntax*		pcParameter;
	size			i;

	pcBindings = NewBindings();
	pcBindings->mpcOuter = pcClass->mpcBindings;
	pcBindings->mpcClass = pcClass;
	pcBindings->mbSelf = pcClass->mpcBindings->mbSelf;
	for (i = 0; (i < pcDeclaration->macpParameters.NumElements()) && (i < pcClass->mpcBindings->macBindings.NumElements()); i++)
	{
		pcParameter = pcDeclaration->macpParameters.GetPtr(i);
		*pcBindings->macBindings.Add() = *pcClass->mpcBindings->macBindings.Get(i);
		pcBindings->macBindings.Tail()->pcParameter = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol;
	}
	return pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a template declaration's declaration defines a class template:
// struct I { ... }; with a name and no template arguments (not a
// specialisation).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsClassTemplateDefinition(CCPPSyntax* pcDeclaration)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxClassSpecifier*		pcSpecifier;

	if (pcDeclaration->GetKind() != CPPS_SimpleDeclaration)
	{
		return false;
	}
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
	if (!pcSimple->mpcSpecifiers || !pcSimple->mpcSpecifiers->mpcType || (pcSimple->mpcSpecifiers->mpcType->GetKind() != CPPS_ClassSpecifier))
	{
		return false;
	}
	pcSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
	return pcSpecifier->mpcSymbol && pcSpecifier->mpcName && !pcSpecifier->mpcName->GetLast()->mbTemplateArguments;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a member template of a class being laid out.  A member function
// template's instances are made when it is called (InstantiateFunction), a
// member class template's when it is named (GetTemplate finds the class's).
// Friend templates and constructor templates are left out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::MemberTemplate(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration)
{
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxName*					pcName;
	CCPPTemplate*					pcTemplate;
	int64							lliName;

	pcDeclaration = pcTemplateDeclaration->mpcDeclaration;
	if (pcTemplateDeclaration->mbInstantiation || (pcTemplateDeclaration->macpParameters.NumElements() == 0) || !pcDeclaration)
	{
		return true;
	}

	if (IsClassTemplateDefinition(pcDeclaration))
	{
		//template<class T> struct I { ... }; in a class: its own for each class it is in.
		pcTemplate = NewTemplate(((CCPPSyntaxClassSpecifier*)((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers->mpcType)->mpcSymbol, pcTemplateDeclaration, ((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers->mpcType, false);
		pcTemplate->mpcClass = pcClass;
		return true;
	}

	pcSpecifiers = NULL;
	pcDeclarator = NULL;
	if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcSpecifiers;
		pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcDeclarator;
	}
	else if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		pcInit = (pcSimple->macpDeclarators.NumElements() == 1) ? (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0) : NULL;
		pcSpecifiers = pcSimple->mpcSpecifiers;
		pcDeclarator = (pcInit && pcInit->mpcDeclarator && pcInit->mpcDeclarator->IsFunction()) ? pcInit->mpcDeclarator : NULL;
	}
	pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	if (!pcName || (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND)))
	{
		return true;
	}
	lliName = GetFunctionName(pcName->GetLast());
	if ((lliName == -1) || (lliName == pcClass->mlliName) || (pcName->GetLast()->meKind == CPPNC_Destructor))
	{
		pcClass->mbConstructorTemplate |= (lliName == pcClass->mlliName);
		return true;
	}

	pcTemplate = NewTemplate(pcName->mpcSymbol, pcTemplateDeclaration, (pcDeclaration->GetKind() == CPPS_FunctionDefinition) ? pcDeclaration : NULL, true);
	pcTemplate->mbMemberTemplate = true;
	pcTemplate->mpcClass = pcClass;
	pcTemplate->mlliName = lliName;
	pcTemplate->mpcDeclarator = pcDeclarator;
	pcTemplate->mpcSpecifiers = pcSpecifiers;
	pcTemplate->mbStatic = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The & or && after a member function's parameters.
//
//////////////////////////////////////////////////////////////////////////
ECPPRefQualifier CCPPUnit::RefQualifier(CCPPSyntaxFunctionSuffix* pcSuffix)
{
	if (CPPHasKeyword(&pcSuffix->macpQualifiers, CT_LogicalAnd))
	{
		return CPPRQ_Rvalue;
	}
	return CPPHasKeyword(&pcSuffix->macpQualifiers, CT_BitwiseAnd) ? CPPRQ_Lvalue : CPPRQ_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A member template declared in its class but defined outside it takes the outside
// definition with its name and parameters (and its template parameters, which the
// definition uses): the one whose parameters are written the same, as overloads
// that differ only in their parameters' types are, or else the first with as many.
// One with no definition stays only declared (its declarator and specifiers in the
// class: CCPPTemplate::FunctionDeclarator), as SFINAE's test functions are.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::DefineMemberTemplate(CCPPTemplate* pcTemplate, CCPPSyntax* pcNode)
{
	CCPPTemplate*				pcOutside;
	CCPPTemplate*				pcFound;
	CCPPSymbol*					pcClassSymbol;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxFunctionSuffix*	pcOutsideSuffix;
	CCPPSymbolKey				cKey;
	CChars						szSignature;
	CChars						szOutside;
	size						i;
	bool						bSame;

	if (pcTemplate->mpcDefinition)
	{
		return true;
	}
	pcClassSymbol = pcTemplate->mpcClass->mpcTemplate ? pcTemplate->mpcClass->mpcTemplate->mpcSymbol : pcTemplate->mpcClass->mpcSymbol;
	pcSuffix = pcTemplate->mpcDeclarator->GetFunctionSuffix();
	cKey.Init(mpcTokeniser, mpcTree);
	szSignature.Init();
	if (pcSuffix)
	{
		cKey.GetSignature(pcSuffix, &szSignature);
	}
	pcFound = NULL;
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcOutside = mapcTemplates.GetPtr(i);
		if (!pcOutside->mbMemberTemplate || pcOutside->mpcClass || (pcOutside->mpcSymbol != pcClassSymbol) || (pcOutside->mlliName != pcTemplate->mlliName))
		{
			continue;
		}
		pcOutsideSuffix = ((CCPPSyntaxFunctionDefinition*)pcOutside->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
		if ((pcOutsideSuffix ? pcOutsideSuffix->NumParameters() : 0) != (pcSuffix ? pcSuffix->NumParameters() : 0))
		{
			continue;
		}
		pcFound = pcFound ? pcFound : pcOutside;
		szOutside.Init();
		if (pcSuffix && pcOutsideSuffix)
		{
			cKey.GetSignature(pcOutsideSuffix, &szOutside);
		}
		bSame = pcSuffix && pcOutsideSuffix && szOutside.Equals(&szSignature);
		szOutside.Kill();
		if (bSame)
		{
			pcFound = pcOutside;
			break;
		}
	}
	szSignature.Kill();
	cKey.Kill();

	if (!pcFound)
	{
		//Only declared: its instances are declarations, which only a call needs defined (CCPP65TypeChecker::Call).
		return true;
	}
	pcTemplate->mpcDeclaration = pcFound->mpcDeclaration;
	pcTemplate->mpcDefinition = pcFound->mpcDefinition;
	pcTemplate->mpcClassDeclaration = pcFound->mpcClassDeclaration;
	return true;
}
