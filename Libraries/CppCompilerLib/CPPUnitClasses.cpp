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
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::NewClass(CCPPSymbol* pcSymbol, int64 lliName)
{
	CCPPClass*	pcClass;

	pcClass = NewMalloc<CCPPClass>();
	pcClass->Init(pcSymbol, lliName);
	pcClass->mpsType = mcTypes.Class(pcClass);
	mapcClasses.Add(pcClass);
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class (not a template instance) a symbol names.  A class used before it is
// defined, or only through pointers, is laid out when it is first needed whole.
// A member class of a class template is each instance's own, with the instance's
// template arguments: the instance's (pcEnclosing) that is given or that the
// bindings in use are of (EnclosingInstance).  [temp.mem.class]
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcEnclosing)
{
	CCPPClass*	pcClass;
	size		i;

	pcEnclosing = pcEnclosing ? pcEnclosing : EnclosingInstance(pcSymbol);
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if ((pcClass->mpcSymbol == pcSymbol) && !pcClass->mpcTemplate && (pcClass->mpcEnclosing == pcEnclosing))
		{
			return pcClass;
		}
	}
	pcClass = NewClass(pcSymbol, pcSymbol->mlliName);
	if (pcEnclosing)
	{
		pcClass->mpcEnclosing = pcEnclosing;
		pcClass->mpcBindings = pcEnclosing->mpcBindings;
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template's instance, or the member class of one, that a symbol
// is a member of with the bindings in use, or NULL if it is a member of
// neither.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::EnclosingInstance(CCPPSymbol* pcSymbol)
{
	CCPPClass*	pcClass;

	pcClass = MemberScopeClass(pcSymbol);
	return (pcClass && (pcClass->mpcTemplate || pcClass->mpcEnclosing)) ? pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a member's symbol is declared in, where the member is named in
// its class but not in a member function (sizeof(t(0)) in an enumerator):
// for a class template, its instance in the bindings in use.  NULL if the
// symbol isn't a member, or its class template's instance isn't in use.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::MemberScopeClass(CCPPSymbol* pcSymbol)
{
	CCPPScope*				pcScope;
	CCPPTemplateArguments*	pcBindings;

	pcScope = pcSymbol ? pcSymbol->mpcScope : NULL;
	if (!pcScope || (pcScope->meKind != CPPSC_Class) || !pcScope->mpcOwner)
	{
		return NULL;
	}
	for (pcBindings = mcContext.GetBindings(); pcBindings; pcBindings = pcBindings->mpcOuter)
	{
		if (pcBindings->mpcClass && (pcBindings->mpcClass->mpcSymbol == pcScope->mpcOwner))
		{
			return pcBindings->mpcClass;
		}
	}
	return mcInstantiator.GetTemplate(pcScope->mpcOwner) ? NULL : GetClass(pcScope->mpcOwner, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A class without a name (typedef struct { ... } SName;) is known by its definition.
// One in a class template is a different class in each instance (its bindings).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*	pcClass;
	CCPPTemplateArguments*	pcBindings;
	CCPPSyntax*		pcParent;
	size			i;

	for (pcBindings = mcContext.GetBindings(); pcBindings; pcBindings = pcBindings->mpcOuter)
	{
		pcParent = pcSpecifier->mpcParent;
		while (pcParent && (!pcBindings->mpcClass || (pcParent != pcBindings->mpcClass->mpcSpecifier)))
		{
			pcParent = pcParent->mpcParent;
		}
		if (pcParent)
		{
			break;
		}
	}

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (!pcClass->mpcSymbol && (pcClass->mpcSpecifier == pcSpecifier) && !pcClass->mpcTemplate && (pcClass->mpcBindings == pcBindings))
		{
			return pcClass;
		}
	}
	pcClass = NewClass(NULL, -1);
	pcClass->mpcSpecifier = pcSpecifier;
	pcClass->mpcBindings = pcBindings;
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A class (not a template instance) by name, for a qualifier the parser
// couldn't resolve: one visible from pcLookup, else any.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::FindClass(int64 lliName, CCPPScope* pcLookup)
{
	CCPPClass*	pcClass;
	CCPPClass*	pcFound;
	size			i;

	pcFound = NULL;
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if ((pcClass->mlliName == lliName) && !pcClass->mpcTemplate)
		{
			if (pcClass->mpcSymbol && pcClass->mpcSymbol->mpcScope->Encloses(pcLookup))
			{
				return pcClass;
			}
			pcFound = pcFound ? pcFound : pcClass;
		}
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a name's component is declared if the parser resolved it, else where
// the name was looked up: what to choose between things of its name with.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPUnit::ComponentLookup(CCPPSyntaxNameComponent* pcComponent)
{
	if (pcComponent->mpcSymbol)
	{
		return pcComponent->mpcSymbol->mpcScope;
	}
	return (pcComponent->mpcParent && (pcComponent->mpcParent->GetKind() == CPPS_Name)) ? ((CCPPSyntaxName*)pcComponent->mpcParent)->mpcLookup : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What an object of a type is aligned to, which is what alignof says: the
// type's alignment, unless the target aligns objects to less
// (CCPPTarget::GetObjectAlignmentLimit).  [basic.align] 2, [expr.alignof]
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::ObjectAlignment(SCPP65Type* psType)
{
	return CCPPClassLayout::LimitedAlignment(psType, mpcTarget->GetObjectAlignmentLimit());
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's alignment: its type's, or more where alignas asks it on the
// declaration or of the type's class.  *piAlignas is what alignas asks (0
// for nothing), which a place in a frame may not give.  [basic.align]
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::VariableAlignment(CCPPVariable* pcVariable, int* piAlignas)
{
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	int								iAlign;
	int								iAlignas;
	int								iDeclared;

	iAlign = ObjectAlignment(pcVariable->mpsType);
	iAlignas = CCPPClassLayout::TypeAlignas(pcVariable->mpsType);
	pcInit = mcInstantiator.VariableDeclarator(pcVariable);
	if (pcInit)
	{
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
		if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && pcDeclaration->mpcSpecifiers)
		{
			iDeclared = AlignasAlignment(&pcDeclaration->mpcSpecifiers->macpSpecifiers);
			iAlignas = (iDeclared > iAlignas) ? iDeclared : iAlignas;
		}
		iDeclared = AlignasAlignment(&pcInit->mpcDeclarator->macpAttributes);
		iAlignas = (iDeclared > iAlignas) ? iDeclared : iAlignas;
	}
	*piAlignas = iAlignas;
	return (iAlignas > iAlign) ? iAlignas : iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// The strictest alignment the alignas(n) and alignas(type) specifiers in
// papcSpecifiers ask for: 0 for none (alignas(0) asks for nothing), -1 after
// an error.  It is kept whatever #pragma pack says, as MSVC does.  A class's
// (bClass) errors are deferred, as it is laid out once.  [dcl.align]
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::AlignasAlignment(CArraySyntax* papcSpecifiers, bool bClass)
{
	CCPPSyntax*		pcSpecifier;
	CCPPSyntax*		pcAlignment;
	SCPP65Type*		psType;
	int64			lliAlign;
	int				iAlign;
	size			i;

	iAlign = 0;
	for (i = 0; i < papcSpecifiers->NumElements(); i++)
	{
		pcSpecifier = papcSpecifiers->GetPtr(i);
		if (pcSpecifier->GetKind() != CPPS_AlignasSpecifier)
		{
			continue;
		}
		pcAlignment = ((CCPPSyntaxAlignasSpecifier*)pcSpecifier)->mpcAlignment;
		if (pcAlignment->GetKind() == CPPS_TypeId)
		{
			psType = mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcAlignment);
			if (!psType || (psType->IsClass() && !CompleteClass(psType->pcClass, pcAlignment)))
			{
				return -1;
			}
			lliAlign = ObjectAlignment(psType);
		}
		else if (!mcFolder.Constant(pcAlignment, &lliAlign))
		{
			mcFolder.ConstantError(pcAlignment, "The alignment must be a constant");
			return -1;
		}
		if ((lliAlign < 0) || (lliAlign > 0x10000) || ((lliAlign & (lliAlign - 1)) != 0))
		{
			if (bClass)
			{
				mcDiagnostics.Defer(pcAlignment->miFirstToken, "The alignment must be a power of two");
			}
			else
			{
				//(A variable's alignment is asked for quietly too, and more than once.)
				mcDiagnostics.ReportOnce(pcAlignment, "The alignment must be a power of two");
			}
			return -1;
		}
		if (lliAlign > iAlign)
		{
			iAlign = (int)lliAlign;
		}
	}
	return iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a name component names: a class, a typedef of one, an instance of a
// class template (Name<Arguments>), a class template's own name inside it, or the
// type of decltype(e)::'s e.  A base template's name without arguments is psContext's
// (or else the current member function's class's) base instance of it.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext)
{
	CCPPTemplate*	pcTemplate;
	CCPPTemplateArguments*	pcBindings;
	SCPPBinding*	psBinding;
	CCPPClass*	pcClass;
	SCPP65Type*		psType;
	int64			lliName;

	if (!pcSymbol && pcComponent)
	{
		pcSymbol = pcComponent->mpcSymbol;
	}
	if (pcComponent && (pcComponent->meKind == CPPNC_Decltype))
	{
		//[dcl.type.simple] 4: a class as it is, not a reference to one.
		psType = mcDeclarators.GetTypeIdType(pcComponent->mpcType);
		return (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	if (!pcSymbol && pcComponent)
	{
		//Names in templates are not always resolved by the parser.
		lliName = GetComponentName(pcComponent);
		pcTemplate = mcInstantiator.GetComponentTemplate(pcComponent);
		if (pcTemplate)
		{
			pcSymbol = pcTemplate->mpcSymbol;
		}
		else if (lliName != -1)
		{
			return FindClass(lliName, ComponentLookup(pcComponent));
		}
	}
	if (!pcSymbol)
	{
		return NULL;
	}

	if (pcSymbol->meKind == CPPSK_Class)
	{
		return GetClass(pcSymbol, pcNode);
	}
	if (pcSymbol->meKind == CPPSK_Typedef)
	{
		psType = mcDeclarators.GetSymbolType(pcSymbol, pcNode);
		return (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TypeParameter)
	{
		psBinding = mcInstantiator.FindBinding(pcSymbol);
		return (psBinding && psBinding->psType && psBinding->psType->IsClass()) ? psBinding->psType->pcClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TemplateTemplateParameter)
	{
		//C<T>, C bound to a class template: its instance.
		psBinding = mcInstantiator.FindBinding(pcSymbol);
		if (!psBinding || !psBinding->pcTemplate || !pcComponent || !pcComponent->mbTemplateArguments)
		{
			mcDiagnostics.Error(pcNode, "A template template parameter used outside an instantiation");
			return NULL;
		}
		return mcInstantiator.InstantiateClass(psBinding->pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
	}
	if (pcSymbol->meKind == CPPSK_ClassTemplate)
	{
		pcTemplate = mcInstantiator.GetTemplate(pcSymbol);
		if (!pcTemplate)
		{
			//A pointer to an instance of a template that is only declared needs no definition.
			pcClass = (pcComponent && pcComponent->mbTemplateArguments) ? mcInstantiator.UndefinedInstance(pcSymbol, &pcComponent->macpTemplateArguments, pcNode) : NULL;
			if (!pcClass)
			{
				mcDiagnostics.Error(pcNode, "The class template has not been defined");
			}
			return pcClass;
		}
		if (pcComponent && pcComponent->mbTemplateArguments)
		{
			return mcInstantiator.InstantiateClass(pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
		}

		//The template's own name inside it is the instance being made, and a base's name
		//in a class derived from an instance is that instance.
		for (pcBindings = mcContext.GetBindings(); pcBindings; pcBindings = pcBindings->mpcOuter)
		{
			if (pcBindings->mpcClass && pcBindings->mpcClass->mpcTemplate && (pcBindings->mpcClass->mpcTemplate->mpcPrimary == pcTemplate))
			{
				return pcBindings->mpcClass;
			}
		}
		if (!pcContext && mcClosures.OuterFunction())
		{
			pcContext = mcClosures.OuterFunction()->mpcClass;
		}
		pcClass = pcContext ? FindBaseInstance(pcContext, pcTemplate) : NULL;
		if (pcClass)
		{
			return pcClass;
		}
		mcDiagnostics.Error(pcNode, "A class template needs template arguments");
		return NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template that is pcClass or one of its bases, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate)
{
	CCPPClass*	pcBase;
	size			i;

	mpcStatistics->Count(CPPCC_BaseWalks);
	if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary == pcTemplate))
	{
		return pcClass;
	}
	mcDiagnostics.BeginQuiet();
	ResolveBases(pcClass, NULL);
	mcDiagnostics.EndQuiet();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcBase = FindBaseInstance(pcClass->macBases.Get(i)->pcClass, pcTemplate);
		if (pcBase)
		{
			return pcBase;
		}
	}
	return NULL;
}


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
// The class a qualified member name (object.Qualifier::f) names: pcClass or one of its bases.  A class
// template's name without arguments is pcClass's base instance of it.  A qualifier the context doesn't
// find is looked up in the object's class: p->Base::v, Base a member typedef.  [basic.lookup.classref] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPClass*	pcQualifier;
	CCPPSymbol*	pcSymbol;
	int64		lliName;

	pcSymbol = NULL;
	if (pcComponent && !pcComponent->mpcSymbol && (pcComponent->meKind == CPPNC_Identifier) && pcClass->mpcSymbol && pcClass->mpcSymbol->mpcMembers)
	{
		lliName = GetComponentName(pcComponent);
		pcSymbol = (lliName != -1) ? pcClass->mpcSymbol->mpcMembers->FindMember(lliName, CPPL_Type) : NULL;
	}
	pcQualifier = GetComponentClass(pcComponent, pcSymbol, pcNode, pcClass);
	if (!pcQualifier || !IsDerived(pcClass, pcQualifier, NULL))
	{
		mcDiagnostics.Error(pcNode, "The qualifier is not the object's class or one of its bases");
		return NULL;
	}
	return pcQualifier;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a (possibly qualified) name's last component names.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSymbol*	pcSymbol;
	CCPPClass*	pcClass;

	//P<char>::Q<long>: P<char>'s member class template's instance.
	pcSymbol = pcName->mpcSymbol;
	if (pcName->GetQualifier() && pcSymbol && (pcSymbol->meKind == CPPSK_ClassTemplate) && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Class))
	{
		return QualifiedClass(pcName, pcNode);
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Class) && pcName->IsQualifiedByInstance())
	{
		//P<char>::Inner: that instance's member class.  [temp.mem.class]
		mcDiagnostics.BeginSilent();
		pcClass = QualifiedClass(pcName, pcNode);
		mcDiagnostics.EndSilent();
		if (pcClass)
		{
			return pcClass;
		}
	}
	return GetComponentClass(pcName->GetLast(), pcSymbol, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Finds a class's base classes without laying it out, so a pointer to it can be
// converted to a pointer to a base.  Offsets are worked out by LayoutClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxBaseSpecifier*	pcBaseSpecifier;
	size						i;
	bool						bResult;

	if (pcClass->HasBases())
	{
		return true;
	}
	if (!FindSpecifier(pcClass))
	{
		return mcDiagnostics.Error(pcNode, "The class has not been defined");
	}

	pcClass->meState = CPPCS_ResolvingBases;
	mcContext.EnterBindings(pcClass->mpcBindings);
	bResult = true;
	for (i = 0; i < pcClass->mpcSpecifier->macpBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcClass->mpcSpecifier->macpBases.GetPtr(i);
		bResult &= pcBaseSpecifier->mbPack ? AddPackBases(pcClass, pcBaseSpecifier) : AddBase(pcClass, pcBaseSpecifier, pcBaseSpecifier->mpcName);
	}
	pcClass->mpcBase = (pcClass->macBases.NumElements() > 0) ? pcClass->macBases.Get(0)->pcClass : NULL;
	pcClass->meState = CPPCS_BasesKnown;
	mcContext.LeaveBindings();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a base specifier names (pcName: its name, or one of its pack
// expansion's) is a base of pcClass, its bases found.  Not one declared
// final.  [class] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AddBase(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier, CCPPSyntaxName* pcName)
{
	CCPPClass*	pcBaseClass;
	SCPPBase*	psBase;
	bool		bResult;

	pcBaseClass = GetNameClass(pcName, pcBaseSpecifier);
	if (!pcBaseClass)
	{
		//[class.derived] 2
		return mcDiagnostics.Error(pcBaseSpecifier, (pcName->mpcSymbol && !pcName->mpcSymbol->IsClass()) ? "A base class is a class: the name is another kind of type" : "This base class is not supported by the compiler yet");
	}
	bResult = ResolveBases(pcBaseClass, pcBaseSpecifier);
	if (pcBaseClass->mpcSpecifier && pcBaseClass->mpcSpecifier->mbFinal)
	{
		bResult &= mcDiagnostics.Error(pcBaseSpecifier, "The base class is declared final");
	}
	psBase = pcClass->macBases.Add();
	psBase->pcClass = pcBaseClass;
	psBase->iOffset = 0;
	psBase->bVirtual = pcBaseSpecifier->mbVirtual;
	psBase->eAccess = (pcBaseSpecifier->meAccess != CPPA_None) ? pcBaseSpecifier->meAccess : ((pcClass->mpcSpecifier->meKey == CT_class) ? CPPA_Private : CPPA_Public);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// : T... and : Leaf<I, T>... have a base for each element of the packs the
// name names, in order: the name expanded as any pattern is (ExpandPattern),
// of type packs and value packs alike.  [temp.variadic] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AddPackBases(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier)
{
	CArraySyntax	apcNames;
	size			k;
	bool			bResult;

	apcNames.Init();
	bResult = ExpandPattern(pcBaseSpecifier->mpcName, pcBaseSpecifier, &apcNames);
	for (k = 0; bResult && (k < apcNames.NumElements()); k++)
	{
		bResult = AddBase(pcClass, pcBaseSpecifier, (CCPPSyntaxName*)apcNames.GetPtr(k));
	}
	apcNames.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// CompleteClass for the class of an object that is being made (a variable,
// a temporary, what new makes): it can't be abstract.  A base of an object
// can, so this is not asked for a base.  [class.abstract]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CompleteObjectClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	if (!CompleteClass(pcClass, pcNode))
	{
		return false;
	}
	if (pcClass->IsAbstract())
	{
		return mcDiagnostics.Error(pcNode, "An object of an abstract class can't be made: a pure virtual function of it has no body");
	}
	return true;
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
bool CCPPUnit::CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	if (pcClass->IsComplete())
	{
		return true;
	}
	if (pcClass->IsBeingMade())
	{
		return mcDiagnostics.Error(pcNode, "A class cannot contain itself");
	}
	if (!FindSpecifier(pcClass))
	{
		return mcDiagnostics.Error(pcNode, "The class has not been defined");
	}
	return LayoutClass(pcClass, pcNode);
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
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
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
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcClass, false);
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
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcClass, false);
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

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
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
// Lays out a class: the bases in order (an empty base takes no space), the vtable
// pointer if no base has one, then the fields.  Members are read first: functions
// are declared (in class definitions are generated when called) and fields are
// collected.  A template instance's members and its members defined outside it are
// worked out with its template arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxBaseSpecifier*		pcBaseSpecifier;
	CCPPSyntaxNameComponent*		pcComponent;
	CCPPTemplate*					pcMember;
	CCPPFunction*					pcFunction;
	size							i;
	size							j;
	int								iClassAlign;
	int								iPack;
	bool							bResult;

	mpcStatistics->Count(CPPCC_ClassLayouts);
	pcSpecifier = pcClass->mpcSpecifier;
	mcContext.EnterBindings(pcClass->mpcBindings);
	bResult = true;

	pcClass->mbUnion = pcSpecifier->meKey == CT_union;
	pcComponent = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
	if (pcComponent && !pcClass->mpcTemplate)
	{
		pcClass->mlliName = GetComponentName(pcComponent);
	}

	bResult &= ResolveBases(pcClass, pcSpecifier);
	pcClass->meState = CPPCS_LayingOut;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcSpecifier->macpBases.GetPtr(i);
		bResult &= CompleteClass(pcClass->macBases.Get(i)->pcClass, pcBaseSpecifier);
		if (pcClass->macBases.Get(i)->pcClass->mbUnion)
		{
			//[class.union] 2
			bResult &= mcDiagnostics.Defer(pcBaseSpecifier->miFirstToken, "A union is no base class");
		}
		for (j = 0; j < i; j++)
		{
			if (pcClass->macBases.Get(j)->pcClass == pcClass->macBases.Get(i)->pcClass)
			{
				//[class.mi] 3
				bResult &= mcDiagnostics.Defer(pcBaseSpecifier->miFirstToken, "A class is a direct base of another once");
				break;
			}
		}
	}

	//Its friends are of its namespace, whatever is being generated when it is completed.
	mcContext.EnterNamespace(pcClass->mpcSymbol ? pcClass->mpcSymbol->mpcScope->GetNamespace() : mcContext.GetNamespace());
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(pcClass, pcSpecifier->macpMembers.GetPtr(i));
	}
	mcContext.LeaveNamespace();

	//A class template's members, and its member classes', defined outside it (so far: one defined later is given to
	//the instances already laid out then, DefineOutsideMembers).
	for (i = 0; (pcClass->mpcTemplate || pcClass->mpcEnclosing) && (i < mapcTemplates.NumElements()); i++)
	{
		pcMember = mapcTemplates.GetPtr(i);
		if (IsOutsideMemberOf(pcMember, pcClass))
		{
			DefineOutsideMember(pcClass, pcMember);
		}
	}

	//Its members a template<> definition specialises for it.
	if (pcClass->mpcTemplate)
	{
		SpecialiseMembers(pcClass);
	}

	CompleteMembers(pcClass);

	//The bases, the vtable pointer, then the fields.
	iClassAlign = AlignasAlignment(&pcSpecifier->macpAttributes, true);
	if (iClassAlign < 0)
	{
		bResult = false;
	}
	//(A member is aligned to at most the lesser of #pragma pack's bytes and the target's limit.)
	iPack = mpcTarget->GetMemberAlignmentLimit();
	iPack = (iPack > 0) ? CCPPClassLayout::PackAlignment(iPack, pcSpecifier->miPack) : pcSpecifier->miPack;
	PlaceMembers(pcClass, iPack, iClassAlign);
	if (mbLayouts)
	{
		AddLayout(pcClass);
	}
	pcClass->meState = CPPCS_Complete;
	bResult &= CheckOverrides(pcClass);
	if (pcClass->mcVtable.mpcAmbiguous)
	{
		//[class.virtual] 2
		bResult &= mcDiagnostics.Defer(pcSpecifier->miFirstToken, "A virtual function of a virtual base has two final overriders in the class, neither overriding the other");
	}

	if (pcClass->mpcInheritedBase)
	{
		InheritConstructors(pcClass);
	}

	//A class that needs one but declares none gets a constructor written for it when it is called.
	if (!pcClass->mbConstructors && pcClass->NeedsConstructor())
	{
		pcFunction = AddImplicitFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, &mcTypes.msVoid, true);
	}
	mcContext.LeaveBindings();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What the compiler adds to a class once its members are read, a
// closure's too: the destructor its bases or fields need to be destroyed,
// for a virtual destructor the deleting one, and its vtable.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CompleteMembers(CCPPClass* pcClass)
{
	CCPPFunction*	pcFunction;

	//A destructor is needed to destroy the bases or the fields.
	if (!pcClass->mpcDestructor && pcClass->NeedsDestructor())
	{
		pcFunction = AddImplicitFunction(pcClass, pcClass->mlliName, CPPFK_Destructor, &mcTypes.msVoid, false);
		pcClass->mpcDestructor = pcFunction;
	}
	if (pcClass->HasVirtualDestructor())
	{
		//delete of a pointer to a base frees the object as its own class does, which only the object knows: by a
		//virtual function of its class, written for it, that destroys it and frees it.  [expr.delete] 3, 9, 10
		pcFunction = AddImplicitFunction(pcClass, CPP_DELETING_NAME, CPPFK_Function, &mcTypes.msVoid, false);
		pcFunction->mbDeleting = true;
		pcFunction->mbVirtual = true;
		pcFunction->mbDefined = true;
		pcClass->mpcDeleting = pcFunction;
	}
	pcClass->mcVtable.LayoutOwn(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a class's bases, vtable pointer and fields (CCPPClassLayout), a
// closure's too, and gives its type their size.  iPack is #pragma pack's
// bytes and iAlign alignas's, 0 for none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::PlaceMembers(CCPPClass* pcClass, int iPack, int iAlign)
{
	CCPPClassLayout	cLayout;

	cLayout.Init(pcClass, iPack, mpcTarget->GetPointerBytes());
	cLayout.Layout(iAlign);
	mcTypes.SetClassBytes(pcClass->mpsType, cLayout.GetBytes());
	cLayout.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A class's functions' override and final ([class.virtual] 4 and 5): one
// declared override must override a base class's virtual function, and none
// may override one declared final.  One that overrides may allow only the
// exceptions the function it overrides allows ([except.spec] 5).  One that overrides and is declared
// neither is warned about (mbWarnMissingOverride), but for a destructor, as
// G++'s -Wsuggest-override doesn't.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckOverrides(CCPPClass* pcClass)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcOverridden;
	size			i;
	bool			bResult;

	bResult = true;
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mbStatic || pcFunction->mbImplicit || !pcFunction->mpcDeclarator || (pcFunction->meKind == CPPFK_Constructor))
		{
			continue;
		}
		pcOverridden = pcClass->FindOverridden(pcFunction);
		if (pcFunction->mbOverride && !pcOverridden)
		{
			bResult &= mcDiagnostics.Error(pcFunction->mpcDeclarator, "The function is declared override but overrides no virtual function of a base class");
		}
		else if (pcFunction->mbFinal && !pcFunction->mbVirtual && !pcOverridden)
		{
			//[class.mem] 8: override and final are a virtual member function's.
			bResult &= mcDiagnostics.Error(pcFunction->mpcDeclarator, "The function is declared final but isn't virtual");
		}
		else if (pcOverridden && pcOverridden->mbFinal)
		{
			bResult &= mcDiagnostics.Error(pcFunction->mpcDeclarator, "The function overrides one declared final");
		}
		else if (pcOverridden && !ReturnsCovariant(pcFunction, pcOverridden))
		{
			//[class.virtual] 7
			bResult &= mcDiagnostics.Defer(pcFunction->mpcDeclarator->miFirstToken, "The function returns another type than the virtual function it overrides, and not a pointer or reference to a class derived from that one's");
		}
		else if (pcOverridden && !mcExceptions.AllowsOnlyWhat(mcExceptions.ExceptionsOf(pcFunction), mcExceptions.ExceptionsOf(pcOverridden)))
		{
			bResult &= mcDiagnostics.Error(pcFunction->mpcDeclarator, "The function allows exceptions that the virtual function it overrides doesn't");
		}
		else if (pcOverridden && !pcFunction->mbOverride && !pcFunction->mbFinal && (pcFunction->meKind != CPPFK_Destructor) && mpcOptions->mbWarnMissingOverride)
		{
			mcDiagnostics.Warning(pcFunction->mpcDeclarator, "The function overrides a virtual function of a base class but isn't declared override");
		}
	}

	//The destructor the compiler declares allows what the class's parts' destructors do, which may be more.
	pcFunction = pcClass->mpcDestructor;
	pcOverridden = (pcFunction && pcFunction->mbImplicit) ? pcClass->FindOverridden(pcFunction) : NULL;
	if (pcOverridden && !mcExceptions.AllowsOnlyWhat(mcExceptions.ExceptionsOf(pcFunction), mcExceptions.ExceptionsOf(pcOverridden)))
	{
		bResult &= mcDiagnostics.Error(pcClass->mpcSpecifier, "The class's destructor allows exceptions that the virtual destructor it overrides doesn't");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an overrider's result may be the overridden function's: the same
// type, or a pointer (or a reference of the same kind) to a class derived
// from the class the overridden's points to, no more const or volatile.
// [class.virtual] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ReturnsCovariant(CCPPFunction* pcOverrider, CCPPFunction* pcOverridden)
{
	SCPP65Type*	psFrom;
	SCPP65Type*	psTo;

	psFrom = pcOverrider->mpsReturn;
	psTo = pcOverridden->mpsReturn;
	if (!psFrom || !psTo || psFrom->Same(psTo) || (psFrom == &mcTypes.msUndeduced) || (psTo == &mcTypes.msUndeduced) || pcOverrider->mpcTemplate || pcOverridden->mpcTemplate)
	{
		return true;
	}
	if (!((psFrom->IsPointer() && psTo->IsPointer()) || (psFrom->IsReference() && psTo->IsReference() && (psFrom->bRvalue == psTo->bRvalue))))
	{
		return false;
	}
	psFrom = psFrom->psBase;
	psTo = psTo->psBase;
	if (!psFrom->IsClass() || !psTo->IsClass() || (psFrom->bConst && !psTo->bConst) || (psFrom->bVolatile && !psTo->bVolatile))
	{
		return false;
	}
	return (psFrom->pcClass == psTo->pcClass) || IsDerived(psFrom->pcClass, psTo->pcClass, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's declarator says override (or final, lliName).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasVirtSpecifier(CCPPSyntaxFunctionSuffix* pcSuffix, int64 lliName)
{
	CCPPToken*	pcToken;
	size		i;

	for (i = 0; pcSuffix && (i < pcSuffix->macpVirtSpecifiers.NumElements()); i++)
	{
		pcToken = mpcTokeniser->Get(pcSuffix->macpVirtSpecifiers.GetPtr(i)->miFirstToken);
		if (pcToken && pcToken->IsIdentifier() && (((CCTIdentifier*)pcToken)->miIdentifier == lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::f; in a class makes Base's functions named f overloads of the class's
// own.  using Base::Base; inherits Base's constructors (see InheritConstructors).
// Other using declarations (of types) are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing)
{
	CCPPSyntaxName*	pcName;
	CCPPClass*	pcBase;
	SCPPUsing*	psUsing;
	int64			lliName;
	CCPPSymbol*		pcSymbol;
	CCPPClass*		pcDeclaring;
	bool			bReached;

	pcName = pcUsing->mpcName;
	if (pcUsing->mbTypename || !pcName || !pcName->IsQualifiedByClass())
	{
		return;
	}
	lliName = GetFunctionName(pcName->GetLast());
	if (lliName == -1)
	{
		return;
	}
	mcDiagnostics.BeginQuiet();
	pcBase = GetComponentClass(pcName->GetQualifier(), NULL, pcUsing, pcClass);
	mcDiagnostics.EndQuiet();
	if (!pcBase || (pcBase == pcClass))
	{
		return;
	}
	if (pcBase->mlliName == lliName)
	{
		pcClass->mpcInheritedBase = pcBase;
		return;
	}

	//What it names is a member the class may name: a base's private member isn't.  [namespace.udecl] 17
	pcSymbol = pcBase->FindMemberSymbol(lliName, CPPSK_Field, &pcDeclaring);
	if (!pcSymbol)
	{
		pcSymbol = pcBase->FindMemberSymbol(lliName, CPPSK_Function, &pcDeclaring);
	}
	if (pcSymbol)
	{
		mcDiagnostics.BeginSilent();
		bReached = mcBinder.UsingReaches(pcSymbol, pcUsing);
		mcDiagnostics.EndSilent();
		if (!bReached)
		{
			mcDiagnostics.Defer(pcUsing->miFirstToken, "The using-declaration names a base's private member, which the class can't reach");
			return;
		}
	}
	psUsing = pcClass->macUsings.Add();
	psUsing->pcBase = pcBase;
	psUsing->lliName = lliName;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::Base;  The class gets a constructor for each of the base's (not its
// default, copy or move constructors, constructor templates, or ones with the same
// parameters as the class's own) that calls the base's with its arguments and makes
// the rest of the object as a default constructor does.  A class that declares no
// constructor of its own keeps its default constructor.  [class.inhctor.init]
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::InheritConstructors(CCPPClass* pcClass)
{
	CCPPClass*	pcBase;
	CCPPFunction*	pcInherited;
	CCPPFunction*	pcOwn;
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;
	size			j;
	size			k;
	bool			bSame;
	bool			bAdded;

	pcBase = pcClass->mpcInheritedBase;
	if (!CompleteClass(pcBase, NULL))
	{
		return;
	}
	bAdded = false;
	for (i = 0; i < pcBase->mapcMembers.NumElements(); i++)
	{
		pcInherited = pcBase->mapcMembers.GetPtr(i);
		if ((pcInherited->meKind != CPPFK_Constructor) || pcInherited->mpcTemplate || pcInherited->mbImplicit || (pcInherited->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = pcInherited->mapsParameters.GetPtr(0);
		if ((pcInherited->mapsParameters.NumElements() == 1) && psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == pcBase))
		{
			continue;
		}

		bSame = false;
		for (j = 0; !bSame && (j < pcClass->mapcMembers.NumElements()); j++)
		{
			pcOwn = pcClass->mapcMembers.GetPtr(j);
			if ((pcOwn->meKind != CPPFK_Constructor) || (pcOwn->mapsParameters.NumElements() != pcInherited->mapsParameters.NumElements()))
			{
				continue;
			}
			//The same signature: const below the top makes another (D(const int*) doesn't stand for B(int*)).
			bSame = pcOwn->TakesParameters(&pcInherited->mapsParameters);
		}
		if (bSame)
		{
			continue;
		}

		pcFunction = AddFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, &mcTypes.msVoid);
		for (k = 0; k < pcInherited->mapsParameters.NumElements(); k++)
		{
			pcFunction->mapsParameters.Add(pcInherited->mapsParameters.GetPtr(k));
			pcFunction->mapcDefaults.Add(pcInherited->mapcDefaults.GetPtr(k));
		}
		pcFunction->mbExplicit = pcInherited->mbExplicit;
		pcFunction->mbConstexpr = pcInherited->mbConstexpr;
		pcFunction->mbImplicit = true;
		pcFunction->mbDefined = true;
		pcFunction->mpcInherited = pcInherited;
		bAdded = true;
	}

	if (bAdded && !pcClass->mbConstructors)
	{
		pcFunction = AddImplicitFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, &mcTypes.msVoid, true);
		pcClass->mbConstructors = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Declares a member of a class being laid out.  Functions defined in the class are
// generated when they are called.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxClassSpecifier*		pcClassSpecifier;
	SCPPField*					psField;
	CCPPVariable*					pcVariable;
	CCPPFunction*					pcFunction;
	SCPP65Type*						psType;
	SCPP65Type*						psElement;
	size							i;
	size							uiMembers;
	bool							bPure;
	bool							bResult;
	bool							bFriend;
	int64							lliZero;
	int								iAlign;

	switch (pcMember->GetKind())
	{
	case CPPS_AccessSpecifier:
	case CPPS_EmptyDeclaration:
	case CPPS_AliasDeclaration:
		return true;

	case CPPS_StaticAssert:
		//A class template's are worked out for each instance, whose arguments they may be about; a class's own were
		//when it was defined (ClassStaticAsserts).
		return !pcClass->mpcTemplate || StaticAssert((CCPPSyntaxStaticAssert*)pcMember);

	case CPPS_UsingDeclaration:
		ClassUsing(pcClass, (CCPPSyntaxUsingDeclaration*)pcMember);
		return true;

	case CPPS_TemplateDeclaration:
		return mcInstantiator.MemberTemplate(pcClass, (CCPPSyntaxTemplateDeclaration*)pcMember);

	case CPPS_FunctionDefinition:
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember;
		if ((pcDefinition->meBody == CPPFB_Default) && pcDefinition->mpcDeclarator->GetName() && (GetFunctionName(pcDefinition->mpcDeclarator->GetName()->GetLast()) == GetOperatorName(CT_Assignment, false)))
		{
			//operator = (...) = default is the implicit copy assignment, which = does when the class declares none.
			return true;
		}
		bFriend = pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_FRIEND);
		uiMembers = pcClass->mapcMembers.NumElements();
		pcFunction = DeclareQuietly(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, bFriend ? NULL : pcClass, false);
		if (!pcFunction && pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL))
		{
			//Leaving out a virtual function would move the vtable's other entries.
			return mcDiagnostics.Error(pcDefinition, "This virtual function's declaration is not supported by the compiler yet");
		}
		if (!pcFunction)
		{
			return true;
		}
		if (IsDeclaredAgainInClass(pcClass, pcFunction, uiMembers))
		{
			return mcDiagnostics.Defer(pcDefinition->mpcDeclarator->miFirstToken, "The member function is declared twice in its class");
		}
		if (!CheckDefaultedOrDeleted(pcFunction, pcDefinition))
		{
			return false;
		}
		pcFunction->SetDefinition(pcDefinition);
		mpcConstants->CheckDefinition(pcFunction);
		return true;

	case CPPS_SimpleDeclaration:
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
		pcSpecifiers = pcDeclaration->mpcSpecifiers;
		if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND))
		{
			//A friend function is a function of the namespace, not a member.
			for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
			{
				pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
				if (pcInit->mpcDeclarator && pcInit->mpcDeclarator->IsFunction())
				{
					DeclareQuietly(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
				}
			}
			return true;
		}
		if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
		{
			return true;
		}
		if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			//A nested class is laid out when it is used.
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType;
			if (!ClassStaticAsserts(pcClassSpecifier))
			{
				return false;
			}
			if (pcClassSpecifier->mpcSymbol)
			{
				GetClass(pcClassSpecifier->mpcSymbol, pcMember)->mpcSpecifier = pcClassSpecifier;
			}
			else if (pcDeclaration->macpDeclarators.NumElements() == 0)
			{
				//union { ... }; is an unnamed field whose members are the class's (FindField looks in it).
				psType = GetUnnamedClass(pcClassSpecifier)->mpsType;
				if (!CompleteClass(psType->pcClass, pcMember))
				{
					return false;
				}
				pcClass->macFields.Add()->Init(NULL, psType);
				return true;
			}
		}

		bResult = true;
		iAlign = pcSpecifiers ? AlignasAlignment(&pcSpecifiers->macpSpecifiers) : 0;
		if (iAlign < 0)
		{
			return false;
		}
		for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
			if (pcInit->mpcDeclarator->IsFunction() || (!pcInit->mpcBitField && IsFunctionByType(pcSpecifiers, pcInit->mpcDeclarator)))
			{
				//virtual int f() = 0;
				bPure = pcInit->mpcInitializer && mcFolder.Constant(pcInit->mpcInitializer, &lliZero) && (lliZero == 0);
				uiMembers = pcClass->mapcMembers.NumElements();
				pcFunction = DeclareQuietly(pcSpecifiers, pcInit->mpcDeclarator, false, pcClass, bPure);
				if (!pcFunction && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
				{
					//Leaving out a virtual function would move the vtable's other entries.
					bResult &= mcDiagnostics.Error(pcInit, "This virtual function's declaration is not supported by the compiler yet");
				}
				else if (!pcFunction)
				{
				}
				else if (IsDeclaredAgainInClass(pcClass, pcFunction, uiMembers))
				{
					bResult &= mcDiagnostics.Defer(pcInit->mpcDeclarator->miFirstToken, "The member function is declared twice in its class");
				}
				else if ((pcInit->meInitializer == CPPI_Default) && (pcFunction->meKind != CPPFK_Function))
				{
					pcFunction->mbImplicit = true;
				}
				continue;
			}
			if (pcInit->mpcBitField)
			{
				bResult &= ClassBitField(pcClass, pcSpecifiers, pcInit);
				continue;
			}
			if (!pcInit->mpcSymbol)
			{
				continue;
			}

			psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				bResult = false;
				continue;
			}
			if (psType->IsFunction())
			{
				//A function's type that IsFunctionByType didn't take for a member function's (one made of a template's
				//parameter): it is no field.  [temp.arg.type] 3
				bResult &= mcDiagnostics.Error(pcInit, "A member function declared by a type that depends on a template parameter is not supported by the compiler yet");
				continue;
			}
			if (!InjectedNameReached(pcClass, pcSpecifiers, psType, pcInit))
			{
				bResult = false;
				continue;
			}
			if (pcSpecifiers->muiFlags & CPPDS_STATIC)
			{
				if (pcInit->mpcInitializer && !(pcSpecifiers->muiFlags & CPPDS_CONSTEXPR) && !(psType->bConst && psType->IsInteger()))
				{
					//[class.static.data] 3
					bResult &= mcDiagnostics.Defer(pcInit->miFirstToken, "Only a static data member that is a const integer or enum, or constexpr, is initialised in its class");
				}
				pcVariable = AddVariable(&mapcGlobals, pcInit->mpcSymbol, psType, CPPVS_Global, 0);
				pcVariable->mpcClass = pcClass;
				pcVariable->mpcInClassInitialiser = pcInit->mpcInitializer;
				continue;
			}

			if (pcClass->mbUnion && psType->IsReference())
			{
				//[class.union] 2
				bResult &= mcDiagnostics.Defer(pcInit->miFirstToken, "A union has no reference member");
				continue;
			}
			psElement = psType->ElementType();
			if (psElement->IsClass() && !CompleteClass(psElement->pcClass, pcInit))
			{
				bResult = false;
				continue;
			}
			if (psType->IsArray() && psElement->IsClass())
			{
				psType = mcTypes.Array(psElement, psType->iCount);
			}
			psField = pcClass->macFields.Add();
			psField->Init(pcInit->mpcSymbol, psType);
			psField->iAlign = iAlign;
			psField->pcInitialiser = pcInit->mpcInitializer;
			psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
		}
		return bResult;

	default:
		return mcDiagnostics.Error(pcMember, "This member is not supported by the compiler yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A member's type named by a base's name alone (A in class C : B { A* p; })
// is that base's injected class name, found in the class as a member of
// the base: it must be reachable from the class, which a private base of a
// base isn't.  False (a class error) if it isn't.  [class.access.spec] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::InjectedNameReached(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, SCPP65Type* psType, CCPPSyntax* pcNode)
{
	CCPPSyntaxName*	pcName;
	CCPPClass*		pcNamed;
	bool			bReached;

	if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return true;
	}
	pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName;
	while (psType && (psType->IsPointer() || psType->IsReference() || psType->IsArray()))
	{
		psType = psType->psBase;
	}
	if (!pcName || !pcName->IsSimple() || !psType || !psType->IsClass())
	{
		return true;
	}
	pcNamed = psType->pcClass;
	if ((pcNamed == pcClass) || (GetComponentName(pcName->GetLast()) != pcNamed->mlliName) || !IsDerived(pcClass, pcNamed, NULL))
	{
		return true;
	}
	mcDiagnostics.BeginSilent();
	bReached = mcBinder.ReachesBase(pcClass, pcNamed, pcNode);
	mcDiagnostics.EndSilent();
	return bReached || mcDiagnostics.Defer(pcNode->miFirstToken, "The class's name here is its injected name, found through a base that can't be reached from here: write ::Name");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction, which a member of pcClass's definition declares, was
// declared by another of its members before: it was a member of the class
// (one of its first uiMembers) declared in the class, not outside it.  A
// member is declared once in its class (a function declared there may be
// defined outside it).  [class.mem] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsDeclaredAgainInClass(CCPPClass* pcClass, CCPPFunction* pcFunction, size uiMembers)
{
	CCPPSyntax*	pcNode;
	size		i;

	if (!pcFunction->mpcDeclarator || (pcFunction->mpcClass != pcClass) || pcFunction->mbImplicit)
	{
		return false;
	}
	for (i = 0; (i < uiMembers) && (pcClass->mapcMembers.GetPtr(i) != pcFunction); i++)
	{
	}
	if (i == uiMembers)
	{
		return false;
	}
	for (pcNode = pcFunction->mpcDeclarator->mpcParent; pcNode; pcNode = pcNode->mpcParent)
	{
		if (pcNode->GetKind() == CPPS_ClassSpecifier)
		{
			return pcNode == pcClass->mpcSpecifier;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A class as it was laid out, as a line of maszLayouts (SetLayouts): its
// name, where it is defined, its size, its alignment and the most alignas
// asks of it, then where each base, the vtable pointer and each field is
// ("name@offset", a bit-field's first bit after a '.').
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AddLayout(CCPPClass* pcClass)
{
	CChars					sz;
	SPreprocessedToken*		psSource;
	SCPPField*				psField;
	CCPPTypeWriter			cWriter;
	size					i;

	cWriter.Init(this, CPPTW_Shown);
	sz.Init();
	cWriter.AppendClass(pcClass, &sz);
	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(pcClass->mpcSpecifier->miFirstToken));
	if (psSource && psSource->pcFile)
	{
		sz.Append(' ')->Append(psSource->pcFile->ShortName())->Append('(')->Append(psSource->iLine + 1)->Append(')');
	}
	sz.Append(" size ")->Append(pcClass->mpsType->iBytes)->Append(" align ")->Append(pcClass->miAlign)->Append(" alignas ")->Append(pcClass->miAlignas);
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		sz.Append(pcClass->macBases.Get(i)->bVirtual ? " virtual " : " base ");
		cWriter.AppendClass(pcClass->macBases.Get(i)->pcClass, &sz);
		if (!pcClass->macBases.Get(i)->bVirtual)
		{
			sz.Append('@')->Append(pcClass->macBases.Get(i)->iOffset);
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		sz.Append(" virtual-base ");
		cWriter.AppendClass(pcClass->macVirtualBases.Get(i)->pcClass, &sz);
		sz.Append('@')->Append(pcClass->macVirtualBases.Get(i)->iOffset);
	}
	if (pcClass->mcVtable.HasPointer())
	{
		sz.Append(" vptr@")->Append(pcClass->mcVtable.miPointerOffset);
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		sz.Append(' ');
		cWriter.AppendIdentifier(psField->lliName, &sz);
		sz.Append('@')->Append(psField->iOffset);
		if (psField->IsBitField())
		{
			sz.Append('.')->Append(psField->iBitOffset);
		}
	}
	maszLayouts.Add(&sz);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A bit-field (int iFlags : 3;), or an unnamed one (int : 3;, int : 0;),
// of an integer, bool or enum type, its width a constant no wider than its
// type.  LayoutClass places it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit)
{
	SCPP65Type*		psType;
	SCPPField*		psField;
	CChars			szMessage;
	int64			lliWidth;
	bool			bResult;

	//A class is laid out once, often first asked about quietly (TypeOf) or by the IR front end, so its own errors are
	//kept to report when the unit ends.
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC))
	{
		return mcDiagnostics.Defer(pcInit->miFirstToken, "A bit-field can't be static");
	}
	psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
	if (!psType)
	{
		return false;
	}
	if (!psType->IsInteger())
	{
		return mcDiagnostics.Defer(pcInit->miFirstToken, "A bit-field must be an integer, bool or enum");
	}
	bResult = true;
	if (!mcFolder.Constant(pcInit->mpcBitField, &lliWidth) || (lliWidth < 0))
	{
		szMessage.Init();
		mcFolder.ConstantMessage("A bit-field's width must be a constant of 0 or more", &szMessage);
		bResult = mcDiagnostics.Defer(pcInit->mpcBitField->miFirstToken, szMessage.Text());
		szMessage.Kill();
		lliWidth = 1;
	}
	if (lliWidth > (int64)psType->iBytes * 8)
	{
		bResult = mcDiagnostics.Defer(pcInit->mpcBitField->miFirstToken, "A bit-field can't be wider than its type");
		lliWidth = psType->iBytes * 8;
	}
	if ((lliWidth == 0) && pcInit->mpcSymbol)
	{
		bResult = mcDiagnostics.Defer(pcInit->mpcBitField->miFirstToken, "A named bit-field can't have a width of 0");
		lliWidth = 1;
	}
	psField = pcClass->macFields.Add();
	psField->Init(pcInit->mpcSymbol, psType, (int)lliWidth);
	psField->pcInitialiser = pcInit->mpcInitializer;
	psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A const integer static data member initialised in its class (static const int
// Max = 100;), or a constexpr float one: where its value is used it is a
// constant, and needs no definition.  Its address, or a reference to it, needs
// one.  [class.static.data] 3, 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsConstantMember(CCPPVariable* pcVariable)
{
	SCPP65Type*	psType;

	if (!pcVariable || !pcVariable->mpcClass || !pcVariable->mpcInClassInitialiser)
	{
		return false;
	}
	psType = pcVariable->mpsType;
	return psType && psType->bConst && (psType->IsInteger() || psType->IsFloat());
}


//////////////////////////////////////////////////////////////////////////
//
// A static data member of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::FindStatic(CCPPClass* pcClass, int64 lliName)
{
	CCPPVariable*	pcVariable;
	size			i;

	for (i = 0; i < mapcGlobals.NumElements(); i++)
	{
		pcVariable = mapcGlobals.GetPtr(i);
		if ((pcVariable->mpcClass == pcClass) && (pcVariable->mpcSymbol->mlliName == lliName))
		{
			return pcVariable;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcVariable = FindStatic(pcClass->macBases.Get(i)->pcClass, lliName);
		if (pcVariable)
		{
			return pcVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the pointer on the left of ->: the object's operator->'s result when
// it is an object of a class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	int				i;

	psType = mcTypeChecker.TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return psType;
	}
	if (!CompleteClass(psType->pcClass, pcNode))
	{
		return NULL;
	}
	pcFunction = mcBinder.FindArrowOperator(psType->pcClass, pcObject, pcNode);
	psType = pcFunction ? pcFunction->mpsReturn : NULL;
	psType = (psType && psType->IsReference() && psType->psBase->IsClass()) ? psType->psBase : psType;
	for (i = 0; psType && psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
	{
		//An operator-> that returns an object, or a reference to one: its operator->'s.  [over.ref] 1
		pcFunction = CompleteClass(psType->pcClass, pcNode) ? mcBinder.FindArrowOperator(psType->pcClass, NULL, pcNode) : NULL;
		psType = pcFunction ? pcFunction->mpsReturn : NULL;
		psType = (psType && psType->IsReference() && psType->psBase->IsClass()) ? psType->psBase : psType;
	}
	return psType;
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
