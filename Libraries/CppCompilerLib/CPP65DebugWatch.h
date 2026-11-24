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
#ifndef __CPP65_DEBUG_WATCH_H__
#define __CPP65_DEBUG_WATCH_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "CPP65DebugValues.h"


enum ECPP65WatchToken
{
	CPP65WT_End,
	CPP65WT_Identifier,
	CPP65WT_Integer,
	CPP65WT_Float,
	CPP65WT_Character,
	CPP65WT_Punctuator,
};


//Where the watch's tokeniser is, so a cast can be tried and taken back.
struct SCPP65WatchPosition
{
	const char*			szNext;
	ECPP65WatchToken	eToken;
	const char*			szStart;
	size				uiLength;
};


//Evaluates a watch expression (DebuggerPlan.txt C3) in a stopped program's frame, as Visual Studio's Watch window
//does, with the program's types as its debug records describe them (an int is 16 bits on the W65C816, 32 on the
//65C64), not the host's.  Names are the frame's locals in scope at its line, then this's members, then the program's statics and
//globals; then . -> [] unary * & - + ! ~, casts to a type the records have ((CSquare*)pcShape, its pointers added),
//sizeof, * / % + - << >> < > <= >= == != & ^ | && || on integers, floats and pointers.  Nothing is called or
//assigned (decision 3).  A format suffix shows the result as Visual Studio's do: ,x hexadecimal, ,d decimal, ,s a
//char*'s string alone, ,n a pointer as an array of n.
//
//The full C++ parser needs a unit's declarations to tell a cast from an expression, so a watch has its own small
//grammar, binding each name against the records as it goes; types it needs and the records haven't (bool, a pointer
//to a class) are added to them.
class CCPP65DebugWatch
{
protected:
	CCPP65DebugValues*		mpcValues;
	CAsm65StackWalker*		mpcWalker;
	SAsm65StackFrame*		mpsFrame;
	CAsm65DebugRecords*		mpcRecords;
	uint32					muiLine;
	SCPP65WatchPosition		msToken;
	CChars					mszError;

public:
	void		Init(CCPP65DebugValues* pcValues, CAsm65StackWalker* pcWalker);
	void		Kill(void);

	bool		Evaluate(SAsm65StackFrame* psFrame, uint32 uiLine, const char* szExpression, SCPP65DebugValue* psResult);
	const char*	GetError(void);
	size		Autos(SAsm65StackFrame* psFrame, uint32 uiLine, const char* szSource, CArrayChars* paszExpressions);

protected:
	bool		Expression(int iLeast, SCPP65DebugValue* psValue);
	bool		Unary(SCPP65DebugValue* psValue);
	bool		Postfix(SCPP65DebugValue* psValue);
	bool		Primary(SCPP65DebugValue* psValue);
	bool		Suffix(SCPP65DebugValue* psValue);
	bool		Name(const char* szName, SCPP65DebugValue* psValue);
	bool		Literal(SCPP65DebugValue* psValue);
	bool		TypeName(uint32* puiType);
	bool		Binary(const char* szOperator, SCPP65DebugValue* psLeft, SCPP65DebugValue* psRight);
	bool		Negate(char cOperator, SCPP65DebugValue* psValue);
	bool		Cast(uint32 uiType, SCPP65DebugValue* psValue);
	bool		Index(SCPP65DebugValue* psValue, SCPP65DebugValue* psIndex);
	bool		Dereference(SCPP65DebugValue* psValue);
	bool		AddressOf(SCPP65DebugValue* psValue);
	bool		Decay(SCPP65DebugValue* psValue);

	bool		Number(SCPP65DebugValue* psValue, int64* piValue, double* pdValue, bool* pbFloat);
	uint32		IntegerType(size uiBytes, bool bSigned);
	uint32		BuiltIn(const char* szName, EAsm65DebugType eKind, uint8 uiFlags, uint32 uiBytes);
	uint32		BuiltInBytes(const struct SCPP65WatchBuiltIn* psBuiltIn);
	uint32		PointerTo(uint32 uiType);
	uint32		FindTypeNamed(const char* szName);
	SAsm65DebugType*	GetType(SCPP65DebugValue* psValue);

	void		Next(void);
	bool		Is(const char* szPunctuator);
	bool		Accept(const char* szPunctuator);
	bool		Expect(const char* szPunctuator);
	void		TokenText(CChars* pszDest);
	void		FindNames(const char* szSource, CArrayChars* paszNames);
	static void	BlankCommentsAndStrings(const char* szSource, CChars* pszDest);
	bool		Error(const char* szError, bool bToken = false);
};


#endif // __CPP65_DEBUG_WATCH_H__

