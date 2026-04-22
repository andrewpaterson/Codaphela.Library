#ifndef __UNICODE_H__
#define __UNICODE_H__
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
#include "Killable.h"


#define UNICODE_ZWJ 0x200D
#define UNICODE_NON_CHARACTER 0xFFFF
#define UNICODE_REPLACEMENT_CHARACTER 0xFFFD



class CUnicode : public CKillable
{
protected:
	uint16	muiError;
	uint16	muiTooSmall;

public:
	void	Init(void);
	void	Kill(void) override;

	size	GetUnicodeCodePointLength(uint16 uiChar);
	size	GetUnicodeCodePointLength(uint32 uiChar);

	size	AppendCodePoint(uint16 uiCodePoint, size uiCodePointLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength);
	size	AppendCodePoint(uint32 uiCodePoint, size uiCodePointLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength);

	size	GetUTF16Length(uint32 uiCodePoint);
	uint16  GetUTF16ElementUint16(uint32 uiCodePoint);
	uint32  GetUTF16ElementUint32(uint32 uiCodePoint);

	size	GetUTF8Length(uint32 uiCodePoint);
	uint16  GetUTF8ElementUint16(uint32 uiCodePoint);
	uint32  GetUTF8ElementUint32(uint32 uiCodePoint);

	size    GetError(void);
};


#endif // __UNICODE_H__

