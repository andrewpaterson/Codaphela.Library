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
#include "ImageReader.h"
#include "ImageSourceDiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageSourceDiskFile::Init(char* szFileName, char* szImageName)
{
	CImageSource::Init(szImageName);
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageSourceDiskFile::Kill(void)
{
	mszFileName.Kill();
	CImageSource::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageSourceDiskFile::LoadImage(void)
{
	bool		bResult;
	CImage*		pcImage;
		
	mpcImage = Allocate(mszFileName.Text());
	pcImage = (CImage*)mpcImage.Object();  //Hack so I don't need to re-write the image reader at this moment.
	bResult = ReadImage(pcImage, mszFileName.Text());
	if (!bResult)
	{
		mpcImage->Kill();
		return false;
	}
	return true;
}

