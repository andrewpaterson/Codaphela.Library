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
#ifndef __C_P_P_65_TYPE_H__
#define __C_P_P_65_TYPE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/HashSetTemplate.h"
#include "BaseLib/Chars.h"
#include "CppTokens.h"


enum ECPP65TypeKind
{
	CPP65TK_Void,
	CPP65TK_Integer,
	CPP65TK_Bool,
	CPP65TK_Pointer,
	CPP65TK_Array,
	CPP65TK_Function,
	CPP65TK_Class,
	CPP65TK_Float,
	CPP65TK_Reference,
	CPP65TK_MemberPointer,
	CPP65TK_NullPointer,	//std::nullptr_t: nullptr's type, a pointer wide, that is no pointer.  [basic.fundamental] 10
};


//Which integer type an integer that isn't an enum is.  Two of one size and signedness are different types all the same:
//int and long on the 65C64, short and int on the W65C816, char and signed char everywhere.  [basic.fundamental]
enum ECPP65IntegerKind
{
	CPP65IK_None,
	CPP65IK_Char,
	CPP65IK_SignedChar,
	CPP65IK_UnsignedChar,
	CPP65IK_WChar,
	CPP65IK_Char16,
	CPP65IK_Char32,
	CPP65IK_Short,
	CPP65IK_UnsignedShort,
	CPP65IK_Int,
	CPP65IK_UnsignedInt,
	CPP65IK_Long,
	CPP65IK_UnsignedLong,
	CPP65IK_LongLong,
	CPP65IK_UnsignedLongLong,
};


const char*	IntegerKindName(ECPP65IntegerKind eInteger);


class CCPPClass;
class CCPPSymbol;


//The type of a value the compiler works with.  Integers are 1, 2, 4 or 8 bytes (int is 4 bytes on the 65C64 and 2
//on the W65C816, long is 4 on both), each a type of its own however wide (eInteger), pointers and references are 4 bytes.  A pointer to a member of pcClass (psBase
//is the member's type) is 4 bytes: a member function's address or a field's offset.  float is 4 bytes, double and
//long double 8.  A class type's layout is in pcClass.  A function type has its parameters.  Types are made by
//CCPP65Types and are never freed while it lives.
struct SCPP65Type
{
	ECPP65TypeKind	eKind;
	int				iBytes;
	bool			bSigned;
	ECPP65IntegerKind	eInteger;	//An integer: which one (CCPP65Types' builtins), an enum's underlying one; else CPP65IK_None.
	SCPP65Type*		psBase;		//The type pointed to, the element type or the return type.
	int				iCount;		//Array elements.
	CCPPClass*	pcClass;
	CArrayTemplatePtr<SCPP65Type>*	papsParameters;		//A function type's parameters.
	bool			bVariadic;
	bool			bConstFunction;	//A function type: a member function's that is const (int() const), a type of its own.  [dcl.fct] 6
	bool			bVolatileFunction;	//And one that is volatile (int() volatile).
	bool			bLongDouble;	//long double: a double, but a type of its own.
	bool			bRvalue;		//A reference: T&&, which binds to what has no name or is about to go; else T&.
	CCPPSymbol*		pcEnum;			//An enum: an integer (int unless its values need more), but a type of its own.
	bool			bScopedEnum;	//...an enum class: no value of it becomes another type, or another type's it, without a cast.
	bool			bNonNegativeEnum;	//...an enum with no fixed type and no negative value: a bit-field of it is unsigned.  [class.bit] 4
	bool			bConst;			//const T: a copy of psUnqualified that can't be written through.
	bool			bVolatile;		//volatile T: a copy of psUnqualified that loads and stores can't remove or move.
	SCPP65Type*		psUnqualified;	//The type without const and volatile (itself when it is neither).
	SCPP65Type*		psNextSameBase;	//The next type made of its kind and base (CCPP65Types::mcByBase), NULL if none.

