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
#include "StandardLib/ChannelsCopier.h"
#include "ImageAccessorCreator.h"
#include "Image.h"
#include "ImageCel.h"
#include "ImageCopier.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Init(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	CImageAccessorCreator	cCreator;

	//Now we have an accessor that takes what's in the source and converts it to the destinations format.
	//Although it may have gaps if the destination has channels the source does not.
	cCreator.Init(pcSource);
	cCreator.AddAccess(pcDest);
	mpcSourceAccessor = cCreator.CreateAndKill();

	//This looks a bit retarded but it creates an accessor capable of readin the source accesses output.
	cCreator.Init(pcDest);
	cCreator.AddAccess(mpcSourceAccessor);
	mpcDestAccessor = cCreator.CreateAndKill();

	mpcSource = pcSource;
	mpcDest = pcDest;
	mbKillAccessors = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Init(CImageAccessor* pcSourceAccessor, CImageAccessor* pcDestAccessor)
{
	mpcSourceAccessor = pcSourceAccessor;
	mpcDestAccessor = pcDestAccessor;
	mpcSource = pcSourceAccessor->GetImage();
	mpcDest = pcDestAccessor->GetImage();
	mbKillAccessors = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Kill(void)
{
	if (mbKillAccessors)
	{
		mpcSourceAccessor->Kill();
		mpcDestAccessor->Kill();
	}
	mpcSourceAccessor = NULL;
	mpcDestAccessor = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(int iDestX, int iDestY, CImageCel* pcSourceCel)
{
	SInt2	sPos;

	sPos = pcSourceCel->GetSubImage()->GetImageDestPos(iDestX, iDestY);
	Copy(sPos.x, sPos.y, &pcSourceCel->GetSubImage()->mcImageRect);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(int iDestX, int iDestY, CRectangle* psSourceRect)
{
	if (psSourceRect)
	{
		Copy(iDestX, iDestY, psSourceRect->miLeft, psSourceRect->miTop, psSourceRect->miRight, psSourceRect->miBottom);
	}
	else
	{
		Copy(iDestX, iDestY, 0, 0, mpcSource->miWidth, mpcSource->miHeight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(int iDestX, int iDestY, int iSourceX1, int iSourceY1, int iSourceX2, int iSourceY2)
{
	int						isx;
	int						isy;
	int						idx;
	int						idy;
	void*					pvData;

	if (mpcDestAccessor->IsEmpty())
	{
		//There was no overlap between the channels in the images.
		return;
	}

	if (iDestY < 0)
	{
		iSourceY1 += -iDestY;
		iDestY = 0;
	}
	if (iDestX < 0)
	{
		iSourceX1 += -iDestX;
		iDestX = 0;
	}

	if ((iSourceX1 >= iSourceX2) || (iSourceY1 >= iSourceY2))
	{
		return;
	}

	if (((iSourceX2 - iSourceX1) + iDestX) >= mpcDest->GetWidth())
	{
		iSourceX2 = iSourceX1 + mpcDest->GetWidth() - iDestX;
	}
	if (((iSourceY2 - iSourceY1) + iDestY) >= mpcDest->GetHeight())
	{
		iSourceY2 = iSourceY1 + mpcDest->GetHeight() - iDestY;
	}

	if ((iSourceX2 <= 0) || (iSourceY2 <= 0))
	{
		return;
	}

	mpcSourceAccessor->SyncDataCache();
	mpcDestAccessor->SyncDataCache();

	for (isy = iSourceY1; isy < iSourceY2; isy++)
	{
		idy = iDestY + isy - iSourceY1;
		for (isx = iSourceX1; isx < iSourceX2; isx++)
		{
			idx = iDestX + isx - iSourceX1;

			pvData = mpcSourceAccessor->Get(isx, isy);
			mpcDestAccessor->Set(idx, idy, pvData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(Ptr<CImage> pcSource, Ptr<CImage> pcDest, int iDestX, int iDestY, CRectangle* psSourceRect)
{
	CImageCopier	cCopier;

	cCopier.Init(pcSource, pcDest);
	cCopier.Copy(iDestX, iDestY, psSourceRect);
	cCopier.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(Ptr<CImage> pcSource, Ptr<CImage> pcDest, int iDestX, int iDestY, int iSourceX1, int iSourceY1, int iSourceX2, int iSourceY2)
{
	CImageCopier	cCopier;

	cCopier.Init(pcSource, pcDest);
	cCopier.Copy(iDestX, iDestY, iSourceX1, iSourceY1, iSourceX2, iSourceY2);
	cCopier.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(CImageCel* pcSource, Ptr<CImage> pcDest, int iDestX, int iDestY)
{
	CImageCopier cCopier;

	cCopier.Init(pcSource->GetSourceImage(), pcDest);
	cCopier.Copy(iDestX, iDestY, pcSource);
	cCopier.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	CImageCopier cCopier;

	cCopier.Init(pcSource, pcDest);
	cCopier.Copy(0, 0, 0, 0, pcSource->GetWidth(), pcSource->GetHeight());
	cCopier.Kill();
}