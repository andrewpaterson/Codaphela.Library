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
#ifndef __RLE_PACKET_ENCODER_H__
#define __RLE_PACKET_ENCODER_H__
#include "BaseLib/PrimitiveTypes.h"


#define RLE_PACKET_MAX_LENGTH	128


//Encodes packet RLE (as used by Targa images).  See CRLEPacketDecoder for the format.
class CRLEPacketEncoder
{
protected:
	size	muiElementSize;

public:
	void	Init(size uiElementSize);
	void	Kill(void);

	size	Encode(uint8* puiSource, int iNumElements, uint8* puiDest);
	size	GetMaxEncodedSize(int iNumElements);

protected:
	bool	IsSameElement(uint8* puiSource, int iFirst, int iSecond);
};


#endif // __RLE_PACKET_ENCODER_H__
