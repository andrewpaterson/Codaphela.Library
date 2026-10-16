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
#include "RLEPacketDecoder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEPacketDecoder::Init(CFileReader* pcFile)
{
	mpcFile = pcFile;
	miPacketRemaining = 0;
	mbRun = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEPacketDecoder::Kill(void)
{
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves to the next element, reading a packet header if needed.  If
// *pbRepeat is false the caller must read the element from the file next.
// If it is true the element is the same as the previous one and nothing
// is read.  Returns false if the data is truncated.
//
//////////////////////////////////////////////////////////////////////////
bool CRLEPacketDecoder::Next(bool* pbRepeat)
{
	uint8	uiHeader;

	if (miPacketRemaining == 0)
	{
		if (!mpcFile->ReadInt8(&uiHeader))
		{
			return false;
		}
		mbRun = (uiHeader & 0x80) != 0;
		miPacketRemaining = (uiHeader & 0x7F) + 1;
		*pbRepeat = false;
	}
	else
	{
		*pbRepeat = mbRun;
	}
	miPacketRemaining--;
	return true;
}
