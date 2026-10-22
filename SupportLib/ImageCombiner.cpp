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
#include "BaseLib/Logger.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
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
void CImageCombiner::Init(EImageCombineLayout eLayout, int iWidth, int iHeight, EImageCombineSize eSize, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bNameCels)
{
	PreInit();

	meSize = eSize;
	msSize.Init(iWidth, iHeight);
	meLayout = eLayout;
	mpcDestImage = NULL;
	meChannels = eChannels;
	mcSourceCels.Init();
	maszSourceFileNames.Init();
	mcDestCels.Init();
	miOutsideEdgeWidth = iOutsideEdgeWidth;
	miInnerEdgeWidth = iInnerEdgeWidth;
	masChannels.Init();
	mbNameCels = bNameCels;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Init(EImageCombineLayout eLayout, EImageCombineSize eSize, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bNameCels)
{
	Init(eLayout, -1, -1, eSize, eChannels, iOutsideEdgeWidth, iInnerEdgeWidth, bNameCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Init(EImageCombineLayout eLayout, int iWidth, int iHeight, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bNameCels)
{
	Init(eLayout, iWidth, iHeight, ICS_UserSpecified, eChannels, iOutsideEdgeWidth, iInnerEdgeWidth, bNameCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Free(void)
{
	maszSourceFileNames.Kill();
	masChannels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::Class(void)
{
	U_Enum(meLayout);
	U_Enum(meSize);
	U_Enum(meChannels);
	U_2Int32(msSize);
	M_Pointer(mpcDestImage);
	M_Embedded(mcSourceCels);
	U_Data(CArrayChars, maszSourceFileNames);
	M_Embedded(mcDestCels);
	U_SInt(miOutsideEdgeWidth);
	U_SInt(miInnerEdgeWidth);
	U_Data(CArrayChannel, masChannels);
	U_Bool(mbNameCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCombiner::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCombiner::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageCombiner::Combine(void)
{
	CArrayPackedRectangle	acPackedRects;
	SInt32Vec2					sTextureSize;

	if (mcSourceCels.NumElements() > 0)
	{
		sTextureSize = Pack(&acPackedRects);
		if (acPackedRects.NumElements() == 0)
		{
			//Every cel was empty or the cels did not fit.
			acPackedRects.Kill();
			return NULL;
		}

		mpcDestImage = InitiailiseDestImage(sTextureSize);
		if (mpcDestImage.IsNull())
		{
			acPackedRects.Kill();
			return NULL;
		}

		Draw(&acPackedRects);
		if (!CreateDestCels(&acPackedRects))
		{
			acPackedRects.Kill();
			mpcDestImage = NULL;
			return NULL;
		}
		acPackedRects.Kill();
		return mpcDestImage;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::AddCel(Ptr<CImageCel> pcCel, char* szFileName)
{
	mcSourceCels.Add(pcCel);
	maszSourceFileNames.Add(szFileName ? szFileName : "");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::AddCels(Ptr<CArrayImageCel> pcCels, CArrayChars* paszFileNames)
{
	size	i;
	size	uiNumCels;

	uiNumCels = pcCels->NumElements();
	for (i = 0; i < uiNumCels; i++)
	{
		if (paszFileNames && (i < paszFileNames->NumElements()))
		{
			AddCel(pcCels->Get(i), paszFileNames->Get(i)->Text());
		}
		else
		{
			AddCel(pcCels->Get(i));
		}
	}
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
Ptr<CImage> CImageCombiner::InitiailiseDestImage(SInt32Vec2 sSize)
{
	bool			bResult; 
	Ptr<CImage>		pcImage;

	pcImage = OMalloc<CImage>();
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
	Ptr<CImageCel>	pcCel;
	CArrayChannel	asChannels;
	Ptr<CImage>		pcLastImage;
	bool			bExists;

	pcLastImage = NULL;
	pcCel = mcSourceCels.StartIteration(&sIter, &bExists);
	while (bExists)
	{
		if (pcCel.IsNotNull())
		{
			if (pcCel->GetSourceImage() != pcLastImage)
			{
				asChannels.Init();
				pcCel->GetAllChannels(&asChannels);
				UpdateChannels(&asChannels);
				asChannels.Kill();

				pcLastImage = pcCel->GetSourceImage();
			}
		}
		pcCel = mcSourceCels.Iterate(&sIter, &bExists);
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

		iIndex = masChannels.FindWithIntKey((int)psSource->iChannel, 0);
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
			pcPacker->AddAttempt(msSize.x - miOutsideEdgeWidth * 2);

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
SInt32Vec2 CImageCombiner::Pack(CArrayPackedRectangle* pacPackedRects)
{
	CRectanglePacker*	pcPacker;
	SSetIterator		sIter;
	Ptr<CImageCel>		pcCel;
	SInt32Vec2				sTextureSize;
	bool				bExists;

	pcPacker = GetPacker();

	pcCel = mcSourceCels.StartIteration(&sIter, &bExists);
	while (bExists)
	{
		if (pcCel.IsNotNull())
		{
			if (!pcCel->GetSubImage()->IsImageEmpty())
			{
				pcPacker->AddRectangle(&pcCel);
			}
		}

		pcCel = mcSourceCels.Iterate(&sIter, &bExists);
	}

	pcPacker->Pack();

	pacPackedRects->Init();
	pcPacker->GetPackedRectangles(pacPackedRects);
	sTextureSize = pcPacker->GetPackedImageSize();
	pcPacker->Kill();

	if (meSize == ICS_UserSpecified)
	{
		if ((sTextureSize.x > msSize.x) || (sTextureSize.y > msSize.y))
		{
			//The cels don't fit in the user specified size.
			pacPackedRects->Kill();
			pacPackedRects->Init();
		}
		sTextureSize = msSize;
	}

	return sTextureSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCombiner::CreateDestCels(CArrayPackedRectangle* pacPackedRects)
{
	Ptr<CImageCel>			pcCelDest;
	size					i;
	CPackedRectangle*		pcPackedRect;
	Ptr<CImageCel>			pcCelSource;
	CMapStringInt			mFileNameCounts;
	CMapStringInt			mFileNameIndices;
	CChars					szName;

	mFileNameCounts.Init();
	mFileNameIndices.Init();
	if (mbNameCels)
	{
		CountFileNames(&mFileNameCounts);
	}

	for (i = 0; i < mcSourceCels.NumElements(); i++)
	{
		pcCelSource = mcSourceCels.Get(i);
		pcPackedRect = GetPackedRectangle(pacPackedRects, pcCelSource);

		szName.Init();
		if (mbNameCels)
		{
			MakeCelName(&szName, i, &mFileNameCounts, &mFileNameIndices);
		}

		if (szName.Empty())
		{
			if (pcPackedRect)
			{
				pcCelDest = OMalloc<CImageCel>(mpcDestImage, pcPackedRect);
			}
			else
			{
				pcCelDest = OMalloc<CImageCel>(mpcDestImage, 0, 0, 0, 0);
			}
		}
		else
		{
			if (pcPackedRect)
			{
				pcCelDest = ONMalloc<CImageCel>(szName.Text(), mpcDestImage, pcPackedRect);
			}
			else
			{
				pcCelDest = ONMalloc<CImageCel>(szName.Text(), mpcDestImage, 0, 0, 0, 0);
			}
		}
		szName.Kill();

		//ONMalloc fails if an object with the same name already exists.
		if (pcCelDest.IsNull())
		{
			mFileNameIndices.Kill();
			mFileNameCounts.Kill();
			return false;
		}
		mcDestCels.Add(pcCelDest);

		pcCelDest->GetSubImage()->msOffsetTopLeft = pcCelSource->GetSubImage()->msOffsetTopLeft;
		pcCelDest->GetSubImage()->msOffsetBottomRight = pcCelSource->GetSubImage()->msOffsetBottomRight;
		pcCelDest->GetSubImage()->msAlignment = pcCelSource->GetSubImage()->msAlignment;
	}

	mFileNameIndices.Kill();
	mFileNameCounts.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCombiner::CountFileNames(CMapStringInt* pmFileNameCounts)
{
	size	i;
	char*	szFileName;

	for (i = 0; i < maszSourceFileNames.NumElements(); i++)
	{
		szFileName = maszSourceFileNames.Get(i)->Text();
		if (!StrEmpty(szFileName))
		{
			pmFileNameCounts->Put(szFileName, pmFileNameCounts->Get(szFileName, 0) + 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//The name is the file name relative to the working directory without its extension, eg: "Input/StarControl/Earthling Cruiser/frame0000".
//If more than one cel came from the same file then each is suffixed with its frame, eg: "Input/Sprites/Walk/Frame0".
void CImageCombiner::MakeCelName(CChars* szName, size iSourceIndex, CMapStringInt* pmFileNameCounts, CMapStringInt* pmFileNameIndices)
{
	char*		szFileName;
	int32		iFrame;
	CFileUtil	cFileUtil;
	CChars		szWorkingDirectory;

	szFileName = maszSourceFileNames.Get(iSourceIndex)->Text();
	if (StrEmpty(szFileName))
	{
		return;
	}

	szName->Append(szFileName);
	cFileUtil.FullPath(szName);
	cFileUtil.RemoveExtension(szName);

	//Files found by searching a directory have full paths.  Files under the working directory are named relative to it.
	szWorkingDirectory.Init();
	cFileUtil.CurrentDirectory(&szWorkingDirectory);
	cFileUtil.FullPath(&szWorkingDirectory);
	if (!szWorkingDirectory.EndsWith(FILE_SEPARATOR[0]))
	{
		szWorkingDirectory.Append(FILE_SEPARATOR[0]);
	}
	if (szName->StartsWith(szWorkingDirectory.Text()))
	{
		szName->RemoveFromStart(szWorkingDirectory.Length());
	}
	szWorkingDirectory.Kill();

	//Always use '/' so cel names are the same on every platform.
	szName->Replace('\\', '/');

	if (pmFileNameCounts->Get(szFileName, 0) > 1)
	{
		iFrame = pmFileNameIndices->Get(szFileName, 0);
		pmFileNameIndices->Put(szFileName, iFrame + 1);
		szName->Append("/Frame");
		szName->Append(iFrame);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackedRectangle* CImageCombiner::GetPackedRectangle(CArrayPackedRectangle* pacPackedRects, Ptr<CImageCel> pcSourceCel)
{
	CPackedRectangle*		pcPackedRect;
	CPackedRectangle*		pcFoundPackedRect;
	size					j;

	pcFoundPackedRect = NULL;
	for (j = 0; j < pacPackedRects->NumElements(); j++)
	{
		pcPackedRect = pacPackedRects->Get(j);
		if ((CImageCel*)pcPackedRect->mpvUserData == &pcSourceCel)
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
	Ptr<CImage>				pcLastImage;
	CArraySize				aiDestChannels;
	CArraySize				aiSourceChannels;
	CArraySize				aiIntersectChannels;
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

	mpcDestImage->Black();

	bDestHasOpacity = false;
	pcOpacityChannel = mpcDestImage->GetChannel(IMAGE_OPACITY);
	if (pcOpacityChannel)
	{
		bDestHasOpacity = true;
		pcDestOpacity = CImageAccessorCreator::Create(&mpcDestImage, pcOpacityChannel->eType, IMAGE_OPACITY, CHANNEL_STOP);
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
			aiDestChannels.Init();
			aiSourceChannels.Init();
			mpcDestImage->GetAllChannels(&aiDestChannels);
			pcCelSource->GetSourceImage()->GetAllChannels(&aiSourceChannels);
			aiIntersectChannels.Init();
			aiIntersectChannels.Intersect(&aiDestChannels, &aiSourceChannels);
			aiDestChannels.Kill();
			aiSourceChannels.Kill();

			pcDest->Kill();
			pcDest = CImageAccessorCreator::Create(&mpcDestImage, &aiIntersectChannels);

			pcSource->Kill();
			pcSource = CImageAccessorCreator::Create(&pcCelSource->GetSourceImage(), &aiIntersectChannels);

			aiIntersectChannels.Kill();
			pcLastImage = pcCelSource->GetSourceImage();
		}

		iDestLeft = pcPackedRect->msTopLeft.x;
		iDestTop = pcPackedRect->msTopLeft.y;
		iSourceLeft = pcCelSource->GetSubImage()->mcImageRect.msTopLeft.x;
		iSourceTop = pcCelSource->GetSubImage()->mcImageRect.msTopLeft.y;
		iSourceRight = pcCelSource->GetSubImage()->mcImageRect.msBottomRight.x;
		iSourceBottom = pcCelSource->GetSubImage()->mcImageRect.msBottomRight.y;

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
Ptr<CArrayImageCel> CImageCombiner::GetCels(void)
{
	return &mcDestCels;
}

