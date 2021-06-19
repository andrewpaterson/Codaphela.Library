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
#ifndef __IMAGE_IMAGE__H__
#define __IMAGE_IMAGE__H__
#include "StandardLib/Unknown.h"
#include "StandardLib/ChannelsHelper.h"
#include "ChannelTypes.h"


enum EImagePurpose	//3 bytes for purpose
{
	IP_Unknown,
	IP_Diffuse,   		//RGB, I	(I is a single intensity channel)
	IP_Specular,  		//I			(RGB also?)
	IP_Illumination,	//RGB, I	(RGB is a bit weird)
	IP_Opacity,   		//I		
	IP_Normal,    		//XYZ,	
	IP_Bump,			//UV
	IP_UVAdjust,		//UV
	IP_Detail,    		//RGB, I
	IP_Decal,			//RGB, I
	IP_Mask,			//I

	//New purposes go here.

	IP_PaletteBase,		//Index
	IP_Palette0 = IP_PaletteBase,
	IP_Palette1,  //If you have more than 2 palettes you're being silly... but if you want use IP_Palette(p)
};


#define IP_Palette(p)	((EImagePurpose)((int)IP_PaletteBase + p))


#define IMAGE_PURPOSE(iChannel) ((EImagePurpose)((iChannel - 1) / NUM_CHANNEL_TYPES))


#define IMAGE_IGNORED				CHANNEL(IP_Unknown, CT_Ignored)
#define IMAGE_DIFFUSE_GREY			CHANNEL(IP_Diffuse, CT_Intensity)
#define IMAGE_DIFFUSE_RED			CHANNEL(IP_Diffuse, CT_Red)
#define IMAGE_DIFFUSE_GREEN			CHANNEL(IP_Diffuse, CT_Green)
#define IMAGE_DIFFUSE_BLUE			CHANNEL(IP_Diffuse, CT_Blue)
#define IMAGE_OPACITY				CHANNEL(IP_Opacity, CT_Intensity)
#define IMAGE_MASK					CHANNEL(IP_Mask, CT_Intensity)
#define IMAGE_BUMP_U				CHANNEL(IP_Bump, CT_U)
#define IMAGE_BUMP_V				CHANNEL(IP_Bump, CT_V)
#define IMAGE_SPECULAR				CHANNEL(IP_Specular, CT_Intensity)
#define IMAGE_NORMAL_X				CHANNEL(IP_Normal, CT_X)
#define IMAGE_NORMAL_Y				CHANNEL(IP_Normal, CT_Y)
#define IMAGE_NORMAL_Z				CHANNEL(IP_Normal, CT_Z)
#define IMAGE_ILLUMINATION			CHANNEL(IP_Illumination, CT_Intensity)
#define IMAGE_ILLUMINATION_RED		CHANNEL(IP_Illumination, CT_Red)
#define IMAGE_ILLUMINATION_GREEN	CHANNEL(IP_Illumination, CT_Green)
#define IMAGE_ILLUMINATION_BLUE		CHANNEL(IP_Illumination, CT_Blue)


class CImageChannelsSource : public CUnknown
{
CONSTRUCTABLE(CImageChannelsSource);
public:
	void				Kill(void);
	virtual int			NumChannels(void) =0;
	virtual	SChannel	GetChannel(int iIndex) =0;
};


//Standard colours are R8 B8 G8 with R in the low byte.
class CChannelRGB : CImageChannelsSource
{
CONSTRUCTABLE(CChannelRGB);
public:
	static CImageChannelsSource* Get(void);

	int 		NumChannels(void);
	SChannel	GetChannel(int iIndex);
};


class CChannelRGBA : CImageChannelsSource
{
CONSTRUCTABLE(CChannelRGBA);
public:
	static CImageChannelsSource* Get(void);

	int 		NumChannels(void);
	SChannel	GetChannel(int iIndex);
};


class CChannelRGBX : CImageChannelsSource
{
CONSTRUCTABLE(CChannelRGBX);
public:
	static CImageChannelsSource* Get(void);

	int 		NumChannels(void);
	SChannel	GetChannel(int iIndex);
};


#endif // !__IMAGE_IMAGE__H__

