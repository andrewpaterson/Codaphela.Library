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
//eg:  The CImageAccessor cannot answer a question such as: GetOpacity, this is done elsewhere.
class CImage;
class CImageAccessor : public CUnknown
{
BASE_FUNCTIONS(CImageAccessor);
protected:
	CChannelsAccessor*	mpcAccessor;
	Ptr<CImage>			mpcImage;
	int					miWidth;

public:
	void 				Init(Ptr<CImage> pcImage, CChannelsAccessor* pcAccessor);
	virtual void		Kill(void);

	virtual void*		Get(int x, int y);
	virtual void		Set(int x, int y, void* pvData);

	int					GetBufferSize(void);
	virtual void		SyncDataCache(void);

	void				Get(int x, int y, void* pvDest);
	BOOL				IsValid(int x, int y);
	void*				SafeGet(int x, int y);
	void				SafeSet(int x, int y, void* pvData);

	BOOL				MakeColour(SImageColour* psDest, CImageColour* pcColour);
	BOOL				IsEmpty(void);
	EPrimitiveTypes		GetType(void);

	int					GetBitSize(void);
	int					GetByteSize(void);
	CChannelsAccessor*	GetAccessor(void);
	Ptr<CImage>				GetImage(void);
	int					GetWidth(void);
};


#endif // __IMAGE_ACCESSOR_H__

