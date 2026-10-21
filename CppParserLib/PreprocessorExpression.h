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
#ifndef __PREPROCESSOR_EXPRESSION_H__
#define __PREPROCESSOR_EXPRESSION_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ErrorTypes.h"


//The value of a #if expression.  C++14 [cpp.cond] evaluates in intmax_t or uintmax_t.
struct SPPValue
{
	uint64	ulliValue;
	bool	bUnsigned;

	void	Init(uint64 ulliValue, bool bUnsigned);
	bool	IsTrue(void);
	int64	Signed(void);
};


//Evaluates the (already macro expanded) text of a #if or #elif.
class CPreprocessorExpression
{
protected:
	char*		mszPos;
	char*		mszStart;
	CChars*		mpszError;
	bool		mbError;

public:
	static TRISTATE	Evaluate(char* szText, CChars* pszError);

	void		Init(char* szText, CChars* pszError);
	void		Kill(void);
	TRISTATE	Evaluate(void);

protected:
	SPPValue	Conditional(bool bEvaluate);
	SPPValue	Binary(int iMinPrecedence, bool bEvaluate);
	SPPValue	Unary(bool bEvaluate);
	SPPValue	Primary(bool bEvaluate);
	SPPValue	Number(void);
	SPPValue	Character(void);
	int			PeekBinaryOperator(int* piLength);
	SPPValue	Apply(int iOperator, SPPValue sLeft, SPPValue sRight, bool bEvaluate);
	bool		GetCharacter(char c);
	void		SkipWhitespace(void);
	bool		IsIdentifierCharacter(char c);
	SPPValue	Error(char* szError);
};


#endif // __PREPROCESSOR_EXPRESSION_H__

