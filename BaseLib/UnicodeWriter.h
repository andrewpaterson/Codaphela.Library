#ifndef __UNICODE_WRITER_H__
#define __UNICODE_WRITER_H__
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
#include "PrimitiveTypes.h"
#include "ArrayUint8.h"
#include "ArrayUint16.h"
#include "Unicode.h"


class CUnicodeReader;
class CUnicodeWriter
{
private:
	EUnicodeEncoding	meOutputEncoding;
	CUnicodeReader*		mpcReader;
	CArrayBlock*		mpauiDest;

public:
	void	Init(CUnicodeReader* pcReader, EUnicodeEncoding eOutputEncoding, CArrayBlock* pauiDest);
	void	Kill(void);

	bool	ReadBOM(void);

	bool	WriteBOM(void);
	int		WriteCharacters(size uiMaxToWrite = SIZE_MAX);

protected:
	size	WriteUTF16LE(CUnicodeReader* pcUnicodeReader, CArrayUint16* puiUTF16Dest, size uiMaxToWrite);
	size	WriteUTF16BE(CUnicodeReader* pcUnicodeReader, CArrayUint16* puiUTF16Dest, size uiMaxToWrite);
	size	WriteUTF8(CUnicodeReader* pcUnicodeReader, CArrayUint8* puiUTF8Dest, size uiMaxToWrite);
};


#endif // __UNICODE_WRITER_H__

