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
#include "CPPUnit.h"
#include "CPPConstants.h"


//The generator's classes: finding and making them, laying them out, their members and what making or destroying one needs.


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
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	CCPPClass*	pcClass;
	size			i;

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if ((pcClass->mpcSymbol == pcSymbol) && !pcClass->mpcTemplate)
		{
			return pcClass;
		}
	}
	return NewClass(pcSymbol, pcSymbol->mlliName);
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
	return GetTemplate(pcScope->mpcOwner) ? NULL : GetClass(pcScope->mpcOwner, NULL);
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
// The strictest alignment the alignas(n) and alignas(type) specifiers in
// papcSpecifiers ask for: 0 for none (alignas(0) asks for nothing), -1 after
// an error.  It is kept whatever #pragma pack says, as MSVC does.  [dcl.align]
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::AlignasAlignment(CArraySyntax* papcSpecifiers)
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
			lliAlign = CCPPClassLayout::TypeAlignment(psType);
		}
		else if (!Constant(pcAlignment, &lliAlign))
		{
			ConstantError(pcAlignment, "The alignment must be a constant");
			return -1;
		}
		if ((lliAlign < 0) || (lliAlign > 0x10000) || ((lliAlign & (lliAlign - 1)) != 0))
		{
			Error(pcAlignment, "The alignment must be a power of two");
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
		pcTemplate = GetComponentTemplate(pcComponent);
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
		psBinding = FindBinding(pcSymbol);
		return (psBinding && psBinding->psType && psBinding->psType->IsClass()) ? psBinding->psType->pcClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TemplateTemplateParameter)
	{
		//C<T>, C bound to a class template: its instance.
		psBinding = FindBinding(pcSymbol);
		if (!psBinding || !psBinding->pcTemplate || !pcComponent || !pcComponent->mbTemplateArguments)
		{
			Error(pcNode, "A template template parameter used outside an instantiation");
			return NULL;
		}
		return InstantiateClass(psBinding->pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
	}
	if (pcSymbol->meKind == CPPSK_ClassTemplate)
	{
		pcTemplate = GetTemplate(pcSymbol);
		if (!pcTemplate)
		{
			//A pointer to an instance of a template that is only declared needs no definition.
			pcClass = (pcComponent && pcComponent->mbTemplateArguments) ? UndefinedInstance(pcSymbol, &pcComponent->macpTemplateArguments, pcNode) : NULL;
			if (!pcClass)
			{
				Error(pcNode, "The class template has not been defined");
			}
			return pcClass;
		}
		if (pcComponent && pcComponent->mbTemplateArguments)
		{
			return InstantiateClass(pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
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
		if (!pcContext && OuterFunction())
		{
			pcContext = OuterFunction()->mpcClass;
		}
		pcClass = pcContext ? FindBaseInstance(pcContext, pcTemplate) : NULL;
		if (pcClass)
		{
			return pcClass;
		}
		Error(pcNode, "A class template needs template arguments");
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

	if (pcClass->mpcTemplate && (pcClass->mpcTemplate->mpcPrimary == pcTemplate))
	{
		return pcClass;
	}
	BeginQuiet();
	ResolveBases(pcClass, NULL);
	EndQuiet();
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
// The class a qualified member name (object.Qualifier::f) names: pcClass or one of its bases.  A class
// template's name without arguments is pcClass's base instance of it.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPClass*	pcQualifier;

	pcQualifier = GetComponentClass(pcComponent, NULL, pcNode, pcClass);
	if (!pcQualifier || !IsDerived(pcClass, pcQualifier, NULL))
	{
		Error(pcNode, "The qualifier is not the object's class or one of its bases");
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

	//P<char>::Q<long>: P<char>'s member class template's instance.
	pcSymbol = pcName->mpcSymbol;
	if (pcName->GetQualifier() && pcSymbol && (pcSymbol->meKind == CPPSK_ClassTemplate) && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Class))
	{
		return QualifiedClass(pcName, pcNode);
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
	if (!pcClass->mpcSpecifier && pcClass->mpcSymbol && pcClass->mpcSymbol->mpcDeclaration && (pcClass->mpcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		pcClass->mpcSpecifier = (CCPPSyntaxClassSpecifier*)pcClass->mpcSymbol->mpcDeclaration;
	}
	if (!pcClass->mpcSpecifier)
	{
		return Error(pcNode, "The class has not been defined");
	}

	pcClass->meState = CPPCS_ResolvingBases;
	mcContext.EnterBindings(pcClass->mpcBindings);
	bResult = true;
	for (i = 0; i < pcClass->mpcSpecifier->macpBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcClass->mpcSpecifier->macpBases.GetPtr(i);
		bResult &= pcBaseSpecifier->mbPack ? AddPackBases(pcClass, pcBaseSpecifier) : AddBase(pcClass, pcBaseSpecifier);
	}
	pcClass->mpcBase = (pcClass->macBases.NumElements() > 0) ? pcClass->macBases.Get(0)->pcClass : NULL;
	pcClass->meState = CPPCS_BasesKnown;
	mcContext.LeaveBindings();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a base specifier names is a base of pcClass, its bases found.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AddBase(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier)
{
	CCPPClass*	pcBaseClass;
	SCPPBase*	psBase;
	bool		bResult;

	pcBaseClass = GetNameClass(pcBaseSpecifier->mpcName, pcBaseSpecifier);
	if (!pcBaseClass)
	{
		return Error(pcBaseSpecifier, "This base class is not supported by the compiler yet");
	}
	bResult = ResolveBases(pcBaseClass, pcBaseSpecifier);
	psBase = pcClass->macBases.Add();
	psBase->pcClass = pcBaseClass;
	psBase->iOffset = 0;
	psBase->bVirtual = pcBaseSpecifier->mbVirtual;
	psBase->eAccess = (pcBaseSpecifier->meAccess != CPPA_None) ? pcBaseSpecifier->meAccess : ((pcClass->mpcSpecifier->meKey == CT_class) ? CPPA_Private : CPPA_Public);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// : T... has a base for each of the type pack's classes, in order, the
// pack's parameter bound to one at a time.  [temp.variadic] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AddPackBases(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier)
{
	CArrayCPPBindingPtrs	apsPacks;
	SCPPBinding*			psPack;
	size					uiElements;
	size					j;
	size					k;
	bool					bResult;

	apsPacks.Init();
	FindPacks(pcBaseSpecifier->mpcName, &apsPacks);
	bResult = (apsPacks.NumElements() > 0) || Error(pcBaseSpecifier, "The pack expansion names no parameter pack");
	uiElements = bResult ? apsPacks.GetPtr(0)->PackSize() : 0;
	for (j = 0; bResult && (j < apsPacks.NumElements()); j++)
	{
		bResult = (apsPacks.GetPtr(j)->papsPack && (apsPacks.GetPtr(j)->PackSize() == uiElements)) || Error(pcBaseSpecifier, "This base class is not supported by the compiler yet");
	}
	for (k = 0; bResult && (k < uiElements); k++)
	{
		for (j = 0; j < apsPacks.NumElements(); j++)
		{
			psPack = apsPacks.GetPtr(j);
			psPack->psType = psPack->papsPack->GetPtr(k);
		}
		bResult = AddBase(pcClass, pcBaseSpecifier);
		for (j = 0; j < apsPacks.NumElements(); j++)
		{
			apsPacks.GetPtr(j)->psType = NULL;
		}
	}
	apsPacks.Kill();
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
		return Error(pcNode, "An object of an abstract class can't be made: a pure virtual function of it has no body");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class the first time it is needed whole.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	if (pcClass->IsComplete())
	{
		return true;
	}
	if (pcClass->IsBeingMade())
	{
		return Error(pcNode, "A class cannot contain itself");
	}
	if (!pcClass->mpcSpecifier && pcClass->mpcSymbol && pcClass->mpcSymbol->mpcDeclaration && (pcClass->mpcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		pcClass->mpcSpecifier = (CCPPSyntaxClassSpecifier*)pcClass->mpcSymbol->mpcDeclaration;
	}
	if (!pcClass->mpcSpecifier)
	{
		return Error(pcNode, "The class has not been defined");
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

	mapcMemberSpecialisations.Add(pcDefinition);
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

	for (i = 0; i < mapcMemberSpecialisations.NumElements(); i++)
	{
		SpecialiseMember(pcClass, (CCPPSyntaxFunctionDefinition*)mapcMemberSpecialisations.GetPtr(i));
	}
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
	CCPPSyntaxNameComponent*	pcQualifier;
	CCPPTemplate*				pcOwner;
	CCPPFunction*				pcFunction;
	CChars						szArguments;
	bool						bSame;

	pcQualifier = pcDefinition->mpcDeclarator->GetName()->GetQualifier();
	pcOwner = pcQualifier ? GetComponentTemplate(pcQualifier) : NULL;
	if (!pcOwner || (pcOwner != pcClass->mpcTemplate->mpcPrimary) || !pcQualifier->mbTemplateArguments)
	{
		return;
	}
	szArguments.Init();
	BeginQuiet();
	bSame = BindArguments(pcOwner->mpcDeclaration, &pcQualifier->macpTemplateArguments, NULL, &szArguments, pcDefinition, NULL) && szArguments.Equals(&pcClass->mszArguments);
	EndQuiet();
	szArguments.Kill();
	if (!bSame)
	{
		return;
	}

	mcContext.EnterBindings(pcClass->mpcBindings);
	BeginQuiet();
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcClass, false);
	EndQuiet();
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

	pcMemberBindings = RenameBindings(pcClass, pcMember->mpcDeclaration);
	pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember->mpcDefinition;
	mcContext.EnterBindings(pcMemberBindings);
	BeginQuiet();
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcClass, false);
	EndQuiet();
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
		if (pcClass->mpcTemplate && (pcClass->mpcTemplate == pcMember->mpcOwner) && pcClass->IsComplete())
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
	CCPPClassLayout					cLayout;
	size							i;
	int								iClassAlign;
	bool							bResult;

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
	}

	//Its friends are of its namespace, whatever is being generated when it is completed.
	mcContext.EnterNamespace(pcClass->mpcSymbol ? pcClass->mpcSymbol->mpcScope->GetNamespace() : mcContext.GetNamespace());
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(pcClass, pcSpecifier->macpMembers.GetPtr(i));
	}
	mcContext.LeaveNamespace();

	//A class template's members defined outside it (so far: one defined later is given to the instances already laid
	//out then, DefineOutsideMembers).
	if (pcClass->mpcTemplate)
	{
		for (i = 0; i < mapcTemplates.NumElements(); i++)
		{
			pcMember = mapcTemplates.GetPtr(i);
			if (pcMember->mbMember && !pcMember->mbMemberTemplate && (pcMember->mpcOwner == pcClass->mpcTemplate))
			{
				DefineOutsideMember(pcClass, pcMember);
			}
		}
	}

	//Its members a template<> definition specialises for it.
	if (pcClass->mpcTemplate)
	{
		SpecialiseMembers(pcClass);
	}

	//A destructor is needed to destroy the bases or the fields.
	if (!pcClass->mpcDestructor && pcClass->NeedsDestructor())
	{
		pcFunction = AddFunction(pcClass, pcClass->mlliName, CPPFK_Destructor, &mcTypes.msVoid);
		pcFunction->mbImplicit = true;
		pcClass->mpcDestructor = pcFunction;
	}
	if (pcClass->HasVirtualDestructor())
	{
		//delete of a pointer to a base frees the object as its own class does, which only the object knows: by a
		//virtual function of its class, written for it, that destroys it and frees it.  [expr.delete] 3, 9, 10
		pcFunction = AddFunction(pcClass, CPP_DELETING_NAME, CPPFK_Function, &mcTypes.msVoid);
		pcFunction->mbImplicit = true;
		pcFunction->mbDeleting = true;
		pcFunction->mbVirtual = true;
		pcFunction->mbDefined = true;
		pcClass->mpcDeleting = pcFunction;
	}
	pcClass->mcVtable.LayoutOwn(pcClass);

	//The bases, the vtable pointer, then the fields.
	iClassAlign = AlignasAlignment(&pcSpecifier->macpAttributes);
	if (iClassAlign < 0)
	{
		bResult = false;
	}
	cLayout.Init(pcClass, pcSpecifier->miPack, mpcTarget->GetPointerBytes());
	cLayout.Layout(iClassAlign);
	mcTypes.SetClassBytes(pcClass->mpsType, cLayout.GetBytes());
	cLayout.Kill();
	pcClass->meState = CPPCS_Complete;
	bResult &= CheckOverrides(pcClass);

	if (pcClass->mpcInheritedBase)
	{
		InheritConstructors(pcClass);
	}

	//A class that needs one but declares none gets a constructor written for it when it is called.
	if (!pcClass->mbConstructors && pcClass->NeedsConstructor())
	{
		pcFunction = AddFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, &mcTypes.msVoid);
		pcFunction->mbImplicit = true;
		pcFunction->mbDefined = true;
	}
	mcContext.LeaveBindings();
	return bResult;
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
			bResult &= Error(pcFunction->mpcDeclarator, "The function is declared override but overrides no virtual function of a base class");
		}
		else if (pcOverridden && pcOverridden->mbFinal)
		{
			bResult &= Error(pcFunction->mpcDeclarator, "The function overrides one declared final");
		}
		else if (pcOverridden && !AllowsOnlyWhat(ExceptionsOf(pcFunction), ExceptionsOf(pcOverridden)))
		{
			bResult &= Error(pcFunction->mpcDeclarator, "The function allows exceptions that the virtual function it overrides doesn't");
		}
		else if (pcOverridden && !pcFunction->mbOverride && !pcFunction->mbFinal && (pcFunction->meKind != CPPFK_Destructor) && mpcOptions->mbWarnMissingOverride)
		{
			Warning(pcFunction->mpcDeclarator, "The function overrides a virtual function of a base class but isn't declared override");
		}
	}

	//The destructor the compiler declares allows what the class's parts' destructors do, which may be more.
	pcFunction = pcClass->mpcDestructor;
	pcOverridden = (pcFunction && pcFunction->mbImplicit) ? pcClass->FindOverridden(pcFunction) : NULL;
	if (pcOverridden && !AllowsOnlyWhat(ExceptionsOf(pcFunction), ExceptionsOf(pcOverridden)))
	{
		bResult &= Error(pcClass->mpcSpecifier, "The class's destructor allows exceptions that the virtual destructor it overrides doesn't");
	}
	return bResult;
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
	BeginQuiet();
	pcBase = GetComponentClass(pcName->GetQualifier(), NULL, pcUsing, pcClass);
	EndQuiet();
	if (!pcBase || (pcBase == pcClass))
	{
		return;
	}
	if (pcBase->mlliName == lliName)
	{
		pcClass->mpcInheritedBase = pcBase;
		return;
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
			bSame = true;
			for (k = 0; bSame && (k < pcOwn->mapsParameters.NumElements()); k++)
			{
				bSame = pcOwn->mapsParameters.GetPtr(k)->Equals(pcInherited->mapsParameters.GetPtr(k));
			}
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
		pcFunction->mbImplicit = true;
		pcFunction->mbDefined = true;
		pcFunction->mpcInherited = pcInherited;
		bAdded = true;
	}

	if (bAdded && !pcClass->mbConstructors)
	{
		pcFunction = AddFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, &mcTypes.msVoid);
		pcFunction->mbImplicit = true;
		pcFunction->mbDefined = true;
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
		return MemberTemplate(pcClass, (CCPPSyntaxTemplateDeclaration*)pcMember);

	case CPPS_FunctionDefinition:
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember;
		if ((pcDefinition->meBody == CPPFB_Default) && pcDefinition->mpcDeclarator->GetName() && (GetFunctionName(pcDefinition->mpcDeclarator->GetName()->GetLast()) == GetOperatorName(CT_Assignment, false)))
		{
			//operator = (...) = default is the implicit copy assignment, which = does when the class declares none.
			return true;
		}
		BeginQuiet();
		bFriend = pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_FRIEND);
		pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, bFriend ? NULL : pcClass, false);
		EndQuiet();
		if (!pcFunction && pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL))
		{
			//Leaving out a virtual function would move the vtable's other entries.
			return Error(pcDefinition, "This virtual function's declaration is not supported by the compiler yet");
		}
		if (!pcFunction)
		{
			mapcFailed.Add(pcDefinition->mpcDeclarator);
			return true;
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
					BeginQuiet();
					pcFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
					EndQuiet();
					if (!pcFunction)
					{
						mapcFailed.Add(pcInit->mpcDeclarator);
					}
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
				psField = pcClass->macFields.Add();
				psField->pcSymbol = NULL;
				psField->lliName = -1;
				psField->psType = psType;
				psField->iOffset = 0;
				psField->iAlign = 0;
				psField->pcInitialiser = NULL;
				psField->bMutable = false;
				psField->iBitWidth = -1;
				psField->iBitOffset = 0;
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
			if (pcInit->mpcDeclarator->IsFunction())
			{
				//virtual int f() = 0;
				bPure = pcInit->mpcInitializer && Constant(pcInit->mpcInitializer, &lliZero) && (lliZero == 0);
				BeginQuiet();
				pcFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, pcClass, bPure);
				EndQuiet();
				if (!pcFunction && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
				{
					//Leaving out a virtual function would move the vtable's other entries.
					bResult &= Error(pcInit, "This virtual function's declaration is not supported by the compiler yet");
				}
				else if (!pcFunction)
				{
					mapcFailed.Add(pcInit->mpcDeclarator);
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
				//typedef int F(); F f; declares a member function ([dcl.fct] 10): it is no field.
				bResult &= Error(pcInit, "A member function declared by a typedef of its type is not supported by the compiler yet");
				continue;
			}
			if (pcSpecifiers->muiFlags & CPPDS_STATIC)
			{
				pcVariable = AddVariable(&mapcGlobals, pcInit->mpcSymbol, psType, CPPVS_Global, 0);
				pcVariable->mpcClass = pcClass;
				pcVariable->mpcInClassInitialiser = pcInit->mpcInitializer;
				continue;
			}

			psElement = psType->IsArray() ? psType->psBase : psType;
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
			psField->pcSymbol = pcInit->mpcSymbol;
			psField->lliName = pcInit->mpcSymbol->mlliName;
			psField->psType = psType;
			psField->iOffset = 0;
			psField->iAlign = iAlign;
			psField->pcInitialiser = pcInit->mpcInitializer;
			psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
			psField->iBitWidth = -1;
			psField->iBitOffset = 0;
		}
		return bResult;

	default:
		return Error(pcMember, "This member is not supported by the compiler yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An error in a class's declaration at iToken, found laying it out: kept
// to report when the unit ends (ReportClassErrors), as a class is laid out
// once and that is often quietly.  Returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ClassError(int iToken, const char* szMessage)
{
	SCPPClassError*	psError;

	psError = masClassErrors.Add();
	psError->iToken = iToken;
	psError->szMessage = szMessage;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The errors laying out classes (ClassError) added to the unit's errors.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportClassErrors(void)
{
	size	i;

	for (i = 0; i < masClassErrors.NumElements(); i++)
	{
		ErrorAt(masClassErrors.Get(i)->iToken, masClassErrors.Get(i)->szMessage);
	}
	masClassErrors.ReInit();
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
		return ClassError(pcInit->miFirstToken, "A bit-field can't be static");
	}
	psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
	if (!psType)
	{
		return false;
	}
	if (!psType->IsInteger())
	{
		return ClassError(pcInit->miFirstToken, "A bit-field must be an integer, bool or enum");
	}
	bResult = true;
	if (!Constant(pcInit->mpcBitField, &lliWidth) || (lliWidth < 0))
	{
		szMessage.Init();
		ConstantMessage("A bit-field's width must be a constant of 0 or more", &szMessage);
		bResult = ClassError(pcInit->mpcBitField->miFirstToken, szMessage.Text());
		szMessage.Kill();
		lliWidth = 1;
	}
	if (lliWidth > (int64)psType->iBytes * 8)
	{
		bResult = ClassError(pcInit->mpcBitField->miFirstToken, "A bit-field can't be wider than its type");
		lliWidth = psType->iBytes * 8;
	}
	if ((lliWidth == 0) && pcInit->mpcSymbol)
	{
		bResult = ClassError(pcInit->mpcBitField->miFirstToken, "A named bit-field can't have a width of 0");
		lliWidth = 1;
	}
	psField = pcClass->macFields.Add();
	psField->pcSymbol = pcInit->mpcSymbol;
	psField->lliName = pcInit->mpcSymbol ? pcInit->mpcSymbol->mlliName : -1;
	psField->psType = psType;
	psField->iOffset = 0;
	psField->iAlign = 0;
	psField->pcInitialiser = pcInit->mpcInitializer;
	psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
	psField->iBitWidth = (int)lliWidth;
	psField->iBitOffset = 0;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A const integer static data member initialised in its class (static const int
// Max = 100;): where its value is used it is a constant, and needs no definition.
// Its address, or a reference to it, needs one.  [class.static.data] 4
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
	return psType && psType->bConst && psType->IsInteger();
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

	psType = TypeOf(pcObject);
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
	for (i = 0; psType && psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
	{
		//An operator-> that returns an object: its operator->'s.  [over.ref] 1
		pcFunction = CompleteClass(psType->pcClass, pcNode) ? mcBinder.FindArrowOperator(psType->pcClass, NULL, pcNode) : NULL;
		psType = pcFunction ? pcFunction->mpsReturn : NULL;
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
