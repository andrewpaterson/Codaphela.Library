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
#ifndef __CPP65_DEBUG_VALUES_H__
#define __CPP65_DEBUG_VALUES_H__
#include "BaseLib/Chars.h"
#include "Assembler65Lib/Asm65StackWalker.h"


#define CPP65_DEBUG_STRING_MOST		256		//The most characters of a char* or char array shown.


//How a value is shown, as a watch expression's format suffix says (Visual Studio's ,x ,d and ,s).
enum ECPP65DebugFormat
{
	CPP65DF_Default,		//As Hexadecimal Display says.
	CPP65DF_Hexadecimal,
	CPP65DF_Decimal,
	CPP65DF_String,			//A char* as its string alone.
};


#define CPP65_DEBUG_NO_REGISTER	0xFF	//A value not read from a register (SCPP65DebugValue::uiRegister).


//A value a debugger shows (DebuggerPlan.txt C1): of a type in a program's debug records, in memory (an address in a
//space; a bit field's bits in the bytes there) or worked out (a watch expression's, its bytes held).  uiArrayCount
//shows that many of the type from the address, as an array (a watch's p,10).  Its children are shown as it is.
struct SCPP65DebugValue
{
	CAsm65DebugRecords*	pcRecords;
	uint32				uiType;
	uint32				uiAddress;
	uint32				uiArrayCount;
	uint8				uiSpace;		//EAsm65StackSpace.
	bool				bInMemory;
	uint8				uiBitOffset;
	uint8				uiBits;			//0 for all of the value's bytes.
	uint8				uiFormat;		//ECPP65DebugFormat.
	uint8				uiRegister;		//A variable's value read from its frame's register (Variable), else CPP65_DEBUG_NO_REGISTER.
	uint8				auiBytes[8];	//A value not in memory.

	void	Init(CAsm65DebugRecords* pcRecords, uint32 uiType, EAsm65StackSpace eSpace, uint32 uiAddress);
	void	Init(CAsm65DebugRecords* pcRecords, uint32 uiType, uint64 uiValue);
};


//What editing a value writes (C2): its bytes at an address in a space, or a register's value (uiRegister, the
//walker's numbering, not CPP65_DEBUG_NO_REGISTER; auiBytes all of it).
struct SCPP65DebugWrite
{
	uint32	uiAddress;
	uint8	uiSpace;		//EAsm65StackSpace.
	uint8	uiSize;
	uint8	uiRegister;
	uint8	auiBytes[8];
};


#define CPP65_DEBUG_OPTIMISED_AWAY	"(optimised away)"	//What a variable kept nowhere a debugger can read shows as its value.


//What a processor's program has, as the debugger reads it: an address's bits (24 on the W65C816, 32 on the 65C64), the
//hexadecimal digits it is shown in, and an int's bytes (CCPPTarget::GetIntBytes).
uint32	CPP65DebugAddressMask(CAsm65DebugRecords* pcRecords);
int		CPP65DebugAddressDigits(CAsm65DebugRecords* pcRecords);
uint32	CPP65DebugIntBytes(CAsm65DebugRecords* pcRecords);


//Shows values as Visual Studio's Locals and Watch windows do, read from a stopped processor's memory (C1), and turns
//what is typed into one's Value into the bytes to write (C2).  Integers are decimal, or hexadecimal (SetHexadecimal);
//a char also as its character, a bool true or false, an enum by its enumerator (or A | B), a pointer as its address
//(and a char*'s string), an array or a class as {...} with its elements or members as children.  A class with a
//vtable pointer is also shown as the class of the object it is in (its type info's name): "CShape* {CSquare}".
//A read that fails (memory not yet fetched) still gives what it can, and says so.
class CCPP65DebugValues
{
protected:
	CAsm65StackMemory*	mpcMemory;
	bool				mbHexadecimal;

public:
	void	Init(CAsm65StackMemory* pcMemory);
	void	Kill(void);

	void	SetHexadecimal(bool bHexadecimal);

	bool	Variable(CAsm65StackWalker* pcWalker, SAsm65StackFrame* psFrame, SAsm65DebugVariable* psVariable, SCPP65DebugValue* psValue);
	void	Global(CAsm65DebugRecords* pcRecords, SAsm65DebugGlobal* psGlobal, EAsm65StackSpace eSpace, SCPP65DebugValue* psValue);
	bool	Result(CAsm65DebugRecords* pcRecords, SAsm65DebugFunction* psFunction, SAsm65StackRegisters* psRegisters, SCPP65DebugValue* psValue);
	bool	AppendCall(CAsm65StackWalker* pcWalker, SAsm65StackFrame* psFrame, CChars* pszDest);

	bool	Format(SCPP65DebugValue* psValue, CChars* pszDest);
	bool	AppendType(SCPP65DebugValue* psValue, CChars* pszDest);
	size	NumChildren(SCPP65DebugValue* psValue);
	bool	GetChild(SCPP65DebugValue* psValue, size uiIndex, CChars* pszName, SCPP65DebugValue* psChild);
	bool	Member(SCPP65DebugValue* psValue, const char* szName, SCPP65DebugValue* psMember);
	bool	Edit(SCPP65DebugValue* psValue, const char* szText, SCPP65DebugWrite* psWrite);

	bool	Read(SCPP65DebugValue* psValue, uint64* puiValue);
	bool	Dereference(SCPP65DebugValue* psValue, SCPP65DebugValue* psTarget);
	bool	DynamicClass(SCPP65DebugValue* psValue, CChars* pszDest);

protected:
	SAsm65DebugType*	GetType(SCPP65DebugValue* psValue);
	bool	ReadBytes(SCPP65DebugValue* psValue, uint32 uiOffset, void* pvDest, size uiBytes);
	bool	ReadLong(CAsm65DebugRecords* pcRecords, EAsm65StackSpace eSpace, uint32 uiAddress, uint32* puiLong);
	void	AppendInteger(SCPP65DebugValue* psValue, SAsm65DebugType* psType, uint64 uiValue, CChars* pszDest);
	void	AppendEnum(SCPP65DebugValue* psValue, SAsm65DebugType* psType, uint64 uiValue, CChars* pszDest);
	bool	AppendString(CAsm65DebugRecords* pcRecords, EAsm65StackSpace eSpace, uint32 uiAddress, size uiMost, CChars* pszDest, char cQuote = '"');
	bool	ClassOf(SCPP65DebugValue* psValue, SCPP65DebugValue* psClass);
	bool	Element(SCPP65DebugValue* psValue, size uiIndex, SCPP65DebugValue* psElement);
	void	MemberValue(SCPP65DebugValue* psValue, SAsm65DebugMember* psRecord, SCPP65DebugValue* psMember);
	bool	ParseInteger(SCPP65DebugValue* psValue, SAsm65DebugType* psType, const char* szText, uint64* puiValue);
};


#endif // __CPP65_DEBUG_VALUES_H__

