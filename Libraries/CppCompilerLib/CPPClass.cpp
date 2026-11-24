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
#include "CPPScope.h"
#include "CPPTemplate.h"
#include "CPPClass.h"


//A class the compiler knows: what it is asked about itself.


//////////////////////////////////////////////////////////////////////////
//
// The class as it is when it has only been named: nothing is known of what is in it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClass::Init(CCPPSymbol* pcSymbol, int64 lliName)
{
	mpcSymbol = pcSymbol;
	mlliName = lliName;
	mpsType = NULL;
	mpcBase = NULL;
	macBases.Init();
	macVirtualBases.Init();
	masBasePaths.Init();
	mbBasePathsKept = false;
	miOwnBytes = 0;
	macUsings.Init();
	macFields.Init();
	mapcMembers.Init();
	mcVtable.Init();
	meState = CPPCS_Declared;
	mbEmpty = false;
	miAlign = 1;
	miAlignas = 0;
	mbUnion = false;
	mbTypeInfo = false;
	mbConstructors = false;
	mpcInheritedBase = NULL;
	mpcDestructor = NULL;
	mpcSpecifier = NULL;
	mpcTemplate = NULL;
	mpcEnclosing = NULL;
	mpcBindings = NULL;
	mszArguments.Init();
	mbExternInstance = false;
	mbPlaceholder = false;
	mpsLambda = NULL;
	mpcDeleting = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClass::Kill(void)
{
	mszArguments.Kill();
	mcVtable.Kill();
	mapcMembers.Kill();
	macFields.Kill();
	macUsings.Kill();
	masBasePaths.Kill();
	macVirtualBases.Kill();
	macBases.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether this class is an instance (of any arguments) of the class template
// pcSymbol names, and not one of the same name in another scope.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsInstanceNamed(CCPPSymbol* pcSymbol)
{
	return !mpcTemplate && (mlliName == pcSymbol->mlliName) && (!mpcSymbol || (mpcSymbol->mpcScope == pcSymbol->mpcScope));
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual base pcBase (direct or not) and where a complete object puts
// it, or NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
SCPPVirtualBase* CCPPClass::GetVirtualBase(CCPPClass* pcBase)
{
	size	i;

	for (i = 0; i < macVirtualBases.NumElements(); i++)
	{
		if (macVirtualBases.Get(i)->pcClass == pcBase)
		{
			return macVirtualBases.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Which of the class's virtual bases pcBase is (its offset's place in the
// class's vtable part, CPP65_VTABLE_VIRTUAL_OFFSET), or -1.
//
//////////////////////////////////////////////////////////////////////////
int CCPPClass::GetVirtualBaseIndex(CCPPClass* pcBase)
{
	size	i;

	for (i = 0; i < macVirtualBases.NumElements(); i++)
	{
		if (macVirtualBases.Get(i)->pcClass == pcBase)
		{
			return (int)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The class and its bases, direct or not, by every way to each, as its
// bases are written: the class itself first, then each base followed by its
// own (depth first, as a complete object's subobjects are made).  What a
// question about the class's bases reads.  The bases as they are resolved:
// the unit resolves them first (CCPPUnit::BasePaths).  Worked out again for
// a class that isn't complete, kept once it is.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPPBasePaths* CCPPClass::BasePaths(void)
{
	if (!mbBasePathsKept)
	{
		masBasePaths.ReInit();
		AddBasePaths(this, 0, NULL, false, true);
		mbBasePathsKept = IsComplete();
	}
	return &masBasePaths;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClass::AddBasePaths(CCPPClass* pcClass, int iOffset, CCPPClass* pcVirtual, bool bVirtualEdge, bool bPublic)
{
	SCPPBasePath*	psPath;
	SCPPBase*		psBase;
	size			i;

	psPath = masBasePaths.Add();
	psPath->pcClass = pcClass;
	psPath->iOffset = iOffset;
	psPath->pcVirtual = pcVirtual;
	psPath->bVirtualEdge = bVirtualEdge;
	psPath->bPublic = bPublic;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (psBase->bVirtual)
		{
			AddBasePaths(psBase->pcClass, 0, psBase->pcClass, true, bPublic && (psBase->eAccess == CPPA_Public));
		}
		else
		{
			AddBasePaths(psBase->pcClass, iOffset + psBase->iOffset, pcVirtual, false, bPublic && (psBase->eAccess == CPPA_Public));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBase is a base of the class, direct or not.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::HasBase(CCPPClass* pcBase)
{
	CArrayCPPBasePaths*	pasPaths;
	size				i;

	pasPaths = BasePaths();
	for (i = 1; i < pasPaths->NumElements(); i++)
	{
		if (pasPaths->Get(i)->pcClass == pcBase)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A field of the class, of an anonymous union or struct in it, or of its bases.
// *piOffset is set to its offset in this class.
//
//////////////////////////////////////////////////////////////////////////
SCPPField* CCPPClass::FindField(int64 lliName, int* piOffset, CCPPClass** ppcVirtual)
{
	SCPPField*	psField;
	SCPPField*	psMember;
	SCPPBase*		psBase;
	size			i;
	int				iOffset;
	CCPPClass*		pcVirtual;

	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.Get(i);
		if (psField->lliName == lliName)
		{
			*piOffset = psField->iOffset;
			return psField;
		}
		if (!psField->pcSymbol && psField->psType->IsClass())
		{
			psMember = psField->psType->pcClass->FindField(lliName, &iOffset);
			if (psMember)
			{
				*piOffset = psField->iOffset + iOffset;
				return psMember;
			}
		}
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		psBase = macBases.Get(i);
		pcVirtual = NULL;
		psField = psBase->pcClass->FindField(lliName, &iOffset, &pcVirtual);
		if (!psField)
		{
			continue;
		}
		if (!pcVirtual && !psBase->bVirtual)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}

		//Through a virtual base: where it is in the base, which the object says.
		if (!ppcVirtual)
		{
			return NULL;
		}
		*ppcVirtual = pcVirtual ? pcVirtual : psBase->pcClass;
		*piOffset = iOffset;
		return psField;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class that declares the field, this one or a base of it: NULL for a
// field of neither (and for one of an anonymous union or struct, which is
// that's).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPClass::FindFieldClass(SCPPField* psField)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; i < macFields.NumElements(); i++)
	{
		if (macFields.Get(i) == psField)
		{
			return this;
		}
	}
	pcClass = NULL;
	for (i = 0; !pcClass && (i < macBases.NumElements()); i++)
	{
		pcClass = macBases.Get(i)->pcClass->FindFieldClass(psField);
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// FindField, for the field a symbol declares.
//
//////////////////////////////////////////////////////////////////////////
SCPPField* CCPPClass::FindFieldSymbol(CCPPSymbol* pcSymbol, int* piOffset, CCPPClass** ppcVirtual)
{
	SCPPField*	psField;
	SCPPField*	psMember;
	SCPPBase*		psBase;
	size			i;
	int				iOffset;
	CCPPClass*		pcVirtual;

	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.Get(i);
		if (psField->pcSymbol == pcSymbol)
		{
			*piOffset = psField->iOffset;
			return psField;
		}
		if (!psField->pcSymbol && psField->psType->IsClass())
		{
			psMember = psField->psType->pcClass->FindFieldSymbol(pcSymbol, &iOffset);
			if (psMember)
			{
				*piOffset = psField->iOffset + iOffset;
				return psMember;
			}
		}
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		psBase = macBases.Get(i);
		pcVirtual = NULL;
		psField = psBase->pcClass->FindFieldSymbol(pcSymbol, &iOffset, &pcVirtual);
		if (!psField)
		{
			continue;
		}
		if (!pcVirtual && !psBase->bVirtual)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}

		//Through a virtual base: where it is in the base, which the object says.
		if (!ppcVirtual)
		{
			return NULL;
		}
		*ppcVirtual = pcVirtual ? pcVirtual : psBase->pcClass;
		*piOffset = iOffset;
		return psField;
	}

	//Names in templates are not always resolved to the instance's fields.
	if (mpcTemplate)
	{
		return FindField(pcSymbol->mlliName, piOffset);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the class's fields an aggregate initialiser ({1, 2}) can
// give values to: all but its unnamed bit-fields, and of a union's the
// first.  [dcl.init.aggr] 16
//
//////////////////////////////////////////////////////////////////////////
size CCPPClass::NumAggregateFields(void)
{
	size	uiFields;

	for (uiFields = 0; AggregateField(uiFields); uiFields++)
	{
	}
	return uiFields;
}


//////////////////////////////////////////////////////////////////////////
//
// The field an aggregate initialiser's value uiValue (its place in the
// list, from 0) fills, or NULL when the class has no more to fill: its
// fields in order but for its unnamed bit-fields, and of a union the
// first only.  [dcl.init.aggr] 6, 16
//
//////////////////////////////////////////////////////////////////////////
SCPPField* CCPPClass::AggregateField(size uiValue)
{
	size	i;

	if (mbUnion && (uiValue > 0))
	{
		return NULL;
	}
	for (i = 0; i < macFields.NumElements(); i++)
	{
		if (macFields.Get(i)->IsUnnamedBitField())
		{
			continue;
		}
		if (uiValue == 0)
		{
			return macFields.Get(i);
		}
		uiValue--;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether making an object of the class runs code: a constructor, a vtable pointer
// to set, a base or a field that needs it, or a default member initialiser.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::NeedsConstructor(void)
{
	SCPPField*	psField;
	SCPP65Type*		psType;
	size			i;

	if (mbConstructors || IsPolymorphic())
	{
		return true;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (macBases.Get(i)->pcClass->NeedsConstructor())
		{
			return true;
		}
	}
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.Get(i);
		if (psField->pcInitialiser)
		{
			return true;
		}
		psType = psField->psType->ElementType();
		if (psType->IsClass() && psType->pcClass->NeedsConstructor())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class's destructor is virtual: declared so, or a base's is
// (which makes the class's own virtual, written or not).  [class.dtor] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::HasVirtualDestructor(void)
{
	size	i;

	if (mpcDestructor && mpcDestructor->mbVirtual)
	{
		return true;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (macBases.Get(i)->pcClass->HasVirtualDestructor())
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
bool CCPPClass::NeedsDestructor(void)
{
	SCPPField*	psField;
	SCPP65Type*		psType;
	size			i;

	if (mpcDestructor)
	{
		return true;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (macBases.Get(i)->pcClass->mpcDestructor)
		{
			return true;
		}
	}
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.Get(i);
		psType = psField->psType->ElementType();
		if (psType->IsClass() && psType->pcClass->mpcDestructor)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class declares a constructor that takes one of its objects, to
// copy or to move: making an object from another is then a choice among its
// constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::HasCopyConstructor(void)
{
	return GetCopier(false, false) || GetCopier(false, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The member the class declares that makes one of its objects from another (a
// constructor) or, with bAssignment, gives one the value of another
// (operator=): the one taking the other by T& or const T&, a copy, or with
// bMove by T&&, a move.  Any other parameters have defaults.  NULL if it
// declares none.  A template's instance is never one, whatever it takes
// (CCPPOverloads::InstanceCopies).  [class.copy] 2 and 17
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClass::GetCopier(bool bAssignment, bool bMove)
{
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;

	for (i = 0; i < mapcMembers.NumElements(); i++)
	{
		pcFunction = mapcMembers.GetPtr(i);
		if (bAssignment ? (pcFunction->mlliName != CPP_OPERATOR_NAME(CT_Assignment)) : (pcFunction->meKind != CPPFK_Constructor))
		{
			continue;
		}
		if (pcFunction->mbImplicit || pcFunction->mpcTemplate || (pcFunction->mapsParameters.NumElements() == 0) || (pcFunction->RequiredParameters() != 1))
		{
			continue;
		}
		psParameter = pcFunction->mapsParameters.GetPtr(0);
		if (psParameter->IsReference() && (psParameter->bRvalue == bMove) && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == this))
		{
			return pcFunction;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The member that copies an object of the class that is part of another
// (a base or a field of an object copied member by member), or moves it
// with bMove: what is moved is copied if the class declares no move.
// NULL if the class declares neither, and its own parts are done.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClass::GetPartCopier(bool bAssignment, bool bMove)
{
	CCPPFunction*	pcFunction;

	pcFunction = bMove ? GetCopier(bAssignment, true) : NULL;
	return pcFunction ? pcFunction : GetCopier(bAssignment, false);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class's objects can't be copied or assigned unless it says
// how: it declares a move constructor or a move assignment, which takes
// away the copies the compiler would have written.  [class.copy] 7, 18
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsCopyDeleted(void)
{
	return GetCopier(false, true) || GetCopier(true, true);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class's objects can't be assigned unless it says how: the
// compiler may not write its operator=, as its copies are taken away
// (IsCopyDeleted), or a field is const or a reference, or a base's or a
// field's class can't be assigned.  [class.copy] 23
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsAssignmentDeleted(void)
{
	SCPP65Type*	psType;
	size		i;

	if (GetCopier(true, false))
	{
		return false;
	}
	if (IsCopyDeleted() || mpsLambda)
	{
		//(A lambda's closure has no copy assignment.  [expr.prim.lambda] 20)
		return true;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (macBases.Get(i)->pcClass->IsAssignmentDeleted())
		{
			return true;
		}
	}
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psType = macFields.Get(i)->psType;
		while (psType->IsArray())
		{
			psType = psType->psBase;
		}
		if (psType->IsReference() || psType->bConst || (psType->IsClass() && psType->pcClass->IsAssignmentDeleted()))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if assigning the class's objects copies their bytes: neither it nor
// its bases and fields have an operator= for their own objects.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsTriviallyAssignable(void)
{
	SCPP65Type*		psType;
	size			i;

	if (GetCopier(true, false) || GetCopier(true, true))
	{
		return false;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (!macBases.Get(i)->pcClass->IsTriviallyAssignable())
		{
			return false;
		}
	}
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psType = macFields.Get(i)->psType;
		psType = psType->ElementType();
		if (psType->IsClass() && !psType->pcClass->IsTriviallyAssignable())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if copying the class's objects copies their bytes: neither it nor its bases
// and fields have copy constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsTriviallyCopyable(void)
{
	SCPP65Type*		psType;
	size			i;

	if (HasCopyConstructor())
	{
		return false;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (!macBases.Get(i)->pcClass->IsTriviallyCopyable())
		{
			return false;
		}
	}
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psType = macFields.Get(i)->psType;
		psType = psType->ElementType();
		if (psType->IsClass() && !psType->pcClass->IsTriviallyCopyable())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if objects of the class have a vtable pointer, its own or a base's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsPolymorphic(void) const
{
	return mcVtable.IsPolymorphic();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function in the class's vtables (its final overriders) is pure
// virtual: no object of the class can be made.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::IsAbstract(void)
{
	return mcVtable.HasPure();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction overrides a virtual function of one of the class's bases.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::OverridesBase(CCPPFunction* pcFunction)
{
	return FindOverridden(pcFunction) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual function of one of the class's bases (a final overrider in its
// vtables) that pcFunction overrides, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClass::FindOverridden(CCPPFunction* pcFunction)
{
	CCPPFunction*	pcOverridden;
	size			i;

	for (i = 0; i < macBases.NumElements(); i++)
	{
		pcOverridden = macBases.Get(i)->pcClass->mcVtable.FindOverridden(pcFunction);
		if (pcOverridden)
		{
			return pcOverridden;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a vtable pointer is in an object of a polymorphic class: its own, else
// the first secondary one's.
//
//////////////////////////////////////////////////////////////////////////
//
// A field of psType declared by pcSymbol (NULL for one with no name: an
// unnamed bit-field, a closure's capture), a bit-field iBitWidth wide (-1
// for one that isn't), not yet placed, with no alignas, no initialiser and
// not mutable.
//
//////////////////////////////////////////////////////////////////////////
void SCPPField::Init(CCPPSymbol* pcSymbol, SCPP65Type* psFieldType, int iFieldBitWidth)
{
	this->pcSymbol = pcSymbol;
	lliName = pcSymbol ? pcSymbol->mlliName : -1;
	psType = psFieldType;
	iOffset = 0;
	iAlign = 0;
	pcInitialiser = NULL;
	bMutable = false;
	iBitWidth = iFieldBitWidth;
	iBitOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
int CCPPClass::GetVtablePointerOffset(void)
{
	return mcVtable.GetPointerOffset();
}


//////////////////////////////////////////////////////////////////////////
//
// The member of the kind eKind named lliName of the class or of a base, and in ppcClass the class that declares it.  A
// specialisation has its own members: its symbol is its primary template's, whose members they aren't.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPClass::FindMemberSymbol(int64 lliName, ECPPSymbolKind eKind, CCPPClass** ppcClass)
{
	CCPPScope*	pcMembers;
	CCPPSymbol*	pcSymbol;
	size		i;

	pcMembers = mpcSpecifier ? mpcSpecifier->mpcScope : (mpcSymbol ? mpcSymbol->mpcMembers : NULL);
	pcSymbol = pcMembers ? pcMembers->FindHere(lliName) : NULL;
	if (pcSymbol && (pcSymbol->meKind == eKind))
	{
		*ppcClass = this;
		return pcSymbol;
	}
	for (i = 0; i < macBases.NumElements(); i++)
	{
		pcSymbol = macBases.Get(i)->pcClass->FindMemberSymbol(lliName, eKind, ppcClass);
		if (pcSymbol)
		{
			return pcSymbol;
		}
	}
	return NULL;
}


bool SCPPField::IsBitField(void) const { return iBitWidth >= 0; }
bool SCPPField::IsUnnamedBitField(void) const { return (iBitWidth >= 0) && (pcSymbol == NULL); }


//////////////////////////////////////////////////////////////////////////
//
// Whether nothing gives the field a value where a constructor doesn't name
// it in its member initialisers: it has no default member initialiser, and
// it is a reference, or a const object (or an array of them) of a type with
// no constructor of its own, a number say.  [class.base.init] 8,
// [class.ctor] 4
//
//////////////////////////////////////////////////////////////////////////
bool SCPPField::NeedsValue(void) const
{
	SCPP65Type*	psElement;

	if (!pcSymbol || pcInitialiser)
	{
		return false;
	}
	psElement = psType->InnermostElement();
	return psElement->IsReference() || (psElement->bConst && !(psElement->IsClass() && psElement->pcClass->mbConstructors));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a field needs a value that nothing but a constructor of the
// class can give it (SCPPField::NeedsValue): with none, its default
// constructor is deleted.  [class.ctor] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::HasFieldNeedingValue(void)
{
	size	i;

	for (i = 0; i < macFields.NumElements(); i++)
	{
		if (macFields.Get(i)->NeedsValue())
		{
			return true;
		}
	}
	return false;
}
bool CCPPClass::IsComplete(void) const { return meState == CPPCS_Complete; }
bool CCPPClass::IsBeingMade(void) const { return (meState == CPPCS_ResolvingBases) || IsLayingOut(); }
bool CCPPClass::IsLayingOut(void) const { return meState == CPPCS_LayingOut; }
bool CCPPClass::HasBases(void) const { return meState >= CPPCS_ResolvingBases; }
bool CCPPClass::HasVirtualBases(void) const { return macVirtualBases.NumElements() > 0; }
bool CCPPClass::IsUnnamed(void) const { return mlliName < 0; }
