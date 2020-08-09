/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __PIXEL_OPACITY_H__
#define __PIXEL_OPACITY_H__
#include "ImageAccessorCreator.h"
#include "ImageColour.h"


class CImage;
class CPixelOpacityBase : public CUnknown
{
CONSTRUCTABLE(CPixelOpacityBase);
public:
	virtual void Kill(void);
	virtual BOOL IsTransparent(int x, int y) =0;
};


class CPixelOpacityChannel : public CPixelOpacityBase
{
CONSTRUCTABLE(CPixelOpacityChannel);
protected:
	CImageAccessor*		mpcOpacityChannel;
	BOOL				mbHasOpacity;

public:
	void	Init(CImage* pcImage);
	void	Kill(void);

	BOOL	IsTransparent(int x, int y);
};


class CPixelOpacityTransparentColour : public CPixelOpacityChannel
{
CONSTRUCTABLE(CPixelOpacityTransparentColour);
protected:
	SImageColour		msTransparentColour;
	CImageAccessor*		mpcAllChannels;

public:
	void	Init(CImage* pcImage, SImageColour* psTransparentColour);
	void	Kill(void);

	BOOL	IsTransparent(int x, int y);
};


class CPixelOpacityMask : public CPixelOpacityChannel
{
CONSTRUCTABLE(CPixelOpacityMask);
protected:
	CImageAccessor*		mpcMaskChannel;
	short				miMask;

public:
	void	Init(CImage* pcSourceImage, CImage* pcMaskImage, short iMask);
	void	Kill(void);

	BOOL	IsTransparent(int x, int y);
};


class CPixelOpacityMaskTransparentColour : public CPixelOpacityMask
{
CONSTRUCTABLE(CPixelOpacityMaskTransparentColour);
protected:
	SImageColour		msTransparentColour;
	CImageAccessor*		mpcAllChannels;

public:
	void	Init(CImage* pcImage, SImageColour* psTransparentColour, CImage* pcMaskImage, short iMask);
	void	Kill(void);

	BOOL	IsTransparent(int x, int y);
};


#endif // __PIXEL_OPACITY_H__

