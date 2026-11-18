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
#include "CPPExceptionSpec.h"


//////////////////////////////////////////////////////////////////////////
//
// Anything is allowed, as for a function declared with no specification.
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::Init(void)
{
	meKind = CPPES_Anything;
	mapsTypes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::Kill(void)
{
	mapsTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::SetAnything(void)
{
	meKind = CPPES_Anything;
	mapsTypes.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::SetNothing(void)
{
	meKind = CPPES_Nothing;
	mapsTypes.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// One more type of a throw(A, B) list.  A specification that allows
// anything already allows it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::AddType(SCPP65Type* psType)
{
	if (meKind == CPPES_Anything)
	{
		return;
	}
	meKind = CPPES_Types;
	if (!HasType(psType))
	{
		mapsTypes.Add(psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::Set(CCPPExceptionSpec* pcSource)
{
	size	i;

	meKind = pcSource->meKind;
	mapsTypes.ReInit();
	for (i = 0; i < pcSource->mapsTypes.NumElements(); i++)
	{
		mapsTypes.Add(pcSource->mapsTypes.GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Allows what pcOther allows too: what a function the compiler writes
// allows is what each function it calls does.  [except.spec] 14
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptionSpec::Add(CCPPExceptionSpec* pcOther)
{
	size	i;

	if (pcOther->meKind == CPPES_Anything)
	{
		SetAnything();
		return;
	}
	for (i = 0; i < pcOther->mapsTypes.NumElements(); i++)
	{
		AddType(pcOther->mapsTypes.GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptionSpec::IsNonThrowing(void) const
{
	return meKind == CPPES_Nothing;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptionSpec::IsAnything(void) const
{
	return meKind == CPPES_Anything;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a throw(A, B) list names the type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptionSpec::HasType(SCPP65Type* psType)
{
	size	i;

	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		if (mapsTypes.GetPtr(i)->Same(psType))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two declarations of one function may have these: both allow
// nothing, both anything, or both the same types.  [except.spec] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptionSpec::IsCompatible(CCPPExceptionSpec* pcOther)
{
	size	i;

	if ((meKind != pcOther->meKind) || (mapsTypes.NumElements() != pcOther->mapsTypes.NumElements()))
	{
		return false;
	}
	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		if (!pcOther->HasType(mapsTypes.GetPtr(i)))
		{
			return false;
		}
	}
	return true;
}
