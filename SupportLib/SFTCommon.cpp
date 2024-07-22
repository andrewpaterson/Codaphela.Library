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
#include "BaseLib/IntegerHelper.h"
#include "SFTCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTImage::Init(uint16 uiType, uint16 uiImageWidth, uint16 uiImageHeight)
{
	this->uiType = uiType;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
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
bool SSFTCelRun::IsStartOfRun(void)
{
	return FixBool(uiFlags & 0x8000);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 SSFTCelRun::GetRunLength(void)
{
	return uiRunLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 SSFTCelRun::GetSkipLength(void)
{
	return uiFlags & 0x7fff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTCelRun::IsDone(void)
{
	return (uiFlags == 0) && (uiRunLength == 0);
}


