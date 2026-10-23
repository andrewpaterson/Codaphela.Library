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
#include <stdlib.h>
#include "CPP65Type.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsInteger(void)
{
	return (eKind == CPP65TK_Integer) || (eKind == CPP65TK_Bool);
}


//////////////////////////////////////////////////////////////////////////
//
// A value that fits in a register.
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsScalar(void)
{
	return (eKind == CPP65TK_Integer) || (eKind == CPP65TK_Bool) || (eKind == CPP65TK_Pointer) || (eKind == CPP65TK_Float);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsPointer(void)
{
	return eKind == CPP65TK_Pointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsArray(void)
{
	return eKind == CPP65TK_Array;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsFunction(void)
{
	return eKind == CPP65TK_Function;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsVoid(void)
{
	return eKind == CPP65TK_Void;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsClass(void)
{
	return eKind == CPP65TK_Class;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsReference(void)
{
	return eKind == CPP65TK_Reference;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsFloat(void)
{
	return eKind == CPP65TK_Float;
}


//////////////////////////////////////////////////////////////////////////
//
// A scalar the registers can work with: not floating point or 8 bytes.
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsSupported(void)
{
	return IsScalar() && !IsFloat() && (iBytes <= 8);
}


//////////////////////////////////////////////////////////////////////////
//
// An 8 byte integer: worked on with the registers 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsWide(void)
{
	return (eKind == CPP65TK_Integer) && (iBytes == 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::Equals(SCPP65Type* psOther)
{
	size	i;

	if (this == psOther)
	{
		return true;
	}
	if ((eKind != psOther->eKind) || (iBytes != psOther->iBytes) || (bSigned != psOther->bSigned) || (iCount != psOther->iCount) || (psClass != psOther->psClass))
	{
		return false;
	}
	if ((psBase && !psOther->psBase) || (!psBase && psOther->psBase))
	{
		return false;
	}
	if (psBase && !psBase->Equals(psOther->psBase))
	{
		return false;
	}
	if (eKind == CPP65TK_Function)
	{
		if ((bVariadic != psOther->bVariadic) || (papsParameters->NumElements() != psOther->papsParameters->NumElements()))
		{
			return false;
		}
		for (i = 0; i < papsParameters->NumElements(); i++)
		{
			if (!papsParameters->GetPtr(i)->Equals(psOther->papsParameters->GetPtr(i)))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The size of what a pointer points to, for pointer arithmetic.  void* steps in bytes
// (a GNU extension).
//
//////////////////////////////////////////////////////////////////////////
int SCPP65Type::GetElementBytes(void)
{
	if (psBase->IsVoid())
	{
		return 1;
	}
	return psBase->iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::Init(void)
{
	mapsTypes.Init();
	InitBuiltin(&msVoid, CPP65TK_Void, 0, false);
	InitBuiltin(&msBool, CPP65TK_Bool, 1, false);
	InitBuiltin(&msChar, CPP65TK_Integer, 1, true);
	InitBuiltin(&msUnsignedChar, CPP65TK_Integer, 1, false);
	InitBuiltin(&msShort, CPP65TK_Integer, 2, true);
	InitBuiltin(&msUnsignedShort, CPP65TK_Integer, 2, false);
	InitBuiltin(&msInt, CPP65TK_Integer, 4, true);
	InitBuiltin(&msUnsignedInt, CPP65TK_Integer, 4, false);
	InitBuiltin(&msLongLong, CPP65TK_Integer, 8, true);
	InitBuiltin(&msUnsignedLongLong, CPP65TK_Integer, 8, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::Kill(void)
{
	size	i;

	SCPP65Type*	psType;

	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		psType = mapsTypes.GetPtr(i);
		if (psType->papsParameters)
		{
			psType->papsParameters->Kill();
			free(psType->papsParameters);
		}
		free(psType);
	}
	mapsTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::InitBuiltin(SCPP65Type* psType, ECPP65TypeKind eKind, int iBytes, bool bSigned)
{
	psType->eKind = eKind;
	psType->iBytes = iBytes;
	psType->bSigned = bSigned;
	psType->psBase = NULL;
	psType->iCount = 0;
	psType->psClass = NULL;
	psType->papsParameters = NULL;
	psType->bVariadic = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Add(ECPP65TypeKind eKind, int iBytes, SCPP65Type* psBase, int iCount)
{
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		psType = mapsTypes.GetPtr(i);
		if ((psType->eKind == eKind) && (psType->psBase == psBase) && (psType->iCount == iCount) && (psType->iBytes == iBytes))
		{
			return psType;
		}
	}

	psType = (SCPP65Type*)malloc(sizeof(SCPP65Type));
	psType->eKind = eKind;
	psType->iBytes = iBytes;
	psType->bSigned = false;
	psType->psBase = psBase;
	psType->iCount = iCount;
	psType->psClass = NULL;
	psType->papsParameters = NULL;
	psType->bVariadic = false;
	mapsTypes.Add(psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Integer(int iBytes, bool bSigned)
{
	switch (iBytes)
	{
	case 1:
		return bSigned ? &msChar : &msUnsignedChar;
	case 2:
		return bSigned ? &msShort : &msUnsignedShort;
	case 8:
		return bSigned ? &msLongLong : &msUnsignedLongLong;
	default:
		return bSigned ? &msInt : &msUnsignedInt;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Pointer(SCPP65Type* psBase)
{
	return Add(CPP65TK_Pointer, 4, psBase, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Array(SCPP65Type* psElement, int iCount)
{
	return Add(CPP65TK_Array, psElement->iBytes * iCount, psElement, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
// A function type.  The parameters are copied.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Function(SCPP65Type* psReturn, CArrayTemplatePtr<SCPP65Type>* papsParameters, bool bVariadic)
{
	SCPP65Type*	psType;
	SCPP65Type*	psOther;
	size		i;

	psType = (SCPP65Type*)malloc(sizeof(SCPP65Type));
	InitBuiltin(psType, CPP65TK_Function, 0, false);
	psType->psBase = psReturn;
	psType->bVariadic = bVariadic;
	psType->papsParameters = (CArrayTemplatePtr<SCPP65Type>*)malloc(sizeof(CArrayTemplatePtr<SCPP65Type>));
	psType->papsParameters->Init();
	for (i = 0; papsParameters && (i < papsParameters->NumElements()); i++)
	{
		psType->papsParameters->Add(papsParameters->GetPtr(i));
	}

	//The same function type is shared.
	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		psOther = mapsTypes.GetPtr(i);
		if ((psOther->eKind == CPP65TK_Function) && psOther->Equals(psType))
		{
			psType->papsParameters->Kill();
			free(psType->papsParameters);
			free(psType);
			return psOther;
		}
	}
	mapsTypes.Add(psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Reference(SCPP65Type* psBase)
{
	return Add(CPP65TK_Reference, 4, psBase, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// float (4), double (8) and long double (16).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Float(int iBytes)
{
	SCPP65Type*	psType;

	psType = Add(CPP65TK_Float, iBytes, NULL, 0);
	psType->bSigned = true;
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A class's type.  Its size is filled in when the class is laid out.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Class(SCPP65Class* psClass)
{
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		psType = mapsTypes.GetPtr(i);
		if ((psType->eKind == CPP65TK_Class) && (psType->psClass == psClass))
		{
			return psType;
		}
	}

	psType = (SCPP65Type*)malloc(sizeof(SCPP65Type));
	psType->eKind = CPP65TK_Class;
	psType->iBytes = 0;
	psType->bSigned = false;
	psType->psBase = NULL;
	psType->iCount = 0;
	psType->psClass = psClass;
	psType->papsParameters = NULL;
	psType->bVariadic = false;
	mapsTypes.Add(psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Integral promotion: bool, char and short become int.  [conv.prom]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Promote(SCPP65Type* psType)
{
	if (psType->IsInteger() && (psType->iBytes < 4))
	{
		return &msInt;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The usual arithmetic conversions of two integers: unsigned when either promoted
// type is unsigned.  [expr] 10
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Arithmetic(SCPP65Type* psLeft, SCPP65Type* psRight)
{
	psLeft = Promote(psLeft);
	psRight = Promote(psRight);
	if (psLeft->IsWide() || psRight->IsWide())
	{
		//long long holds every unsigned int, so only an unsigned long long makes it unsigned.
		if ((psLeft->IsWide() && !psLeft->bSigned) || (psRight->IsWide() && !psRight->bSigned))
		{
			return &msUnsignedLongLong;
		}
		return &msLongLong;
	}
	if (!psLeft->bSigned || !psRight->bSigned)
	{
		return &msUnsignedInt;
	}
	return &msInt;
}
