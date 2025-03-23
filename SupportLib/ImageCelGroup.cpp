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
#include "ImageCelGroup.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelGroup::Init(void)
{
	PreInit();

	mcImageCels.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelGroup::Free(void)
{
	mcImageCels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelGroup::Class(void)
{
	CObject::ClassNotImplemented();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelGroup::Load(CObjectReader* pcFile)
{
	return false;
}


bool CImageCelGroup::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelGroup::AddCel(CImageCel* pcCel)
{
	mcImageCels.Add(pcCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelGroup::AddCels(CArrayCommonUnknown* pcCels)
{
	mcImageCels.AddAll(pcCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImageCelGroup::NumCels(void)
{
	return mcImageCels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageCel* CImageCelGroup::GetCel(size iIndex)
{
	return mcImageCels.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImage* CImageCelGroup::GetImage(void)
{
	size			i;
	CImageCel*		pcCel;
	Ptr<CImage>		pcImage;

	//If there is only one image in use by the group (the normal case) then return it, otherwise NULL.
	pcImage = NULL;
	for (i = 0; i < mcImageCels.NumElements(); i++)
	{
		pcCel = mcImageCels.Get(i);
		if (pcImage == NULL)
		{
			pcImage = pcCel->GetSourceImage();
		}
		else if (pcImage != pcCel->GetSourceImage())
		{
			return NULL;
		}
	}
	return &pcImage;
}