	bool	IsInteger(void) const;
	bool	IsScalar(void) const;
	bool	IsPointer(void) const;
	bool	IsArray(void) const;
	bool	IsFunction(void) const;
	bool	IsVoid(void) const;
	bool	IsClass(void) const;
	bool	IsReference(void) const;
	bool	IsRvalueReference(void) const;
	bool	IsLvalueReference(void) const;
	bool	IsMemberPointer(void) const;
	bool	IsMemberFunctionPointer(void) const;
	const char*	FunctionEnd(void) const;
	bool	IsNullPointer(void) const;
	bool	IsFloat(void) const;
	bool	IsSupported(void) const;
	bool	IsWide(void) const;
	bool	Equals(SCPP65Type* psOther);
	bool	HasQualifiersOf(SCPP65Type* psFrom);
	bool	Same(SCPP65Type* psOther);
	SCPP65Type*	Unqualified(void);
	bool		IsScopedEnum(void) const;
	SCPP65Type*	WithoutReference(void);
	SCPP65Type*	ElementType(void);
	int64		Least(void) const;
	int64		Most(void) const;
	int		GetElementBytes(void) const;
	void	Append(CChars* psz);
};


typedef CArrayTemplatePtr<SCPP65Type>	CArrayCPP65Types;


//The least and the most an integer of iBytes bytes holds, signed or not.  For under 8 bytes: an int64 has no room for
//the most of an unsigned 8.
inline int64 CPPIntegerLeast(int iBytes, bool bSigned)
{
	return bSigned ? -((int64)1 << (iBytes * 8 - 1)) : 0;
}


inline int64 CPPIntegerMost(int iBytes, bool bSigned)
{
	return bSigned ? ((int64)1 << (iBytes * 8 - 1)) - 1 : ((int64)1 << (iBytes * 8)) - 1;
}


//The first and last types made with one key, in the order they were made, linked by a next pointer.
struct SCPP65TypeChain
{
	SCPP65Type*	psFirst;
	SCPP65Type*	psLast;
};


//Makes and owns the types.  The built in types are shared.  A type is found again without searching them all: the
//maps give the first made of what is asked for, as a search in the order they were made would.
class CCPP65Types
{
protected:
	//A function type is found by what it is: hashed by HashStructure, compared by SCPP65Type::Equals.
	struct SHashOfFunction
	{
		static uint32		Hash(SCPP65Type* const* ppsType);
		static HashEquals	Equals(void);
		static bool			TypesEqual(const void* pvKey, const void* pvEntryKey);
	};

	CArrayCPP65Types					mapsTypes;
	CHashMapLongTemplate<SCPP65TypeChain>	mcByBase;		//The types made of a kind and base (Add, MemberPointer).
	CHashSetTemplate<SCPP65Type*, SHashOfFunction>	mcFunctions;	//The function types (Function), each made once.
	CHashMapLongTemplate<SCPP65Type*>		mcClasses;		//A class's type, by its CCPPClass.
	CHashMapLongTemplate<SCPP65Type*>		mcEnums;		//An enum's type, by its symbol.
	CHashMapLongTemplate<SCPP65Type*>		mcQualified;	//A type's const and volatile copies, by the unqualified type and the qualifiers (QualifiedKey).
	int									miPointerBytes;	//The target's pointers, references and pointers to members.

public:
	SCPP65Type			msVoid;
	SCPP65Type			msUndeduced;	//What auto f() returns until its definition says (CCPPUnit::ResolveReturn): void, but itself.
	SCPP65Type			msBool;
	SCPP65Type			msNullPointer;		//std::nullptr_t.
	SCPP65Type			msChar;				//Signed, but not signed char.
	SCPP65Type			msSignedChar;
	SCPP65Type			msUnsignedChar;
	SCPP65Type			msWChar;			//2 bytes, unsigned, as MSVC's.
	SCPP65Type			msChar16;
	SCPP65Type			msChar32;
	SCPP65Type			msShort;
	SCPP65Type			msUnsignedShort;
	SCPP65Type			msInt;				//The target's int: 4 bytes on the 65C64, 2 on the W65C816.
	SCPP65Type			msUnsignedInt;
	SCPP65Type			msLong;				//4 bytes: another type than int though int is as wide (the 65C64).
	SCPP65Type			msUnsignedLong;
	SCPP65Type			msLongLong;
	SCPP65Type			msUnsignedLongLong;
	SCPP65Type			msFloat;
	SCPP65Type			msDouble;
	SCPP65Type			msLongDouble;

