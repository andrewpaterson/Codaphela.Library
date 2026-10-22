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
#include "BaseLib/FileReader.h"


//Decodes packet RLE (as used by Targa images).  Each packet starts with a header byte.  If the top bit is set the
//packet is a run and is followed by one element repeated (header & 0x7F) + 1 times.  Otherwise it is followed by
//(header & 0x7F) + 1 raw elements.  An element is any fixed number of bytes, typically one pixel.
//
//The decoder reads the packet headers and the caller reads the elements from the same file.
class CRLEPacketDecoder
{
protected:
	CFileReader*	mpcFile;
	int				miPacketRemaining;
	bool			mbRun;

public:
	void	Init(CFileReader* pcFile);
	void	Kill(void);

	bool	Next(bool* pbRepeat);
};


#endif // __RLE_PACKET_DECODER_H__
