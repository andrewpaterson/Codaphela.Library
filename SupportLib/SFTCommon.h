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
#ifndef __SFT_COMMON_H__
#define __SFT_COMMON_H__
#include "BaseLib/PrimitiveTypes.h"


#define SFT_TYPE_OPAQUE			(uint16)0xe457
#define SFT_TYPE_TRANSPARENT	(uint16)0xe4f9
#define SFT_TYPE_CONTAINER		(uint16)0xe4ef


struct SSFTImage
{
	uint16	uiType;
	uint16	uiImageWidth;
	uint16	uiImageHeight;

	void	Init(uint16 uiType, uint16 uiImageWidth, uint16 uiImageHeight);
};


struct SSFTCelRun
{
	uint16	uiFlags;  //0x8000: Start of run flag.  //0x7fff: run skip mask.  or how many pixels to add before the current run starts drawing
	uint16	uiRunLength;

	void	Init(bool bStartOfRun);
	void	Set(uint16 uiRunLength, uint16 uiSkipLength);

	bool	IsStartOfRun(void);
	uint16	GetRunLength(void);
	uint16	GetSkipLength(void);
	bool	IsDone(void);
};


struct SSFTContainer
{
	uint16	uiType;
	uint16	uiCount;  //Count of contained images.
};


struct SSFTContained
{
	uint16	uiSkipToNext;  //How many bytes to seek forward to the next contained (or zero if no more images contained).  Relative to the end of this struct.
	uint16	uiSkipToName;  //How many bytes to seek forward to this images name (or zero if no name).  Relative to the end of this struct.
};  //end of struct == start of SSFT image type.


#endif // __SFT_COMMON_H__

