/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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


class CCharsImmutable
{
CONSTRUCTABLE(CCharsImmutable);
friend class CFileReader;
friend class CFileWriter;
protected:
	bool		mbMallocated;
	UString		muString;

public:
	void	Init(char* sz);
	void	Init(char* sz, CMallocator* pcMalloc);
	void	Init(char* sz, size uiLength);
	void	Init(char* sz, size uiLength, CMallocator* pcMalloc);
	void	Init(void);
	void	Init(size uiLength, CMallocator* pcMalloc);
	void	Kill(void);
	void	Dump(void);

	char*	Text(void);
	size	Length(void);

	bool	WriteString(CFileWriter* pcWriter);
	bool	ReadString(CFileReader* pcReader);
};


#endif // __CHARS_IMMUTABLE_H____CHARS_H__

