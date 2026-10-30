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
#ifndef __C_P_P_65_TYPE_H__
#define __C_P_P_65_TYPE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Chars.h"


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
};


struct SCPP65Class;
class CCPPSymbol;


//The type of a value the 65C64 generator works with.  Integers are 1, 2, 4 or 8 bytes (int and long are 4 bytes,
//as on Windows), pointers and references are 4 bytes.  A pointer to a member of psClass (psBase is the member's
//type) is 4 bytes: a member function's address or a field's offset.  float is 4 bytes, double and long double 8.  A class type's layout is in psClass.  A function type has its parameters.  Types are made by
//CCPP65Types and are never freed while it lives.
struct SCPP65Type
{
	ECPP65TypeKind	eKind;
	int				iBytes;
	bool			bSigned;
	SCPP65Type*		psBase;		//The type pointed to, the element type or the return type.
	int				iCount;		//Array elements.
	SCPP65Class*	psClass;
	CArrayTemplatePtr<SCPP65Type>*	papsParameters;		//A function type's parameters.
	bool			bVariadic;
	bool			bLongDouble;	//long double: a double, but a type of its own.
	CCPPSymbol*		pcEnum;			//An enum: an integer (int unless its values need more), but a type of its own.

	bool	IsInteger(void);
	bool	IsScalar(void);
	bool	IsPointer(void);
	bool	IsArray(void);
	bool	IsFunction(void);
	bool	IsVoid(void);
	bool	IsClass(void);
	bool	IsReference(void);
	bool	IsMemberPointer(void);
	bool	IsFloat(void);
	bool	IsSupported(void);
	bool	IsWide(void);
	bool	Is64(void);
	bool	Equals(SCPP65Type* psOther);
	int		GetElementBytes(void);
	void	Append(CChars* psz);
};


typedef CArrayTemplatePtr<SCPP65Type>	CArrayCPP65Types;


//Makes and owns the types.  The built in types are shared.
class CCPP65Types
{
protected:
	CArrayCPP65Types	mapsTypes;

public:
	SCPP65Type			msVoid;
	SCPP65Type			msBool;
	SCPP65Type			msChar;
	SCPP65Type			msUnsignedChar;
	SCPP65Type			msShort;
	SCPP65Type			msUnsignedShort;
	SCPP65Type			msInt;				//The target's int: 4 bytes on the 65C64, 2 on the W65C816.
	SCPP65Type			msUnsignedInt;
	SCPP65Type			msLong;				//4 bytes.  The same type as int when int is 4 bytes.
	SCPP65Type			msUnsignedLong;
	SCPP65Type			msLongLong;
	SCPP65Type			msUnsignedLongLong;
	SCPP65Type			msFloat;
	SCPP65Type			msDouble;
	SCPP65Type			msLongDouble;

	void				Init(int iIntBytes = 4);
	void				Kill(void);

	SCPP65Type*			Integer(int iBytes, bool bSigned);
	SCPP65Type*			Long(bool bSigned);
	SCPP65Type*			Pointer(SCPP65Type* psBase);
	SCPP65Type*			Array(SCPP65Type* psElement, int iCount);
	SCPP65Type*			Function(SCPP65Type* psReturn, CArrayTemplatePtr<SCPP65Type>* papsParameters, bool bVariadic);
	SCPP65Type*			Reference(SCPP65Type* psBase);
	SCPP65Type*			MemberPointer(SCPP65Class* psClass, SCPP65Type* psMember);
	SCPP65Type*			Enum(CCPPSymbol* pcEnum, int iBytes, bool bSigned);
	SCPP65Type*			FindEnum(CCPPSymbol* pcEnum);
	SCPP65Type*			Float(int iBytes);
	SCPP65Type*			LongDouble(void);
	SCPP65Type*			Class(SCPP65Class* psClass);
	SCPP65Type*			Promote(SCPP65Type* psType);
	SCPP65Type*			Arithmetic(SCPP65Type* psLeft, SCPP65Type* psRight);

protected:
	void				InitBuiltin(SCPP65Type* psType, ECPP65TypeKind eKind, int iBytes, bool bSigned);
	SCPP65Type*			Add(ECPP65TypeKind eKind, int iBytes, SCPP65Type* psBase, int iCount);
};


#endif // __C_P_P_65_TYPE_H__
