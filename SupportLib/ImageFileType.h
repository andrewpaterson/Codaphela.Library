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
#ifndef __IMAGE_FILE_TYPE_H__
#define __IMAGE_FILE_TYPE_H__


enum EImageType
{
	IT_Unknown,
	IT_BMP,  // Windows BMP 24 bit
	IT_EMF,  // EMF
	IT_GIF,  // GIF 
	IT_ICO,  // Windows ICO 
	IT_JPG,  // 24 Bit JPG format
	IT_WMF,  // WMF
	IT_TGA,  // Targa
	IT_RAD,  // Raw file with a RAW descriptor file (RAD)
	IT_RAW,  // Raw file.
	IT_PNG,  // PNG
	IT_TIFF, // Tiff
	IT_SFT,  // SFT (Andre's Comuputer Format)
};


EImageType GuessImageType(char *szFilename);
char* ImageTypeToString(EImageType eType);


#endif // __IMAGE_FILE_TYPE_H__