	void				Init(int iIntBytes, int iPointerBytes);
	void				Kill(void);

	SCPP65Type*			Integer(int iBytes, bool bSigned);
	SCPP65Type*			Long(bool bSigned);
	SCPP65Type*			IntegerOfKind(ECPP65IntegerKind eInteger);
	SCPP65Type*			Size(void);
	SCPP65Type*			Pointer(SCPP65Type* psBase);
	SCPP65Type*			Array(SCPP65Type* psElement, int iCount);
	SCPP65Type*			Function(SCPP65Type* psReturn, CArrayTemplatePtr<SCPP65Type>* papsParameters, bool bVariadic, bool bConstFunction = false, bool bVolatileFunction = false);
	SCPP65Type*			Reference(SCPP65Type* psBase, bool bRvalue = false);
	SCPP65Type*			MemberPointer(CCPPClass* pcClass, SCPP65Type* psMember);
	SCPP65Type*			Enum(CCPPSymbol* pcEnum, SCPP65Type* psUnderlying, bool bScoped);
	SCPP65Type*			FindEnum(CCPPSymbol* pcEnum);
	SCPP65Type*			Float(int iBytes);
	SCPP65Type*			LongDouble(void);
	SCPP65Type*			Class(CCPPClass* pcClass);
	SCPP65Type*			Qualified(SCPP65Type* psType, bool bConst, bool bVolatile);
	SCPP65Type*			Const(SCPP65Type* psType);
	SCPP65Type*			Volatile(SCPP65Type* psType);
	void				SetClassBytes(SCPP65Type* psType, int iBytes);
	SCPP65Type*			Promote(SCPP65Type* psType);
	SCPP65Type*			PromoteBitField(SCPP65Type* psType, int iBits);
	SCPP65Type*			Arithmetic(SCPP65Type* psLeft, SCPP65Type* psRight);
	SCPP65Type*			FloatArithmetic(SCPP65Type* psLeft, SCPP65Type* psRight);
	SCPP65Type*			PointerDifference(void);
	SCPP65Type*			Operation(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight);

protected:
	void				InitBuiltin(SCPP65Type* psType, ECPP65TypeKind eKind, int iBytes, bool bSigned);
	void				InitInteger(SCPP65Type* psType, ECPP65IntegerKind eInteger, int iBytes, bool bSigned);
	SCPP65Type*			PromoteToHold(int iBytes, bool bSigned);
	int					IntegerRank(SCPP65Type* psType);
	SCPP65Type*			NewType(ECPP65TypeKind eKind, int iBytes);
	void				AddType(SCPP65Type* psType);
	static int64		GetBaseKey(ECPP65TypeKind eKind, SCPP65Type* psBase);
	static int64		GetPointerKey(void* pv);
	static int64		QualifiedKey(SCPP65Type* psUnqualified, bool bConst, bool bVolatile);
	static uint64		HashStructure(SCPP65Type* psType);
	static SCPP65Type*	GetMapped(CHashMapLongTemplate<SCPP65Type*>* pcMap, void* pvKey);
	SCPP65Type*			Add(ECPP65TypeKind eKind, int iBytes, SCPP65Type* psBase, int iCount, bool bRvalue = false);
};


#endif // __C_P_P_65_TYPE_H__
