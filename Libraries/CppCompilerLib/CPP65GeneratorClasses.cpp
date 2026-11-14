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
#include "CPP65Generator.h"


//The generator's classes: finding and making them, laying them out, their members and what making or destroying one needs.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::NewClass(CCPPSymbol* pcSymbol, int64 lliName)
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
CCPPClass* CCPP65Generator::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
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
// A class without a name (typedef struct { ... } SName;) is known by its definition.
// One in a class template is a different class in each instance (its bindings).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*	pcClass;
	CCPPTemplateArguments*	pcBindings;
	CCPPSyntax*		pcParent;
	size			i;

	for (pcBindings = mpcBindings; pcBindings; pcBindings = pcBindings->mpcOuter)
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
CCPPClass* CCPP65Generator::FindClass(int64 lliName, CCPPScope* pcLookup)
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
CCPPScope* CCPP65Generator::ComponentLookup(CCPPSyntaxNameComponent* pcComponent)
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
int CCPP65Generator::AlignasAlignment(CArraySyntax* papcSpecifiers)
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
			psType = GetTypeIdType((CCPPSyntaxTypeId*)pcAlignment);
			if (!psType || (psType->IsClass() && !CompleteClass(psType->pcClass, pcAlignment)))
			{
				return -1;
			}
			lliAlign = CCPPClassLayout::TypeAlignment(psType);
		}
		else if (!Constant(pcAlignment, &lliAlign))
		{
			Error(pcAlignment, "The alignment must be a constant");
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
// class template (Name<Arguments>), or a class template's own name inside it.  A
// base template's name without arguments is psContext's (or else the current member
// function's class's) base instance of it.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext)
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
		psType = GetSymbolType(pcSymbol, pcNode);
		return (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TypeParameter)
	{
		psBinding = FindBinding(pcSymbol);
		return (psBinding && psBinding->psType && psBinding->psType->IsClass()) ? psBinding->psType->pcClass : NULL;
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
		for (pcBindings = mpcBindings; pcBindings; pcBindings = pcBindings->mpcOuter)
		{
			if (pcBindings->mpcClass && pcBindings->mpcClass->mpcTemplate && (pcBindings->mpcClass->mpcTemplate->mpcPrimary == pcTemplate))
			{
				return pcBindings->mpcClass;
			}
		}
		if (!pcContext && mpcFunction)
		{
			pcContext = mpcFunction->mpcClass;
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
CCPPClass* CCPP65Generator::FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate)
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
CCPPClass* CCPP65Generator::QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
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
CCPPClass* CCPP65Generator::GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	return GetComponentClass(pcName->GetLast(), pcName->mpcSymbol, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Finds a class's base classes without laying it out, so a pointer to it can be
// converted to a pointer to a base.  Offsets are worked out by LayoutClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxBaseSpecifier*	pcBaseSpecifier;
	CCPPClass*				pcBaseClass;
	SCPPBase*					psBase;
	CCPPTemplateArguments*				pcSaved;
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
	pcSaved = mpcBindings;
	mpcBindings = pcClass->mpcBindings;
	bResult = true;
	for (i = 0; i < pcClass->mpcSpecifier->macpBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcClass->mpcSpecifier->macpBases.GetPtr(i);
		if (pcBaseSpecifier->mbVirtual)
		{
			bResult = Error(pcBaseSpecifier, "Virtual inheritance is not supported by the compiler yet");
			continue;
		}
		pcBaseClass = GetNameClass(pcBaseSpecifier->mpcName, pcBaseSpecifier);
		if (!pcBaseClass)
		{
			bResult = Error(pcBaseSpecifier, "This base class is not supported by the compiler yet");
			continue;
		}
		bResult &= ResolveBases(pcBaseClass, pcBaseSpecifier);
		psBase = pcClass->macBases.Add();
		psBase->pcClass = pcBaseClass;
		psBase->iOffset = 0;
	}
	pcClass->mpcBase = (pcClass->macBases.NumElements() > 0) ? pcClass->macBases.Get(0)->pcClass : NULL;
	pcClass->meState = CPPCS_BasesKnown;
	mpcBindings = pcSaved;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class the first time it is needed whole.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
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
// Lays out a class: the bases in order (an empty base takes no space), the vtable
// pointer if no base has one, then the fields.  Members are read first: functions
// are declared (in class definitions are generated when called) and fields are
// collected.  A template instance's members and its members defined outside it are
// worked out with its template arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPScope*						pcEnclosing;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxBaseSpecifier*		pcBaseSpecifier;
	CCPPSyntaxNameComponent*		pcComponent;
	CCPPTemplate*					pcMember;
	CCPPFunction*					pcFunction;
	CCPPTemplateArguments*			pcSaved;
	CCPPTemplateArguments*			pcMemberBindings;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPClassLayout					cLayout;
	size							i;
	int								iClassAlign;
	bool							bResult;

	pcSpecifier = pcClass->mpcSpecifier;
	pcSaved = mpcBindings;
	mpcBindings = pcClass->mpcBindings;
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
	pcEnclosing = mpcDeclaringNamespace;
	if (pcClass->mpcSymbol)
	{
		mpcDeclaringNamespace = pcClass->mpcSymbol->mpcScope->GetNamespace();
	}
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(pcClass, pcSpecifier->macpMembers.GetPtr(i));
	}
	mpcDeclaringNamespace = pcEnclosing;

	//A class template's members defined outside it.  Their template parameters are
	//new names for the class's.
	if (pcClass->mpcTemplate)
	{
		for (i = 0; i < mapcTemplates.NumElements(); i++)
		{
			pcMember = mapcTemplates.GetPtr(i);
			if (!pcMember->mbMember || pcMember->mbMemberTemplate || (pcMember->mpcOwner != pcClass->mpcTemplate))
			{
				continue;
			}
			pcMemberBindings = RenameBindings(pcClass, pcMember->mpcDeclaration);

			pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember->mpcDefinition;
			mpcBindings = pcMemberBindings;
			BeginQuiet();
			pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcClass, false);
			EndQuiet();
			mpcBindings = pcClass->mpcBindings;
			if (pcFunction && (pcDefinition->meBody == CPPFB_Body))
			{
				pcFunction->mpcDefinition = pcDefinition;
				pcFunction->mpcBindings = pcMemberBindings;
				pcFunction->mbInline = true;
			}
		}
	}

	//A destructor is needed to destroy the bases or the fields.
	if (!pcClass->mpcDestructor && pcClass->NeedsDestructor())
	{
		pcFunction = AddFunction(pcClass, pcClass->mlliName, CPPFK_Destructor, &mcTypes.msVoid);
		pcFunction->mbImplicit = true;
		pcClass->mpcDestructor = pcFunction;
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
	mpcBindings = pcSaved;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::f; in a class makes Base's functions named f overloads of the class's
// own.  using Base::Base; inherits Base's constructors (see InheritConstructors).
// Other using declarations (of types) are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing)
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
void CCPP65Generator::InheritConstructors(CCPPClass* pcClass)
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
// Whether pcFunction, a function named lliName, is one of pcClass's overloads: its
// own, or a base's brought in by a using declaration and not hidden by one of its
// own with the same parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsOverloadOf(CCPPClass* pcClass, CCPPFunction* pcFunction, int64 lliName)
{
	CCPPFunction*	pcOwn;
	SCPPUsing*	psUsing;
	size			i;
	size			j;
	bool			bUsed;

	if (pcFunction->mpcClass == pcClass)
	{
		return true;
	}
	if (!pcClass || !pcFunction->mpcClass)
	{
		return false;
	}
	bUsed = false;
	for (i = 0; i < pcClass->macUsings.NumElements(); i++)
	{
		psUsing = pcClass->macUsings.Get(i);
		if ((psUsing->pcBase == pcFunction->mpcClass) && (psUsing->lliName == lliName))
		{
			bUsed = true;
			break;
		}
	}
	if (!bUsed)
	{
		return false;
	}
	for (pcOwn = FirstFunctionOfName(lliName); pcOwn; pcOwn = pcOwn->mpcNextOfName)
	{
		if ((pcOwn->mpcClass != pcClass) || (pcOwn->meKind != pcFunction->meKind) || pcOwn->mpcTemplate || (pcOwn->mapsParameters.NumElements() != pcFunction->mapsParameters.NumElements()))
		{
			continue;
		}
		for (j = 0; j < pcOwn->mapsParameters.NumElements(); j++)
		{
			if (!pcOwn->mapsParameters.GetPtr(j)->Equals(pcFunction->mapsParameters.GetPtr(j)))
			{
				break;
			}
		}
		if (j == pcOwn->mapsParameters.NumElements())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Declares a member of a class being laid out.  Functions defined in the class are
// generated when they are called.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember)
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
	case CPPS_StaticAssert:
	case CPPS_AliasDeclaration:
		return true;

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
		pcFunction->SetDefinition(pcDefinition);
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

			psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				bResult = false;
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
bool CCPP65Generator::ClassError(int iToken, const char* szMessage)
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
void CCPP65Generator::ReportClassErrors(void)
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
bool CCPP65Generator::ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit)
{
	SCPP65Type*		psType;
	SCPPField*		psField;
	int64			lliWidth;
	bool			bResult;

	//A class is laid out once, often first asked about quietly (TypeOf) or by the IR front end, so its own errors are
	//kept to report when the unit ends.
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC))
	{
		return ClassError(pcInit->miFirstToken, "A bit-field can't be static");
	}
	psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
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
		bResult = ClassError(pcInit->mpcBitField->miFirstToken, "A bit-field's width must be a constant of 0 or more");
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
// A static data member of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPP65Generator::FindStatic(CCPPClass* pcClass, int64 lliName)
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
// The conversion function (operator T, its class's own or a base's) that converts
// an object of pcClass to psTo best: by its result's conversion to psTo (see
// ConversionRank; a condition is a conversion to bool).  explicit ones only when
// bExplicit (a cast or a condition).  NULL, quietly, if none converts.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPP65Generator::FindConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcBest;
	int				iRank;
	int				iBest;

	mpcStatistics->Count(CPPCC_FindConversion);
	if (!CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	pcBest = NULL;
	iBest = CPPCR_None;
	for (pcFunction = FirstFunctionOfName(CPP_CONVERSION_NAME); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (pcFunction->mpcTemplate || (pcFunction->mbExplicit && !bExplicit) || !IsDerived(pcClass, pcFunction->mpcClass, NULL))
		{
			continue;
		}
		iRank = ConversionRank(psTo, pcFunction->mpsReturn, NULL, false);
		if (iRank > iBest)
		{
			pcBest = pcFunction;
			iBest = iRank;
		}
	}
	return pcBest;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the pointer on the left of ->: the object's operator->'s result when
// it is an object of a class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	CArraySyntax	apcNone;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return psType;
	}
	if (!CompleteClass(psType->pcClass, pcNode))
	{
		return NULL;
	}
	apcNone.Init();
	pcFunction = FindFunction(psType->pcClass, GetOperatorName(CT_MemberByPointer, false), CPPFK_Function, &apcNone, true, pcNode, IsConstObject(pcObject));
	apcNone.Kill();
	return pcFunction ? pcFunction->mpsReturn : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The identifier of a name component, or -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetComponentName(CCPPSyntaxNameComponent* pcComponent)
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
