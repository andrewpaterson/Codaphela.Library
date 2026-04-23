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
#include "UnicodeReader.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUnicodeReader::Init(void)
{
	CUnicode::Init();

	muiUTF8ZWJBytes = 0x8D80E2;
	muiUTF16LEZWJBytes = 0x0D20;
	muiUTF16BEZWJBytes = 0x200D;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUnicodeReader::Kill(void)
{
	CUnicode::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUnicodeReader::IsTooSmallOrError(uint16 ui)
{
	return (ui == muiError) || (ui == muiTooSmall);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUnicodeReader::IsError(uint16 ui)
{
	return ui == muiError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUnicodeReader::IsTooSmall(uint16 ui)
{
	return ui == muiTooSmall;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicodeReader::GetUTFZWJLength(EUnicodeEncoding eEncoding)
{
	switch (eEncoding)
	{
	case UE_UTF8:
	case UE_UTF8BOM:
		return 3;
	case UE_UTF16LE:
	case UE_UTF16BE:
		return 2;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint8* CUnicodeReader::GetUTFZWJBytes(EUnicodeEncoding eEncoding)
{
	switch (eEncoding)
	{
	case UE_UTF8:
	case UE_UTF8BOM:
		return (uint8*)&muiUTF8ZWJBytes;
	case UE_UTF16LE:
		return (uint8*)&muiUTF16LEZWJBytes;
	case UE_UTF16BE:
		return (uint8*)&muiUTF16BEZWJBytes;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 CUnicodeReader::MakeCodePointUint32FromBuffer(uint8* puiCodePointBuffer, size uiBufferLength)
{
	uint32	uiCodePoint;
	size	uiShift;
	size	ui;

	ui = 0;
	uiShift = 0;
	uiCodePoint = 0;
	while (ui != uiBufferLength)
	{
		uiCodePoint |= puiCodePointBuffer[ui] << uiShift;
		ui++;
		uiShift += 8;
	}
	return uiCodePoint;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUnicodeReader::MakeCodePointUint16FromBuffer(uint8* puiCodePointBuffer, size uiBufferLength)
{
	uint16	uiCodePoint;

	uiCodePoint = puiCodePointBuffer[0];
	if (uiBufferLength == 2)
	{
		uiCodePoint |= puiCodePointBuffer[1] << 8;
	}
	return uiCodePoint;
}

