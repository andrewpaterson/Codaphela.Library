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
#include "Killable.h"
#include "ArrayUint8.h"
#include "ArrayUint16.h"


class CUnicodeReader;
class CUnicodeWriter : public CKillable
{
public:
	size WriteUTF16LE(CUnicodeReader* pcUnicodeReader, CArrayUint16* puiUTF16Dest, size uiMaxToWrite = SIZE_MAX);
	size WriteUTF16BE(CUnicodeReader* pcUnicodeReader, CArrayUint16* puiUTF16Dest, size uiMaxToWrite = SIZE_MAX);
	size WriteUTF8(CUnicodeReader* pcUnicodeReader, CArrayUint8* puiUTF8Dest, size uiMaxToWrite = SIZE_MAX);
};


#endif // __UNICODE_WRITER_H__

