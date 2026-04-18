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
bool CImageCopier::Init(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	CImageAccessorCreator	cCreator;

	//Now we have an accessor that takes what's in the source and converts it to the destinations format.
	//Although it may have gaps if the destination has channels the source does not.
	cCreator.Init(&pcSource);
	cCreator.AddAccess(&pcDest);
	mpcSourceAccessor = cCreator.CreateAndKill();

	if (mpcSourceAccessor)
	{
		//This looks a bit retarded but it creates an accessor capable of reading the source accesses output.
		cCreator.Init(&pcDest);
		cCreator.AddAccess(mpcSourceAccessor);
		mpcDestAccessor = cCreator.CreateAndKill();
	}
	else
	{
		mpcDestAccessor = NULL;
	}

	mpcSource = pcSource;
	mpcDest = pcDest;
	mbKillAccessors = true;

	return (mpcSourceAccessor != NULL) && (mpcDestAccessor != NULL);
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
		if (mpcSourceAccessor)
		{
			mpcSourceAccessor->Kill();
		}
		if (mpcDestAccessor)
		{
			mpcDestAccessor->Kill();
		}
	}
	mpcSourceAccessor = NULL;
	mpcDestAccessor = NULL;

	mpcSource = NULL;
	mpcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(int iDestX, int iDestY, CImageCel* pcSourceCel)
{
	SInt2		sPos;
	SImageCopy	sCopy;

	pcSourceCel->CopyParam(&sCopy, iDestX, iDestY, mpcDest->miWidth, mpcDest->miHeight);
	Copy(&sCopy);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(int iDestX, int iDestY, CRectangle* psSourceRect)
{
	SImageCopy	sCopy;

	if (psSourceRect)
	{
		sCopy.Init(iDestX, iDestY, psSourceRect->miLeft, psSourceRect->miTop, psSourceRect->miRight, psSourceRect->miBottom, mpcDest->miWidth, mpcDest->miHeight);
	}
	else
	{
		sCopy.Init(iDestX, iDestY, 0, 0, mpcSource->miWidth, mpcSource->miHeight, mpcDest->miWidth, mpcDest->miHeight);
	}

	Copy(&sCopy);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(int iDestX, int iDestY, int iSourceX1, int iSourceY1, int iSourceX2, int iSourceY2)
{
	SImageCopy	sCopy;

	sCopy.Init(iDestX, iDestY, iSourceX1, iSourceY1, iSourceX2, iSourceY2, mpcDest->miWidth, mpcDest->miHeight);
	Copy(&sCopy);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCopier::Copy(SImageCopy* psCopy)
{
	uint	isx;
	uint	isy;
	uint	idx;
	uint	idy;
	void*	pvData;

	if (mpcDestAccessor->IsEmpty() || (!psCopy->bValid))
	{
		//There was no overlap between the channels in the images.
		return;
	}

	mpcSourceAccessor->SyncDataCache();
	mpcDestAccessor->SyncDataCache();

	for (isy = psCopy->uiSourceY1; isy < psCopy->uiSourceY2; isy++)
	{
		idy = psCopy->uiDestY + isy - psCopy->uiSourceY1;
		for (isx = psCopy->uiSourceX1; isx < psCopy->uiSourceX2; isx++)
		{
			idx = psCopy->uiDestX + isx - psCopy->uiSourceX1;

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