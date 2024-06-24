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
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "ImageSourceDiskFile.h"
#include "ImageSourceMemory.h"
#include "ImageCombiner.h"
#include "ImageCelsSource.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageSourceWithCelSources::Init(CImageSource* pcImageSource, CImageCelSource* pcCelsSource)
{
	mpcImageSource = pcImageSource;
	mpcCelsSource = pcCelsSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageSourceWithCelSources::Kill(void)
{
	//Don't kill pcCelsSource, it's held by macCelSources.
	mpcImageSource->Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageSource* CImageSourceWithCelSources::GetImageSource(void) { return mpcImageSource; }
CImageCelSource* CImageSourceWithCelSources::GetCelsSource(void) { return mpcCelsSource; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::Init(bool bPackOnLoad)
{
	mbPackOnLoad = bPackOnLoad;

	macCelSources.Init();
	macImageSources.Init();
	mcModifiers.Init();

	macImageCels.Init();
	macFillMasks.Init();
	macImages.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::Kill(void)
{
	macImages.Kill();
	macFillMasks.Kill();
	macImageCels.Kill();
	macImageSources.Kill();
	mcModifiers.Kill();
	macCelSources.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddSource(CImageSource* pcImageSource, CImageCelSource* pcCelSource)
{
	CImageSourceWithCelSources*		pcImageSourceWithCelSource;

	pcImageSourceWithCelSource = macImageSources.Add();
	pcImageSourceWithCelSource->Init(pcImageSource, pcCelSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddDiskFileSource(char* szFileName, char* szImageName, CImageCelSource* pcCelSource)
{
	CImageSourceDiskFile*	pcDiskFile;
	size					iLen;
	CChars					szNewName;
	CFileUtil				cFileUtil;

	if (!szFileName)
	{
		return;
	}
	iLen = strlen(szFileName);
	if (iLen == 0)
	{
		return;
	}

	pcDiskFile = UMalloc(CImageSourceDiskFile);

	if (szImageName)
	{
		pcDiskFile->Init(szFileName, szImageName);
	}
	else
	{
		szNewName.Init(szFileName);
		cFileUtil.RemovePath(&szNewName);
		pcDiskFile->Init(szFileName, szNewName.Text());
		szNewName.Kill();
	}
	AddSource(pcDiskFile, pcCelSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddDiskFileSources(char* szPathName, char* szFileNameContains, char* szImageName, CImageCelSource* pcCelSource)
{
	CFileUtil				cFileUtil;
	CArrayChars				cFileNames;
	size					i;
	CChars*					pszName;
	CImageSourceDiskFile*	pcDiskFile;
	CChars					szNiceName;
	size					iIndex;
	size					iLen;

	if (!szFileNameContains)
	{
		return;
	}
	iLen = strlen(szFileNameContains);
	if (iLen == 0)
	{
		return;
	}

	cFileNames.Init();
	cFileUtil.FindFilesWithNameContaining(szPathName, szFileNameContains, &cFileNames, false);

	for (i = 0; i < cFileNames.NumElements(); i++)
	{
		pszName = cFileNames.Get(i);
		pcDiskFile = UMalloc(CImageSourceDiskFile);

		if (szImageName)
		{
			szNiceName.Init(*pszName);
			cFileUtil.RemovePath(&szNiceName);
			cFileUtil.RemoveExtension(&szNiceName);
			iIndex = szNiceName.Find(0, szFileNameContains);
			iIndex += iLen;
			szNiceName.RemoveFromStart(iIndex);
			szNiceName.Insert(0, szImageName);

			pcDiskFile->Init(pszName->Text(), szNiceName.Text());
			szNiceName.Kill();
		}
		else
		{
			pcDiskFile->Init(pszName->Text());
		}
		AddSource(pcDiskFile, pcCelSource);
	}

	cFileNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddMemorySource(CImage* pcImage, CImageCelSource* pcCelSource)
{
	CImageSourceMemory*		pcMemory;

	pcMemory = UMalloc(CImageSourceMemory);
	pcMemory->Init(pcImage);
	AddSource(pcMemory, pcCelSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddModifier(CImageModifier* pcModifier)
{
	mcModifiers.AddModifier(pcModifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSource::Load(void)
{
	SSetIterator				sIter;
	CImageSourceWithCelSources*	pcImageSourceWithCelSources;
	bool						bResult;
	Ptr<CImage>					pcMask;
	CImageSource*				pcImageSource;
	CImageCelSource*			pcCelsSource;
	int							iFirstCelIndex;
	Ptr<CImage>					pcCombined;

	pcImageSourceWithCelSources = (CImageSourceWithCelSources*)macImageSources.StartIteration(&sIter);
	while (pcImageSourceWithCelSources)
	{
		pcMask = NULL;
		pcImageSource = pcImageSourceWithCelSources->GetImageSource();
		pcCelsSource = pcImageSourceWithCelSources->GetCelsSource();

		bResult = pcImageSource->LoadImage();
		if (!bResult)
		{
			return false;
		}

		mcModifiers.SetImage((CImage*)pcImageSource->GetImage().Object());  //Not sure if this is a hack or not.
		mcModifiers.ApplyAll();

		if (pcCelsSource->NeedsMask())
		{
			pcMask = OMallocNoI<CImage>();
			macFillMasks.Add(pcMask);
		}
		iFirstCelIndex = macImageCels.NumElements();
		pcCelsSource->Divide((CImage*)pcImageSource->GetImage().Object(), &macImageCels, (CImage*)pcMask.Object());

		if (mbPackOnLoad)
		{
			pcCombined = Combine(iFirstCelIndex);
			pcImageSource->GetImage()->Kill();
			pcImageSource->SetImage(pcCombined);
			if (pcMask.IsNotNull())
			{
				macFillMasks.Remove(pcMask);
			}
		}

		pcImageSourceWithCelSources = (CImageSourceWithCelSources*)macImageSources.Iterate(&sIter);
	}

	mcModifiers.SetImage(NULL);

	PopulateImageArray();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayUnknown* CImageCelsSource::GetCels(void)
{
	return &macImageCels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageCelsSource::Combine(size iFirstCelIndex)
{
	CImageCombiner	cImageCombiner;
	Ptr<CImage>		pcDest;
	size			i;
	CImageCel*		pcCel;

	pcDest = OMalloc<CImage>();
	cImageCombiner.Init(ICL_Best, ICS_Arbitrary);

	for (i = iFirstCelIndex; i < macImageCels.NumElements(); i++)
	{
		pcCel = (CImageCel*)macImageCels.Get(i);
		cImageCombiner.AddCel(pcCel);
	}
	pcDest = cImageCombiner.Combine();
	if (pcDest.IsNotNull())
	{
		macImageCels.RemoveEnd(iFirstCelIndex);
		macImageCels.AddAll(&cImageCombiner.mcDestCels);
		cImageCombiner.Kill();
		return pcDest;
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
void CImageCelsSource::PopulateImageArray(void)
{
	size							i;
	CImageSourceWithCelSources*		pcSource;
	Ptr<CImage>						pcImage;

	for (i = 0; i < macImageSources.NumElements(); i++)
	{
		pcSource = macImageSources.Get(i);
		pcImage = pcSource->GetImageSource()->GetImage();
		macImages.Add(pcImage);
	}	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayUnknown* CImageCelsSource::GetImageCels(void)
{
	return &macImageCels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArray<CImage>> CImageCelsSource::GetImages(void)
{
	return Ptr<CImage>(&macImages);
}

