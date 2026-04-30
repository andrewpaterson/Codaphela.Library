#ifndef __UTF_16_H__
#define __UTF_16_H__
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


class CUTF16 : public CUnicodeReader
{
protected:
	uint16*		mszText;  //underlying wide char array.
	size		muiTextLength;  //count of uint16s (not bytes).
    bool        mbLittleEndian;

	size		muiPos;

public:
    void    Init(uint16* sz, size uiLength);
    void    Kill(void) override;
    void    SetBigEndian(void);

    bool    GetByteOrderMark(void);
    void*   GetCurrentData(void);

    uint16  GetCodePointUint16(void) override;
    uint32  GetCodePointUint32(void) override;
    size    GetCodePointMulti(uint8* puiBuffer, size uiBufferLength) override;

    size	PeekUTFBytes(void) override;

    size    GetPosition(void) override;

protected:
    size	GetUTF16ElementLength(void);
};


#endif // __UTF_16_H__

