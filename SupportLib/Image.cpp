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
Ptr<CImage> CImage::Init(void)
{
	PreInit();
	PrivateInit();
	PostInit();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(int iWidth, int iHeight)
{
	Init(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(int iWidth, int iHeight, EPrimitiveType eType, EChannel eFirst, ...)
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
	while (eIC != CHANNEL_ZERO)
	{
		AddChannel(eIC, eType);
		iCount++;
		eIC = va_arg(vaMarker, EChannel);
	}
	va_end(vaMarker);

	SetSize(iWidth, iHeight);
	EndChange();

	PostInit();
	
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(int iWidth, int iHeight, void* pvUserData, EPrimitiveType eType, EChannel eFirst, ...)
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
	while (eIC != CHANNEL_ZERO)
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

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(Ptr<CImage> pcSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcSource);
	SetSize(pcSource->GetWidth(), pcSource->GetHeight());
	EndChange();

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(int iWidth, int iHeight, CImageChannelsSource* pcSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcSource);
	SetSize(iWidth, iHeight);
	EndChange();

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(int iWidth, int iHeight, void* pvUserData, CImageChannelsSource* pcSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcSource);
	SetSize(iWidth, iHeight);
	SetData(pvUserData);
	EndChange();

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImage::Init(int iWidth, int iHeight, Ptr<CImage> pcChannelsSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcChannelsSource);
	SetSize(iWidth, iHeight);
	EndChange();

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::Class(void)
{
	Embedded(&mcChannels, "mcChannels");
	UnmanagedSInt(&miWidth, "miWidth");
	UnmanagedSInt(&miHeight, "miHeight");
	UnmanagedPointer((void**)&mpsImageChangingDesc, "mpsImageChangingDesc");
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
	while (eIC != CHANNEL_ZERO)
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
	CArrayInt		mai;
	EChannel		eChannel;
	size			i;
	EImagePurpose	eCurrent;

	if (IsChanging())
	{
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
void CImage::Clear(void)
{
	mcChannels.Clear();
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
CChannels* CImage::GetChannels(void)
{
	return &mcChannels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImage::GetChannelsCount(void)
{
	return mcChannels.GetNumChannels();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImage::GetAllChannels(CArrayInt* paiChannels)
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
void CImage::GetChannelsForType(EPrimitiveType eType, CArrayInt* paiChannels)
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
	while (eIC != CHANNEL_ZERO)
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
void CImage::Print(CChars* psz)
{
	CChannels*			pcChannels;
	CArrayInt			aiChannels;
	size				i;
	uint				uiChannel;
	CChannel*			pcChannel;
	char*				szChannelName;
	int					x;
	int					y;
	CChannelsAccessor*	apcAccessors[8];
	uint				ui;

	psz->Append("Object[");
	GetIdentifier(psz);
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

