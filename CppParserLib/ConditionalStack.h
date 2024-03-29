/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __CONDITIONAL_STACK_H__
#define __CONDITIONAL_STACK_H__
#include "BaseLib/ArrayTemplate.h"


enum EConditionalType
{
	CT_IfNotDefined,
	CT_IfDefined,
	CT_Else,
	CT_ElseIf,
	CT_If,
	CT_EndIf,  //Should never appear
};


struct SDefineConditional
{
	bool				bEvaluated;  //If this is false then any 'real' text is being ignored preprocessor macros are still considered.
	EConditionalType	eType;  //For error checking.
	bool				bElseIfPassed;  //When an else (or if) evaluates to true then no more should be tested.
};

typedef CArrayTemplate<SDefineConditional> CArrayConditionals;


class CConditionalStack
{
public:
	CArrayConditionals	mcStack;
	bool				mbParsing;

	void Init(void);
	void Kill(void);

	bool IsParsing(void);
	void CalculateIsParsing(void);
	void PushIfNotDefined(bool bEvaluated);  //bEvaluated is the the value that the macro conditional evaluated too.
	void PushIfDefined(bool bEvaluated);
	void SwapForElse(void);
	void SwapForElseIf(bool bEvaluated);  //The else portion of bEvaluated will be calculated, don't include it in bEvaluated.
	void PushIf(bool bEvaluated);
	void PopEndIf(void);

	void Push(EConditionalType eType, bool bEvaluated, bool bElseIfPassed);
};


#endif //__CONDITIONAL_STACK_H__

