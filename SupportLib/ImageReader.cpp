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
#include <math.h>
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "ColourARGB32.h"
#include "PNGReader.h"
#include "TGAReader.h"
#include "BMPReader.h"
#include "RADReader.h"
#include "ImageCopier.h"
#include "ImageReaderHelper.h"
#include "ImageReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> ReadImage(char* szFilename, EImageType eType, bool bAddDebug)
{
	if (eType == IT_Unknown)
	{
		eType = GuessImageType(szFilename);
	}

	switch (eType)
	{
		case IT_BMP:
			return (LoadBMP(szFilename, bAddDebug));
		case IT_TGA:
			return (LoadTGA(szFilename, bAddDebug));
		case IT_PNG:
			return (LoadPNG(szFilename, bAddDebug));
		case IT_RAD:
			return (LoadRAD(szFilename)); // uses a text-format RAW descriptor (RAD) file to load a raw file.
	}

	gcLogger.Error2(__METHOD__, " Could not read image of type [", ImageTypeToString(eType), " (", IntToString(eType), ")].", NULL);
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadRawImage(Ptr<CImage> pcImage, char* szFilename)
{
	return LoadRAW(szFilename, pcImage);
}

