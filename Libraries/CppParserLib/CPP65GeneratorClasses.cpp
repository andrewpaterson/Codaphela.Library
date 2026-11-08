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
#include "CPP65Generator.h"


//The generator's classes: finding and making them, laying them out, their members and what making or destroying one needs.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::NewClass(CCPPSymbol* pcSymbol, int64 lliName)
{
	SCPP65Class*	psClass;

	psClass = NewMalloc<SCPP65Class>();
	psClass->pcSymbol = pcSymbol;
	psClass->lliName = lliName;
	psClass->psType = mcTypes.Class(psClass);
	psClass->psBase = NULL;
	psClass->macBases.Init();
	psClass->macUsings.Init();
	psClass->macFields.Init();
	psClass->mapsMembers.Init();
	psClass->mapsVirtuals.Init();
	psClass->iVtableOffset = -1;
	psClass->macVtables.Init();
	psClass->bComplete = false;
	psClass->bLayingOut = false;
	psClass->bBases = false;
	psClass->bEmpty = false;
	psClass->iAlign = 1;
	psClass->bUnion = false;
	psClass->bVtable = false;
	psClass->bTypeInfo = false;
	psClass->bConstructors = false;
	psClass->psInheritedBase = NULL;
	psClass->psDestructor = NULL;
	psClass->pcSpecifier = NULL;
	psClass->psTemplate = NULL;
	psClass->psBindings = NULL;
	psClass->mszArguments.Init();
	mapsClasses.Add(psClass);
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class (not a template instance) a symbol names.  A class used before it is
// defined, or only through pointers, is laid out when it is first needed whole.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	SCPP65Class*	psClass;
	size			i;

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if ((psClass->pcSymbol == pcSymbol) && !psClass->psTemplate)
		{
			return psClass;
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
SCPP65Class* CCPP65Generator::GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	SCPP65Class*	psClass;
	SCPP65Bindings*	psBindings;
	CCPPSyntax*		pcParent;
	size			i;

	for (psBindings = mpsBindings; psBindings; psBindings = psBindings->psOuter)
	{
		pcParent = pcSpecifier->mpcParent;
		while (pcParent && (!psBindings->psClass || (pcParent != psBindings->psClass->pcSpecifier)))
		{
			pcParent = pcParent->mpcParent;
		}
		if (pcParent)
		{
			break;
		}
	}

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (!psClass->pcSymbol && (psClass->pcSpecifier == pcSpecifier) && !psClass->psTemplate && (psClass->psBindings == psBindings))
		{
			return psClass;
		}
	}
	psClass = NewClass(NULL, -1);
	psClass->pcSpecifier = pcSpecifier;
	psClass->psBindings = psBindings;
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A class (not a template instance) by name, for a qualifier the parser
// couldn't resolve: one visible from pcLookup, else any.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindClass(int64 lliName, CCPPScope* pcLookup)
{
	SCPP65Class*	psClass;
	SCPP65Class*	psFound;
	size			i;

	psFound = NULL;
	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if ((psClass->lliName == lliName) && !psClass->psTemplate)
		{
			if (psClass->pcSymbol && psClass->pcSymbol->mpcScope->Encloses(pcLookup))
			{
				return psClass;
			}
			psFound = psFound ? psFound : psClass;
		}
	}
	return psFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psClass is an instance (of any arguments) of the class template
// pcSymbol names, and not one of the same name in another scope.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsInstanceNamed(SCPP65Class* psClass, CCPPSymbol* pcSymbol)
{
	return !psClass->psTemplate && (psClass->lliName == pcSymbol->mlliName) && (!psClass->pcSymbol || (psClass->pcSymbol->mpcScope == pcSymbol->mpcScope));
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
// An alignment limited by #pragma pack(iPack) (0 for none).
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::PackAlignment(int iAlign, int iPack)
{
	return ((iPack > 0) && (iAlign > iPack)) ? iPack : iAlign;
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
			if (!psType || (psType->IsClass() && !CompleteClass(psType->psClass, pcAlignment)))
			{
				return -1;
			}
			lliAlign = TypeAlignment(psType);
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
SCPP65Class* CCPP65Generator::GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, SCPP65Class* psContext)
{
	SCPP65Template*	psTemplate;
	SCPP65Bindings*	psBindings;
	SCPP65Binding*	psBinding;
	SCPP65Class*	psClass;
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
		psTemplate = GetComponentTemplate(pcComponent);
		if (psTemplate)
		{
			pcSymbol = psTemplate->pcSymbol;
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
		return (psType && psType->IsClass()) ? psType->psClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_TypeParameter)
	{
		psBinding = FindBinding(pcSymbol);
		return (psBinding && psBinding->psType && psBinding->psType->IsClass()) ? psBinding->psType->psClass : NULL;
	}
	if (pcSymbol->meKind == CPPSK_ClassTemplate)
	{
		psTemplate = GetTemplate(pcSymbol);
		if (!psTemplate)
		{
			//A pointer to an instance of a template that is only declared needs no definition.
			psClass = (pcComponent && pcComponent->mbTemplateArguments) ? UndefinedInstance(pcSymbol, &pcComponent->macpTemplateArguments, pcNode) : NULL;
			if (!psClass)
			{
				Error(pcNode, "The class template has not been defined");
			}
			return psClass;
		}
		if (pcComponent && pcComponent->mbTemplateArguments)
		{
			return InstantiateClass(psTemplate, &pcComponent->macpTemplateArguments, pcNode);
		}

		//The template's own name inside it is the instance being made, and a base's name
		//in a class derived from an instance is that instance.
		for (psBindings = mpsBindings; psBindings; psBindings = psBindings->psOuter)
		{
			if (psBindings->psClass && psBindings->psClass->psTemplate && (psBindings->psClass->psTemplate->psPrimary == psTemplate))
			{
				return psBindings->psClass;
			}
		}
		if (!psContext && mpsFunction)
		{
			psContext = mpsFunction->psClass;
		}
		psClass = psContext ? FindBaseInstance(psContext, psTemplate) : NULL;
		if (psClass)
		{
			return psClass;
		}
		Error(pcNode, "A class template needs template arguments");
		return NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A type's alignment in a class, as MSVC lays classes out (BaseLib reads and writes them as raw bytes): a
// scalar is aligned to its size (8 for long long and double), a pointer or reference to 4, an array as its
// element and a class to its largest member's alignment.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::TypeAlignment(SCPP65Type* psType)
{
	if (psType->IsClass())
	{
		return (psType->psClass->iAlign < 1) ? 1 : psType->psClass->iAlign;
	}
	if (psType->IsArray())
	{
		return TypeAlignment(psType->psBase);
	}
	if (psType->IsReference() || psType->IsPointer() || psType->IsMemberPointer())
	{
		return 4;
	}
	if (psType->iBytes >= 8)
	{
		return 8;
	}
	if (psType->iBytes >= 4)
	{
		return 4;
	}
	return (psType->iBytes < 1) ? 1 : psType->iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a class template that is psClass or one of its bases, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindBaseInstance(SCPP65Class* psClass, SCPP65Template* psTemplate)
{
	SCPP65Class*	psBase;
	size			i;

	if (psClass->psTemplate && (psClass->psTemplate->psPrimary == psTemplate))
	{
		return psClass;
	}
	BeginQuiet();
	ResolveBases(psClass, NULL);
	EndQuiet();
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = FindBaseInstance(psClass->macBases.Get(i)->psClass, psTemplate);
		if (psBase)
		{
			return psBase;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified member name (object.Qualifier::f) names: psClass or one of its bases.  A class
// template's name without arguments is psClass's base instance of it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::QualifierClass(SCPP65Class* psClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	SCPP65Class*	psQualifier;

	psQualifier = GetComponentClass(pcComponent, NULL, pcNode, psClass);
	if (!psQualifier || !IsDerived(psClass, psQualifier, NULL))
	{
		Error(pcNode, "The qualifier is not the object's class or one of its bases");
		return NULL;
	}
	return psQualifier;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a (possibly qualified) name's last component names.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	return GetComponentClass(pcName->GetLast(), pcName->mpcSymbol, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Finds a class's base classes without laying it out, so a pointer to it can be
// converted to a pointer to a base.  Offsets are worked out by LayoutClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ResolveBases(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxBaseSpecifier*	pcBaseSpecifier;
	SCPP65Class*				psBaseClass;
	SCPP65Base*					psBase;
	SCPP65Bindings*				psSaved;
	size						i;
	bool						bResult;

	if (psClass->bBases)
	{
		return true;
	}
	if (!psClass->pcSpecifier && psClass->pcSymbol && psClass->pcSymbol->mpcDeclaration && (psClass->pcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		psClass->pcSpecifier = (CCPPSyntaxClassSpecifier*)psClass->pcSymbol->mpcDeclaration;
	}
	if (!psClass->pcSpecifier)
	{
		return Error(pcNode, "The class has not been defined");
	}

	psClass->bBases = true;
	psSaved = mpsBindings;
	mpsBindings = psClass->psBindings;
	bResult = true;
	for (i = 0; i < psClass->pcSpecifier->macpBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)psClass->pcSpecifier->macpBases.GetPtr(i);
		if (pcBaseSpecifier->mbVirtual)
		{
			bResult = Error(pcBaseSpecifier, "Virtual inheritance is not supported by the compiler yet");
			continue;
		}
		psBaseClass = GetNameClass(pcBaseSpecifier->mpcName, pcBaseSpecifier);
		if (!psBaseClass)
		{
			bResult = Error(pcBaseSpecifier, "This base class is not supported by the compiler yet");
			continue;
		}
		bResult &= ResolveBases(psBaseClass, pcBaseSpecifier);
		psBase = psClass->macBases.Add();
		psBase->psClass = psBaseClass;
		psBase->iOffset = 0;
	}
	psClass->psBase = (psClass->macBases.NumElements() > 0) ? psClass->macBases.Get(0)->psClass : NULL;
	mpsBindings = psSaved;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class the first time it is needed whole.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CompleteClass(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	if (psClass->bComplete)
	{
		return true;
	}
	if (psClass->bLayingOut)
	{
		return Error(pcNode, "A class cannot contain itself");
	}
	if (!psClass->pcSpecifier && psClass->pcSymbol && psClass->pcSymbol->mpcDeclaration && (psClass->pcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier))
	{
		psClass->pcSpecifier = (CCPPSyntaxClassSpecifier*)psClass->pcSymbol->mpcDeclaration;
	}
	if (!psClass->pcSpecifier)
	{
		return Error(pcNode, "The class has not been defined");
	}
	return LayoutClass(psClass, pcNode);
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
bool CCPP65Generator::LayoutClass(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	CCPPScope*						pcEnclosing;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxBaseSpecifier*		pcBaseSpecifier;
	CCPPSyntaxNameComponent*		pcComponent;
	SCPP65Template*					psMember;
	SCPP65Base*						psBase;
	SCPP65Field*					psField;
	SCPP65Function*					psFunction;
	SCPP65Bindings*					psSaved;
	SCPP65Bindings*					psMemberBindings;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	size							i;
	int								iBytes;
	int								iAlign;
	int								iPack;
	int								iFieldAlign;
	int								iClassAlign;
	bool							bResult;

	pcSpecifier = psClass->pcSpecifier;
	psClass->bLayingOut = true;
	psSaved = mpsBindings;
	mpsBindings = psClass->psBindings;
	bResult = true;

	psClass->bUnion = pcSpecifier->meKey == CT_union;
	pcComponent = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
	if (pcComponent && !psClass->psTemplate)
	{
		psClass->lliName = GetComponentName(pcComponent);
	}

	bResult &= ResolveBases(psClass, pcSpecifier);
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcSpecifier->macpBases.GetPtr(i);
		bResult &= CompleteClass(psClass->macBases.Get(i)->psClass, pcBaseSpecifier);
	}

	//Its friends are of its namespace, whatever is being generated when it is completed.
	pcEnclosing = mpcDeclaringNamespace;
	if (psClass->pcSymbol)
	{
		mpcDeclaringNamespace = NamespaceOf(psClass->pcSymbol->mpcScope);
	}
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(psClass, pcSpecifier->macpMembers.GetPtr(i));
	}
	mpcDeclaringNamespace = pcEnclosing;

	//A class template's members defined outside it.  Their template parameters are
	//new names for the class's.
	if (psClass->psTemplate)
	{
		for (i = 0; i < mapsTemplates.NumElements(); i++)
		{
			psMember = mapsTemplates.GetPtr(i);
			if (!psMember->bMember || psMember->bMemberTemplate || (psMember->psOwner != psClass->psTemplate))
			{
				continue;
			}
			psMemberBindings = RenameBindings(psClass, psMember->pcDeclaration);

			pcDefinition = (CCPPSyntaxFunctionDefinition*)psMember->pcDefinition;
			mpsBindings = psMemberBindings;
			BeginQuiet();
			psFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, psClass, false);
			EndQuiet();
			mpsBindings = psClass->psBindings;
			if (psFunction && (pcDefinition->meBody == CPPFB_Body))
			{
				psFunction->pcDefinition = pcDefinition;
				psFunction->psBindings = psMemberBindings;
				psFunction->bInline = true;
			}
		}
	}

	//A destructor is needed to destroy the bases or the fields.
	if (!psClass->psDestructor && NeedsDestructor(psClass))
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Destructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psClass->psDestructor = psFunction;
	}
	bResult &= LayoutVirtuals(psClass, pcSpecifier);

	//The bases, the vtable pointer, then the fields.  #pragma pack(n) aligns each to at most n bytes.
	iPack = pcSpecifier->miPack;
	iBytes = 0;
	iAlign = 1;
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		if (psBase->psClass->bEmpty)
		{
			psBase->iOffset = 0;
			continue;
		}
		iFieldAlign = PackAlignment(psBase->psClass->iAlign, iPack);
		iBytes = (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psBase->iOffset = iBytes;
		iBytes += psBase->psClass->psType->iBytes;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
	}
	if (psClass->psBase && (psClass->psBase->iVtableOffset != -1) && (psClass->macBases.Get(0)->iOffset == 0))
	{
		psClass->iVtableOffset = psClass->psBase->iVtableOffset;
	}
	else if (psClass->mapsVirtuals.NumElements() > 0)
	{
		iFieldAlign = PackAlignment(mpsTarget->iPointerBytes, iPack);
		iBytes = (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psClass->iVtableOffset = iBytes;
		iBytes += mpsTarget->iPointerBytes;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
	}
	LayoutSecondaryVtables(psClass);
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		iFieldAlign = PackAlignment(TypeAlignment(psField->psType), iPack);
		iFieldAlign = (psField->iAlign > iFieldAlign) ? psField->iAlign : iFieldAlign;
		iBytes = psClass->bUnion ? 0 : (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psField->iOffset = iBytes;
		iBytes += psField->psType->iBytes;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
		if (psClass->bUnion && (iBytes > psClass->psType->iBytes))
		{
			psClass->psType->iBytes = iBytes;
		}
	}
	if (psClass->bUnion)
	{
		iBytes = psClass->psType->iBytes;
	}
	iClassAlign = AlignasAlignment(&pcSpecifier->macpAttributes);
	if (iClassAlign < 0)
	{
		bResult = false;
	}
	iAlign = (iClassAlign > iAlign) ? iClassAlign : iAlign;
	iBytes = (iBytes + iAlign - 1) & ~(iAlign - 1);

	psClass->bEmpty = (psClass->macFields.NumElements() == 0) && (psClass->iVtableOffset == -1);
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psClass->bEmpty &= psClass->macBases.Get(i)->psClass->bEmpty;
	}
	psClass->iAlign = iAlign;
	mcTypes.SetClassBytes(psClass->psType, (iBytes > 0) ? iBytes : 1);
	psClass->bComplete = true;
	psClass->bLayingOut = false;

	if (psClass->psInheritedBase)
	{
		InheritConstructors(psClass);
	}

	//A class that needs one but declares none gets a constructor written for it when it is called.
	if (!psClass->bConstructors && NeedsConstructor(psClass))
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
	}
	mpsBindings = psSaved;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::f; in a class makes Base's functions named f overloads of the class's
// own.  using Base::Base; inherits Base's constructors (see InheritConstructors).
// Other using declarations (of types) are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ClassUsing(SCPP65Class* psClass, CCPPSyntaxUsingDeclaration* pcUsing)
{
	CCPPSyntaxName*	pcName;
	SCPP65Class*	psBase;
	SCPP65Using*	psUsing;
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
	psBase = GetComponentClass(pcName->GetQualifier(), NULL, pcUsing, psClass);
	EndQuiet();
	if (!psBase || (psBase == psClass))
	{
		return;
	}
	if (psBase->lliName == lliName)
	{
		psClass->psInheritedBase = psBase;
		return;
	}
	psUsing = psClass->macUsings.Add();
	psUsing->psBase = psBase;
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
void CCPP65Generator::InheritConstructors(SCPP65Class* psClass)
{
	SCPP65Class*	psBase;
	SCPP65Function*	psInherited;
	SCPP65Function*	psOwn;
	SCPP65Function*	psFunction;
	SCPP65Type*		psParameter;
	size			i;
	size			j;
	size			k;
	bool			bSame;
	bool			bAdded;

	psBase = psClass->psInheritedBase;
	if (!CompleteClass(psBase, NULL))
	{
		return;
	}
	bAdded = false;
	for (i = 0; i < psBase->mapsMembers.NumElements(); i++)
	{
		psInherited = psBase->mapsMembers.GetPtr(i);
		if ((psInherited->eKind != CPP65FK_Constructor) || psInherited->psTemplate || psInherited->bImplicit || (psInherited->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = psInherited->mapsParameters.GetPtr(0);
		if ((psInherited->mapsParameters.NumElements() == 1) && psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->psClass == psBase))
		{
			continue;
		}

		bSame = false;
		for (j = 0; !bSame && (j < psClass->mapsMembers.NumElements()); j++)
		{
			psOwn = psClass->mapsMembers.GetPtr(j);
			if ((psOwn->eKind != CPP65FK_Constructor) || (psOwn->mapsParameters.NumElements() != psInherited->mapsParameters.NumElements()))
			{
				continue;
			}
			bSame = true;
			for (k = 0; bSame && (k < psOwn->mapsParameters.NumElements()); k++)
			{
				bSame = psOwn->mapsParameters.GetPtr(k)->Equals(psInherited->mapsParameters.GetPtr(k));
			}
		}
		if (bSame)
		{
			continue;
		}

		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		for (k = 0; k < psInherited->mapsParameters.NumElements(); k++)
		{
			psFunction->mapsParameters.Add(psInherited->mapsParameters.GetPtr(k));
			psFunction->mapcDefaults.Add(psInherited->mapcDefaults.GetPtr(k));
		}
		psFunction->bExplicit = psInherited->bExplicit;
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		psFunction->psInherited = psInherited;
		bAdded = true;
	}

	if (bAdded && !psClass->bConstructors)
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		psClass->bConstructors = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psFunction, a function named lliName, is one of psClass's overloads: its
// own, or a base's brought in by a using declaration and not hidden by one of its
// own with the same parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsOverloadOf(SCPP65Class* psClass, SCPP65Function* psFunction, int64 lliName)
{
	SCPP65Function*	psOwn;
	SCPP65Using*	psUsing;
	size			i;
	size			j;
	bool			bUsed;

	if (psFunction->psClass == psClass)
	{
		return true;
	}
	if (!psClass || !psFunction->psClass)
	{
		return false;
	}
	bUsed = false;
	for (i = 0; i < psClass->macUsings.NumElements(); i++)
	{
		psUsing = psClass->macUsings.Get(i);
		if ((psUsing->psBase == psFunction->psClass) && (psUsing->lliName == lliName))
		{
			bUsed = true;
			break;
		}
	}
	if (!bUsed)
	{
		return false;
	}
	for (psOwn = FirstFunctionOfName(lliName); psOwn; psOwn = psOwn->psNextOfName)
	{
		if ((psOwn->psClass != psClass) || (psOwn->eKind != psFunction->eKind) || psOwn->psTemplate || (psOwn->mapsParameters.NumElements() != psFunction->mapsParameters.NumElements()))
		{
			continue;
		}
		for (j = 0; j < psOwn->mapsParameters.NumElements(); j++)
		{
			if (!psOwn->mapsParameters.GetPtr(j)->Equals(psFunction->mapsParameters.GetPtr(j)))
			{
				break;
			}
		}
		if (j == psOwn->mapsParameters.NumElements())
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
bool CCPP65Generator::ClassMember(SCPP65Class* psClass, CCPPSyntax* pcMember)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxClassSpecifier*		pcClassSpecifier;
	SCPP65Field*					psField;
	SCPP65Variable*					psVariable;
	SCPP65Function*					psFunction;
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
		ClassUsing(psClass, (CCPPSyntaxUsingDeclaration*)pcMember);
		return true;

	case CPPS_TemplateDeclaration:
		return MemberTemplate(psClass, (CCPPSyntaxTemplateDeclaration*)pcMember);

	case CPPS_FunctionDefinition:
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcMember;
		if ((pcFunction->meBody == CPPFB_Default) && pcFunction->mpcDeclarator->GetName() && (GetFunctionName(pcFunction->mpcDeclarator->GetName()->GetLast()) == GetOperatorName(CT_Assignment, false)))
		{
			//operator = (...) = default is the implicit copy assignment, which = does when the class declares none.
			return true;
		}
		BeginQuiet();
		bFriend = pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & CPPDS_FRIEND);
		psFunction = DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, true, bFriend ? NULL : psClass, false);
		EndQuiet();
		if (!psFunction && pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL))
		{
			//Leaving out a virtual function would move the vtable's other entries.
			return Error(pcFunction, "This virtual function's declaration is not supported by the compiler yet");
		}
		if (!psFunction)
		{
			mapcFailed.Add(pcFunction->mpcDeclarator);
			return true;
		}
		SetDefinition(psFunction, pcFunction);
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
					psFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
					EndQuiet();
					if (!psFunction)
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
				GetClass(pcClassSpecifier->mpcSymbol, pcMember)->pcSpecifier = pcClassSpecifier;
			}
			else if (pcDeclaration->macpDeclarators.NumElements() == 0)
			{
				//union { ... }; is an unnamed field whose members are the class's (FindField looks in it).
				psType = GetUnnamedClass(pcClassSpecifier)->psType;
				if (!CompleteClass(psType->psClass, pcMember))
				{
					return false;
				}
				psField = psClass->macFields.Add();
				psField->pcSymbol = NULL;
				psField->lliName = -1;
				psField->psType = psType;
				psField->iOffset = 0;
				psField->iAlign = 0;
				psField->pcInitialiser = NULL;
				psField->bMutable = false;
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
				psFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, psClass, bPure);
				EndQuiet();
				if (!psFunction && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
				{
					//Leaving out a virtual function would move the vtable's other entries.
					bResult &= Error(pcInit, "This virtual function's declaration is not supported by the compiler yet");
				}
				else if (!psFunction)
				{
					mapcFailed.Add(pcInit->mpcDeclarator);
				}
				else if ((pcInit->meInitializer == CPPI_Default) && (psFunction->eKind != CPP65FK_Function))
				{
					psFunction->bImplicit = true;
				}
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
				psVariable = AddVariable(&mapsGlobals, pcInit->mpcSymbol, psType, CPP65S_Global, 0);
				psVariable->psClass = psClass;
				psVariable->pcInClassInitialiser = pcInit->mpcInitializer;
				continue;
			}

			psElement = psType->IsArray() ? psType->psBase : psType;
			if (psElement->IsClass() && !CompleteClass(psElement->psClass, pcInit))
			{
				bResult = false;
				continue;
			}
			if (psType->IsArray() && psElement->IsClass())
			{
				psType = mcTypes.Array(psElement, psType->iCount);
			}
			psField = psClass->macFields.Add();
			psField->pcSymbol = pcInit->mpcSymbol;
			psField->lliName = pcInit->mpcSymbol->mlliName;
			psField->psType = psType;
			psField->iOffset = 0;
			psField->iAlign = iAlign;
			psField->pcInitialiser = pcInit->mpcInitializer;
			psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
		}
		return bResult;

	default:
		return Error(pcMember, "This member is not supported by the compiler yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A field of the class, of an anonymous union or struct in it, or of its bases.
// *piOffset is set to its offset in psClass.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindField(SCPP65Class* psClass, int64 lliName, int* piOffset)
{
	SCPP65Field*	psField;
	SCPP65Field*	psMember;
	SCPP65Base*		psBase;
	size			i;
	int				iOffset;

	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->lliName == lliName)
		{
			*piOffset = psField->iOffset;
			return psField;
		}
		if (!psField->pcSymbol && psField->psType->IsClass())
		{
			psMember = FindField(psField->psType->psClass, lliName, &iOffset);
			if (psMember)
			{
				*piOffset = psField->iOffset + iOffset;
				return psMember;
			}
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		psField = FindField(psBase->psClass, lliName, &iOffset);
		if (psField)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol, int* piOffset)
{
	SCPP65Field*	psField;
	SCPP65Field*	psMember;
	SCPP65Base*		psBase;
	size			i;
	int				iOffset;

	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->pcSymbol == pcSymbol)
		{
			*piOffset = psField->iOffset;
			return psField;
		}
		if (!psField->pcSymbol && psField->psType->IsClass())
		{
			psMember = FindFieldSymbol(psField->psType->psClass, pcSymbol, &iOffset);
			if (psMember)
			{
				*piOffset = psField->iOffset + iOffset;
				return psMember;
			}
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		psField = FindFieldSymbol(psBase->psClass, pcSymbol, &iOffset);
		if (psField)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}
	}

	//Names in templates are not always resolved to the instance's fields.
	if (psClass->psTemplate)
	{
		return FindField(psClass, pcSymbol->mlliName, piOffset);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A static data member of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::FindStatic(SCPP65Class* psClass, int64 lliName)
{
	SCPP65Variable*	psVariable;
	size			i;

	for (i = 0; i < mapsGlobals.NumElements(); i++)
	{
		psVariable = mapsGlobals.GetPtr(i);
		if ((psVariable->psClass == psClass) && (psVariable->pcSymbol->mlliName == lliName))
		{
			return psVariable;
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psVariable = FindStatic(psClass->macBases.Get(i)->psClass, lliName);
		if (psVariable)
		{
			return psVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether making an object of the class runs code: a constructor, a vtable pointer
// to set, a base or a field that needs it, or a default member initialiser.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::NeedsConstructor(SCPP65Class* psClass)
{
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;

	if (psClass->bConstructors || IsPolymorphic(psClass))
	{
		return true;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (NeedsConstructor(psClass->macBases.Get(i)->psClass))
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->pcInitialiser)
		{
			return true;
		}
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && NeedsConstructor(psType->psClass))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether destroying an object of the class runs code.  While the class is being
// laid out this asks whether its bases or fields need destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::NeedsDestructor(SCPP65Class* psClass)
{
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;

	if (psClass->psDestructor)
	{
		return true;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (psClass->macBases.Get(i)->psClass->psDestructor)
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && psType->psClass->psDestructor)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Records a function's definition.  Functions defined in classes, inline,
// constexpr and template functions are generated when they are first called.
// Bodies that were skipped (in system headers) are not definitions; = default
// makes the generator write a constructor or destructor.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetDefinition(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	if ((pcDefinition->meBody == CPPFB_Default) && (psFunction->eKind != CPP65FK_Function))
	{
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		return;
	}
	if (pcDefinition->meBody != CPPFB_Body)
	{
		return;
	}
	psFunction->pcDefinition = pcDefinition;
	psFunction->bDefined = true;
	psFunction->bInline = true;
	if (!psFunction->psBindings && psFunction->psClass)
	{
		psFunction->psBindings = psFunction->psClass->psBindings;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function (operator T, its class's own or a base's) that converts
// an object of psClass to psTo best: by its result's conversion to psTo (see
// ConversionRank; a condition is a conversion to bool).  explicit ones only when
// bExplicit (a cast or a condition).  NULL, quietly, if none converts.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindConversion(SCPP65Class* psClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode)
{
	SCPP65Function*	psFunction;
	SCPP65Function*	psBest;
	int				iRank;
	int				iBest;

	if (!CompleteClass(psClass, pcNode))
	{
		return NULL;
	}
	psBest = NULL;
	iBest = CPP65CR_None;
	for (psFunction = FirstFunctionOfName(CPP65_CONVERSION_NAME); psFunction; psFunction = psFunction->psNextOfName)
	{
		if (psFunction->psTemplate || (psFunction->bExplicit && !bExplicit) || !IsDerived(psClass, psFunction->psClass, NULL))
		{
			continue;
		}
		iRank = ConversionRank(psTo, psFunction->psReturn, NULL, false);
		if (iRank > iBest)
		{
			psBest = psFunction;
			iBest = iRank;
		}
	}
	return psBest;
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
	SCPP65Function*	psFunction;
	CArraySyntax	apcNone;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return psType;
	}
	if (!CompleteClass(psType->psClass, pcNode))
	{
		return NULL;
	}
	apcNone.Init();
	psFunction = FindFunction(psType->psClass, GetOperatorName(CT_MemberByPointer, false), CPP65FK_Function, &apcNone, true, pcNode, IsConstObject(pcObject));
	apcNone.Kill();
	return psFunction ? psFunction->psReturn : NULL;
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
