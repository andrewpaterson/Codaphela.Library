/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_OUTPUT_H__
#define __ASM65_OUTPUT_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint8.h"


//A run of bytes at consecutive addresses.
struct SAsm65Segment
{
	uint32		uiAddress;
	CArrayUint8	cBytes;
};


typedef CArrayTemplate<SAsm65Segment>	CArrayAsm65Segments;


//The assembled bytes.  Writing to the address after a segment's last byte extends it,
//any other address starts a new segment.  Segments are in the order they were written.
class CAsm65Output
{
protected:
	CArrayAsm65Segments	mcSegments;

public:
	void			Init(void);
	void			Kill(void);
	void			Clear(void);

	void			Write(uint32 uiAddress, uint8 uiByte);

	size			NumSegments(void);
	SAsm65Segment*	GetSegment(size uiIndex);
	size			NumBytes(void);
	bool			Get(uint32 uiAddress, uint8* puiByte);
	uint16			GetWord(uint32 uiAddress);
	uint32			GetLong(uint32 uiAddress);
};


#endif // __ASM65_OUTPUT_H__
