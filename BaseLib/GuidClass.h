/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __GUID_CLASS_H__
#define __GUID_CLASS_H__
#include "PrimitiveTypes.h"
#include "Define.h"
#include "TextParser.h"
#include "Chars.h"


class CGuidClass
{
public:
	uint32	muiData1;
	uint16	muiData2; 
	uint16	muiData3;
	uint8	maubData4[8];	

	void Init(char8* sz);
	void Init(uint32 uiData1, uint16 uiData2, uint16 uiData3, uint8* paubData4);
	void Clear(void);
	BOOL Get32Format(CTextParser* pcParser, uint64 uiValue);
	BOOL Get8_4_4_4_12Format(CTextParser* pcParser, uint32 uiValue);
	BOOL Get0xFormat(CTextParser* pcParser, uint32 uiValue);
	void Append(CChars* psz);
};


#endif // __GUID_CLASS_H__

