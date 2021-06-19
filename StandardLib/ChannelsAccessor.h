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
#ifndef __CHANNELS_ACCESSOR_H__
#define __CHANNELS_ACCESSOR_H__
#include "Unknown.h"
#include "ChannelAccessor.h"
#include "Channels.h"


class CChannelsAccessor : public CUnknown
{
CONSTRUCTABLE(CChannelsAccessor);
protected:
	CChannels*				mpcChannels;
	CArrayChannelAccessor	macAccessors;
	int						miByteSize;
	int						miBitSize;
	char*					mpvGetBuffer;
	int						miBufferSize;  //How many bytes in the buffer.

public:
	void						Init(CChannels* pcChannels, CArrayChannelAccessor* pcAccessors, int iByteSize, int iBitSize, int iBufferSize);
	virtual void				Kill(void);

	virtual void*				Get(int iPos) =0;
	virtual void				Set(int iPos, void* pvData) =0;

	virtual BOOL				IsContiguous(void);

	EPrimitiveType				GetType(void);
	int							GetBitSize(void);
	int							GetByteSize(void);
	int							GetBufferSize(void);
	CArrayChannelAccessor*		GetAccessors(void);
	CChannels*					GetChannels(void);
};


#endif // !__CHANNELS_ACCESSOR_H__

