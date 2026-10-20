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
#ifndef __CHANNELS_ACCESSOR_H__
#define __CHANNELS_ACCESSOR_H__
#include "Unknown.h"
#include "ChannelAccessor.h"
#include "Channels.h"


//Note: this is an abstract class
class CChannelsAccessor : public CUnknown
{
CONSTRUCTABLE(CChannelsAccessor)
protected:
	CChannels*				mpcChannels;
	CArrayChannelAccessor	macAccessors;
	uint32					miByteSize;
	uint32					miBitSize;
	CStackMemory<8>			msBuffer;  //Where the gotten data is stored.

public:
			void					Init(CChannels* pcChannels, CArrayChannelAccessor* pcAccessors, uint32 iByteSize, uint32 iBitSize);
	virtual void					Kill(void);

	virtual void*					Get(uint32 iPos) =0;
	virtual void					Set(uint32 iPos, void* pvData) =0;

	virtual bool					IsContiguous(void);

			EPrimitiveType			GetType(void);
			uint32					GetBitSize(void);
			uint32					GetByteSize(void);
			uint32					GetBufferSize(void);
			CArrayChannelAccessor*	GetAccessors(void);
			CChannels*				GetChannels(void);
};


#endif // __CHANNELS_ACCESSOR_H__

