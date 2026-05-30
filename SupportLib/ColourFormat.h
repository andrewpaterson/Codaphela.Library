#ifndef __COLOUR_FORMAT_H__
#define __COLOUR_FORMAT_H__
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
#include "BaseLib/DataTypes.h"
#include "ChannelTypes.h"


//Lowest index on the left in memory so RGB would be R[0], G[1], B[2]
enum EColourOrder
{
	CCO_Unknown,
	
	CCO_RGB,
	CCO_BRG,
	CCO_GBR,
	
	CCO_BGR,
	CCO_RBG,
	CCO_GRB,
};


//Lowest index on the left in memory so ARGB would be A[0], R[1], B[2], G[3]
enum EColourFormat
{
	CFT_Unknown,

	CFT_RGB,  //Note, RGB is a stand in for any RGB order.
	CFT_RGBX,
	CFT_XRGB,
	CFT_RGBA,
	CFT_ARGB,
};


enum ERGBColourBits
{
	CRGB_Unknown,

	CRGB_24bit,		//R8 G8 B8
	CRGB_30bit,
	CRGB_36bit,
	CRGB_48bit,
	CRGB_Float3,
	CRGB_8bit332,	//R3G3B2
	CRGB_8bit323,	//R3G2B3
	CRGB_8bit233,	//R2G3B3
	CRGB_6bit,
	CRGB_15bit, 
	CRGB_16bit,		//R5 G6 B5
};


//This also covers Ignored (X) bits.
enum ERGBAlphaBits
{
	ARGB_Unknown,

	ARGB_None,
	ARGB_8bit,
	ARGB_10bit,
	ARGB_12bit,
	ARGB_16bit,
	ARGB_Float,
	ARGB_2bit,
};


enum EColourOpacity
{
	CPO_Unknown,

	CPO_None,		  //There is no opacity channel.
	CPO_Opaque,		  //There is an opacity channel but all pixel are opaque.
	CPO_Translucent,  //Some pixels are translucent (including transparent).
	CPO_Transparent,  //Some pixels are transparent
};



class CColourFormatHelper
{
public:
	EColourFormat	meFormat;
	EColourOrder	meOrder;
	ERGBColourBits	meColourBits;
	ERGBAlphaBits	meAlphaBits;

	void			Init(EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits);

	size			GetNumChannels(void);

	EChannel		GetChannel(size uiIndex);
	EPrimitiveType	GetType(size uiIndex);

	EChannel		GetColourChannel(size uiIndex);
	EPrimitiveType	GetColourType(size uiIndex);
	EPrimitiveType	GetAlphaType(void);

	size			GetFirstColourIndex(void);
	size			GetAlphaIndex(void);
};


#endif // __COLOUR_FORMAT_H__

