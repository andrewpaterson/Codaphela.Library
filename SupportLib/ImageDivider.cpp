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
#include "BaseLib/FastFunctions.h"
#include "StandardLib/ClassDefines.h"
#include "ImageAccessorCreator.h"
#include "ImageColour.h"
#include "ImageCelMask.h"
#include "ImageCelTransparent.h"
#include "ImageCelMaskTransparent.h"
#include "ImageDivider.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::Init(Ptr<CImage> pcImage, SImageColour* psTransparentColour, bool bIgnoreEmpty, bool bCropTransparentBorders)
{
	PreInit();

	CImageAccessor*		pcAccessor;
	int					iDataSize;

	mpcImage = pcImage;
	mbIgnoreEmpty = bIgnoreEmpty;
	mbCropTransparentBorders = bCropTransparentBorders;
	if (psTransparentColour)
	{
		pcAccessor = CImageAccessorCreator::Create(&pcImage);
		iDataSize = pcAccessor->GetBufferSize();
		pcAccessor->Kill();

		msTransparentColour.Zero();
		memcpy_fast(&msTransparentColour, psTransparentColour, iDataSize);
		mbUseTransparentColour = true;
	}
	else
	{
		mbUseTransparentColour = false;
	}
	mcDestImageCels.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::Class(void)
{
	M_Pointer(mpcImage);
	U_Bool(mbIgnoreEmpty);
	U_Data(SImageColour, msTransparentColour);
	U_Bool(mbUseTransparentColour);
	U_Bool(mbCropTransparentBorders);
	M_Embedded(mcDestImageCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDivider::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDivider::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::GenerateFromBorder(Ptr<CImage> pcFillMask)
{
	CImageAccessor*		pcImageAccessor;
	int					x, y;
	SImageColour		cCellColour;
	CImageFiller		cFiller;
	CFillRectangle		cFillBounds;
	bool				bResult;
	short				iMask;

	pcImageAccessor = CImageAccessorCreator::Create(&mpcImage);

	//The top left colour is the border colour.
	pcImageAccessor->Get(0, 0, &cCellColour);

	cFiller.Init(&cCellColour, pcImageAccessor, &pcFillMask);
	iMask = 1;

	for (y = 0; y < mpcImage->miHeight; y++)
	{
		for (x = 0; x < mpcImage->miWidth; x++)
		{
			bResult = cFiller.Fill(x, y, &cFillBounds, iMask);
			if (bResult)
			{
				AddImageCel(&cFillBounds, pcFillMask);
				iMask++;
			}
		}
	}

	cFiller.Kill();
	pcImageAccessor->Kill();
	CropTransparentBorders();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::GenerateFromNumbers(CImageDividerNumbers* pcNumbers)
{
	CRectangle			cFillRect;
	int					iRow;
	int					iColumn;
	int					iLeft;
	int					iTop;

	pcNumbers->Calculate(mpcImage->miWidth, mpcImage->miHeight);

	for (iRow = 0; iRow < pcNumbers->iRowCount; iRow++)
	{
		iTop = pcNumbers->iTopOffset + iRow * (pcNumbers->iCelHeight + pcNumbers->iHeightSpacing);

		for (iColumn = 0; iColumn < pcNumbers->iColumnCount; iColumn++)
		{
			iLeft = pcNumbers->iLeftOffset + iColumn * (pcNumbers->iCelWidth + pcNumbers->iWidthSpacing);
			
			cFillRect.Init(iLeft, iTop, iLeft + pcNumbers->iCelWidth, iTop + pcNumbers->iCelHeight);
			AddImageCel(&cFillRect);
		}
	}

	CropTransparentBorders();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::GenerateFromRectangles(CArrayRectangle* pacRectangles)
{
	size			i;
	CRectangle*		pcRectangle;

	for (i = 0; i < pacRectangles->NumElements(); i++)
	{
		pcRectangle = pacRectangles->Get(i);
		AddImageCel(pcRectangle);
	}

	CropTransparentBorders();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::AddImageCel(CFillRectangle* pcRect, Ptr<CImage> pcFillMask)
{
	if (!((pcRect->GetWidth() == 0) && mbIgnoreEmpty))
	{
		if (!mbUseTransparentColour)
		{
			Ptr<CImageCelMask>	pcImageCel;

			pcImageCel = OMalloc<CImageCelMask>(mpcImage, pcFillMask, pcRect);
			mcDestImageCels.Add(pcImageCel);
		}
		else
		{
			Ptr<CImageCelMaskTransparent>	pcImageCel;

			pcImageCel = OMalloc<CImageCelMaskTransparent>(mpcImage, &msTransparentColour, pcFillMask, pcRect);
			mcDestImageCels.Add(pcImageCel);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::AddImageCel(CRectangle* pcRect)
{
	if (!((pcRect->GetWidth() == 0) && mbIgnoreEmpty))
	{
		if (!mbUseTransparentColour)
		{
			Ptr<CImageCel>	pcImageCel = OMalloc<CImageCel>(mpcImage, pcRect);
			mcDestImageCels.Add(pcImageCel);
		}
		else
		{
			Ptr<CImageCelTransparent> pcImageCel = OMalloc<CImageCelTransparent>(mpcImage, &msTransparentColour, pcRect);
			mcDestImageCels.Add(pcImageCel);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::CropTransparentBorders(void)
{
	SSetIterator		sIter;
	Ptr<CImageCel>		pcImageCel;

	if (mbCropTransparentBorders)
	{
		pcImageCel = mcDestImageCels.StartIteration(&sIter);
		while (pcImageCel.IsNotNull())
		{
			pcImageCel->CropTransparentBorders();
			pcImageCel = mcDestImageCels.Iterate(&sIter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::CopyCellsTo(Ptr<CArrayImageCel> pcImageCels)
{
	pcImageCels->AddAll(&mcDestImageCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> CImageDivider::GetDestImageCels(void) {return &mcDestImageCels;}

