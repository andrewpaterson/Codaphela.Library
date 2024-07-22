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
#include "SFTCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTOpaque::Init(uint16 uiImageWidth, uint16 uiImageHeight)
{
	this->uiType = SFT_TYPE_OPAQUE;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTOpaqueCel::Init(uint16 uiImageWidth, uint16 uiImageHeight, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	this->uiType = SFT_TYPE_OPAQUE_CEL;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
	this->iCelLeft = iCelLeft;
	this->iCelTop = iCelTop;
	this->uiCelWidth = uiCelWidth;
	this->uiCelHeight = uiCelHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTCelRun::Init(bool bStartOfRun)
{
	if (bStartOfRun)
	{
		uiFlags = 0x8000;
	}
	else
	{
		uiFlags = 0;
	}
	uiRunLength = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTCelRun::Set(uint16 uiRunLength, uint16 uiSkipLength)
{
	this->uiFlags |= (uiSkipLength & 0x7fff);
	this->uiRunLength = uiRunLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTTransparentCel::Init(uint16 uiImageWidth, uint16 uiImageHeight, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	this->uiType = SFT_TYPE_TRANSPARENT_CEL;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
	this->iCelLeft = iCelLeft;
	this->iCelTop = iCelTop;
	this->uiCelWidth = uiCelWidth;
	this->uiCelHeight = uiCelHeight;
}

