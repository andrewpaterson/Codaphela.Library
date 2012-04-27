/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/AdditionalTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Define.h"
#include "ClassStorage.h"
#include "Channels.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(void)
{
	masChannelOffsets.Init();
	miSize = 0;
	miByteStride = 0;
	miBitStride = 0;
	mbOnlyBasicTypes = TRUE;
	mabData.Init();
	mpvUserData = NULL;

	mpsChangingDesc = NULL;
	mpvDataCache = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(int iSize, EPrimitiveTypes eType, int iFirst, ...)
{
	va_list					vaMarker;
	int						iCount;
	int						i;

	Init();
	iCount = 0;
	i = iFirst;

	BeginChange();
	va_start(vaMarker, iFirst);
	while (i != -1)
	{
		AddChannel(i, eType);
		iCount++;
		i = va_arg(vaMarker, int);
	}
	va_end(vaMarker);

	SetSize(iSize);
	EndChange();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(int iSize, void* pvUserData, EPrimitiveTypes eType, int iFirst, ...)
{
	va_list					vaMarker;
	int						iCount;
	int						i;

	Init();
	iCount = 0;
	i = iFirst;

	BeginChange();
	va_start(vaMarker, iFirst);
	while (i != -1)
	{
		AddChannel(i, eType);
		iCount++;
		i = va_arg(vaMarker, int);
	}
	va_end(vaMarker);

	SetSize(iSize);
	SetData(pvUserData);
	EndChange();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(CChannels* pcSource)
{
	CChannel*			psSource;
	int					i;

	Init();
	BeginChange();

	for (i = 0; i < pcSource->masChannelOffsets.NumElements(); i++)
	{
		psSource = pcSource->masChannelOffsets.Get(i);
		AddChannel(psSource->iChannel, psSource->eType, psSource->bReverse);
	}

	SetSize(pcSource->miSize);
	EndChange();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Kill(void)
{
	mabData.Kill();
	masChannelOffsets.Kill();
	miSize = 0;
	miByteStride = 0;
	miBitStride = 0;
	mpvUserData = NULL;
	mpvDataCache = NULL;

	SafeKill(mpsChangingDesc);
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::BeginChange(void)
{
	mpsChangingDesc = (SChannelsChangingDesc*)malloc(sizeof(SChannelsChangingDesc));
	mpsChangingDesc->Init();
	mpsChangingDesc->iSize = miSize;
	mpsChangingDesc->pvUserData = mpvUserData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetSize(int iSize)
{
	if (IsChanging())
	{
		mpsChangingDesc->iSize = iSize;
	}
	else
	{
		gcUserError.Set("Call BeginChange() before setting size.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AddChannel(int iChannel, EPrimitiveTypes eType, BOOL bReverse)
{
	if (IsChanging())
	{
		PrivateAddChannel(iChannel, eType, bReverse);
	}
	else
	{
		gcUserError.Set("Call BeginChange() before adding channels.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AddChannel(int iChannel1, int iChannel2, EPrimitiveTypes eType, BOOL bReverse)
{
	if (IsChanging())
	{
		PrivateAddChannel(iChannel1, eType, bReverse);
		PrivateAddChannel(iChannel2, eType, bReverse);
	}
	else
	{
		gcUserError.Set("Call BeginChange() before adding channels.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AddChannel(int iChannel1, int iChannel2, int iChannel3, EPrimitiveTypes eType, BOOL bReverse)
{
	if (IsChanging())
	{
		PrivateAddChannel(iChannel1, eType, bReverse);
		PrivateAddChannel(iChannel2, eType, bReverse);
		PrivateAddChannel(iChannel3, eType, bReverse);
	}
	else
	{
		gcUserError.Set("Call BeginChange() before adding channels.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AddChannel(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveTypes eType, BOOL bReverse)
{
	if (IsChanging())
	{
		PrivateAddChannel(iChannel1, eType, bReverse);
		PrivateAddChannel(iChannel2, eType, bReverse);
		PrivateAddChannel(iChannel3, eType, bReverse);
		PrivateAddChannel(iChannel4, eType, bReverse);
	}
	else
	{
		gcUserError.Set("Call BeginChange() before adding channels.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AddChannels(CArrayChannel* pasChannels)
{
	int			i;
	SChannel*	psChannel;

	if (IsChanging())
	{
		for (i = 0; i < pasChannels->NumElements(); i++)
		{
			psChannel = pasChannels->Get(i);
			PrivateAddChannel(psChannel->iChannel, psChannel->eType, psChannel->bReverse);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AddChannels(CChannels* pcSourceChannels)
{
	int			i;
	CChannel*	pcChannel;

	if (IsChanging())
	{
		for (i = 0; i < pcSourceChannels->masChannelOffsets.NumElements(); i++)
		{
			pcChannel = pcSourceChannels->masChannelOffsets.Get(i);
			PrivateAddChannel(pcChannel->iChannel, pcChannel->eType, pcChannel->bReverse);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::PrivateAddChannel(int iChannel, EPrimitiveTypes eType, BOOL bReverse)
{
	SChannel*	psChannel;

	psChannel = mpsChangingDesc->asAddedChannels.Add();
	psChannel->iChannel = iChannel;
	psChannel->eType = eType;
	psChannel->bReverse = bReverse;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::RemoveChannel(int iChannel)
{
	SChannel* psChannel;

	if (IsChanging())
	{
		if ((HasChannel(iChannel)) && (!IsChannelRemoved(iChannel)))
		{
			psChannel = mpsChangingDesc->asRemovedChannels.Add();
			psChannel->iChannel = iChannel;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsChannelRemoved(int iChannel)
{
	if (IsChanging())
	{
		return (mpsChangingDesc->asRemovedChannels.FindWithIntKey(iChannel, 0) != -1);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsChannelAdded(int iChannel)
{
	if (IsChanging())
	{
		return (mpsChangingDesc->asAddedChannels.FindWithIntKey(iChannel, 0) != -1);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsChanging(void)
{
	return (mpsChangingDesc != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::EndChange(void)
{
	int				i;
	SChannel*		psAddedChannel;
	SChannel*		psRemovedChannel;
	CChannel*		psChannel;
	int				iSize;
	int				iOffset;
	BOOL			bAnyAdded;
	int				iOldBitStride;
	int				iAddedBitStride;
	int				iOldByteStride;
	BOOL			bResult;

	//I don't think this handles the case where the channels have been re-ordered...
	//Which as there *IS* no way of re-ordering them shouldn't be a problem.

	bResult = TRUE;
	if (IsChanging())
	{
		if (mpvUserData != mpsChangingDesc->pvUserData)
		{
			//Switch between using mabData and mpvUserData.
			if ((mpvUserData == NULL) && (mpsChangingDesc->pvUserData != NULL))
			{
				FreeData();
			}
			else if ((mpvUserData != NULL) && (mpsChangingDesc->pvUserData == NULL))
			{
				AllocateData();
				//should copy from mpvUserData
			}
			mpvUserData = (char*)mpsChangingDesc->pvUserData;
		}

		if (!IsUserAllocated())
		{
			//We're using mabData and have already allocated it.
			if ((miBitStride != 0) && (miSize != 0))
			{
				//Remove
				for (i = 0; i < mpsChangingDesc->asRemovedChannels.NumElements(); i++)
				{
					psRemovedChannel = mpsChangingDesc->asRemovedChannels.Get(i);
					psChannel = GetChannel(psRemovedChannel->iChannel);
					if (psChannel->Is8BitAligned())
					{
						iOffset = psChannel->miByteOffset;
						iSize = psChannel->miByteSize;
						mabData.BatchRemoveElements(iOffset, iSize, miSize, miByteStride);
					}
					else
					{
						//Deal with bitty removal.
					}
					masChannelOffsets.RemoveAt(masChannelOffsets.GetIndex(psChannel), TRUE);
					Recalculate();
				}
			}

			//Add
			bAnyAdded = FALSE;
			for (i = 0; i < mpsChangingDesc->asAddedChannels.NumElements(); i++)
			{
				psAddedChannel = mpsChangingDesc->asAddedChannels.Get(i);

				psChannel = masChannelOffsets.Add();
				psChannel->Init(psAddedChannel->iChannel, psAddedChannel->eType, psAddedChannel->bReverse);
				bAnyAdded = TRUE;
			}

			if (bAnyAdded)
			{
				iOldBitStride = miBitStride;
				iOldByteStride = miByteStride;
				Recalculate();
				iAddedBitStride = miBitStride - iOldBitStride;
				if ((iAddedBitStride != 0) && (miSize != 0))
				{
					if (Is8BitAligned(iOldBitStride, iAddedBitStride))
					{
						iOffset = iOldByteStride;
						iSize = iAddedBitStride / 8;
						mabData.BatchInsertElements(iOffset, iSize, miSize, iOldByteStride);
					}
					else
					{
						//Deal with bitty addition.
					}
				}
			}

			//Change size due to size change.
			if (mpsChangingDesc->iSize != miSize)
			{
				miSize = mpsChangingDesc->iSize;
				iSize = CalculateByteSize(miBitStride, miSize);
				mabData.GrowToNumElements(iSize);
			}
		}
		else  //Using user allocated memory.
		{
			//This assumes the mpvUserData is *already* in the form being specified.
			for (i = 0; i < mpsChangingDesc->asRemovedChannels.NumElements(); i++)
			{
				psRemovedChannel = mpsChangingDesc->asRemovedChannels.Get(i);
				psChannel = GetChannel(psRemovedChannel->iChannel);
				masChannelOffsets.RemoveAt(masChannelOffsets.GetIndex(psChannel), TRUE);
			}

			for (i = 0; i < mpsChangingDesc->asAddedChannels.NumElements(); i++)
			{
				psAddedChannel = mpsChangingDesc->asAddedChannels.Get(i);

				psChannel = masChannelOffsets.Add();
				psChannel->Init(psAddedChannel->iChannel, psAddedChannel->eType, psAddedChannel->bReverse);
			}
			miSize = mpsChangingDesc->iSize;
			Recalculate();
		}

		mpsChangingDesc->Kill();
		free(mpsChangingDesc);
		mpsChangingDesc = NULL;
	}

	if (IsUserAllocated())
	{
		mpvDataCache = mpvUserData;
	}
	else
	{
		mpvDataCache = mabData.GetData();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Recalculate(void)
{
	int			i;
	CChannel*	pcChannel;
	int			iBitSize;
	int			iSize;

	miBitStride = 0;
	mbOnlyBasicTypes = TRUE;
	for (i = 0; i < masChannelOffsets.NumElements(); i++)
	{
		pcChannel = masChannelOffsets.Get(i);

		iSize = gcClassStorage.GetSize(pcChannel->eType);
		if (iSize & BIT_SIZE)
		{
			iBitSize = iSize & ~BIT_SIZE;
			mbOnlyBasicTypes = FALSE;
		}
		else
		{
			iBitSize = iSize*8;
		}

		pcChannel->miBitSize = iBitSize;
		if (Is8BitAligned(miBitStride, iBitSize))
		{
			pcChannel->miByteSize = iSize;
			pcChannel->miByteOffset = miBitStride / 8;
		}
		else
		{
			pcChannel->miByteSize = -1;
			pcChannel->miByteOffset = -1;
		}
		pcChannel->miBitOffset = miBitStride;

		miBitStride += iBitSize;
	}
	miByteStride = CalculateByteStride(miBitStride);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::Is8BitAligned(int iBitStart, int iBitLength)
{
	if ((iBitStart % 8 == 0) && (iBitLength % 8 == 0))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::CalculateByteStride(int iBitStride)
{
	if (iBitStride % 8 == 0)
	{
		return iBitStride / 8;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::CalculateByteSize(int iBitStride, int iSize)
{
	int	iByteSize;
	int	iBitSize;

	iBitSize = iBitStride * iSize;
	if (iBitSize % 8 != 0)
	{
		iByteSize = iBitSize / 8 + 1;
	}
	else
	{
		iByteSize = iBitSize / 8;
	}
	return iByteSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetData(void* pvMem)
{
	if (IsChanging())
	{
		mpsChangingDesc->pvUserData = pvMem;
	}
	else if (IsUserAllocated())
	{
		mpvUserData = (char*)pvMem;
		mpvDataCache = (char*)pvMem;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::RenameChannel(int iOldName, int iNewName)
{
	CChannel*	pcChannel;

	pcChannel = GetChannel(iOldName);
	if (pcChannel)
	{
		pcChannel->iChannel = iNewName;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::ByteAlign(void)
{
	int			i;
	SChannel*	psChannel;

	if (IsChanging())
	{
		for (i = 0; i < mpsChangingDesc->asAddedChannels.NumElements(); i++)
		{
			psChannel = mpsChangingDesc->asAddedChannels.Get(i);
			if ((psChannel->eType >= PRIMTIVE_OPERATOR_END) && (psChannel->eType < PRIMTIVE_TYPE_END))
			{
				psChannel->eType = PT_uchar;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::FreeData(void)
{
	if (!IsUserAllocated())
	{
		mabData.Kill();
		mpvDataCache = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChannels::GetData(void)
{
	//Don't try and get the data whilst the data is changing...
	if (IsChanging())
	{
		return NULL;
	}

	return mpvDataCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::AllocateData(void)
{
	int		iSize;

	iSize = CalculateByteSize(miBitStride, miSize);
	FreeData();
	mabData.Allocate(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsUserAllocated(void)
{
	return (mpvUserData != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::GetNumChannels(void)
{
	return masChannelOffsets.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CChannels::GetChannelAtIndex(int iIndex)
{
	return masChannelOffsets.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::GetIndexOfChannel(CChannel* pcChannel)
{
	return masChannelOffsets.GetIndex(pcChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CChannels::GetChannel(int iChannel)
{
	int		iIndex;
	iIndex = masChannelOffsets.FindWithIntKey(iChannel, 0);
	if (iIndex == -1)
	{
		return NULL;
	}
	return masChannelOffsets.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::HasChannel(int iChannel)
{
	CChannel*	psChannel;

	psChannel = GetChannel(iChannel);
	return (psChannel != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::HasChannel(int iChannel1, int iChannel2)
{
	if (HasChannel(iChannel1))
	{
		return HasChannel(iChannel2);
	}
	return FALSE;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::HasChannel(int iChannel1, int iChannel2, int iChannel3)
{
	if (HasChannel(iChannel1))
	{
		if (HasChannel(iChannel2))
		{
			return HasChannel(iChannel3);
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::HasChannels(int iFirst, ...)
{
	va_list		vaMarker;
	int			eIC;
	BOOL		bResult;

	eIC = iFirst;
	bResult = TRUE;

	va_start(vaMarker, iFirst);
	while (eIC != -1)
	{
		bResult &= HasChannel(eIC);
		eIC = va_arg(vaMarker, int);
	}
	va_end(vaMarker);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::Load(CFileReader* pcFile)
{
	mpsChangingDesc = NULL;
	mpvUserData = NULL;
	ReturnOnFalse(pcFile->ReadArrayTemplate(&masChannelOffsets));
	ReturnOnFalse(pcFile->ReadInt(&miSize));
	ReturnOnFalse(pcFile->ReadInt(&miByteStride));
	ReturnOnFalse(pcFile->ReadInt(&miBitStride));
	ReturnOnFalse(pcFile->ReadBool(&mbOnlyBasicTypes));
	ReturnOnFalse(pcFile->ReadArrayTemplate(&mabData));
	mpvDataCache = mabData.GetData();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteArrayTemplate(&masChannelOffsets));
	ReturnOnFalse(pcFile->WriteInt(miSize));
	ReturnOnFalse(pcFile->WriteInt(miByteStride));
	ReturnOnFalse(pcFile->WriteInt(miBitStride));
	ReturnOnFalse(pcFile->WriteBool(mbOnlyBasicTypes));
	ReturnOnFalse(pcFile->WriteArrayTemplate(&mabData));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsSameFormat(CChannels* psOther)
{
	int				iIndex;
	CChannel*	psSourceChannel;
	CChannel*	psThisChannel;

	if (GetNumChannels() == psOther->GetNumChannels())
	{
		for (iIndex = 0; iIndex < GetNumChannels(); iIndex++)
		{
			psSourceChannel = psOther->GetChannelAtIndex(iIndex);
			psThisChannel = GetChannelAtIndex(iIndex);
			if (!(	(psSourceChannel->eType == psThisChannel->eType) &&
					(psSourceChannel->iChannel == psThisChannel->iChannel) &&
					(psSourceChannel->bReverse == psThisChannel->bReverse)))
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsIgnored(void)
{
	if (IsChanging())
	{
		return FALSE;
	}
	return miBitStride == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Clear(void)
{
	if (!IsChanging())
	{
		mabData.Zero();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Copy(CChannels* pcData)
{
	//This assumes channels is not initiialised.

	masChannelOffsets.Init();
	masChannelOffsets.Copy(&pcData->masChannelOffsets);
	miSize = pcData->miSize;
	miByteStride = pcData->miByteStride;
	miBitStride = pcData->miBitStride;
	mbOnlyBasicTypes = pcData->mbOnlyBasicTypes;
	mabData.Init();
	mabData.Copy(&pcData->mabData);
	mpvUserData = pcData->mpvUserData;
	if (IsUserAllocated())
	{
		mpvDataCache = mpvUserData;
	}
	else
	{
		mpvDataCache = mabData.GetData();
	}
	if (pcData->IsChanging())
	{
		BeginChange();
		mpsChangingDesc->iSize = pcData->mpsChangingDesc->iSize;
		mpsChangingDesc->pvUserData = pcData->mpsChangingDesc->pvUserData;
		mpsChangingDesc->asAddedChannels.Copy(&pcData->mpsChangingDesc->asAddedChannels);
		mpsChangingDesc->asRemovedChannels.Copy(&pcData->mpsChangingDesc->asRemovedChannels);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsValid(int iPos)
{
	if ((iPos < 0) || (iPos >= miSize))
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::GetSize(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::GetByteSize(void)
{
	if (!IsUserAllocated())
	{
		return mabData.NumElements();
	}
	else
	{
		if (miByteStride != -1)
		{
			return miByteStride * miSize;
		}
		else
		{
			if (miBitStride % 8 == 0)
			{
				return miBitStride / 8;
			}
			else
			{
				return miBitStride / 8 + 1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChannelOffset* CChannels::GetChannelOffsets(void)
{
	return &masChannelOffsets;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::GetByteStride(void)
{
	return miByteStride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannels::GetBitStride(void)
{
	return miBitStride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetAllChannels(CArrayInt* paiChannels)
{
	int					j;
	CChannel*			psChannel;

	paiChannels->Init(masChannelOffsets.NumElements());

	for (j = 0; j < masChannelOffsets.NumElements(); j++)
	{
		psChannel = masChannelOffsets.Get(j);
		paiChannels->Add(psChannel->iChannel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetAllChannels(CArrayChannel* pasChannels)
{
	int					j;
	CChannel*			psChannelSource;
	SChannel*			psChannelDest;

	pasChannels->Allocate(masChannelOffsets.NumElements());

	for (j = 0; j < masChannelOffsets.NumElements(); j++)
	{
		psChannelSource = masChannelOffsets.Get(j);
		psChannelDest = pasChannels->Get(j);
		psChannelDest->eType = psChannelSource->eType;
		psChannelDest->iChannel = psChannelSource->iChannel;
		psChannelDest->bReverse = psChannelSource->bReverse;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveTypes CChannels::GetPrimitiveType(void)
{
	int					j;
	CChannel*			psChannel;
	EPrimitiveTypes		eType;

	eType = PT_Undefined;
	for (j = 0; j < masChannelOffsets.NumElements(); j++)
	{
		psChannel = masChannelOffsets.Get(j);
		if (eType == PT_Undefined)
		{
			eType = psChannel->eType;
		}
		else if (eType != psChannel->eType)
		{
			eType = PT_Undefined;
		}
	}
	return eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetAllPrimitiveTypes(CArrayInt* paiPrimitiveTypes)
{
	int					j;
	CChannel*			psChannel;

	paiPrimitiveTypes->Init(4);
	for (j = 0; j < masChannelOffsets.NumElements(); j++)
	{
		psChannel = masChannelOffsets.Get(j);
		paiPrimitiveTypes->AddIfUnique((int)psChannel->eType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetChannelsForType(EPrimitiveTypes eType, CArrayInt* paiChannels)
{
	int					j;
	CChannel*			psChannel;

	paiChannels->Init(4);

	for (j = 0; j < masChannelOffsets.NumElements(); j++)
	{
		psChannel = masChannelOffsets.Get(j);
		if (psChannel->eType == eType)
		{
			paiChannels->Add(psChannel->iChannel);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannels::IsOnlyBasicTypes(void)
{
	return mbOnlyBasicTypes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetByteStride(int iByteStride)
{
	miByteStride = iByteStride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetBitStrideFromByteStride(void)
{
	miBitStride = miByteStride*8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Dump(int iLineLength)
{
	int			i;
	CChannel*	psChannel;
	char*		szTypeName;
	CChars		c;
	int			iRemain;

	c.Init();

	for (i = 0; i < masChannelOffsets.NumElements(); i++)
	{
		psChannel = masChannelOffsets.Get(i);
		szTypeName = gcClassStorage.GetName(psChannel->eType);
		c.Append("Channel[");
		c.Append(psChannel->iChannel);
		c.Append("]: Type[");
		c.Append(szTypeName);
		c.Append("], ByteOffset[");
		c.Append(psChannel->miByteOffset);
		c.Append("], BitOffset[");
		c.Append(psChannel->miBitOffset);
		c.Append("]\n");
	}

	c.Append("Number of elements[");
	c.Append(miSize);
	c.Append("], Data array size[");
	c.Append(mabData.NumElements());
	c.Append("]\n");

	c.Append("ByteStride[");
	c.Append(miByteStride);
	c.Append("]\n");
	c.Append("BitStride[");
	c.Append(miBitStride);
	c.Append("]\n");

	if (iLineLength != 0)
	{
		for (i = 0; i <= mabData.NumElements()/iLineLength; i++)
		{
			if (mabData.NumElements() - iLineLength*i < iLineLength)
			{
				iRemain = mabData.NumElements() - iLineLength*i;
			}
			else
			{
				iRemain = iLineLength;
			}
			c.AppendHexLoHi(mabData.Get(i*iLineLength), iRemain);
			c.AppendNewLine();
		}
	}
	c.Dump();
	c.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveTypes CChannels::GetLargestPrimitiveType(void)
{
	CChannel*			pcChannel;
	int					i;
	EPrimitiveTypes		eLargeType;
	int					iLargeSize;
	int					iSize;

	//This (correctly) doesn't handle embedded structs.  An external channel should only contain primitive types.

	eLargeType = PT_Undefined;
	iLargeSize = -1;
	for (i = 0; i < masChannelOffsets.NumElements(); i++)
	{
		pcChannel = masChannelOffsets.Get(i);
		iSize = pcChannel->miByteSize;
		if (iSize > iLargeSize)
		{
			iLargeSize = iSize;
			eLargeType = pcChannel->eType;
		}
	}
	return eLargeType;
}
