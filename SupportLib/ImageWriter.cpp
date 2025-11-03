/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ImageFileType.h"
#include "ColourARGB32.h"
#include "RADWriter.h"
#include "SFTWriter.h"
#include "PNGWriter.h"
#include "GDIPlusWriter.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WriteImage(Ptr<CImage> pcImage, char* szFilename, EImageType eType)
{
	if (eType == IT_Unknown)
	{
		eType = GuessImageType(szFilename);
	}

	//Oi! Check to make sure none of the below expect an initialised image.
	switch (eType)
	{
	case IT_BMP:
	case IT_EMF:
	case IT_GIF:
	case IT_ICO:
	case IT_JPG:
	case IT_WMF:
	case IT_TGA:
		WriteGDIPlusImage(pcImage, szFilename, eType);
	case IT_PNG:
		SavePNG(pcImage, szFilename);
		return true;
	case IT_RAD:
		return (SaveRAD(pcImage, szFilename));
	case IT_RAW:
		return (SaveRAW(pcImage, szFilename));
	case IT_SFT:
		return (SaveSFT(pcImage, szFilename));
	}
	return false;
}

