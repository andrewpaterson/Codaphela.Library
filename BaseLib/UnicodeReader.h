#ifndef __UNICODE_READER_H__
#define __UNICODE_READER_H__
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
#include "Unicode.h"


class CUnicodeReader : public CUnicode
{
protected:
	uint32		muiUTF8ZWJBytes;
	uint32      muiUTF16LEZWJBytes;
	uint32      muiUTF16BEZWJBytes;
	uint32		muiUTF8BOMBytes;
	uint32      muiUTF16LEBOMBytes;
	uint32      muiUTF16BEBOMBytes;

public:
	void	Init(void);
	void	Kill(void) override;

	virtual uint16	GetCodePointUint16(void) =0;  // Retuns 0xFFFD if larger than uint16
	virtual uint32	GetCodePointUint32(void) =0;  // Retuns 0xFFFD if larger than uint32
	virtual size	GetCodePointMulti(uint8* puiBuffer, size uiBufferLength) =0;

	virtual void*	GetCurrentData(void) =0;
	virtual bool	GetByteOrderMark(void) =0;

	virtual size	PeekUTFBytes(void) =0;

	virtual size	GetPosition(void) =0;

			size	GetZWJLength(EUnicodeEncoding eEncoding);
			uint8*	GetZWJBytes(EUnicodeEncoding eEncoding);
			size	GetBOMLength(EUnicodeEncoding eEncoding);
			uint8*	GetBOMBytes(EUnicodeEncoding eEncoding);

			bool	IsTooSmallOrError(size ui);
			bool	IsError(size ui);
			bool	IsTooSmall(size ui);

			uint32	MakeCodePointUint32FromBuffer(uint8* puiCodePointBuffer, size uiBufferLength);
			uint16	MakeCodePointUint16FromBuffer(uint8* puiCodePointBuffer, size uiBufferLength);
};


#endif // __UNICODE_READER_H__

