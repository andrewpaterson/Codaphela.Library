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
#include <stdlib.h>
#include <math.h>
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/Operators.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "ColourARGB32.h"
#include "SubImage.h"
#include "ImageAccessorCreator.h"
#include "Image.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::PrivateInit(void)
{
	mcChannels.Init();
	miWidth = 0;
	miHeight = 0;
	mpsImageChangingDesc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(void)
{
	PreInit();
	PrivateInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight)
{
	Init(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, EPrimitiveType eType, EChannel eFirst, ...)
{
	va_list		vaMarker;
	size		iCount;
	EChannel	eIC;

	PreInit();

	PrivateInit();
	iCount = 0;
	eIC = eFirst;

	BeginChange();
	va_start(vaMarker, eFirst);
	while (eIC != CHANNEL_STOP)
	{
		AddChannel(eIC, eType);
		iCount++;
		eIC = va_arg(vaMarker, EChannel);
	}
	va_end(vaMarker);

	SetSize(iWidth, iHeight);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, void* pvUserData, EPrimitiveType eType, EChannel eFirst, ...)
{
	va_list		vaMarker;
	size		iCount;
	EChannel	eIC;

	PreInit();

	PrivateInit();
	iCount = 0;
	eIC = eFirst;

	BeginChange();
	va_start(vaMarker, eFirst);
	while (eIC != CHANNEL_STOP)
	{
		AddChannel(eIC, eType);
		iCount++;
		eIC = va_arg(vaMarker, EChannel);
	}
	va_end(vaMarker);

	SetSize(iWidth, iHeight);
	SetData(pvUserData);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(Ptr<CImage> pcSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcSource);
	SetSize(pcSource->GetWidth(), pcSource->GetHeight());
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, CImageChannelsSource* pcSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcSource);
	SetSize(iWidth, iHeight);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, void* pvUserData, CImageChannelsSource* pcSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcSource);
	SetSize(iWidth, iHeight);
	SetData(pvUserData);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, Ptr<CImage> pcChannelsSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcChannelsSource);
	SetSize(iWidth, iHeight);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits)
{
	CColourFormatHelper		cHelper;

	cHelper.Init(eFormat, eOrder, eColourBits, eAlphaBits);

	Init(iWidth, iHeight, &cHelper);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, CArrayChannel* pasNewChannels)
{
	PreInit();

	size					iNumChannels;
	size					i;
	SChannel*				psChannel;
	
	PrivateInit();

	iNumChannels = pasNewChannels->NumElements();
	if (iNumChannels > 0)
	{
		BeginChange();

		for (i = 0; i < iNumChannels; i++)
		{
			psChannel = pasNewChannels->Get(i);
			AddChannel(psChannel);
		}

		SetSize(iWidth, iHeight);
		EndChange();
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Init(int iWidth, int iHeight, CColourFormatHelper* pcHelper)
{
	PreInit();

	size					iNumChannels;
	size					i;
	EChannel				eChannel;
	EPrimitiveType			eType;
	
	PrivateInit();

	iNumChannels = pcHelper->GetNumChannels();
	if (iNumChannels > 0)
	{
		BeginChange();

		for (i = 0; i < iNumChannels; i++)
		{
			eChannel = pcHelper->GetChannel(i);
			eType = pcHelper->GetType(i);
			AddChannel((size)eChannel, eType);
		}

		SetSize(iWidth, iHeight);
		EndChange();
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Class(void)
{
	M_Embedded(mcChannels);
	U_SInt(miWidth);
	U_SInt(miHeight);
	U_Pointer(mpsImageChangingDesc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Free(void)
{
	SafeFree(mpsImageChangingDesc);
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::ReInit(void)
{
	Free();
	mcChannels.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::ReInit(int iWidth, int iHeight, EPrimitiveType eType, EChannel eFirst, ...)
{
	va_list		vaMarker;
	size		iCount;
	EChannel	eIC;

	Free();
	mcChannels.ReInit();

	iCount = 0;
	eIC = eFirst;

	BeginChange();
	va_start(vaMarker, eFirst);
	while (eIC != CHANNEL_STOP)
	{
		AddChannel(eIC, eType);
		iCount++;
		eIC = va_arg(vaMarker, EChannel);
	}
	va_end(vaMarker);

	SetSize(iWidth, iHeight);
	EndChange();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::Load(CObjectReader* pcFile)
{
	mpsImageChangingDesc = NULL;

	ReturnOnFalse(pcFile->ReadSInt(&miWidth));
	ReturnOnFalse(pcFile->ReadSInt(&miHeight));

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteSInt(miWidth));
	ReturnOnFalse(pcFile->WriteSInt(miHeight));

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::BeginChange(void)
{
	mcChannels.BeginChange();
	mpsImageChangingDesc = (SImageChangingDesc*)malloc(sizeof(SImageChangingDesc));
	mpsImageChangingDesc->iWidth = miWidth;
	mpsImageChangingDesc->iHeight = miHeight;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannel(SChannel* psChannel)
{
	mcChannels.AddChannel(psChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannel(size iChannel, EPrimitiveType eType, bool bReverse)
{
	mcChannels.AddChannel(iChannel, eType, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannel(size iChannel, EPrimitiveType eType, char* szShortName, char* szLongName, bool bReverse)
{
	mcChannels.AddChannel(iChannel, eType, szShortName, szLongName, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::AddChannel(Ptr<CImage> pcSourceImage, size iChannel, EPrimitiveType eType)
{
	CChannel*	pcChannel;
	char*		szShortName;
	char*		szLongName;

	pcChannel = pcSourceImage->GetChannel(iChannel);
	if (pcChannel)
	{
		szShortName = pcSourceImage->GetChannelShortName(iChannel);
		szLongName = pcSourceImage->GetChannelLongName(iChannel);
		mcChannels.AddChannel(iChannel, eType, szShortName, szLongName, pcChannel->bReverse);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannel(size iChannel1, size iChannel2, EPrimitiveType eType, bool bReverse)
{
	AddChannel(iChannel1, eType, bReverse);
	AddChannel(iChannel2, eType, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannel(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType, bool bReverse)
{
	AddChannel(iChannel1, eType, bReverse);
	AddChannel(iChannel2, eType, bReverse);
	AddChannel(iChannel3, eType, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannel(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType, bool bReverse)
{
	AddChannel(iChannel1, eType, bReverse);
	AddChannel(iChannel2, eType, bReverse);
	AddChannel(iChannel3, eType, bReverse);
	AddChannel(iChannel4, eType, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannels(CImageChannelsSource* pcSource)
{
	size		i;
	SChannel	sChannel;

	for (i = 0; i < pcSource->NumChannels(); i++)
	{
		sChannel = pcSource->GetChannel(i);
		AddChannel(sChannel.iChannel, sChannel.eType, sChannel.bReverse);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannels(CArrayChannel* pasChannels)
{
	mcChannels.AddChannels(pasChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::AddChannels(Ptr<CImage> pcSourceChannels)
{
	mcChannels.AddChannels(&pcSourceChannels->mcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::RemoveChannel(size iChannel)
{
	return mcChannels.RemoveChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CImage::GetChannelLongName(size iChannel)
{
	return mcChannels.GetChannelLongName(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CImage::GetChannelShortName(size iChannel)
{
	return mcChannels.GetChannelShortName(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::SetSize(int iWidth, int iHeight)
{
	if (IsChanging())
	{
		mpsImageChangingDesc->iWidth = iWidth;
		mpsImageChangingDesc->iHeight = iHeight;
		mcChannels.SetSize(mpsImageChangingDesc->iWidth * mpsImageChangingDesc->iHeight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImage::GetHeight(void)
{
	if (IsChanging())
	{
		return mpsImageChangingDesc->iHeight;	
	}
	return miHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImage::GetWidth(void)
{
	if (IsChanging())
	{
		return mpsImageChangingDesc->iWidth;
	}
	return miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::EndChange(void)
{
	bool	bResult;

	if (IsChanging())
	{
		miWidth = mpsImageChangingDesc->iWidth;
		miHeight = mpsImageChangingDesc->iHeight;
		bResult = mcChannels.EndChange();
		SafeFree(mpsImageChangingDesc);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::IsChanging(void)
{
	return mcChannels.IsChanging();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Copy(Ptr<CImage> pcSource)
{
	//This assumes Image is not initialised.
	PreInit();

	miWidth = pcSource->miWidth;
	miHeight = pcSource->miHeight;
	if (pcSource->mpsImageChangingDesc)
	{
		mpsImageChangingDesc = (SImageChangingDesc*)malloc(sizeof(SImageChangingDesc));
		mpsImageChangingDesc->iWidth = pcSource->mpsImageChangingDesc->iWidth;
		mpsImageChangingDesc->iHeight = pcSource->mpsImageChangingDesc->iHeight;
	}
	else
	{
		mpsImageChangingDesc = NULL;
	}

	mcChannels.Copy(&pcSource->mcChannels);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::CopyIntoInitialised(Ptr<CImage> pcSource)
{
	//This assumes Image IS initialised.

	miWidth = pcSource->miWidth;
	miHeight = pcSource->miHeight;
	if (pcSource->mpsImageChangingDesc)
	{
		mpsImageChangingDesc = (SImageChangingDesc*)malloc(sizeof(SImageChangingDesc));
		mpsImageChangingDesc->iWidth = pcSource->mpsImageChangingDesc->iWidth;
		mpsImageChangingDesc->iHeight = pcSource->mpsImageChangingDesc->iHeight;
	}
	else
	{
		mpsImageChangingDesc = NULL;
	}

	mcChannels.CopyIntoInitialised(&pcSource->mcChannels);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::RemovePurpose(EImagePurpose ePurpose)
{
	CArraySize		mai;
	EChannel		eChannel;
	size			i;
	EImagePurpose	eCurrent;

	if (IsChanging())
	{
		mai.Init();
		GetAllChannels(&mai);

		for (i = 0; i < mai.NumElements(); i++)
		{
			eChannel = mai.GetValue(i);
			eCurrent = IMAGE_PURPOSE(eChannel);
			if (ePurpose == eCurrent)
			{
				RemoveChannel(eChannel);
			}
		}

		mai.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::RenameChannel(size iOldName, size iNewName)
{
	return mcChannels.RenameChannel(iOldName, iNewName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::ByteAlignChannels(void)
{
	mcChannels.ByteAlign();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Black(void)
{
	mcChannels.Black();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::White(void)
{
	mcChannels.White();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::SetData(void* pvData)
{
	mcChannels.SetData(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CImage::GetData(void)
{
	return mcChannels.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImage::GetByteSize(void)
{
	return mcChannels.GetByteSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImage::GetPixelSize(void)
{
	return mcChannels.GetSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImage::GetPixelByteStride(void)
{
	return mcChannels.GetByteStride();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImage::GetPixelBitStride(void)
{
	return mcChannels.GetBitStride();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSizeVec2 CImage::GetDimension(void)
{
	SSizeVec2	s;
	s.Init(miWidth, miHeight);
	return s;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannels* CImage::GetChannels(void)
{
	return &mcChannels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChannelOffset* CImage::GetChannelOffsets(void)
{
	return mcChannels.GetChannelOffsets();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::GetAllChannels(CArraySize* paiChannels)
{
	mcChannels.GetAllChannels(paiChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::GetAllChannels(CArrayChannel* pasChannels)
{
	mcChannels.GetAllChannels(pasChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CImage::GetPrimitiveType(void)
{
	return mcChannels.GetPrimitiveType();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::GetAllPrimitiveTypes(CArrayInt* paiPrimitiveTypes)
{
	mcChannels.GetAllPrimitiveTypes(paiPrimitiveTypes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::GetChannelsForType(EPrimitiveType eType, CArraySize* paiChannels)
{
	mcChannels.GetChannelsForType(eType, paiChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CImage::GetChannel(size iChannel)
{
	return mcChannels.GetChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CImage::GetChannelAtIndex(size iIndex)
{
	return mcChannels.GetChannelAtIndex(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::IsValid(int x, int y)
{
	if ((x < 0) || (x >= miWidth) || (y < 0) || (y >= miHeight))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::IsSameFormat(Ptr<CImage> psOther)
{
	return mcChannels.IsSameFormat(&psOther->mcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::HasChannel(size iChannel)
{
	return mcChannels.HasChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::HasChannels(size iFirst, ...)
{
	va_list		vaMarker;
	size		eIC;
	bool		bResult;

	eIC = iFirst;
	bResult = true;

	va_start(vaMarker, iFirst);
	while (eIC != CHANNEL_STOP)
	{
		bResult &= HasChannel(eIC);
		eIC = va_arg(vaMarker, size);
	}
	va_end(vaMarker);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImage::NumChannels(void)
{
	return mcChannels.NumChannels();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::Matches(CColourFormatHelper* pcHelper, bool bReverse)
{
	size			iNumChannels;
	size			eHelperChannel;
	size			iIndex;
	CChannel*		pcChannel;
	EPrimitiveType	eHelperType;

	iNumChannels = NumChannels();
	if (iNumChannels != pcHelper->GetNumChannels())
	{
		return false;
	}

	for (iIndex = 0; iIndex < iNumChannels; iIndex++)
	{
		pcChannel = GetChannelAtIndex(iIndex);

		eHelperChannel = (size)pcHelper->GetChannel(iIndex);
		if (pcChannel->iChannel != eHelperChannel)
		{
			return false;
		}

		eHelperType = pcHelper->GetType(iIndex);
		if (pcChannel->eType!= eHelperType)
		{
			return false;
		}

		if (pcChannel->bReverse != bReverse)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImage::Matches(CArrayChannel* pasChannels)
{
	size			iNumChannels;
	size			iIndex;
	CChannel*		pcChannel;
	SChannel*		psChannel;

	iNumChannels = NumChannels();
	if (iNumChannels != pasChannels->NumElements())
	{
		return false;
	}

	for (iIndex = 0; iIndex < iNumChannels; iIndex++)
	{
		pcChannel = GetChannelAtIndex(iIndex);
		psChannel = pasChannels->Get(iIndex);

		if (pcChannel->iChannel != psChannel->iChannel)
		{
			return false;
		}

		if (pcChannel->eType!= psChannel->eType)
		{
			return false;
		}

		if (pcChannel->bReverse != psChannel->bReverse)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::SetChannelDebugNames(size iChannel)
{
	char* szShortName;
	char* szLongName;

	szLongName = gmiszImageChannelLongNames.Get(iChannel);
	szShortName = gmiszImageChannelShortNames.Get(iChannel);
	mcChannels.SetChannelDebugNames(iChannel, szShortName, szLongName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EColourOrder CImage::GetColourOrder(void)
{
	//It's possible to lose information by assuming only RGB colour channel.

	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	int						iOpacity;
	int						iIgnored;
	int						iRed;
	int						iGreen;
	int						iBlue;
	int						iRedGreen;
	int						iGreenBlue;
	int						iBlueRed;

	iOpacity = -1;
	iIgnored = -1;
	iRed = -1;
	iGreen = -1;
	iBlue = -1;

	uiNumChannels = NumChannels();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = GetChannelAtIndex(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_DIFFUSE_RED)
		{
			iRed = (int)uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_GREEN)
		{
			iGreen = (int)uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_BLUE)
		{
			iBlue = (int)uiChannelIndex;
		}
	}

	if ((iRed == -1) || (iGreen == -1) || (iBlue == -1))
	{
		return CCO_Unknown;
	}

	iRedGreen = (int)iRed - (int)iGreen;
	iGreenBlue = (int)iGreen - (int)iBlue;
	iBlueRed = (int)iBlue - (int)iRed;

	if ((iRedGreen == -1) && (iGreenBlue == -1) && (iBlueRed == 2))
	{
		return CCO_RGB;
	}
	else if ((iBlueRed == -1) && (iRedGreen == -1) && (iGreenBlue == 2))
	{
		return CCO_BRG;
	}
	else if ((iGreenBlue == -1) && (iBlueRed == -1) && (iRedGreen == 2))
	{
		return CCO_GBR;
	}
	else if ((iBlueRed == -2) && (iGreenBlue == 1) && (iRedGreen == 1))
	{
		return CCO_BGR;
	}
	else if ((iBlueRed == -1) && (iGreenBlue == -1) && (iRedGreen == 2))
	{
		return CCO_RBG;
	}
	else if ((iBlueRed == -1) && (iGreenBlue == -1) && (iRedGreen == 2))
	{
		return CCO_GRB;
	}

	return CCO_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CImage::GetColourFormat(void)
{
	//It's possible to lose information by assuming only RGB and a single alpha or unused colour channel.
	//We should already know the colour order at this point.

	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	int						iOpacity;
	int						iIgnored;
	int						iRed;

	iOpacity = -1;
	iIgnored = -1;
	iRed = -1;

	uiNumChannels = NumChannels();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = GetChannelAtIndex(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_OPACITY)
		{
			iOpacity = uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_IGNORED)
		{
			iIgnored = uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_RED)
		{
			iRed = uiChannelIndex;
		}
	}

	if ((iOpacity != -1) && (iIgnored != -1))
	{
		return CFT_Unknown;
	}

	if (iOpacity != -1)
	{
		if (iRed > iOpacity)
		{
			return CFT_ARGB;
		}
		else
		{
			return CFT_RGBA;
		}
	}

	if (iIgnored != -1)
	{
		if (iRed > iIgnored)
		{
			return CFT_XRGB;
		}
		else
		{
			return CFT_RGBX;
		}
	}

	return CFT_RGB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERGBColourBits CImage::GetColourBits(void)
{
	//We should already know the colour exists at this point.

	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	EPrimitiveType			eRedType;
	EPrimitiveType			eGreenType;
	EPrimitiveType			eBlueType;

	eRedType = PT_Undefined;
	eGreenType = PT_Undefined;
	eBlueType = PT_Undefined;

	uiNumChannels = NumChannels();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = GetChannelAtIndex(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_DIFFUSE_RED)
		{
			eRedType = pcChannel->eType;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_GREEN)
		{
			eGreenType = pcChannel->eType;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_BLUE)
		{
			eBlueType = pcChannel->eType;
		}
	}

	if ((eRedType == PT_uint8) && (eGreenType == PT_uint8) && (eBlueType == PT_uint8))
	{
		return CRGB_24bit;
	}
	else if ((eRedType == PT_uint16) && (eGreenType == PT_uint16) && (eBlueType == PT_uint16))
	{
		return CRGB_48bit;
	}
	else if ((eRedType == PT_float32) && (eGreenType == PT_float32) && (eBlueType == PT_float32))
	{
		return CRGB_Float3;
	}
	else if ((eRedType == PT_nickle) && (eGreenType == PT_nickle) && (eBlueType == PT_nickle))
	{
		return CRGB_15bit;
	}
	else if ((eRedType == PT_nickle) && (eGreenType == PT_sixbits) && (eBlueType == PT_nickle))
	{
		return CRGB_16bit;
	}
	else if ((eRedType == PT_crumb) && (eGreenType == PT_crumb) && (eBlueType == PT_crumb))
	{
		return CRGB_6bit;
	}
	else if ((eRedType == PT_tribble) && (eGreenType == PT_tribble) && (eBlueType == PT_crumb))
	{
		return CRGB_8bit332;
	}
	else if ((eRedType == PT_tribble) && (eGreenType == PT_crumb) && (eBlueType == PT_tribble))
	{
		return CRGB_8bit323;
	}
	else if ((eRedType == PT_crumb) && (eGreenType == PT_tribble) && (eBlueType == PT_tribble))
	{
		return CRGB_8bit323;
	}

	return CRGB_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERGBAlphaBits CImage::GetAlphaBits(void)
{
	//We should already know the colour exists at this point.

	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	EPrimitiveType			eOpacityType;
	EPrimitiveType			eIgnoredType;

	eIgnoredType = PT_Undefined;
	eOpacityType = PT_Undefined;

	uiNumChannels = NumChannels();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = GetChannelAtIndex(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_OPACITY)
		{
			eOpacityType = pcChannel->eType;
		}
		else if (pcChannel->iChannel == IMAGE_IGNORED)
		{
			eIgnoredType = pcChannel->eType;
		}
	}

	if ((eIgnoredType != PT_Undefined) && (eOpacityType != PT_Undefined))
	{
		return ARGB_Unknown;
	}
	else if ((eIgnoredType == PT_Undefined) && (eOpacityType == PT_Undefined))
	{
		return ARGB_None;
	}
	if (eOpacityType != PT_Undefined)
	{
		if (eOpacityType == PT_uint8)
		{
			return ARGB_8bit;
		}
		else if (eOpacityType == PT_uint16)
		{
			return ARGB_16bit;
		}
		else if (eOpacityType == PT_crumb)
		{
			return 	ARGB_2bit;
		}
	}
	if (eIgnoredType != PT_Undefined)
	{
		if (eIgnoredType == PT_uint8)
		{
			return ARGB_8bit;
		}
		else if (eIgnoredType == PT_uint16)
		{
			return ARGB_16bit;
		}
		else if (eIgnoredType == PT_crumb)
		{
			return 	ARGB_2bit;
		}
	}
	return ARGB_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Print(CChars* psz)
{
	CChannels*			pcChannels;
	CArraySize			aiChannels;
	size				i;
	uint				uiChannel;
	CChannel*			pcChannel;
	char*				szChannelName;
	int					x;
	int					y;
	CChannelsAccessor*	apcAccessors[8];
	uint				ui;

	psz->Append("Object[");
	PrintIdentifier(psz);
	psz->Append("]  Channels");
	pcChannels = GetChannels();

	aiChannels.Init();
	pcChannels->GetAllChannels(&aiChannels);

	for (i = 0; i < aiChannels.NumElements(); i++)
	{
		uiChannel = aiChannels.GetValue(i);
		pcChannel = GetChannel(uiChannel);

		szChannelName = GetChannelLongName(uiChannel);

		psz->Append("[(");
		PrintPrimitiveType(pcChannel->eType, psz);
		psz->Append(")\"");
		psz->Append(szChannelName);
		psz->Append("\"");

		if (pcChannel->miByteOffset != CHANNEL_NON_ALIGNED_BYTES)
		{
			psz->Append(" B:");
			psz->Append(pcChannel->miByteOffset);
		}
		psz->Append(" b:");
		psz->Append(pcChannel->miBitOffset);
		psz->Append("]  ");

		apcAccessors[i] = CChannelsAccessorCreator::CreateSingleChannelAccessor(GetChannels(), uiChannel);
	}
	psz->RemoveFromEnd(2);
	psz->AppendNewLine();
	psz->AppendNewLine();

	for (x = 0; x < miWidth; x++)
	{
		for (i = 0; i < aiChannels.NumElements(); i++)
		{
			uiChannel = aiChannels.GetValue(i);
			pcChannel = GetChannel(uiChannel);

			szChannelName = GetChannelShortName(uiChannel);
			psz->Append(szChannelName);
			psz->Append(" ");
		}
		psz->Append(" ");
	}
	psz->AppendNewLine();

	for (y = 0; y < miHeight; y++)
	{
		for (x = 0; x < miWidth; x++)
		{
			for (i = 0; i < aiChannels.NumElements(); i++)
			{
				ui = *((uint*)apcAccessors[i]->Get(x + y * miWidth));
				psz->Append(" ");
				psz->AppendHexLoHi(&ui, 1);
				psz->Append(" ");
			}
			psz->Append(" ");
		}
		psz->AppendNewLine();
	}

	aiChannels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

