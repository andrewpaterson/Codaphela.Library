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
#include "CPPClass.h"
#include "CPPFunction.h"
#include "CPPLambda.h"


//A function the compiler knows: what it is asked about itself.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunction::Init(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn)
{
	mlliName = lliName;
	mpsReturn = psReturn;
	mapsParameters.Init();
	mbDefined = false;
	mpcClass = pcClass;
	mpcNamespace = NULL;
	meKind = eKind;
	mbStatic = false;
	mbVirtual = false;
	mbPure = false;
	miSlot = -1;
	mbCLinkage = false;
	mbExplicit = false;
	mbConstThis = false;
	mbVolatileThis = false;
	mbConstexpr = false;
	mcExceptions.Init();
	mbExceptionsWritten = false;
	mbExceptionsKnown = false;
	mbOverride = false;
	mbFinal = false;
	mpcAddressOf = NULL;
	meRefQualifier = CPPRQ_None;
	mbImplicit = false;
	mbDeleting = false;
	mbDeleted = false;
	mpcInherited = NULL;
	mapcDefaults.Init();
	mbVariadic = false;
	mpcDefinition = NULL;
	mpcDeclarator = NULL;
	mpcBindings = pcClass ? pcClass->mpcBindings : NULL;
	mbInline = false;
	mbQueued = false;
	mbGenerated = false;
	mbCalled = false;
	mpcTemplate = NULL;
	mszArguments.Init();
	mbExternInstance = false;
	mpcNextOfName = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunction::Kill(void)
{
	mcExceptions.Kill();
	mszArguments.Kill();
	mapcDefaults.Kill();
	mapsParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The number of parameters a call must give: those before the trailing ones
// with defaults.
//
//////////////////////////////////////////////////////////////////////////
size CCPPFunction::RequiredParameters(void)
{
	size	uiRequired;

	uiRequired = mapsParameters.NumElements();
	while ((uiRequired > 0) && mapcDefaults.GetPtr(uiRequired - 1))
	{
		uiRequired--;
	}
	return uiRequired;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call can give the function uiArguments arguments: at least its
// required parameters, and no more than it has unless it is variadic.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::TakesArgumentCount(size uiArguments)
{
	return (uiArguments >= RequiredParameters()) && ((uiArguments <= mapsParameters.NumElements()) || mbVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::SameParameters(CCPPFunction* pcOther)
{
	size	i;

	if ((mapsParameters.NumElements() != pcOther->mapsParameters.NumElements()) || (mbVariadic != pcOther->mbVariadic))
	{
		return false;
	}
	for (i = 0; i < mapsParameters.NumElements(); i++)
	{
		if (!mapsParameters.GetPtr(i)->Same(pcOther->mapsParameters.GetPtr(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if this function overrides pcVirtual: destructors override destructors, other
// functions need the same name and parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::Overrides(CCPPFunction* pcVirtual)
{
	return (pcVirtual->meKind == meKind) && ((meKind == CPPFK_Destructor) || ((pcVirtual->mlliName == mlliName) && pcVirtual->SameParameters(this)));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function can be called on the object (eObject): a const object
// takes only const and static member functions (a volatile one, volatile),
// and a function declared & or && only an object that is, or isn't, an
// rvalue.  [over.match.funcs] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::TakesObject(ECPPCalledObject eObject)
{
	if ((eObject == CPPCO_None) || !mpcClass || mbStatic)
	{
		return true;
	}
	if ((meRefQualifier != CPPRQ_None) && ((meRefQualifier == CPPRQ_Rvalue) != CPPCalledRvalue(eObject)))
	{
		//f() const & takes an object that is going too, as const T& does.
		if (!((meRefQualifier == CPPRQ_Lvalue) && mbConstThis))
		{
			return false;
		}
	}
	return (!CPPCalledConst(eObject) || mbConstThis) && (!CPPCalledVolatile(eObject) || mbVolatileThis);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether it is its class's copy (or move) constructor or assignment: a
// constructor or operator= whose one parameter is a reference to the class.
// [class.copy] 2, 17
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::IsCopy(void)
{
	SCPP65Type*	psParameter;

	if (!mpcClass || (mapsParameters.NumElements() != 1) || !((meKind == CPPFK_Constructor) || ((meKind == CPPFK_Function) && (mlliName == CPP_OPERATOR_NAME(CT_Assignment)))))
	{
		return false;
	}
	psParameter = mapsParameters.GetPtr(0);
	return psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == mpcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// How many of const and volatile a member function's this has that the
// object it is called on (eObject) hasn't: of two functions that take the
// object, the one that adds fewer is the better.  [over.ics.rank] 3
//
//////////////////////////////////////////////////////////////////////////
int CCPPFunction::QualifiersAdded(ECPPCalledObject eObject)
{
	return ((mbConstThis && !CPPCalledConst(eObject)) ? 1 : 0) + ((mbVolatileThis && !CPPCalledVolatile(eObject)) ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A static function, a member of a class without a name, or what is in a
// namespace without a name is the unit's own: its label is local to the object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::IsUnitFunction(void) const
{
	CCPPClass*	pcClass;

	pcClass = mpcClass;
	if (pcClass && pcClass->mpsLambda)
	{
		//A closure is the unit's own unless the lambda is in a function every unit may write (SCPPLambda::szName).
		return pcClass->mpsLambda->szName.Empty();
	}
	if (pcClass)
	{
		return (pcClass->IsUnnamed()) || (pcClass->mpcSymbol && pcClass->mpcSymbol->mpcScope->IsInUnnamedNamespace());
	}
	return mbStatic || (mpcNamespace && mpcNamespace->IsInUnnamedNamespace());
}


//////////////////////////////////////////////////////////////////////////
//
// Records the function's definition.  Functions defined in classes, inline,
// constexpr and template functions are generated when they are first called.
// Bodies that were skipped (in system headers) are not definitions; = default
// makes the generator write a constructor or destructor, and = delete makes
// choosing the function an error (CCPPOverloads::FindFunction).
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunction::SetDefinition(CCPPSyntaxFunctionDefinition* pcDefinition)
{
	if ((pcDefinition->meBody == CPPFB_Default) && ((meKind != CPPFK_Function) || IsCopy()))
	{
		//A copy assignment's too, defaulted after its class (in the class it isn't declared: = does what it would).
		mbImplicit = true;
		mbDefined = true;
		return;
	}
	if (pcDefinition->meBody == CPPFB_Delete)
	{
		mbDeleted = true;
		return;
	}
	if (pcDefinition->meBody != CPPFB_Body)
	{
		return;
	}
	mpcDefinition = pcDefinition;
	mbDefined = true;
	mbInline = true;
	if (!mpcBindings && mpcClass)
	{
		mpcBindings = mpcClass->mpcBindings;
	}
}


bool CCPPFunction::IsConversion(void) const { return mlliName == CPP_CONVERSION_NAME; }
bool CCPPFunction::IsOperator(void) const { return mlliName < 0; }
