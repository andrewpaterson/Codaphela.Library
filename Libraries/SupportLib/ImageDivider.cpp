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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include <string.h>
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
void CImageDivider::Init(Ptr<CImage> pImage, SImageColour* psTransparentColour, bool bIgnoreEmpty, bool bCropTransparentBorders)
{
	PreInit();

	CImageAccessor*		pcAccessor;
	int					iDataSize;

	mpImage = pImage;
	mbIgnoreEmpty = bIgnoreEmpty;
	mbCropTransparentBorders = bCropTransparentBorders;
	if (psTransparentColour)
	{
		pcAccessor = CImageAccessorCreator::Create(&pImage);
		iDataSize = (int)pcAccessor->GetBufferSize();
		pcAccessor->Kill();

		msTransparentColour.Zero();
		memcpy(&msTransparentColour, psTransparentColour, iDataSize);
		mbUseTransparentColour = true;
	}
	else
	{
		mbUseTransparentColour = false;
	}

	PostInit();

	mpDestImageCels = OMalloc<CArrayImageCel>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::Free(void)
{
	mbIgnoreEmpty = false;
	msTransparentColour.Zero();
	mbUseTransparentColour = false;
	mbCropTransparentBorders = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::Class(void)
{
	M_Pointer(mpImage);
	U_Bool(mbIgnoreEmpty);
	U_Data(SImageColour, msTransparentColour);
	U_Bool(mbUseTransparentColour);
	U_Bool(mbCropTransparentBorders);
	M_Pointer(mpDestImageCels);
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
void CImageDivider::GenerateFromBorder(Ptr<CImage> pFillMask)
{
	CImageAccessor*		pcImageAccessor;
	uint32				x, y;
	SImageColour		cCellColour;
	CImageFiller		cFiller;
	CFillRectangle		cFillBounds;
	bool				bResult;
	short				iMask;
	uint32				iHeight;
	uint32				iWidth;

	pcImageAccessor = CImageAccessorCreator::Create(&mpImage);

	//The top left colour is the border colour.
	pcImageAccessor->Get(0, 0, &cCellColour);

	cFiller.Init(&cCellColour, pcImageAccessor, &pFillMask);
	iMask = 1;

	iHeight = mpImage->GetImageHeight();
	iWidth = mpImage->GetImageWidth();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			bResult = cFiller.Fill(x, y, &cFillBounds, iMask);
			if (bResult)
			{
				AddImageCel(&cFillBounds, pFillMask);
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
	CInt32Rect			cFillRect;
	size				iRow;
	size				iColumn;
	int					iLeft;
	int					iTop;

	pcNumbers->Calculate(mpImage->GetImageWidth(), mpImage->GetImageHeight());

	for (iRow = 0; iRow < (size)pcNumbers->miRowCount; iRow++)
	{
		iTop = pcNumbers->miTopOffset + (int)iRow * (pcNumbers->miCelHeight + pcNumbers->miHeightSpacing);

		for (iColumn = 0; iColumn < (size)pcNumbers->miColumnCount; iColumn++)
		{
			iLeft = pcNumbers->miLeftOffset + (int)iColumn * (pcNumbers->miCelWidth + pcNumbers->miWidthSpacing);
			
			cFillRect.Init(iLeft, iTop, iLeft + pcNumbers->miCelWidth, iTop + pcNumbers->miCelHeight);
			AddImageCel(&cFillRect);
		}
	}

	CropTransparentBorders();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::GenerateFromRectangles(CArrayInt32Rect* pacRectangles)
{
	size			i;
	CInt32Rect*		pcRectangle;
	size			uiNumElements;

	uiNumElements = pacRectangles->NumElements();
	for (i = 0; i < uiNumElements; i++)
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
void CImageDivider::AddImageCel(CFillRectangle* pcRect, Ptr<CImage> pFillMask)
{
	if (!((pcRect->GetWidth() == 0) && mbIgnoreEmpty))
	{
		if (!mbUseTransparentColour)
		{
			Ptr<CImageCelMask>	pImageCel;

			pImageCel = OMalloc<CImageCelMask>(mpImage, pFillMask, pcRect);
			mpDestImageCels->Add(pImageCel);
		}
		else
		{
			Ptr<CImageCelMaskTransparent>	pImageCel;

			pImageCel = OMalloc<CImageCelMaskTransparent>(mpImage, &msTransparentColour, pFillMask, pcRect);
			mpDestImageCels->Add(pImageCel);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::AddImageCel(CInt32Rect* pcRect)
{
	if (!((pcRect->GetWidth() == 0) && mbIgnoreEmpty))
	{
		if (!mbUseTransparentColour)
		{
			Ptr<CImageCel>	pImageCel = OMalloc<CImageCel>(mpImage, pcRect);
			mpDestImageCels->Add(pImageCel);
		}
		else
		{
			Ptr<CImageCelTransparent> pImageCel = OMalloc<CImageCelTransparent>(mpImage, &msTransparentColour, pcRect);
			mpDestImageCels->Add(pImageCel);
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
	Ptr<CImageCel>		pImageCel;
	bool				bExists;

	if (mbCropTransparentBorders)
	{
		pImageCel = mpDestImageCels->StartIteration(&sIter, &bExists);
		while (bExists)
		{
			if (pImageCel.IsNotNull())
			{
				pImageCel->CropTransparentBorders();
				pImageCel = mpDestImageCels->Iterate(&sIter, &bExists);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDivider::CopyCellsTo(Ptr<CArrayImageCel> pImageCels)
{
	pImageCels->AddAll(mpDestImageCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> CImageDivider::GetDestImageCels(void) { return mpDestImageCels; }

