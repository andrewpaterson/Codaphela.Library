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

** ------------------------------------------------------------------------ **/
#include "StandardLib/Unknowns.h"
#include "ImageReader.h"
#include "ObjectSourceImage.h"
#include "ObjectConverterImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterImage::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterImage::Kill(void)
{
	CObjectConverter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// No single extension: the type is guessed from each filename.
//
//////////////////////////////////////////////////////////////////////////
const char* CObjectConverterImage::GetFileExtension(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectConverterImage::IsFor(CAbstractFile* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pcFile is not used: the image is read by szFilename, which is also the
// object's name.
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterImage::CreateSource(CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSourceImage*	pcSource;

	if (StrEmpty(szFilename))
	{
		return NULL;
	}

	pcSource = UMalloc(CObjectSourceImage);
	pcSource->Init(this, szFilename);
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectConverterImage::Convert(CObjectSource* pcSource, const char* szObjectName)
{
	Ptr<CImage>	pImage;

	if (StrEmpty(szObjectName))
	{
		return NULL;
	}

	pImage = ReadImage((char*)szObjectName, IT_Unknown, false, szObjectName);
	if (pImage.IsNull())
	{
		return NULL;
	}
	return (CBaseObject*)pImage.Return();
}

