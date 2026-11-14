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
	return IsInteger() || (eKind == CPP65TK_Pointer) || (eKind == CPP65TK_Float) || (eKind == CPP65TK_MemberPointer);
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
bool SCPP65Type::IsMemberPointer(void)
{
	return eKind == CPP65TK_MemberPointer;
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
// The type a reference refers to; any other type is itself.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* SCPP65Type::WithoutReference(void)
{
	return IsReference() ? psBase : this;
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
// A scalar the generator can work with: up to 8 bytes (floating point is the soft
// float runtime's).
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsSupported(void)
{
	return IsScalar() && (iBytes <= 8);
}


//////////////////////////////////////////////////////////////////////////
//
// An 8 byte integer: worked on with the registers 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
//
// Whether the type has every qualifier psFrom has (const T from T or const T,
// not T from const T).
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::HasQualifiersOf(SCPP65Type* psFrom)
{
	return (bConst || !psFrom->bConst) && (bVolatile || !psFrom->bVolatile);
}


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

	//const and volatile don't make a type different here (Same tells them apart).
	if ((this == psOther) || (psUnqualified == psOther->psUnqualified))
	{
		return true;
	}
	if ((eKind != psOther->eKind) || (iBytes != psOther->iBytes) || (bSigned != psOther->bSigned) || (iCount != psOther->iCount) || (pcClass != psOther->pcClass) || (bLongDouble != psOther->bLongDouble) || (pcEnum != psOther->pcEnum))
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
// The type as C++ for messages.  Integers are named by their size, not as
// declared (a 2 byte int is "short", a 4 byte one "long"), so the widths
// that differ between targets show.  Classes and enums aren't named.
//
//////////////////////////////////////////////////////////////////////////
void SCPP65Type::Append(CChars* psz)
{
	size	i;

	//const int, but int* const.
	if (!IsPointer() && !IsMemberPointer())
	{
		psz->Append(bConst ? "const " : "");
		psz->Append(bVolatile ? "volatile " : "");
	}
	switch (eKind)
	{
	case CPP65TK_Void:
		psz->Append("void");
		break;
	case CPP65TK_Bool:
		psz->Append("bool");
		break;
	case CPP65TK_Integer:
		if (pcEnum)
		{
			psz->Append("enum");
			break;
		}
		if (!bSigned)
		{
			psz->Append("unsigned ");
		}
		switch (iBytes)
		{
		case 1:		psz->Append("char");		break;
		case 2:		psz->Append("short");		break;
		case 4:		psz->Append("long");		break;
		default:	psz->Append("long long");	break;
		}
		break;
	case CPP65TK_Float:
		psz->Append(bLongDouble ? "long double" : ((iBytes == 4) ? "float" : "double"));
		break;
	case CPP65TK_Pointer:
		psBase->Append(psz);
		psz->Append("*");
		break;
	case CPP65TK_Reference:
		psBase->Append(psz);
		psz->Append("&");
		break;
	case CPP65TK_Array:
		psBase->Append(psz);
		psz->Append("[")->Append(iCount)->Append("]");
		break;
	case CPP65TK_Function:
		psBase->Append(psz);
		psz->Append("(");
		for (i = 0; i < papsParameters->NumElements(); i++)
		{
			if (i != 0)
			{
				psz->Append(", ");
			}
			papsParameters->GetPtr(i)->Append(psz);
		}
		if (bVariadic)
		{
			psz->Append((papsParameters->NumElements() != 0) ? ", ..." : "...");
		}
		psz->Append(")");
		break;
	case CPP65TK_Class:
		psz->Append("class");
		break;
	case CPP65TK_MemberPointer:
		psBase->Append(psz);
		psz->Append(" class::*");
		break;
	}
	if (IsPointer() || IsMemberPointer())
	{
		psz->Append(bConst ? " const" : "");
		psz->Append(bVolatile ? " volatile" : "");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the types are the same (Equals) with the same const and volatile
// at every level: const int* isn't int*.  [basic.type.qualifier]
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::Same(SCPP65Type* psOther)
{
	size	i;

	if (this == psOther)
	{
		return true;
	}
	if ((bConst != psOther->bConst) || (bVolatile != psOther->bVolatile) || !Equals(psOther))
	{
		return false;
	}
	if (psBase && !psBase->Same(psOther->psBase))
	{
		return false;
	}
	if (eKind == CPP65TK_Function)
	{
		for (i = 0; i < papsParameters->NumElements(); i++)
		{
			if (!papsParameters->GetPtr(i)->Same(psOther->papsParameters->GetPtr(i)))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The type without const and volatile.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* SCPP65Type::Unqualified(void)
{
	return psUnqualified;
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
void CCPP65Types::Init(int iIntBytes, int iPointerBytes)
{
	miPointerBytes = iPointerBytes;
	mapsTypes.Init();
	mcByBase.Init(true);
	mcFunctions.Init(true);
	mcClasses.Init(true);
	mcEnums.Init(true);
	mcQualified.Init(true);
	InitBuiltin(&msVoid, CPP65TK_Void, 0, false);
	InitBuiltin(&msBool, CPP65TK_Bool, 1, false);
	InitBuiltin(&msChar, CPP65TK_Integer, 1, true);
	InitBuiltin(&msUnsignedChar, CPP65TK_Integer, 1, false);
	InitBuiltin(&msShort, CPP65TK_Integer, 2, true);
	InitBuiltin(&msUnsignedShort, CPP65TK_Integer, 2, false);
	InitBuiltin(&msInt, CPP65TK_Integer, iIntBytes, true);
	InitBuiltin(&msUnsignedInt, CPP65TK_Integer, iIntBytes, false);
	InitBuiltin(&msLong, CPP65TK_Integer, 4, true);
	InitBuiltin(&msUnsignedLong, CPP65TK_Integer, 4, false);
	InitBuiltin(&msLongLong, CPP65TK_Integer, 8, true);
	InitBuiltin(&msUnsignedLongLong, CPP65TK_Integer, 8, false);
	InitBuiltin(&msFloat, CPP65TK_Float, 4, true);
	InitBuiltin(&msDouble, CPP65TK_Float, 8, true);
	InitBuiltin(&msLongDouble, CPP65TK_Float, 8, true);
	msLongDouble.bLongDouble = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::Kill(void)
{
	size		i;
	SCPP65Type*	psType;

	for (i = 0; i < mapsTypes.NumElements(); i++)
	{
		psType = mapsTypes.GetPtr(i);
		if (psType->papsParameters)
		{
			psType->papsParameters->Kill();
			SafeFree(psType->papsParameters);
		}
		SafeFree(psType);
	}
	mapsTypes.Kill();
	mcQualified.Kill();
	mcEnums.Kill();
	mcClasses.Kill();
	mcFunctions.Kill();
	mcByBase.Kill();
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
	psType->pcClass = NULL;
	psType->papsParameters = NULL;
	psType->bVariadic = false;
	psType->bLongDouble = false;
	psType->pcEnum = NULL;
	psType->bConst = false;
	psType->bVolatile = false;
	psType->psUnqualified = psType;
	psType->psNextSameBase = NULL;
	psType->psNextSameSignature = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A new type of eKind and iBytes (as InitBuiltin makes it: unsigned, no
// base, class, parameters or enum).  AddType puts it with the others.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::NewType(ECPP65TypeKind eKind, int iBytes)
{
	SCPP65Type*	psType;

	psType = NewMalloc<SCPP65Type>();
	InitBuiltin(psType, eKind, iBytes, false);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// psType (made with NewType, its kind and base set) is one of the types:
// it is freed with them, and found by Add after those made before it of
// its kind and base.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::AddType(SCPP65Type* psType)
{
	mapsTypes.Add(psType);
	AppendToChain(&mcByBase, GetBaseKey(psType->eKind, psType->psBase), psType, &SCPP65Type::psNextSameBase);
}


//////////////////////////////////////////////////////////////////////////
//
// psType goes last on the chain of lliKey in pcChains, linked by its
// ppsNext.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::AppendToChain(CMapLongTemplate<SCPP65TypeChain>* pcChains, int64 lliKey, SCPP65Type* psType, SCPP65Type* SCPP65Type::* ppsNext)
{
	SCPP65TypeChain*	psChain;
	SCPP65TypeChain		sChain;

	psType->*ppsNext = NULL;
	psChain = pcChains->Get(lliKey);
	if (psChain)
	{
		psChain->psLast->*ppsNext = psType;
		psChain->psLast = psType;
	}
	else
	{
		sChain.psFirst = psType;
		sChain.psLast = psType;
		pcChains->Put(lliKey, &sChain);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type pcMap holds for pvKey, NULL if none.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::GetMapped(CMapLongTemplate<SCPP65Type*>* pcMap, void* pvKey)
{
	SCPP65Type**	ppsType;

	ppsType = pcMap->Get(GetPointerKey(pvKey));
	return ppsType ? *ppsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Types::GetPointerKey(void* pv)
{
	return (int64)(size)pv;
}


//////////////////////////////////////////////////////////////////////////
//
// A hash of what SCPP65Type::Equals compares, through the base and
// parameter types, so equal types always hash the same.  Volatile isn't
// in it, as Equals ignores it.
//
//////////////////////////////////////////////////////////////////////////
uint64 CCPP65Types::HashStructure(SCPP65Type* psType)
{
	uint64	ulliHash;
	size	i;

	ulliHash = (uint64)psType->eKind;
	ulliHash = ulliHash * 31 + (uint64)(int64)psType->iBytes;
	ulliHash = ulliHash * 31 + (uint64)psType->bSigned;
	ulliHash = ulliHash * 31 + (uint64)(int64)psType->iCount;
	ulliHash = ulliHash * 31 + (uint64)(size)psType->pcClass;
	ulliHash = ulliHash * 31 + (uint64)psType->bLongDouble;
	ulliHash = ulliHash * 31 + (uint64)(size)psType->pcEnum;
	if (psType->psBase)
	{
		ulliHash = ulliHash * 31 + HashStructure(psType->psBase);
	}
	if (psType->eKind == CPP65TK_Function)
	{
		ulliHash = ulliHash * 31 + (uint64)psType->bVariadic;
		ulliHash = ulliHash * 31 + (uint64)psType->papsParameters->NumElements();
		for (i = 0; i < psType->papsParameters->NumElements(); i++)
		{
			ulliHash = ulliHash * 31 + HashStructure(psType->papsParameters->GetPtr(i));
		}
	}
	return ulliHash;
}


//////////////////////////////////////////////////////////////////////////
//
// The key of the types of eKind made from psBase.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Types::GetBaseKey(ECPP65TypeKind eKind, SCPP65Type* psBase)
{
	return ((int64)(size)psBase << 4) | (int64)eKind;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Add(ECPP65TypeKind eKind, int iBytes, SCPP65Type* psBase, int iCount)
{
	SCPP65Type*			psType;
	SCPP65TypeChain*	psChain;

	//The types of this kind and base, in the order they were made: the first that fits, as a search of them all.
	psChain = mcByBase.Get(GetBaseKey(eKind, psBase));
	for (psType = psChain ? psChain->psFirst : NULL; psType; psType = psType->psNextSameBase)
	{
		if ((psType->eKind == eKind) && (psType->psBase == psBase) && (psType->iCount == iCount) && (psType->iBytes == iBytes))
		{
			return psType;
		}
	}

	psType = NewType(eKind, iBytes);
	psType->psBase = psBase;
	psType->iCount = iCount;
	AddType(psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Integer(int iBytes, bool bSigned)
{
	if (iBytes == msInt.iBytes)
	{
		return bSigned ? &msInt : &msUnsignedInt;
	}
	switch (iBytes)
	{
	case 1:
		return bSigned ? &msChar : &msUnsignedChar;
	case 2:
		return bSigned ? &msShort : &msUnsignedShort;
	case 8:
		return bSigned ? &msLongLong : &msUnsignedLongLong;
	default:
		return bSigned ? &msLong : &msUnsignedLong;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// long: 4 bytes (int itself when int is 4 bytes).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Long(bool bSigned)
{
	return Integer(4, bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// sizeof's type: unsigned int, or unsigned long long where pointers are 8
// bytes (the host's size_t).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Size(void)
{
	return (miPointerBytes == 8) ? &msUnsignedLongLong : &msUnsignedInt;
}


//////////////////////////////////////////////////////////////////////////
//
// psType's unqualified type with const and volatile as asked: a copy of it,
// kept with the types (one for each), that is const (can't be written
// through) or volatile (its loads and stores are kept and in order).
// Functions and references can't be qualified, and an array's elements are.
// [basic.type.qualifier]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Qualified(SCPP65Type* psType, bool bConst, bool bVolatile)
{
	SCPP65Type*		psUnqualified;
	SCPP65Type*		psQualified;
	SCPP65Type**	ppsFound;
	int64			lliKey;

	if (!psType || psType->IsFunction() || psType->IsReference())
	{
		return psType;
	}
	if (psType->IsArray())
	{
		//An array's qualifiers are its elements', which keep their own.  [basic.type.qualifier] 5
		return Array(Qualified(psType->psBase, bConst || psType->psBase->bConst, bVolatile || psType->psBase->bVolatile), psType->iCount);
	}
	psUnqualified = psType->psUnqualified;
	if (!bConst && !bVolatile)
	{
		return psUnqualified;
	}
	lliKey = QualifiedKey(psUnqualified, bConst, bVolatile);
	ppsFound = mcQualified.Get(lliKey);
	if (ppsFound)
	{
		return *ppsFound;
	}
	psQualified = NewType(psUnqualified->eKind, psUnqualified->iBytes);
	*psQualified = *psUnqualified;
	psQualified->papsParameters = NULL;
	psQualified->bConst = bConst;
	psQualified->bVolatile = bVolatile;
	psQualified->psUnqualified = psUnqualified;
	AddType(psQualified);
	mcQualified.Put(lliKey, &psQualified);
	return psQualified;
}


//////////////////////////////////////////////////////////////////////////
//
// const psType (with its volatile).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Const(SCPP65Type* psType)
{
	return psType ? Qualified(psType, true, psType->bVolatile) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// volatile psType (with its const).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Volatile(SCPP65Type* psType)
{
	return psType ? Qualified(psType, psType->bConst, true) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A class type's size, once it is laid out: its const and volatile copies
// (made when it was named, perhaps before) get it too.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::SetClassBytes(SCPP65Type* psType, int iBytes)
{
	SCPP65Type**	ppsQualified;
	int				iQualifiers;

	psType = psType->psUnqualified;
	psType->iBytes = iBytes;
	for (iQualifiers = 1; iQualifiers < 4; iQualifiers++)
	{
		ppsQualified = mcQualified.Get(QualifiedKey(psType, (iQualifiers & 1) != 0, (iQualifiers & 2) != 0));
		if (ppsQualified)
		{
			(*ppsQualified)->iBytes = iBytes;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The key of the qualified copy of psUnqualified (QualifiedKey's two bits
// below the pointer, which types are aligned to more than).
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Types::QualifiedKey(SCPP65Type* psUnqualified, bool bConst, bool bVolatile)
{
	return ((int64)(size)psUnqualified << 2) | (bConst ? 1 : 0) | (bVolatile ? 2 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Pointer(SCPP65Type* psBase)
{
	return Add(CPP65TK_Pointer, miPointerBytes, psBase, 0);
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
	SCPP65Type*			psType;
	SCPP65Type*			psOther;
	SCPP65TypeChain*	psChain;
	int64				lliKey;
	size				i;

	psType = NewType(CPP65TK_Function, 0);
	psType->psBase = psReturn;
	psType->bVariadic = bVariadic;
	psType->papsParameters = NewMalloc<CArrayTemplatePtr<SCPP65Type> >();
	psType->papsParameters->Init();
	for (i = 0; papsParameters && (i < papsParameters->NumElements()); i++)
	{
		psType->papsParameters->Add(papsParameters->GetPtr(i));
	}

	//The same function type is shared.  Equal types hash the same, so the first equal one is on its hash's chain.
	lliKey = (int64)HashStructure(psType);
	psChain = mcFunctions.Get(lliKey);
	for (psOther = psChain ? psChain->psFirst : NULL; psOther; psOther = psOther->psNextSameSignature)
	{
		if (psOther->Equals(psType))
		{
			psType->papsParameters->Kill();
			SafeFree(psType->papsParameters);
			SafeFree(psType);
			return psOther;
		}
	}
	AddType(psType);
	AppendToChain(&mcFunctions, lliKey, psType, &SCPP65Type::psNextSameSignature);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Reference(SCPP65Type* psBase)
{
	return Add(CPP65TK_Reference, miPointerBytes, psBase, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer to a member of pcClass whose type is psMember.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::MemberPointer(CCPPClass* pcClass, SCPP65Type* psMember)
{
	SCPP65Type*			psType;
	SCPP65TypeChain*	psChain;

	psChain = mcByBase.Get(GetBaseKey(CPP65TK_MemberPointer, psMember));
	for (psType = psChain ? psChain->psFirst : NULL; psType; psType = psType->psNextSameBase)
	{
		if (psType->pcClass == pcClass)
		{
			return psType;
		}
	}

	psType = NewType(CPP65TK_MemberPointer, miPointerBytes);
	psType->psBase = psMember;
	psType->pcClass = pcClass;
	AddType(psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// An enum: an integer of its underlying type's size that is a type of its own, for
// overloads and templates.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Enum(CCPPSymbol* pcEnum, int iBytes, bool bSigned)
{
	SCPP65Type*	psType;

	psType = FindEnum(pcEnum);
	if (psType)
	{
		return psType;
	}

	psType = NewType(CPP65TK_Integer, iBytes);
	psType->bSigned = bSigned;
	psType->pcEnum = pcEnum;
	AddType(psType);
	mcEnums.Put(GetPointerKey(pcEnum), &psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The enum's type if it has been made, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::FindEnum(CCPPSymbol* pcEnum)
{
	return GetMapped(&mcEnums, pcEnum);
}


//////////////////////////////////////////////////////////////////////////
//
// float (4), double and long double (8, as MSVC).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Float(int iBytes)
{
	return (iBytes == 4) ? &msFloat : &msDouble;
}


//////////////////////////////////////////////////////////////////////////
//
// long double is a double (as MSVC) but overloads apart from it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::LongDouble(void)
{
	return &msLongDouble;
}


//////////////////////////////////////////////////////////////////////////
//
// A class's type.  Its size is filled in when the class is laid out.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Class(CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	psType = GetMapped(&mcClasses, pcClass);
	if (psType)
	{
		return psType;
	}

	psType = NewType(CPP65TK_Class, 0);
	psType->pcClass = pcClass;
	AddType(psType);
	mcClasses.Put(GetPointerKey(pcClass), &psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Integral promotion: bool, char and short become int.  [conv.prom]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Promote(SCPP65Type* psType)
{
	psType = psType->Unqualified();
	if (psType->IsInteger() && ((psType->iBytes < msInt.iBytes) || psType->pcEnum))
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
	if (!psLeft->IsInteger() || !psRight->IsInteger())
	{
		return (!psLeft->bSigned || !psRight->bSigned) ? &msUnsignedInt : &msInt;
	}

	//The wider type, unsigned when it is or when both are the same width and either is.  (A long holds every
	//unsigned int where int is narrower.)
	if (psLeft->iBytes != psRight->iBytes)
	{
		return (psLeft->iBytes > psRight->iBytes) ? Integer(psLeft->iBytes, psLeft->bSigned) : Integer(psRight->iBytes, psRight->bSigned);
	}
	return Integer(psLeft->iBytes, psLeft->bSigned && psRight->bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a floating point operation: long double if either operand is, then
// double, otherwise float (an integer operand takes the other's type).  [expr] 10
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::FloatArithmetic(SCPP65Type* psLeft, SCPP65Type* psRight)
{
	if ((psLeft && psLeft->bLongDouble) || (psRight && psRight->bLongDouble))
	{
		return LongDouble();
	}
	if ((psLeft && psLeft->IsFloat() && (psLeft->iBytes == 8)) || (psRight && psRight->IsFloat() && (psRight->iBytes == 8)))
	{
		return Float(8);
	}
	return Float(4);
}


//////////////////////////////////////////////////////////////////////////
//
// ptrdiff_t, pointer - pointer: a signed integer as wide as a pointer (int on the
// 65C64, long on the W65C816).  [expr.add] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::PointerDifference(void)
{
	return Integer(Pointer(&msVoid)->iBytes, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of left op right for the arithmetic, bitwise and shift operators, or NULL
// when the operator doesn't take these operands.  Floats: + - * and / at the wider
// float type (an integer operand is converted to it).  Pointers: pointer + integer,
// integer + pointer and pointer - integer are the pointer, pointer - pointer their
// difference.  Integers: the usual arithmetic conversions, and a shift is its left
// operand's promoted type.  [expr.mul] [expr.add] [expr.shift] [expr.bit.and]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Operation(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight)
{
	if (psLeft->IsFloat() || psRight->IsFloat())
	{
		if (((eOperator != CT_Add) && (eOperator != CT_Subtract) && (eOperator != CT_Multiply) && (eOperator != CT_Divide)) ||
			!(psLeft->IsInteger() || psLeft->IsFloat()) || !(psRight->IsInteger() || psRight->IsFloat()))
		{
			return NULL;
		}
		return FloatArithmetic(psLeft, psRight);
	}

	if (psLeft->IsPointer() || psRight->IsPointer())
	{
		if (((eOperator == CT_Add) || (eOperator == CT_Subtract)) && psLeft->IsPointer() && psRight->IsInteger())
		{
			return psLeft;
		}
		if ((eOperator == CT_Add) && psLeft->IsInteger() && psRight->IsPointer())
		{
			return psRight;
		}
		if ((eOperator == CT_Subtract) && psLeft->IsPointer() && psRight->IsPointer())
		{
			return PointerDifference();
		}
		return NULL;
	}

	if (!psLeft->IsInteger() || !psRight->IsInteger())
	{
		return NULL;
	}
	switch (eOperator)
	{
	case CT_LeftShift:
	case CT_RightShift:
		return Promote(psLeft);
	case CT_Add:
	case CT_Subtract:
	case CT_Multiply:
	case CT_Divide:
	case CT_Modulus:
	case CT_BitwiseAnd:
	case CT_BitwiseOr:
	case CT_BitwiseXor:
		return Arithmetic(psLeft, psRight);
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y for scalar (or void) x and y: a float with a number is the
// float arithmetic's type, the same enum stays that enum, integers take the usual
// arithmetic conversions, a void side makes it void, and otherwise it is the
// pointer's type.  [expr.cond]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Conditional(SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	if ((psTrue->IsFloat() && psFalse->IsScalar() && !psFalse->IsPointer()) || (psFalse->IsFloat() && psTrue->IsScalar() && !psTrue->IsPointer()))
	{
		return FloatArithmetic(psTrue, psFalse);
	}
	if ((psTrue == psFalse) && psTrue->pcEnum)
	{
		return psTrue;
	}
	if (psTrue->IsInteger() && psFalse->IsInteger())
	{
		return Arithmetic(psTrue, psFalse);
	}
	if (psTrue->IsVoid() || psFalse->IsVoid())
	{
		return &msVoid;
	}
	return psTrue->IsPointer() ? psTrue : psFalse;
}
