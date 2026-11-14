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
	mbImplicit = false;
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
	mpcNextOfName = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunction::Kill(void)
{
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
// Whether a function can be called on the object (iConstObject 1 if it is
// const, 0 if not, -1 for none): a const object takes only const and static
// member functions.  [over.match.funcs] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::TakesObject(int iConstObject)
{
	return (iConstObject != 1) || !mpcClass || mbStatic || mbConstThis;
}


//////////////////////////////////////////////////////////////////////////
//
// A static function, a member of a class without a name, or what is in a
// namespace without a name is the unit's own: its label is local to the object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunction::IsUnitFunction(void)
{
	CCPPClass*	pcClass;

	pcClass = mpcClass;
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
// makes the generator write a constructor or destructor.
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunction::SetDefinition(CCPPSyntaxFunctionDefinition* pcDefinition)
{
	if ((pcDefinition->meBody == CPPFB_Default) && (meKind != CPPFK_Function))
	{
		mbImplicit = true;
		mbDefined = true;
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

