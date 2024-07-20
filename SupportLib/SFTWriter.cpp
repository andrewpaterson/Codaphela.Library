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
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "ImageToR3G3B2A.h"
#include "SFTWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFT(Ptr<CImage> pcImage, char* szFileName)
{
	
	CImageR3G3B2A	cRGBTo8bit;
	CChannel*		pcAlpha;

	cRGBTo8bit.Init();
	pcImage = cRGBTo8bit.Modify(&pcImage);
	cRGBTo8bit.Kill();

	if (pcImage.IsNull())
	{
		return false;
	}

	CFileBasic	mcFile;

	pcAlpha = pcImage->GetChannel(IMAGE_OPACITY);
	if (pcAlpha)
	{
		int xxx = 0;
	}

	mcFile.Init(DiskFile(szFileName));
	if (szFileName)
	{
		if (mcFile.Open(EFM_Write_Create))
		{
			mcFile.WriteData(pcImage->GetData(), pcImage->GetByteSize());
			mcFile.Close();
			mcFile.Kill();
			return true;
		}
		mcFile.Kill();
	}
	return false;
}

