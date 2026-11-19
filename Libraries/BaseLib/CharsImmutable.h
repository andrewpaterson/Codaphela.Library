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
#ifndef __CHARS_IMMUTABLE_H__
#define __CHARS_IMMUTABLE_H__
#include "Mallocator.h"
#include "ExternalString.h"


#define MAX_EMBEDDED_CHARS	12


struct SStringAlloc
{
	CMallocator*	pcMalloc;
	char*			szString;
	size			uiLength;
};


union UString
{
	SStringAlloc	sStringAlloc;
	char			sz[MAX_EMBEDDED_CHARS];
};


class CChars;
class CCharsImmutable
{
NAMED_CLASS(CCharsImmutable)
friend class CFileReader;
friend class CFileWriter;
protected:
	bool		mbMallocated;
	UString		muString;

public:
	void		_Init(void);
	void		Init(const char* sz);
	void		Init(const char* sz, CMallocator* pcMalloc);
	void		Init(const char* sz, size uiLength);
	void		Init(const char* sz, size uiLength, CMallocator* pcMalloc);
	void		Init(void);
	void		InitLength(size uiLength);
	void		InitLength(size uiLength, CMallocator* pcMalloc);
	void		Kill(void);
	void		Dump(void) const;

	char*		Text(void);
	const char*	Text(void) const;
	size		Length(void) const;
	void		Set(const char* sz);
	bool		Empty(void) const;

	bool		Equals(const char* szString) const;
	bool		Equals(const char* szString, size iLen) const;
	bool		Equals(CChars* pszString) const;
	bool		EqualsIgnoreCase(const char* szString) const;
	bool		EqualsIgnoreCase(CChars* pszString) const;
	bool		Contains(const char* szString) const;
	bool		ContainsIgnoreCase(const char* szString) const;
	bool		Contains(char c) const;
	bool		EndsWith(const char* szString) const;
	bool		EndsWithIgnoreCase(const char* szString) const;
	bool		EndsWith(char c) const;
	bool		StartsWith(const char* szString) const;
	bool		StartsWithIgnoreCase(const char* szString) const;
	bool		StartsWith(char c) const;
	bool		SubStringEquals(size iStart, const char* szString) const;
	bool		SubStringEqualsIgnoreCase(size iStart, const char* szString) const;
	size		FindFromEnd(const char* szString) const;
	size		FindFromEnd(size iPos, const char* szString) const;
	size		FindFromEnd(char c) const;
	size		FindFromEnd(size iIndex, char c) const;
	size		Find(const char* szString) const;
	size		Find(size iPos, const char* szString) const;
	size		Find(size iPos, char c) const;
	size		Find(char c) const;

	bool		WriteString(CFileWriter* pcWriter) const;
	bool		ReadString(CFileReader* pcReader);
};


#endif // __CHARS_IMMUTABLE_H____CHARS_H__

