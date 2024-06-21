/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "ImageCombiner.h"
#include "ImageReader.h"
#include "ImageWriter.h"
#include "ImageAccessorCreator.h"
#include "ImageCopier.h"
#include "RectangleBestPacker.h"
#include "RectanglePow2Packer.h"
#include "RectangleLinePacker.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Init(EImageCombineLayout eLayout, int iWidth, int iHeight, EImageCombineSize eSize, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bKillDestCels)
{
	meSize = eSize;
	msSize.Init(iWidth, iHeight);
	meLayout = eLayout;
	mpcDestImage = NULL;
	meChannels = eChannels;
	mcSourceCels.Init();
	mcSourceCels.KillElements(false);
	mcDestCels.Init();
	mcDestCels.KillElements(bKillDestCels);
	miOutsideEdgeWidth = iOutsideEdgeWidth;
	miInnerEdgeWidth = iInnerEdgeWidth;
	masChannels.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Init(EImageCombineLayout eLayout, EImageCombineSize eSize, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bKillDestCels)
{
	Init(eLayout, -1, -1, eSize, eChannels, iOutsideEdgeWidth, iInnerEdgeWidth, bKillDestCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Init(EImageCombineLayout eLayout, int iWidth, int iHeight, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bKillDestCels)
{
	Init(eLayout, iWidth, iHeight, ICS_UserSpecified, eChannels, iOutsideEdgeWidth, iInnerEdgeWidth, bKillDestCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Kill(void)
{
	masChannels.Kill();
	mcSourceCels.Kill();
	mcDestCels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageCombiner::Combine(void)
{
	CArrayPackedRectangle	acPackedRects;
	SInt2					sTextureSize;

	if (mcSourceCels.NumElements() > 0)
	{
		sTextureSize = Pack(&acPackedRects);
		mpcDestImage = InitiailiseDestImage(sTextureSize);
		if (mpcDestImage.IsNull())
		{
			return false;
		}

		Draw(&acPackedRects);
		CreateDestCels(&acPackedRects);
		acPackedRects.Kill();
		return mpcDestImage;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::AddCel(CImageCel* pcCel)
{
	mcSourceCels.Add(pcCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::AddCels(CArrayUnknown* pcCels)
{
	mcSourceCels.AddAll(pcCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::AddChannel(EChannel eChannel, EPrimitiveType eType)
{
	SChannel*	psChannel;

	psChannel = masChannels.Add();
	psChannel->eType = eType;
	psChannel->iChannel = eChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageCombiner::InitiailiseDestImage(SInt2 sSize)
{
	bool			bResult; 
	Ptr<CImage>		pcImage;

	pcImage = OMalloc<CImage>();
	pcImage->Init();
	pcImage->BeginChange();

	if (meChannels != ICC_UserSpecified)
	{
		CalculateChannels();
	}
	pcImage->AddChannels(&masChannels);

	pcImage->SetSize(sSize.x, sSize.y);
	bResult = pcImage->EndChange();
	if (bResult)
	{
		return pcImage;
	}
	else
	{
		//Should this be ONull?
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::CalculateChannels(void)
{
	if (meChannels == ICC_ARGB8bit)
	{
		AddChannel(IMAGE_OPACITY, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_RED, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_GREEN, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	else if (meChannels == ICC_RGBA8bit)
	{
		AddChannel(IMAGE_DIFFUSE_RED, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_GREEN, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_BLUE, PT_uint8);
		AddChannel(IMAGE_OPACITY, PT_uint8);
	}
	else if (meChannels == ICC_RGB8bit)
	{
		AddChannel(IMAGE_DIFFUSE_RED, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_GREEN, PT_uint8);
		AddChannel(IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	else if (meChannels == ICC_FromCels)
	{
		CalculateChannelsFromCels();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::CalculateChannelsFromCels(void)
{
	SSetIterator	sIter;
	CImageCel*		pcCel;
	CArrayChannel	asChannels;
	CImage*			pcLastImage;

	pcLastImage = NULL;
	pcCel = (CImageCel*)mcSourceCels.StartIteration(&sIter);
	while (pcCel)
	{
		if (pcCel->GetSourceImage() != pcLastImage)
		{
			pcCel->GetAllChannels(&asChannels);
			UpdateChannels(&asChannels);
			asChannels.Kill();

			pcLastImage = pcCel->GetSourceImage();
		}
			
		pcCel = (CImageCel*)mcSourceCels.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::UpdateChannels(CArrayChannel* pasSource)
{
	size		i;
	SChannel*	psSource;
	SChannel*	psDest;
	size		iSourceSize;
	size		iDestSize;
	size		iIndex;

	for (i = 0; i < pasSource->NumElements(); i++)
	{
		psSource = pasSource->Get(i);

		iIndex = masChannels.FindWithIntKey(psSource->iChannel, 0);
		if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			psDest = masChannels.Get(iIndex);
			iSourceSize = gcTypeNames.GetByteSize(psSource->eType);
			iDestSize = gcTypeNames.GetByteSize(psDest->eType);

			if (iSourceSize > iDestSize)
			{
				psDest->eType = psSource->eType;
			}
		}
		else
		{
			AddChannel(psSource->iChannel, psSource->eType);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRectanglePacker* CImageCombiner::GetPacker(void)
{
	if (meLayout == ICL_Best)
	{
		if (meSize == ICS_Arbitrary)
		{
			CRectangleBestPacker* pcPacker;

			pcPacker = UMalloc(CRectangleBestPacker);
			pcPacker->Init(miOutsideEdgeWidth, miInnerEdgeWidth);
			return pcPacker;
		}
		else if (meSize == ICS_PowerOf2)
		{
			CRectanglePow2Packer* pcPacker;

			pcPacker = UMalloc(CRectanglePow2Packer);
			pcPacker->Init(miOutsideEdgeWidth, miInnerEdgeWidth);
			return pcPacker;
		}
		else if (meSize == ICS_UserSpecified)
		{
			CRectangleBestPacker* pcPacker;

			pcPacker = UMalloc(CRectangleBestPacker);
			pcPacker->Init(miOutsideEdgeWidth, miInnerEdgeWidth);
			pcPacker->AddAttempt(msSize.x);

			return pcPacker;
		}
	}
	else 
	{
		CRectangleLinePacker* pcPacker;

		pcPacker = UMalloc(CRectangleLinePacker);
		pcPacker->Init(meLayout == ICL_LeftToRight, meSize == ICS_PowerOf2, miOutsideEdgeWidth, miInnerEdgeWidth);
		return pcPacker;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CImageCombiner::Pack(CArrayPackedRectangle* pacPackedRects)
{
	CRectanglePacker*		pcPacker;
	SSetIterator			sIter;
	CImageCel*				pcCel;
	SInt2					sTextureSize;
	bool					bResult;
	bool					bNonEmpty;

	pcPacker = GetPacker();

	pcCel = (CImageCel*)mcSourceCels.StartIteration(&sIter);
	bNonEmpty = false;
	while (pcCel)
	{
		if (!pcCel->GetSubImage()->IsImageEmpty())
		{
			bResult = pcPacker->AddRectangle(pcCel);
			bNonEmpty = true;
		}
		pcCel = (CImageCel*)mcSourceCels.Iterate(&sIter);
	}

	pcPacker->Pack();

	pacPackedRects->Init();
	pcPacker->GetPackedRectangles(pacPackedRects);
	sTextureSize = pcPacker->GetPackedImageSize();
	pcPacker->Kill();

	return sTextureSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::CreateDestCels(CArrayPackedRectangle* pacPackedRects)
{
	CImageCel*			pcCelDest;
	size				i;
	CPackedRectangle*	pcPackedRect;
	CImageCel*			pcCelSource;

	for (i = 0; i < mcSourceCels.NumElements(); i++)
	{
		pcCelSource = (CImageCel*)mcSourceCels.Get(i);
		pcPackedRect = GetPackedRectangle(pacPackedRects, pcCelSource);
		
		pcCelDest = (CImageCel*)mcDestCels.Add<CImageCel>();
		if (pcPackedRect)
		{
			pcCelDest->Init(&mpcDestImage, pcPackedRect);
		}
		else
		{
			pcCelDest->Init(&mpcDestImage, 0, 0, 0, 0);
		}
		pcCelDest->GetSubImage()->msOffsetTopLeft = pcCelSource->GetSubImage()->msOffsetTopLeft;
		pcCelDest->GetSubImage()->msOffsetBottomRight = pcCelSource->GetSubImage()->msOffsetBottomRight;
		pcCelDest->GetSubImage()->msAlignment = pcCelSource->GetSubImage()->msAlignment;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackedRectangle* CImageCombiner::GetPackedRectangle(CArrayPackedRectangle* pacPackedRects, CImageCel* pcSourceCel)
{
	CPackedRectangle*		pcPackedRect;
	CPackedRectangle*		pcFoundPackedRect;
	size					j;

	pcFoundPackedRect = NULL;
	for (j = 0; j < pacPackedRects->NumElements(); j++)
	{
		pcPackedRect = pacPackedRects->Get(j);
		if ((CImageCel*)pcPackedRect->mpvUserData == pcSourceCel)
		{
			pcFoundPackedRect = pcPackedRect;
			break;
		}
	}
	return pcFoundPackedRect;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Draw(CArrayPackedRectangle* pacPackedRects)
{
	size					i;
	CPackedRectangle*		pcPackedRect;
	CImageCel*				pcCelSource;
	CImageAccessor*			pcSource;
	CImageAccessor*			pcDest;
	CImage*					pcLastImage;
	CArrayInt				aiDestChannels;
	CArrayInt				aiSourceChannels;
	CArrayInt				aiIntersectChannels;
	bool					bDestHasOpacity;
	CChannel*				pcOpacityChannel;
	CImageAccessor*			pcDestOpacity;
	int						iDestTop;
	int						iDestLeft;
	int						iSourceTop;
	int						iSourceLeft;
	int						iSourceBottom;
	int						iSourceRight;
	CImageCopier			cCopier;

	mpcDestImage->Clear();

	bDestHasOpacity = false;
	pcOpacityChannel = mpcDestImage->GetChannel(IMAGE_OPACITY);
	if (pcOpacityChannel)
	{
		bDestHasOpacity = true;
		pcDestOpacity = CImageAccessorCreator::Create(&mpcDestImage, pcOpacityChannel->eType, IMAGE_OPACITY, CHANNEL_ZERO);
	}
	else
	{
		pcDestOpacity = NULL;
	}

	pcLastImage = NULL;
	pcDest = CImageAccessorCreator::CreateEmpty();
	pcSource = CImageAccessorCreator::CreateEmpty();
	for (i = 0; i < pacPackedRects->NumElements(); i++)
	{
		pcPackedRect = pacPackedRects->Get(i);
		pcCelSource = (CImageCel*)pcPackedRect->mpvUserData;

		if (pcLastImage != pcCelSource->GetSourceImage())
		{
			mpcDestImage->GetAllChannels(&aiDestChannels);
			pcCelSource->GetSourceImage()->GetAllChannels(&aiSourceChannels);
			aiIntersectChannels.Init();
			aiIntersectChannels.Intersect(&aiDestChannels, &aiSourceChannels);
			aiDestChannels.Kill();
			aiSourceChannels.Kill();

			pcDest->Kill();
			pcDest = CImageAccessorCreator::Create(&mpcDestImage, &aiIntersectChannels);

			pcSource->Kill();
			pcSource = CImageAccessorCreator::Create(pcCelSource->GetSourceImage(), &aiIntersectChannels);

			aiIntersectChannels.Kill();
			pcLastImage = pcCelSource->GetSourceImage();
		}

		iDestLeft = pcPackedRect->miLeft;
		iDestTop = pcPackedRect->miTop;
		iSourceLeft = pcCelSource->GetSubImage()->mcImageRect.miLeft;
		iSourceTop = pcCelSource->GetSubImage()->mcImageRect.miTop;
		iSourceRight = pcCelSource->GetSubImage()->mcImageRect.miRight;
		iSourceBottom = pcCelSource->GetSubImage()->mcImageRect.miBottom;

		cCopier.Init(pcSource, pcDest);
		cCopier.Copy(iDestLeft, iDestTop, iSourceLeft, iSourceTop, iSourceRight, iSourceBottom);
		cCopier.Kill();

		if (bDestHasOpacity)
		{
			if (pcCelSource->MustFixDrawOpacity())
			{
				pcCelSource->FixDrawOpacity(pcDestOpacity, iDestLeft, iDestTop);
			}
		}
	}

	if (bDestHasOpacity)
	{
		pcDestOpacity->Kill();
	}
	pcSource->Kill();
	pcDest->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayUnknown* CImageCombiner::GetCels(void)
{
	return &mcDestCels;
}

