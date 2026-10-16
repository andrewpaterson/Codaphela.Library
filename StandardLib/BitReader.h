/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __BIT_READER_H__
#define __BIT_READER_H__
#include "BaseLib/FileReader.h"


//Reads bits least significant first, as deflate packs them.  Bytes are only read from the file as they are needed so
//after AlignToByte the file is positioned at the next whole byte.
class CBitReader
{
protected:
	CFileReader*	mpcFile;
	uint32			muiBits;
	int				miNumBits;

public:
	void	Init(CFileReader* pcFile);
	void	Kill(void);

	bool	ReadBits(int iCount, uint32* puiValue);
	bool	ReadBit(uint32* puiValue);
	void	AlignToByte(void);
};


#endif // __BIT_READER_H__
