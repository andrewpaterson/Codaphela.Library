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
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "ImageFileType.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EImageType GuessImageType(char *szFilename)
{
	//char*		szExtension;
	int			iIndex;
	CChars		szTest;
	EImageType	eType;
	CFileUtil	cFileUtil;
	
	iIndex = cFileUtil.FindExtension(szFilename);
	szTest.Init(szFilename, iIndex);
	
	//----------------------------------------
	// guess the file format based on the 
	// extension. Not very robust but good
	// enough for now.
	//----------------------------------------
	if (szTest.EqualsIgnoreCase(".bmp"))
		eType = IT_BMP;
	else if (szTest.EqualsIgnoreCase(".png"))
		eType = IT_PNG;
	else if (szTest.EqualsIgnoreCase(".emf"))
		eType = IT_EMF;
	else if (szTest.EqualsIgnoreCase(".gif"))
		eType = IT_GIF;
	else if (szTest.EqualsIgnoreCase(".ico"))
		eType = IT_ICO;
	else if (szTest.EqualsIgnoreCase(".jpg"))
		eType = IT_JPG;
	else if (szTest.EqualsIgnoreCase(".jpeg"))
		eType = IT_JPG;
	else if (szTest.EqualsIgnoreCase(".jfif"))
		eType = IT_JPG;
	else if (szTest.EqualsIgnoreCase(".wmf"))
		eType = IT_WMF;
	else if (szTest.EqualsIgnoreCase(".tga"))
		eType = IT_TGA;
	else if (szTest.EqualsIgnoreCase(".rad"))
		eType = IT_RAD;
	else if (szTest.EqualsIgnoreCase(".raw"))
		eType = IT_RAW;
	else if (szTest.EqualsIgnoreCase(".sft"))
		eType = IT_SFT;
	else
		eType = IT_Unknown;
	szTest.Kill();

	return eType;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ImageTypeToString(EImageType eType)
{
	char* sz;

	switch (eType)
	{
	case IT_BMP:
		sz = "BMP";
		break;
	case IT_PNG:
		sz = "PNG";
		break;
	case IT_EMF:
		sz = "EMF";
		break;
	case IT_GIF:
		sz = "GIF";
		break;
	case IT_ICO:
		sz = "ICO";
		break;
	case IT_JPG:
		sz = "JPG";
		break;
	case IT_WMF:
		sz = "WMF";
		break;
	case IT_TGA:
		sz = "TGA";
		break;
	case IT_RAD:
		sz = "RAD";
		break;
	case IT_RAW:
		sz = "RAW";
		break;
	case IT_SFT:
		sz = "SFT";
		break;
	case IT_Unknown:
		sz = "Unknown";
		break;
	default:
		sz = "Unknown";
		break;
	}

	return sz;
}

