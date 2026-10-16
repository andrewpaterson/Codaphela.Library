/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RLE_PACKET_DECODER_H__
#define __RLE_PACKET_DECODER_H__
#include "BaseLib/PrimitiveTypes.h"


//Decodes packet RLE (as used by Targa images).  Each packet starts with a header byte.  If the top bit is set the
//packet is a run and is followed by one element repeated (header & 0x7F) + 1 times.  Otherwise it is followed by
//(header & 0x7F) + 1 raw elements.  An element is any fixed number of bytes, typically one pixel.
class CRLEPacketDecoder
{
protected:
	uint8*	mpuiData;
	size	muiDataSize;
	size	muiElementSize;
	size	muiPosition;
	int		miPacketRemaining;
	bool	mbRun;
	uint8*	mpuiRunElement;

public:
	void	Init(uint8* puiData, size uiDataSize, size uiElementSize);
	void	Kill(void);

	uint8*	Next(void);
	size	GetPosition(void);
};


#endif // __RLE_PACKET_DECODER_H__
