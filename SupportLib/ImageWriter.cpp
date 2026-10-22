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
#include "ImageFileType.h"
#include "ColourARGB32.h"
#include "RADWriter.h"
#include "SFTWriter.h"
#include "PNGWriter.h"
#include "BMPWriter.h"
#include "TGAWriter.h"


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
		return SaveBMP(pcImage, szFilename);
	case IT_TGA:
		return SaveTGA(pcImage, szFilename);
	case IT_PNG:
		return SavePNG(pcImage, szFilename);
	case IT_RAD:
		return (SaveRAD(pcImage, szFilename));
	case IT_RAW:
		return (SaveRAW(pcImage, szFilename));
	case IT_SFT:
		return (SaveSFT(pcImage, szFilename));
	default:
		break;
	}

	gcLogger.Error2(__METHOD__, " Could not write image of type [", ImageTypeToString(eType), " (", IntToString(eType), ")] to [", szFilename, "].", NULL);
	return false;
}

