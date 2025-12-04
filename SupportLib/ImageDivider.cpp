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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::Kill(void)
{
	mcDestImageCels.Kill();
	mpcImage = NULL;
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
			CImageCelMask*	pcImageCel;

			pcImageCel = mcDestImageCels.Add<CImageCelMask>();
			pcImageCel->Init(&mpcImage, &pcFillMask, pcRect);
		}
		else
		{
			CImageCelMaskTransparent*	pcImageCel;

			pcImageCel = mcDestImageCels.Add<CImageCelMaskTransparent>();
			pcImageCel->Init(&mpcImage, &msTransparentColour, &pcFillMask, pcRect);
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
			CImageCel*	pcImageCel;

			pcImageCel = mcDestImageCels.Add<CImageCel>();
			pcImageCel->Init(mpcImage, pcRect);
		}
		else
		{
			CImageCelTransparent*	pcImageCel;

			pcImageCel = mcDestImageCels.Add<CImageCelTransparent>();
			pcImageCel->Init(&mpcImage, &msTransparentColour, pcRect);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::CropTransparentBorders(void)
{
	SSetIterator	sIter;
	CImageCel*		pcImageCel;

	if (mbCropTransparentBorders)
	{
		pcImageCel = (CImageCel*)mcDestImageCels.StartIteration(&sIter);
		while (pcImageCel)
		{
			pcImageCel->CropTransparentBorders();
			pcImageCel = (CImageCel*)mcDestImageCels.Iterate(&sIter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::CopyCellsTo(CArrayCommonUnknown* pcImageCels)
{
	pcImageCels->AddAll(&mcDestImageCels);

	//Don't kill the image cels when the divider is destroyed.  They're held in another list now.
	mcDestImageCels.KillElements(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayUnknown* CImageDivider::GetDestImageCels(void) {return &mcDestImageCels;}

