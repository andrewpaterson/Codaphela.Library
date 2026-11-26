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
bool SCPP65Type::IsInteger(void) const
{
	return (eKind == CPP65TK_Integer) || (eKind == CPP65TK_Bool);
}


//////////////////////////////////////////////////////////////////////////
//
// A value that fits in a register.
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsScalar(void) const
{
	return IsInteger() || (eKind == CPP65TK_Pointer) || (eKind == CPP65TK_Float) || (eKind == CPP65TK_MemberPointer) || (eKind == CPP65TK_NullPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsPointer(void) const
{
	return eKind == CPP65TK_Pointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsMemberPointer(void) const
{
	return eKind == CPP65TK_MemberPointer;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer to a member function: the function's address and, above it,
// what is added to the object's address for the function's this (the
// function may be a base's, further on in the object).  A pointer to a
// data member is the member's offset alone (CPP_MEMBER_POINTER_BIAS).
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsMemberFunctionPointer(void) const
{
	return (eKind == CPP65TK_MemberPointer) && psBase->IsFunction();
}


//////////////////////////////////////////////////////////////////////////
//
// What a function type's name has after its parameters: the bracket, and a
// member function's const and volatile.
//
//////////////////////////////////////////////////////////////////////////
const char* SCPP65Type::FunctionEnd(void) const
{
	if (bVolatileFunction)
	{
		return bConstFunction ? ") const volatile" : ") volatile";
	}
	return bConstFunction ? ") const" : ")";
}


//////////////////////////////////////////////////////////////////////////
//
// std::nullptr_t (const or not): nullptr's type.  It isn't a pointer
// (IsPointer), though its one value is a null pointer and it is as wide.
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsNullPointer(void) const
{
	return eKind == CPP65TK_NullPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsArray(void) const
{
	return eKind == CPP65TK_Array;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsFunction(void) const
{
	return eKind == CPP65TK_Function;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsVoid(void) const
{
	return eKind == CPP65TK_Void;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsClass(void) const
{
	return eKind == CPP65TK_Class;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsReference(void) const
{
	return eKind == CPP65TK_Reference;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsRvalueReference(void) const
{
	return (eKind == CPP65TK_Reference) && bRvalue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsLvalueReference(void) const
{
	return (eKind == CPP65TK_Reference) && !bRvalue;
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
// An array's element; any other type is itself.  One level: an array of
// arrays' element is an array.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* SCPP65Type::ElementType(void)
{
	return IsArray() ? psBase : this;
}


//////////////////////////////////////////////////////////////////////////
//
// An array's element however many arrays deep: an array of arrays of int's
// is int.  Any other type is itself.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* SCPP65Type::InnermostElement(void)
{
	SCPP65Type*	psType;

	psType = this;
	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The least and the most an integer type of under 8 bytes holds.
//
//////////////////////////////////////////////////////////////////////////
int64 SCPP65Type::Least(void) const
{
	return CPPIntegerLeast(iBytes, bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 SCPP65Type::Most(void) const
{
	return CPPIntegerMost(iBytes, bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsFloat(void) const
{
	return eKind == CPP65TK_Float;
}


//////////////////////////////////////////////////////////////////////////
//
// A scalar the generator can work with: up to 8 bytes (floating point is the soft
// float runtime's).
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsSupported(void) const
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
bool SCPP65Type::IsWide(void) const
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
	if ((eKind != psOther->eKind) || (iBytes != psOther->iBytes) || (bSigned != psOther->bSigned) || (eInteger != psOther->eInteger) || (iCount != psOther->iCount) || (pcClass != psOther->pcClass) || (bLongDouble != psOther->bLongDouble) || (pcEnum != psOther->pcEnum) || (bRvalue != psOther->bRvalue))
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
		if ((bVariadic != psOther->bVariadic) || (bConstFunction != psOther->bConstFunction) || (bVolatileFunction != psOther->bVolatileFunction) || (papsParameters->NumElements() != psOther->papsParameters->NumElements()))
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
// An integer type's name as C++ writes it: "unsigned short", "char16_t".
//
//////////////////////////////////////////////////////////////////////////
const char* IntegerKindName(ECPP65IntegerKind eInteger)
{
	switch (eInteger)
	{
	case CPP65IK_Char:				return "char";
	case CPP65IK_SignedChar:			return "signed char";
	case CPP65IK_UnsignedChar:			return "unsigned char";
	case CPP65IK_WChar:				return "wchar_t";
	case CPP65IK_Char16:				return "char16_t";
	case CPP65IK_Char32:				return "char32_t";
	case CPP65IK_Short:				return "short";
	case CPP65IK_UnsignedShort:			return "unsigned short";
	case CPP65IK_Int:				return "int";
	case CPP65IK_UnsignedInt:			return "unsigned int";
	case CPP65IK_Long:				return "long";
	case CPP65IK_UnsignedLong:			return "unsigned long";
	case CPP65IK_LongLong:			return "long long";
	case CPP65IK_UnsignedLongLong:		return "unsigned long long";
	default:						return "int";
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type as C++ for messages, its integers as declared (IntegerKindName).
// Classes and enums aren't named.
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
	case CPP65TK_NullPointer:
		psz->Append("std::nullptr_t");
		break;
	case CPP65TK_Integer:
		if (pcEnum)
		{
			psz->Append("enum");
			break;
		}
		psz->Append(IntegerKindName(eInteger));
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
		psz->Append(bRvalue ? "&&" : "&");
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
		psz->Append(FunctionEnd());
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
// Whether it is an enum class's type, const or not.  [dcl.enum] 10
//
//////////////////////////////////////////////////////////////////////////
bool SCPP65Type::IsScopedEnum(void) const
{
	return (pcEnum != NULL) && (bScopedEnum || psUnqualified->bScopedEnum);
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
int SCPP65Type::GetElementBytes(void) const
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
	mcByBase.Init();
	mcFunctions.Init();
	mcClasses.Init();
	mcEnums.Init();
	mcQualified.Init();
	InitBuiltin(&msVoid, CPP65TK_Void, 0, false);
	InitBuiltin(&msUndeduced, CPP65TK_Void, 0, false);
	InitBuiltin(&msBool, CPP65TK_Bool, 1, false);
	InitBuiltin(&msNullPointer, CPP65TK_NullPointer, iPointerBytes, false);
	InitInteger(&msChar, CPP65IK_Char, 1, true);
	InitInteger(&msSignedChar, CPP65IK_SignedChar, 1, true);
	InitInteger(&msUnsignedChar, CPP65IK_UnsignedChar, 1, false);
	InitInteger(&msWChar, CPP65IK_WChar, 2, false);
	InitInteger(&msChar16, CPP65IK_Char16, 2, false);
	InitInteger(&msChar32, CPP65IK_Char32, 4, false);
	InitInteger(&msShort, CPP65IK_Short, 2, true);
	InitInteger(&msUnsignedShort, CPP65IK_UnsignedShort, 2, false);
	InitInteger(&msInt, CPP65IK_Int, iIntBytes, true);
	InitInteger(&msUnsignedInt, CPP65IK_UnsignedInt, iIntBytes, false);
	InitInteger(&msLong, CPP65IK_Long, 4, true);
	InitInteger(&msUnsignedLong, CPP65IK_UnsignedLong, 4, false);
	InitInteger(&msLongLong, CPP65IK_LongLong, 8, true);
	InitInteger(&msUnsignedLongLong, CPP65IK_UnsignedLongLong, 8, false);
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
void CCPP65Types::InitInteger(SCPP65Type* psType, ECPP65IntegerKind eInteger, int iBytes, bool bSigned)
{
	InitBuiltin(psType, CPP65TK_Integer, iBytes, bSigned);
	psType->eInteger = eInteger;
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
	psType->eInteger = CPP65IK_None;
	psType->psBase = NULL;
	psType->iCount = 0;
	psType->pcClass = NULL;
	psType->papsParameters = NULL;
	psType->bVariadic = false;
	psType->bConstFunction = false;
	psType->bVolatileFunction = false;
	psType->bLongDouble = false;
	psType->bRvalue = false;
	psType->pcEnum = NULL;
	psType->bScopedEnum = false;
	psType->bNonNegativeEnum = false;
	psType->bConst = false;
	psType->bVolatile = false;
	psType->psUnqualified = psType;
	psType->psNextSameBase = NULL;
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
// its kind and base (last on their chain, linked by psNextSameBase).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Types::AddType(SCPP65Type* psType)
{
	SCPP65TypeChain*	psChain;
	bool				bAdded;

	mapsTypes.Add(psType);
	psType->psNextSameBase = NULL;
	psChain = mcByBase.FindOrAdd(GetBaseKey(psType->eKind, psType->psBase), &bAdded);
	if (bAdded)
	{
		psChain->psFirst = psType;
	}
	else
	{
		psChain->psLast->psNextSameBase = psType;
	}
	psChain->psLast = psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type pcMap holds for pvKey, NULL if none.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::GetMapped(CHashMapLongTemplate<SCPP65Type*>* pcMap, void* pvKey)
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
		ulliHash = ulliHash * 31 + (uint64)psType->bConstFunction;
		ulliHash = ulliHash * 31 + (uint64)psType->bVolatileFunction;
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
SCPP65Type* CCPP65Types::Add(ECPP65TypeKind eKind, int iBytes, SCPP65Type* psBase, int iCount, bool bRvalue)
{
	SCPP65Type*			psType;
	SCPP65TypeChain*	psChain;

	//The types of this kind and base, in the order they were made: the first that fits, as a search of them all.
	psChain = mcByBase.Get(GetBaseKey(eKind, psBase));
	for (psType = psChain ? psChain->psFirst : NULL; psType; psType = psType->psNextSameBase)
	{
		if ((psType->eKind == eKind) && (psType->psBase == psBase) && (psType->iCount == iCount) && (psType->iBytes == iBytes) && (psType->bRvalue == bRvalue))
		{
			return psType;
		}
	}

	psType = NewType(eKind, iBytes);
	psType->psBase = psBase;
	psType->iCount = iCount;
	psType->bRvalue = bRvalue;
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
	return bSigned ? &msLong : &msUnsignedLong;
}


//////////////////////////////////////////////////////////////////////////
//
// The integer type of the kind.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::IntegerOfKind(ECPP65IntegerKind eInteger)
{
	switch (eInteger)
	{
	case CPP65IK_Char:				return &msChar;
	case CPP65IK_SignedChar:			return &msSignedChar;
	case CPP65IK_UnsignedChar:			return &msUnsignedChar;
	case CPP65IK_WChar:				return &msWChar;
	case CPP65IK_Char16:				return &msChar16;
	case CPP65IK_Char32:				return &msChar32;
	case CPP65IK_Short:				return &msShort;
	case CPP65IK_UnsignedShort:			return &msUnsignedShort;
	case CPP65IK_Int:				return &msInt;
	case CPP65IK_UnsignedInt:			return &msUnsignedInt;
	case CPP65IK_Long:				return &msLong;
	case CPP65IK_UnsignedLong:			return &msUnsignedLong;
	case CPP65IK_LongLong:			return &msLongLong;
	case CPP65IK_UnsignedLongLong:		return &msUnsignedLongLong;
	default:						return NULL;
	}
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
// A function type.  The parameters are copied.  bConstFunction and
// bVolatileFunction are a member function's const and volatile, after its
// parameters, which make it another type: a pointer to a member function
// that is const isn't one to a function that isn't.  [dcl.fct] 6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Function(SCPP65Type* psReturn, CArrayTemplatePtr<SCPP65Type>* papsParameters, bool bVariadic, bool bConstFunction, bool bVolatileFunction)
{
	SCPP65Type*		psType;
	SCPP65Type**	ppsOther;
	size			i;

	psType = NewType(CPP65TK_Function, 0);
	psType->psBase = psReturn;
	psType->bVariadic = bVariadic;
	psType->bConstFunction = bConstFunction;
	psType->bVolatileFunction = bVolatileFunction;
	psType->papsParameters = NewMalloc<CArrayTemplatePtr<SCPP65Type> >();
	psType->papsParameters->Init();
	for (i = 0; papsParameters && (i < papsParameters->NumElements()); i++)
	{
		psType->papsParameters->Add(papsParameters->GetPtr(i));
	}

	//The same function type is shared.
	ppsOther = mcFunctions.Get(&psType);
	if (ppsOther)
	{
		psType->papsParameters->Kill();
		SafeFree(psType->papsParameters);
		SafeFree(psType);
		return *ppsOther;
	}
	AddType(psType);
	mcFunctions.Add(&psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65Types::SHashOfFunction::Hash(SCPP65Type* const* ppsType)
{
	return HashMix64(HashStructure(*ppsType));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
HashEquals CCPP65Types::SHashOfFunction::Equals(void)
{
	return &SHashOfFunction::TypesEqual;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Types::SHashOfFunction::TypesEqual(const void* pvKey, const void* pvEntryKey)
{
	return (*(SCPP65Type* const*)pvKey)->Equals(*(SCPP65Type* const*)pvEntryKey);
}


//////////////////////////////////////////////////////////////////////////
//
// A reference to psBase: T& or, with bRvalue, T&&.  A reference to a
// reference is one reference: an rvalue reference only if both are, as
// T&& is int& when T is int&.  [dcl.ref] 6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Reference(SCPP65Type* psBase, bool bRvalue)
{
	if (psBase->IsReference())
	{
		bRvalue = bRvalue && psBase->bRvalue;
		psBase = psBase->psBase;
	}
	return Add(CPP65TK_Reference, miPointerBytes, psBase, 0, bRvalue);
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

	psType = NewType(CPP65TK_MemberPointer, psMember->IsFunction() ? 2 * miPointerBytes : miPointerBytes);
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
SCPP65Type* CCPP65Types::Enum(CCPPSymbol* pcEnum, SCPP65Type* psUnderlying, bool bScoped)
{
	SCPP65Type*	psType;

	psType = FindEnum(pcEnum);
	if (psType)
	{
		return psType;
	}

	psType = NewType(CPP65TK_Integer, psUnderlying->iBytes);
	psType->bSigned = psUnderlying->bSigned;
	psType->eInteger = psUnderlying->eInteger;
	psType->pcEnum = pcEnum;
	psType->bScopedEnum = bScoped;
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
// Integral promotion: bool, the chars, short and unsigned short become int,
// or unsigned int if int can't hold all their values (unsigned short where
// int is 16 bits); wchar_t, char16_t, char32_t and an unscoped enum the
// first of int, unsigned int, long, unsigned long, long long and unsigned
// long long that holds all theirs; an enum as its underlying type.
// [conv.prom]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Promote(SCPP65Type* psType)
{
	psType = psType->Unqualified();
	if (!psType->IsInteger())
	{
		return psType;
	}
	if (psType->pcEnum)
	{
		return psType->eInteger ? Promote(IntegerOfKind(psType->eInteger)) : PromoteToHold(psType->iBytes, psType->bSigned);
	}
	switch (psType->eKind == CPP65TK_Bool ? CPP65IK_UnsignedChar : psType->eInteger)
	{
	case CPP65IK_Char:
	case CPP65IK_SignedChar:
	case CPP65IK_UnsignedChar:
	case CPP65IK_Short:
	case CPP65IK_UnsignedShort:
	case CPP65IK_WChar:
	case CPP65IK_Char16:
	case CPP65IK_Char32:
		return PromoteToHold(psType->iBytes, psType->bSigned);
	default:
		return psType;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Integral promotion of a bit-field iBits wide that is declared as psType:
// int if an int holds every value it can have, unsigned int if that does,
// and otherwise its own type.  An enum's bit-field is promoted as the enum
// is.  [conv.prom] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::PromoteBitField(SCPP65Type* psType, int iBits)
{
	int	iIntBits;

	iIntBits = msInt.iBytes * 8;
	if (psType->pcEnum || (iBits > iIntBits))
	{
		return Promote(psType);
	}
	return ((iBits < iIntBits) || psType->bSigned) ? &msInt : &msUnsignedInt;
}


//////////////////////////////////////////////////////////////////////////
//
// The first of int, unsigned int, long, unsigned long, long long and
// unsigned long long that holds every value of an integer iBytes wide (and
// bSigned): a promotion's type.  [conv.prom]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::PromoteToHold(int iBytes, bool bSigned)
{
	SCPP65Type*	apsOrder[] = { &msInt, &msUnsignedInt, &msLong, &msUnsignedLong, &msLongLong, &msUnsignedLongLong };
	SCPP65Type*	psTo;
	size		i;

	for (i = 0; i < sizeof(apsOrder) / sizeof(SCPP65Type*); i++)
	{
		psTo = apsOrder[i];
		if (psTo->bSigned ? (bSigned ? (psTo->iBytes >= iBytes) : (psTo->iBytes > iBytes)) : (!bSigned && (psTo->iBytes >= iBytes)))
		{
			return psTo;
		}
	}
	return &msUnsignedLongLong;
}


//////////////////////////////////////////////////////////////////////////
//
// The usual arithmetic conversions of two integers, each promoted: the same
// type stays it; of two both signed or both unsigned, the higher rank's
// (long long, long, int); else the unsigned one if its rank is as high,
// the signed one if it holds all the unsigned one's values, or the
// unsigned type of the signed one's rank.  [expr] 10, [conv.rank]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Types::Arithmetic(SCPP65Type* psLeft, SCPP65Type* psRight)
{
	SCPP65Type*	psSigned;
	SCPP65Type*	psUnsigned;

	psLeft = Promote(psLeft);
	psRight = Promote(psRight);
	if (!psLeft->IsInteger() || !psRight->IsInteger())
	{
		return (!psLeft->bSigned || !psRight->bSigned) ? &msUnsignedInt : &msInt;
	}
	if (psLeft->Equals(psRight))
	{
		return psLeft;
	}
	if (psLeft->bSigned == psRight->bSigned)
	{
		return (IntegerRank(psLeft) >= IntegerRank(psRight)) ? psLeft : psRight;
	}
	psSigned = psLeft->bSigned ? psLeft : psRight;
	psUnsigned = psLeft->bSigned ? psRight : psLeft;
	if (IntegerRank(psUnsigned) >= IntegerRank(psSigned))
	{
		return psUnsigned;
	}
	if (psSigned->iBytes > psUnsigned->iBytes)
	{
		return psSigned;
	}
	return (psSigned->eInteger == CPP65IK_LongLong) ? &msUnsignedLongLong : ((psSigned->eInteger == CPP65IK_Long) ? &msUnsignedLong : &msUnsignedInt);
}


//////////////////////////////////////////////////////////////////////////
//
// A promoted integer's rank: int's 3, long's 4, long long's 5 (signed or
// not).  [conv.rank]
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Types::IntegerRank(SCPP65Type* psType)
{
	switch (psType->eInteger)
	{
	case CPP65IK_LongLong:
	case CPP65IK_UnsignedLongLong:
		return 5;
	case CPP65IK_Long:
	case CPP65IK_UnsignedLong:
		return 4;
	case CPP65IK_Short:
	case CPP65IK_UnsignedShort:
		return 2;
	case CPP65IK_Char:
	case CPP65IK_SignedChar:
	case CPP65IK_UnsignedChar:
		return 1;
	default:
		return 3;
	}
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
// 65C64, long on the W65C816), Libc65's std::ptrdiff_t.  [expr.add] 5
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
