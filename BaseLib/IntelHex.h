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
#ifndef __INTEL_HEX_H__
#define __INTEL_HEX_H__
#include "ArrayTemplate.h"
#include "ArrayUint8.h"
#include "Chars.h"


//A run of bytes at consecutive addresses.
struct SIntelHexSegment
{
	uint32				uiAddress;
	CArrayLargeUint8	cBytes;
};


typedef CArrayTemplate<SIntelHexSegment>	CArrayIntelHexSegments;


//The bytes of an Intel HEX file: data records (00), end of file (01), extended segment
//address (02) and extended linear address (04).  Start addresses (03 and 05) are ignored.
//The segments are in address order, joined where they touch; a byte given twice, a bad
//checksum, bad hex or a missing end record is an error.
class CIntelHex
{
protected:
	CArrayIntelHexSegments	macSegments;
	CChars					mszError;

public:
	void				Init(void);
	void				Kill(void);

	bool				Read(const char* szFilename);
	bool				Parse(const char* szText);
	char*				GetError(void);

	size				NumSegments(void);
	SIntelHexSegment*	GetSegment(size uiIndex);
	bool				IsEmpty(void);
	uint32				GetLowAddress(void);
	uint64				GetEndAddress(void);
	bool				Get(uint32 uiAddress, uint8* puiByte);
	void				Fill(uint32 uiAddress, uint32 uiLength, uint8 uiFill, uint8* puiDest);

protected:
	void				Clear(void);
	bool				ParseRecord(const char* szLine, size uiLength, uint32 uiLine, uint32* puiBase, bool* pbEnd);
	void				Write(uint32 uiAddress, uint8* puiBytes, size uiLength);
	bool				Join(void);
	bool				Error(uint32 uiLine, const char* szMessage);
};


#endif // __INTEL_HEX_H__
