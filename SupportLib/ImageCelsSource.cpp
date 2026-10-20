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
void CImageSourceWithCelSources::Init(Ptr<CImageSource> pImageSource, CImageCelSource* pcCelsSource)
{
	PreInit();

	mpImageSource = pImageSource;
	mpcCelsSource = pcCelsSource;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageSourceWithCelSources::Class(void)
{
	M_Pointer(mpImageSource);
	U_Pointer(mpcCelsSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageSourceWithCelSources::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageSourceWithCelSources::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageSourceWithCelSources::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageSource> CImageSourceWithCelSources::GetImageSource(void) { return mpImageSource; }
CImageCelSource* CImageSourceWithCelSources::GetCelsSource(void) { return mpcCelsSource; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::Init(bool bPackOnLoad)
{
	PreInit();

	mbPackOnLoad = bPackOnLoad;

	macCelSources.Init();
	maImageSources.Init();
	mcModifiers.Init();

	maImageCels.Init();
	maszCelFileNames.Init();
	maFillMasks.Init();
	maImages.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::Free(void)
{
	maszCelFileNames.Kill();
	macCelSources.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::Class(void)
{
	U_Bool(mbPackOnLoad);
	M_Embedded(maImageSources);
	M_Embedded(mcModifiers);
	U_Data(CArrayUnknown, macCelSources);
	M_Embedded(maImageCels);
	U_Data(CArrayChars, maszCelFileNames);
	M_Embedded(maFillMasks);
	M_Embedded(maImages);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSource::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSource::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddSource(Ptr<CImageSource> pImageSource, CImageCelSource* pcCelSource)
{
	Ptr<CImageSourceWithCelSources>	pSource;

	pSource = OMalloc<CImageSourceWithCelSources>(pImageSource, pcCelSource);
	maImageSources.Add(pSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddDiskFileSource(char* szFilename, char* szImageName, CImageCelSource* pcCelSource)
{
	Ptr<CImageSourceDiskFile>	pDiskFile;
	size					iLen;
	CChars					szNewName;
	CFileUtil				cFileUtil;

	if (!szFilename)
	{
		return;
	}
	iLen = strlen(szFilename);
	if (iLen == 0)
	{
		return;
	}

	if (szImageName)
	{
		pDiskFile = OMalloc<CImageSourceDiskFile>(szFilename, szImageName);
	}
	else
	{
		szNewName.Init(szFilename);
		cFileUtil.RemovePath(&szNewName);
		pDiskFile = OMalloc<CImageSourceDiskFile>(szFilename, szNewName.Text());
		szNewName.Kill();
	}
	AddSource(pDiskFile, pcCelSource);
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
	Ptr<CImageSourceDiskFile>	pDiskFile;
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

		if (szImageName)
		{
			szNiceName.Init(*pszName);
			cFileUtil.RemovePath(&szNiceName);
			cFileUtil.RemoveExtension(&szNiceName);
			iIndex = szNiceName.Find(0, szFileNameContains);
			iIndex += iLen;
			szNiceName.RemoveFromStart(iIndex);
			szNiceName.Insert(0, szImageName);

			pDiskFile = OMalloc<CImageSourceDiskFile>(pszName->Text(), szNiceName.Text());
			szNiceName.Kill();
		}
		else
		{
			pDiskFile = OMalloc<CImageSourceDiskFile>(pszName->Text());
		}
		AddSource(pDiskFile, pcCelSource);
	}

	cFileNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddMemorySource(Ptr<CImage> pcImage, CImageCelSource* pcCelSource)
{
	Ptr<CImageSourceMemory>	pMemory;

	pMemory = OMalloc<CImageSourceMemory>(pcImage);
	AddSource(pMemory, pcCelSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelsSource::AddModifier(Ptr<CImageModifier> pModifier)
{
	mcModifiers.AddModifier(pModifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSource::Load(void)
{
	size								i;
	Ptr<CImageSourceWithCelSources>	pSource;
	bool							bResult;
	Ptr<CImage>						pcMask;
	Ptr<CImageSource>				pImageSource;
	CImageCelSource*				pcCelsSource;
	int								iFirstCelIndex;
	Ptr<CImage>						pcCombined;
	size							j;
	char*							szFileName;

	for (i = 0; i < maImageSources.NumElements(); i++)
	{
		pSource = maImageSources.Get(i);
		if (pSource.IsNotNull())
		{
			pcMask = NULL;
			pImageSource = pSource->GetImageSource();
			pcCelsSource = pSource->GetCelsSource();

			bResult = pImageSource->LoadImage();
			if (!bResult)
			{
				return false;
			}

			mcModifiers.SetImage(&pImageSource->GetImage());  //Not sure if this is a hack or not.
			mcModifiers.ApplyAll();

			if (pcCelsSource->NeedsMask())
			{
				pcMask = OMalloc<CImage>();
				maFillMasks.Add(pcMask);
			}
			iFirstCelIndex = maImageCels.NumElements();
			pcCelsSource->Divide((CImage*)pImageSource->GetImage().Object(), &maImageCels, (CImage*)pcMask.Object());

			if (mbPackOnLoad)
			{
				pcCombined = Combine(iFirstCelIndex);
				pImageSource->GetImage()->Kill();
				pImageSource->SetImage(pcCombined);
				if (pcMask.IsNotNull())
				{
					maFillMasks.Remove(pcMask);
				}
			}

			szFileName = pImageSource->GetFileName();
			for (j = iFirstCelIndex; j < maImageCels.NumElements(); j++)
			{
				maszCelFileNames.Add(szFileName ? szFileName : "");
			}
		}
	}

	mcModifiers.SetImage(NULL);

	PopulateImageArray();

	return true;
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
	Ptr<CImageCel>	pCel;
	size			uiNumElements;

	pcDest = OMalloc<CImage>();
	cImageCombiner.Init(ICL_Best, ICS_Arbitrary);

	uiNumElements = maImageCels.NumElements();
	for (i = iFirstCelIndex; i < uiNumElements; i++)
	{
		pCel = maImageCels.Get(i);
		cImageCombiner.AddCel(pCel);
	}
	pcDest = cImageCombiner.Combine();
	if (pcDest.IsNotNull())
	{
		maImageCels.RemoveEnd(iFirstCelIndex);
		maImageCels.AddAll(&cImageCombiner.mcDestCels);
		cImageCombiner.Kill();
		return pcDest;
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
void CImageCelsSource::PopulateImageArray(void)
{
	size							i;
	Ptr<CImageSourceWithCelSources>	pSource;
	Ptr<CImage>						pcImage;

	for (i = 0; i < maImageSources.NumElements(); i++)
	{
		pSource = maImageSources.Get(i);
		pcImage = pSource->GetImageSource()->GetImage();
		maImages.Add(pcImage);
	}	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> CImageCelsSource::GetImageCels(void)
{
	return &maImageCels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CImageCelsSource::GetCelFileNames(void)
{
	return &maszCelFileNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImage> CImageCelsSource::GetImages(void)
{
	return Ptr<CImage>(&maImages);
}

