#ifndef __LATIN_1_H__
#define __LATIN_1_H__
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
#include "Chars.h"
#include "UnicodeReader.h"


//Reads Latin-1 (ISO 8859-1) text, as X11's STRING is, so a CUnicodeWriter can write it as UTF8 or UTF16.  Every byte
//is a character, the code point of the same value, so Latin-1 text is never invalid.
class CLatin1 : public CUnicodeReader
{
protected:
	uint8*		mszText;
	size		muiTextLength;
	size		muiPos;

public:
	void	Init(const char* sz);
	void    Init(uint8* sz, size uiLength);
	void	Kill(void) override;

	bool	GetByteOrderMark(void) override;
	void*	GetCurrentData(void) override;
	uint16	GetCodePointUint16(void) override;
	uint32	GetCodePointUint32(void) override;
	size	GetCodePointMulti(uint8* puiBuffer, size uiBufferLength) override;
	size	PeekUTFBytes(void) override;
	size	GetPosition(void) override;
};


#endif // __LATIN_1_H__

