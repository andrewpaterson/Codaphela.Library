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
#include "Latin1.h"


//////////////////////////////////////////////////////////////////////////
//
// sz ends at its zero.
//
//////////////////////////////////////////////////////////////////////////
void CLatin1::Init(const char* sz)
{
	CUnicodeReader::Init();
	mszText = (uint8*)sz;
	muiTextLength = StrLen(sz);
	muiPos = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLatin1::Init(uint8* sz, size uiLength)
{
	CUnicodeReader::Init();
	mszText = sz;
	muiTextLength = uiLength;
	muiPos = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLatin1::Kill(void)
{
	mszText = NULL;
	muiTextLength = 0;
	muiPos = 0;
	CUnicodeReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Latin-1 has no byte order mark.
//
//////////////////////////////////////////////////////////////////////////
bool CLatin1::GetByteOrderMark(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLatin1::GetCurrentData(void)
{
	return &mszText[muiPos];
}


//////////////////////////////////////////////////////////////////////////
//
// Zero at the end of the text.
//
//////////////////////////////////////////////////////////////////////////
uint16 CLatin1::GetCodePointUint16(void)
{
	return (uint16)GetCodePointUint32();
}


//////////////////////////////////////////////////////////////////////////
//
// Zero at the end of the text.
//
//////////////////////////////////////////////////////////////////////////
uint32 CLatin1::GetCodePointUint32(void)
{
	uint8	c;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	c = mszText[muiPos];
	muiPos++;
	return c;
}


//////////////////////////////////////////////////////////////////////////
//
// Latin-1 has no joined characters, so this is one code point, put in
// puiBuffer as GetCodePointMulti always does.  Returns its length, 0 at
// the end of the text.
//
//////////////////////////////////////////////////////////////////////////
size CLatin1::GetCodePointMulti(uint8* puiBuffer, size uiBufferLength)
{
	uint32	c32;
	size	uiCodePointLength;
	size	uiBufferPos;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	c32 = GetCodePointUint32();
	uiCodePointLength = GetUnicodeCodePointLength(c32);
	uiBufferPos = AppendCodePoint(c32, uiCodePointLength, puiBuffer, 0, uiBufferLength);
	AppendCodePoint((uint16)0, 1, puiBuffer, uiBufferPos, uiBufferLength);  //Append a final trailing 0 if possible but don't include it in the buffer size.
	return uiCodePointLength;
}


//////////////////////////////////////////////////////////////////////////
//
// Every character is one byte.  0 at the end of the text.
//
//////////////////////////////////////////////////////////////////////////
size CLatin1::PeekUTFBytes(void)
{
	if (muiPos >= muiTextLength)
	{
		return 0;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLatin1::GetPosition(void) { return muiPos; }

