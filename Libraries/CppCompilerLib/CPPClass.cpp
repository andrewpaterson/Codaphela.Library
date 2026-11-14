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
	macUsings.Init();
	macFields.Init();
	mapcMembers.Init();
	mcVtable.Init();
	meState = CPPCS_Declared;
	mbEmpty = false;
	miAlign = 1;
	mbUnion = false;
	mbTypeInfo = false;
	mbConstructors = false;
	mpcInheritedBase = NULL;
	mpcDestructor = NULL;
	mpcSpecifier = NULL;
	mpcTemplate = NULL;
	mpcBindings = NULL;
	mszArguments.Init();
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
// A field of the class, of an anonymous union or struct in it, or of its bases.
// *piOffset is set to its offset in this class.
//
//////////////////////////////////////////////////////////////////////////
SCPPField* CCPPClass::FindField(int64 lliName, int* piOffset)
{
	SCPPField*	psField;
	SCPPField*	psMember;
	SCPPBase*		psBase;
	size			i;
	int				iOffset;

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
		psField = psBase->pcClass->FindField(lliName, &iOffset);
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
SCPPField* CCPPClass::FindFieldSymbol(CCPPSymbol* pcSymbol, int* piOffset)
{
	SCPPField*	psField;
	SCPPField*	psMember;
	SCPPBase*		psBase;
	size			i;
	int				iOffset;

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
		psField = psBase->pcClass->FindFieldSymbol(pcSymbol, &iOffset);
		if (psField)
		{
			*piOffset = psBase->iOffset + iOffset;
			return psField;
		}
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
// give values to: all but its unnamed bit-fields.
//
//////////////////////////////////////////////////////////////////////////
size CCPPClass::NumAggregateFields(void)
{
	size	i;
	size	uiFields;

	uiFields = 0;
	for (i = 0; i < macFields.NumElements(); i++)
	{
		if (!macFields.Get(i)->IsUnnamedBitField())
		{
			uiFields++;
		}
	}
	return uiFields;
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
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && psType->pcClass->NeedsConstructor())
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
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && psType->pcClass->mpcDestructor)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class declares a constructor that takes one of its objects (a copy
// constructor).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::HasCopyConstructor(void)
{
	return GetCopyConstructor() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's copy constructor (taking one of its objects by reference, any other
// parameters having defaults), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClass::GetCopyConstructor(void)
{
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;

	for (i = 0; i < mapcMembers.NumElements(); i++)
	{
		pcFunction = mapcMembers.GetPtr(i);
		if ((pcFunction->meKind != CPPFK_Constructor) || pcFunction->mbImplicit || (pcFunction->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = pcFunction->mapsParameters.GetPtr(0);
		if (psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == this) && (pcFunction->RequiredParameters() == 1))
		{
			return pcFunction;
		}
	}
	return NULL;
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
		psType = psType->IsArray() ? psType->psBase : psType;
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
bool CCPPClass::IsPolymorphic(void)
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
// True if pcFunction overrides a virtual function in any of the class's bases'
// vtables.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClass::OverridesBase(CCPPFunction* pcFunction)
{
	size	i;

	for (i = 0; i < macBases.NumElements(); i++)
	{
		if (macBases.Get(i)->pcClass->mcVtable.IsOverriddenBy(pcFunction))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a vtable pointer is in an object of a polymorphic class: its own, else
// the first secondary one's.
//
//////////////////////////////////////////////////////////////////////////
int CCPPClass::GetVtablePointerOffset(void)
{
	return mcVtable.GetPointerOffset();
}

