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
#ifndef __C_P_P_ERRORS_H__
#define __C_P_P_ERRORS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"


//Where an error is.  Lines and columns are 0 based; -1 if not known.  iToken is the index of the token in the
//stage's own tokens (preprocessed tokens for the preprocessor and tokeniser, C++ tokens for the parser and the
//65C64 generator), -1 if there isn't one.
struct SCPPErrorPosition
{
	int32	iLine;
	int32	iColumn;
	int32	iToken;
	size	uiFile;		//Index of the file name in CCPPErrors' file names, or ARRAY_ELEMENT_NOT_FOUND.
};


typedef CArrayTemplate<SCPPErrorPosition>	CArrayCPPErrorPositions;


//The errors one stage (the preprocessor, tokeniser, parser or 65C64 generator) found in the source, in the order
//found.  Each is a "File (line, column): message [token]" line.
class CCPPErrors
{
protected:
	CArrayChars				maszErrors;
	CArrayCPPErrorPositions	masPositions;
	CArrayChars				maszFiles;
	CChars					mszText;

public:
	void	Init(void);
	void	Kill(void);
	void	Clear(void);

	void	Add(const char* szFile, int iLine, int iColumn, int iToken, const char* szMessage, const char* szDetail = NULL);
	void	Truncate(size uiNumErrors);

	size	NumErrors(void);
	char*	Get(size iIndex);
	char*	GetFile(size iIndex);
	int		GetLine(size iIndex);
	int		GetColumn(size iIndex);
	int		GetToken(size iIndex);
	char*	Text(void);
	void	AppendTo(CCPPErrors* pcDest);

protected:
	size	GetFileIndex(const char* szFile);
};


#endif // __C_P_P_ERRORS_H__
