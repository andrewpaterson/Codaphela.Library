#ifndef __UTF_16_H__
#define __UTF_16_H__
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
#include "Chars.h"
#include "Unicode.h"


class CUTF16
{
protected:
	uint16*		mszText;  //underlying wide char array.
	size		muiTextLength;  //count of uint16s (not bytes).

	size		muiPos;
	size		muiCodeLength;  //This is the UTF8 encoded length, not the unicode code point length.
	size		muiError;

public:
    void    Init(uint16* sz);  // raw UTF-16 buffer (null-terminated)
    void    Init(uint16* sz, size length); // explicit length version
    void    Kill(void);

    uint16  GetUint16(void);  // Retuns 0xFFFD if larger than uint16
    uint32  GetUint32(void);  // Retuns 0xFFFD if larger than uint32

    size    GetMulti(uint16* puiBuffer, size uiBufferLength);

    size    Step(void);

    size    GetPosition(void);
    size    GetError(void);
    size    GetCodeLength(void);

protected:
    size    GetElementLength(void);
};


#endif // __UTF_16_H__

