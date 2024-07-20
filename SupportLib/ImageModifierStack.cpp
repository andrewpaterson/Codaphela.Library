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
#include "ImageModifierStack.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierStack::Init(Ptr<CImage> pcImage)
{
	mpcImage = pcImage;
	macModifiers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierStack::Kill(void)
{
	macModifiers.Kill();
	mpcImage = NULL;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierStack::AddModifier(CImageModifier* pcModifier)
{
	macModifiers.Add(pcModifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierStack::SetImage(Ptr<CImage> pcImage)
{
	mpcImage = pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierStack::ApplyAll(void)
{
	size				i;
	CImageModifier*		pcModifier;
	Ptr<CImage>			pcImage;

	if (mpcImage.IsNotNull())
	{
		pcImage = mpcImage;

		for (i = 0; i < macModifiers.NumElements(); i++)
		{
			pcModifier = (CImageModifier*)macModifiers.Get(i);
			pcImage = pcModifier->Modify(pcImage);
		}

		if (pcImage != mpcImage)
		{
			mpcImage->ReInit();
			mpcImage->Copy2(pcImage);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImageModifierStack::NumModifiers(void)
{
	return macModifiers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageModifierStack::GetImage(void) {return mpcImage;}
