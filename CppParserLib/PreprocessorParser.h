/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __PREPROCESSOR_PARSER_H__
#define __PREPROCESSOR_PARSER_H__
#include "BaseLib/Define.h"


class CPreprocessorParser
{
public:
	char*	mszStart;
	char*	mszPos;
	char*	mszEnd;
	BOOL	mbEndOfFile;
	int		miLine;
	int		miColumn;
	BOOL	mbAllowAnnotations;

	void	Init(char* szStart, char* szEnd, BOOL bAllowAnnotations);

	void	StepRight(void);
	void	StepRight(int iDist);
	void	StepLeft(void);
	void	SkipWhiteSpace(void);
	void	TestEnd(void);
	BOOL	IsWhiteSpace(void);
	void	SkipCStyleComment(void);
	void	SkipCPPStyleComment(void);
	BOOL	IsFirstIdentifier(void);
	BOOL	IsDigit(void);
	void	SkipNewLine(void);
	int		CalculateColumn(void);
	void	BackupNewLine(void);
	BOOL	IsNewLine(void);
};


#endif // __PREPROCESSOR_PARSER_H__

