/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __IMAGE_ACCESSOR_H__
#define __IMAGE_ACCESSOR_H__
#include "StandardLib/Pointer.h"
#include "StandardLib/ChannelsAccessor.h"
#include "ImageChannel.h"
#include "ImageColour.h"


//CImageAccessor is a way to access, say the RGB, channels in a consistent manner.
//It only deals with channels that it knows about, it must not interrogate the image about it's channels contents.
//eg:  The CImageAccessor cannot answer a question such as: GetOpacity, that is done elsewhere.
class CImage;
class CImageAccessor : public CUnknown
{
CONSTRUCTABLE(CImageAccessor);
protected:
	CChannelsAccessor*	mpcAccessor;
	CImage*				mpcImage;
	uint32				miWidth;
	bool				mbKillChannelAccessor;

public:
	void 				Init(CImage* pcImage, CChannelsAccessor* pcAccessor, bool bKillChannelAccessor);
	virtual void		Kill(void);

	virtual void*		Get(int32 x, int32 y);
	virtual void		Set(int32 x, int32 y, void* pvData);

	uint32				GetBufferSize(void);
	virtual void		SyncDataCache(void);

	void				Get(int32 x, int32 y, void* pvDest);
	bool				IsValid(int32 x, int32 y);
	void*				SafeGet(int32 x, int32 y);
	void				SafeSet(int32 x, int32 y, void* pvData);

	bool				MakeColour(SImageColour* psDest, CImageColour* pcColour);
	bool				IsEmpty(void);
	EPrimitiveType		GetType(void);

	uint32				GetBitSize(void);
	uint32				GetByteSize(void);
	CChannelsAccessor*	GetAccessor(void);
	CImage*				GetImage(void);
	uint32				GetWidth(void);
};


#endif // __IMAGE_ACCESSOR_H__

