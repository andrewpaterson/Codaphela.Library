/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/DataTypes.h"
#include "ChannelsHelper.h"


#define CHANNEL_NON_ALIGNED_BYTES		SIZE_MAX


class CChannel : public SChannel
{
public:
	size	miByteSize;
	size	miBitSize;
	size	miByteOffset;
	size	miBitOffset;

	void	Init(size iChannel, EPrimitiveType eType, bool bReverse);
	void	Init(size iChannel, EPrimitiveType eType, size iByteSize, size iBitSize, size iByteOffset, size iBitOffset, bool bReverse = false);

	bool	Is8BitAligned(void);
};


typedef CArrayTemplate<CChannel>	CArrayChannelOffset;


#endif // __CHANNEL_H__

