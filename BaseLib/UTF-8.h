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
#ifndef __UTF_8_H__
#define __UTF_8_H__
#include "Chars.h"


#define	UNICODE_ZWJ		0x200D
#define UNICODE_ERROR	SIZE_MAX;

class CUTF8
{
protected:
	char*		mszText;  //underlying byte array.
	size		muiTextLength;

	size		muiPos;
	size		muiGlyphLength;
	size		muiError;

public:
	void	Init(CChars* sz);
	void	Init(char* sz);
	void	Kill(void);

	uint16	GetUint16(void);  // Retuns 0xFFFD if larger than uint16
	uint32	GetUint32(void);  // Retuns 0xFFFD if larger than uint32
	size	GetMulti(uint8* puiBuffer, size uiBufferLength);

	size	Step(void);

	size	GetPosition(void);
	size	GetError(void);
	size	GetGlyphLength(void);

protected:
	size	GetElementLength(void);
};


#endif // __UTF_8_H__

