#ifndef __IMAGE_ROW_BLITTER_CACHE_VALUE_H__
#define __IMAGE_ROW_BLITTER_CACHE_VALUE_H__
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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "StandardLib/Pointer.h"
#include "StandardLib/MapObject.h"
#include "StandardLib/ArrayObject.h"
#include "Image.h"
#include "ImageRowBlitter.h"


class CImageRowBlitterCacheDest : public CObject
{
CONSTRUCTABLE(CImageRowBlitterCacheDest);
DESTRUCTABLE(CImageRowBlitterCacheDest);
protected:
	Ptr<CImage>			mpDestImage;
	CImageRowBlitter*	mpcBlitter;
	size				muiUseCount;

public:
	void				Init(Ptr<CImage> pDestImage, CImageRowBlitter* pcBlitter);
	void				Class(void);
	void				Free(void);

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	bool				Is(Ptr<CImage> pDestImage, const char* szBlitterClass);
	CImageRowBlitter*	Get(void);

	size				AddUsage(void);
	size				RemoveUsage(void);
};


class CImageRowBlitterCacheValue : public CObject
{
CONSTRUCTABLE(CImageRowBlitterCacheValue);
DESTRUCTABLE(CImageRowBlitterCacheValue);
protected:
	Ptr<CImage>		mpSourceImage;
	CArrayObject	maDestBlitters;

public:
	void							Init(Ptr<CImage> pSourceImage);
	void							Class(void);
	void							Free(void);

	bool							Save(CObjectWriter* pcFile) override;
	bool							Load(CObjectReader* pcFile) override;

	CImageRowBlitter*				Get(Ptr<CImage> pDest, const char* szBlitterClass);
	bool							Add(Ptr<CImage> pDest, CImageRowBlitter* pcRowBlitter);
	bool							Remove(Ptr<CImage> pDest, const char* szBlitterClass);

protected:
	Ptr<CImageRowBlitterCacheDest>	GetCacheDest(Ptr<CImage> pDest, const char* szBlitterClass);
};


#endif // __IMAGE_ROW_BLITTER_CACHE_VALUE_H__

