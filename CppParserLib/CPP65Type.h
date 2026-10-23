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


enum ECPP65TypeKind
{
	CPP65TK_Void,
	CPP65TK_Integer,
	CPP65TK_Bool,
	CPP65TK_Pointer,
	CPP65TK_Array,
	CPP65TK_Function,
	CPP65TK_Class,
};


struct SCPP65Class;


//The type of a value the 65C64 generator works with.  Integers are 1, 2 or 4 bytes (int and long are 4 bytes, as
//on Windows), pointers are 4 bytes.  A class type's layout is in psClass.  Types are made by CCPP65Types and are
//never freed while it lives.
struct SCPP65Type
{
	ECPP65TypeKind	eKind;
	int				iBytes;
	bool			bSigned;
	SCPP65Type*		psBase;		//The type pointed to, the element type or the return type.
	int				iCount;		//Array elements.
	SCPP65Class*	psClass;

	bool	IsInteger(void);
	bool	IsScalar(void);
	bool	IsPointer(void);
	bool	IsArray(void);
	bool	IsFunction(void);
	bool	IsVoid(void);
	bool	IsClass(void);
	bool	Equals(SCPP65Type* psOther);
	int		GetElementBytes(void);
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
	SCPP65Type			msInt;
	SCPP65Type			msUnsignedInt;

	void				Init(void);
	void				Kill(void);

	SCPP65Type*			Integer(int iBytes, bool bSigned);
	SCPP65Type*			Pointer(SCPP65Type* psBase);
	SCPP65Type*			Array(SCPP65Type* psElement, int iCount);
	SCPP65Type*			Function(SCPP65Type* psReturn);
	SCPP65Type*			Class(SCPP65Class* psClass);
	SCPP65Type*			Promote(SCPP65Type* psType);
	SCPP65Type*			Arithmetic(SCPP65Type* psLeft, SCPP65Type* psRight);

protected:
	void				InitBuiltin(SCPP65Type* psType, ECPP65TypeKind eKind, int iBytes, bool bSigned);
	SCPP65Type*			Add(ECPP65TypeKind eKind, int iBytes, SCPP65Type* psBase, int iCount);
};


#endif // __C_P_P_65_TYPE_H__
