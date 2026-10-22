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
#ifndef __BIT_WRITER_H__
#define __BIT_WRITER_H__
#include "BaseLib/FileWriter.h"


#define BIT_WRITER_BUFFER_SIZE	4096


//Writes bits least significant first, as deflate packs them.  Whole bytes are buffered and written to the file when
//the buffer fills and on Flush.  A failed write is remembered and reported by Flush.
class CBitWriter
{
protected:
	CFileWriter*	mpcFile;
	uint32			muiBits;
	int				miNumBits;
	uint8			mauiBuffer[BIT_WRITER_BUFFER_SIZE];
	size			muiBufferUsed;
	bool			mbFailed;

public:
	void	Init(CFileWriter* pcFile);
	void	Kill(void);

	void	WriteBits(uint32 uiValue, int iCount);
	void	AlignToByte(void);
	void	WriteBytes(uint8* puiData, size uiLength);
	bool	Flush(void);

protected:
	void	WriteByte(uint8 uiByte);
};


#endif // __BIT_WRITER_H__
