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
	muiUTF16LEZWJBytes = 0x200D;
	muiUTF16BEZWJBytes = 0x0D20;

	muiUTF8BOMBytes = UTF8_BOM;
	muiUTF16LEBOMBytes = UTF16_LITTLE_ENDIAN_BOM;
	muiUTF16BEBOMBytes = UTF16_BIG_ENDIAN_BOM;
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
bool CUnicodeReader::IsTooSmallOrError(size ui)
{
	return (ui == muiError) || (ui == muiTooSmall);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUnicodeReader::IsError(size ui)
{
	return ui == muiError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUnicodeReader::IsTooSmall(size ui)
{
	return ui == muiTooSmall;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicodeReader::GetBOMLength(EUnicodeEncoding eEncoding)
{
	switch (eEncoding)
	{
	case UE_UTF8:
		return 0;
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
uint8* CUnicodeReader::GetBOMBytes(EUnicodeEncoding eEncoding)
{
	switch (eEncoding)
	{
	case UE_UTF8:
		return NULL;
	case UE_UTF8BOM:
		return (uint8*)&muiUTF8BOMBytes;
	case UE_UTF16LE:
		return (uint8*)&muiUTF16LEBOMBytes;
	case UE_UTF16BE:
		return (uint8*)&muiUTF16BEBOMBytes;
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicodeReader::GetZWJLength(EUnicodeEncoding eEncoding)
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
uint8* CUnicodeReader::GetZWJBytes(EUnicodeEncoding eEncoding)
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
		return NULL;
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

