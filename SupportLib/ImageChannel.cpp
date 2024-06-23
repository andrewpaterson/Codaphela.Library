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
#include "ImageChannel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageChannelsSource::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannelRGB::NumChannels(void)
{
	return 3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SChannel CChannelRGB::GetChannel(size iIndex)
{
	SChannel	sChannel;

	switch(iIndex)
	{
	case 0:
		sChannel.Init(IMAGE_DIFFUSE_RED, PT_uint8, false);
		break;
	case 1:
		sChannel.Init(IMAGE_DIFFUSE_GREEN, PT_uint8, false);
		break;
	case 2:
		sChannel.Init(IMAGE_DIFFUSE_BLUE, PT_uint8, false);
		break;
	default:
		sChannel.Init(CHANNEL_ZERO, PT_Undefined, false);
	}
	return sChannel;
}

CChannelRGB		gcRGB;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageChannelsSource* CChannelRGB::Get(void)
{
	return &gcRGB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannelRGBA::NumChannels(void)
{
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SChannel CChannelRGBA::GetChannel(size iIndex)
{
	SChannel	sChannel;

	switch(iIndex)
	{
	case 0:
		sChannel.Init(IMAGE_DIFFUSE_RED, PT_uint8, false);
		break;
	case 1:
		sChannel.Init(IMAGE_DIFFUSE_GREEN, PT_uint8, false);
		break;
	case 2:
		sChannel.Init(IMAGE_DIFFUSE_BLUE, PT_uint8, false);
		break;
	case 3:
		sChannel.Init(IMAGE_OPACITY, PT_uint8, false);
		break;
	default:
		sChannel.Init(CHANNEL_ZERO, PT_Undefined, false);
	}
	return sChannel;
}


CChannelRGBA		gcRGBA;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageChannelsSource* CChannelRGBA::Get(void)
{
	return &gcRGBA;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannelRGBX::NumChannels(void)
{
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SChannel CChannelRGBX::GetChannel(size iIndex)
{
	SChannel	sChannel;

	switch(iIndex)
	{
	case 0:
		sChannel.Init(IMAGE_DIFFUSE_RED, PT_uint8, false);
		break;
	case 1:
		sChannel.Init(IMAGE_DIFFUSE_GREEN, PT_uint8, false);
		break;
	case 2:
		sChannel.Init(IMAGE_DIFFUSE_BLUE, PT_uint8, false);
		break;
	case 3:
		sChannel.Init(IMAGE_IGNORED, PT_uint8, false);
		break;
	default:
		sChannel.Init(CHANNEL_ZERO, PT_Undefined, false);
	}
	return sChannel;
}


CChannelRGBX		gcRGBX;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageChannelsSource* CChannelRGBX::Get(void)
{
	return &gcRGBX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintImagePurpose(EImagePurpose ePurpose, CChars* psz)
{
	switch (ePurpose)
	{
	case IP_Unknown:
		psz->Append("Unknown");
		break;
	case IP_Diffuse:
		psz->Append("Diffuse");
		break;
	case IP_Specular:
		psz->Append("Specular");
		break;
	case IP_Illumination:
		psz->Append("Illumination");
		break;
	case IP_Opacity:
		psz->Append("Opacity");
		break;
	case IP_Normal:
		psz->Append("Normal");
		break;
	case IP_Bump:
		psz->Append("Bump");
		break;
	case IP_UVAdjust:
		psz->Append("UVAdjust");
		break;
	case IP_Detail:
		psz->Append("Detail");
		break;
	case IP_Decal:
		psz->Append("Decal");
		break;
	case IP_Mask:
		psz->Append("Mask");
		break;
	case IP_Palette0:
		psz->Append("Palette0");
		break;
	case IP_Palette1:
		psz->Append("Palette1");
		break;
	default:
		psz->Append("INVALID");
		break;
	}
}

