/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __XML_WRITER_H__
#define __XML_WRITER_H__
#include "Chars.h"
#include "ArrayChars.h"


//Writes an XML document, two spaces indenting each level.  Begin opens an element; attributes are added until its
//first child or End, which closes it (as <Name/> when it has no children).  Text in attributes and between elements is
//escaped.  Floats are written with enough digits to read back exactly.
class CXMLWriter
{
protected:
	CLargeChars		mszText;
	CArrayChars		maszOpen;
	bool			mbStartOpen;		//The last element's start tag still takes attributes.
	bool			mbHasChildren;		//The innermost element has children (elements or text).

public:
	void	Init(void);
	void	Kill(void);

	void	Begin(const char* szName);
	void	End(void);

	void	Attribute(const char* szName, const char* szValue);
	void	Attribute(const char* szName, int32 iValue);
	void	Attribute(const char* szName, int64 iValue);
	void	Attribute(const char* szName, bool bValue);
	void	Attribute(const char* szName, float32 fValue);
	void	Attribute(const char* szName, float64 fValue);
	void	Text(const char* szText);

	bool	IsComplete(void);
	char*	GetText(void);
	bool	Write(const char* szFilename);

protected:
	void	CloseStart(void);
	void	AppendEscaped(const char* szText, bool bAttribute);
	void	Indent(void);
};


#endif // __XML_WRITER_H__
