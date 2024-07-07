/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/ArrayIntMinimal.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Define.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/TypeNames.h"
#include "Channels.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::PrivateInit(void)
{
	masChannelOffsets.Init();
	miSize = 0;
	miByteStride = 0;
	miBitStride = 0;
	mbOnlyBasicTypes = true;
	mabData.Init();
	mpvUserData = NULL;

	mpsChangingDesc = NULL;
	mpvDataCache = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(void)
{
	PreInit();
	PrivateInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(size iSize, EPrimitiveType eType, size iFirst, ...)
{
	PreInit();

	va_list		vaMarker;
	size		iCount;
	size		i;

	PrivateInit();
	iCount = 0;
	i = iFirst;

	BeginChange();
	va_start(vaMarker, iFirst);
	while (i != ARRAY_ELEMENT_NOT_FOUND)
	{
		AddChannel(i, eType);
		iCount++;
		i = va_arg(vaMarker, size);
	}
	va_end(vaMarker);

	SetSize(iSize);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(size iSize, void* pvUserData, EPrimitiveType eType, size iFirst, ...)
{
	PreInit();

	va_list		vaMarker;
	size		iCount;
	size		i;

	PrivateInit();
	iCount = 0;
	i = iFirst;

	BeginChange();
	va_start(vaMarker, iFirst);
	while (i != ARRAY_ELEMENT_NOT_FOUND)
	{
		AddChannel(i, eType);
		iCount++;
		i = va_arg(vaMarker, size);
	}
	va_end(vaMarker);

	SetSize(iSize);
	SetData(pvUserData);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(CChannels* pcSource)
{
	PreInit();

	CChannel*	psSource;
	size		i;
	size		uiOffsets;

	PrivateInit();
	BeginChange();

	uiOffsets = pcSource->masChannelOffsets.NumElements();
	for (i = 0; i < uiOffsets; i++)
	{
		psSource = pcSource->masChannelOffsets.Get(i);
		AddChannel(psSource->iChannel, psSource->eType, psSource->bReverse);
	}

	SetSize(pcSource->miSize);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Class(void)
{
	/*
	CArrayChannelOffset		masChannelOffsets;
	size					miSize;				//The number of 'pixels' in the channels (not the size in bytes).
	size					miByteStride;		//The number of bytes between 'pixels' zero if not a whole byte.
	size					miBitStride;		//The number of bits between pixels.
	bool					mbOnlyBasicTypes;	//Channels are only chars, shorts, ints, longs and floats.  Nothing bit'ty.

	CArrayChar				mabData;
	char*					mpvUserData;

	SChannelsChangingDesc*	mpsChangingDesc;
	char*					mpvDataCache;		//A pointer to either mabData.pvArray or mpvUserData
	*/

	UnmanagedSize(&miSize, "miSize");
	UnmanagedSize(&miByteStride, "miByteStride");
	UnmanagedSize(&miBitStride, "miBitStride");
	UnmanagedBool(&mbOnlyBasicTypes, "mbOnlyBasicTypes");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Free(void)
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
bool CChannels::Load(CObjectReader* pcFile)
{
	mpsChangingDesc = NULL;
	mpvUserData = NULL;
	ReturnOnFalse(masChannelOffsets.Read(pcFile));
	ReturnOnFalse(pcFile->ReadInt32(&miSize));
	ReturnOnFalse(pcFile->ReadInt32(&miByteStride));
	ReturnOnFalse(pcFile->ReadInt32(&miBitStride));
	ReturnOnFalse(pcFile->ReadBool(&mbOnlyBasicTypes));
	ReturnOnFalse(mabData.Read(pcFile));
	mpvDataCache = mabData.GetData();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(masChannelOffsets.Write(pcFile));
	ReturnOnFalse(pcFile->WriteSize(miSize));
	ReturnOnFalse(pcFile->WriteSize(miByteStride));
	ReturnOnFalse(pcFile->WriteSize(miBitStride));
	ReturnOnFalse(pcFile->WriteBool(mbOnlyBasicTypes));
	ReturnOnFalse(mabData.Write(pcFile));
	return true;
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
void CChannels::SetSize(size iSize)
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
void CChannels::AddChannel(size iChannel, EPrimitiveType eType, bool bReverse)
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
void CChannels::AddChannel(size iChannel1, size iChannel2, EPrimitiveType eType, bool bReverse)
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
void CChannels::AddChannel(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType, bool bReverse)
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
void CChannels::AddChannel(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType, bool bReverse)
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
	size		i;
	SChannel*	psChannel;
	size		uiChannels;

	if (IsChanging())
	{
		uiChannels = pasChannels->NumElements();
		for (i = 0; i < uiChannels; i++)
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
	size		i;
	CChannel*	pcChannel;
	size		uiOffsets;

	if (IsChanging())
	{
		uiOffsets = pcSourceChannels->masChannelOffsets.NumElements();
		for (i = 0; i < uiOffsets; i++)
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
void CChannels::PrivateAddChannel(size iChannel, EPrimitiveType eType, bool bReverse)
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
bool CChannels::RemoveChannel(size iChannel)
{
	SChannel* psChannel;

	if (IsChanging())
	{
		if ((HasChannel(iChannel)) && (!IsChannelRemoved(iChannel)))
		{
			psChannel = mpsChangingDesc->asRemovedChannels.Add();
			psChannel->iChannel = iChannel;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::IsChannelRemoved(size iChannel)
{
	if (IsChanging())
	{
		return (mpsChangingDesc->asRemovedChannels.FindWithIntKey(iChannel, 0) != ARRAY_ELEMENT_NOT_FOUND);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::IsChannelAdded(size iChannel)
{
	if (IsChanging())
	{
		return (mpsChangingDesc->asAddedChannels.FindWithIntKey(iChannel, 0) != ARRAY_ELEMENT_NOT_FOUND);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::IsChanging(void)
{
	return (mpsChangingDesc != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::EndChange(void)
{
	size			i;
	SChannel*		psAddedChannel;
	SChannel*		psRemovedChannel;
	CChannel*		psChannel;
	size			iSize;
	size			iOffset;
	bool			bAnyAdded;
	size			iOldBitStride;
	size			iAddedBitStride;
	size			iOldByteStride;
	bool			bResult;
	size			uiRemoved;
	size			uiAdded;

	//I don't think this handles the case where the channels have been re-ordered...
	//Which as there *IS* no way of re-ordering them shouldn't be a problem.

	bResult = true;
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
				uiRemoved = mpsChangingDesc->asRemovedChannels.NumElements();
				for (i = 0; i < uiRemoved; i++)
				{
					psRemovedChannel = mpsChangingDesc->asRemovedChannels.Get(i);
					psChannel = GetChannel(psRemovedChannel->iChannel);
					if (psChannel->Is8BitAligned())
					{
						iOffset = psChannel->miByteOffset;
						iSize = psChannel->miByteSize;
						mabData.RemoveBatch(iOffset, iSize, miSize, miByteStride-iSize);
					}
					else
					{
						//Deal with bitty removal.
					}
					masChannelOffsets.RemoveAt(masChannelOffsets.GetIndex(psChannel), true);
					Recalculate();
				}
			}

			//Add
			bAnyAdded = false;
			uiAdded = mpsChangingDesc->asAddedChannels.NumElements();
			for (i = 0; i < uiAdded; i++)
			{
				psAddedChannel = mpsChangingDesc->asAddedChannels.Get(i);

				psChannel = masChannelOffsets.Add();
				psChannel->Init(psAddedChannel->iChannel, psAddedChannel->eType, psAddedChannel->bReverse);
				bAnyAdded = true;
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
						mabData.InsertBatch(iOffset, iSize, miSize, iOldByteStride);
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
				mabData.Resize(iSize);
			}
		}
		else  //Using user allocated memory.
		{
			//This assumes the mpvUserData is *already* in the form being specified.
			uiRemoved = mpsChangingDesc->asRemovedChannels.NumElements();
			for (i = 0; i < uiRemoved; i++)
			{
				psRemovedChannel = mpsChangingDesc->asRemovedChannels.Get(i);
				psChannel = GetChannel(psRemovedChannel->iChannel);
				masChannelOffsets.RemoveAt(masChannelOffsets.GetIndex(psChannel), true);
			}

			uiAdded = mpsChangingDesc->asAddedChannels.NumElements();
			for (i = 0; i < uiAdded; i++)
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
	size		i;
	CChannel*	pcChannel;
	size		iBitSize;
	size		iByteSize;
	size		uiOffsets;

	miBitStride = 0;
	mbOnlyBasicTypes = true;
	uiOffsets = masChannelOffsets.NumElements();
	for (i = 0; i < uiOffsets; i++)
	{
		pcChannel = masChannelOffsets.Get(i);

		iByteSize = gcTypeNames.GetByteSize(pcChannel->eType);
		iBitSize = gcTypeNames.GetBitSize(pcChannel->eType);
		if (iByteSize == 0)
		{
			mbOnlyBasicTypes = false;
		}

		pcChannel->miBitSize = iBitSize;
		if (Is8BitAligned(miBitStride, iBitSize))
		{
			pcChannel->miByteSize = iByteSize;
			pcChannel->miByteOffset = miBitStride / 8;
		}
		else
		{
			pcChannel->miByteSize = CHANNEL_NON_ALIGNED_BYTES;
			pcChannel->miByteOffset = CHANNEL_NON_ALIGNED_BYTES;
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
bool CChannels::Is8BitAligned(size iBitStart, size iBitLength)
{
	if ((iBitStart % 8 == 0) && (iBitLength % 8 == 0))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::CalculateByteStride(size iBitStride)
{
	if (iBitStride % 8 == 0)
	{
		return iBitStride / 8;
	}
	return SIZE_MAX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::CalculateByteSize(size iBitStride, size iSize)
{
	size	iByteSize;
	size	iBitSize;

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
bool CChannels::RenameChannel(size iOldName, size iNewName)
{
	CChannel*	pcChannel;

	pcChannel = GetChannel(iOldName);
	if (pcChannel)
	{
		pcChannel->iChannel = iNewName;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::ByteAlign(void)
{
	size		i;
	SChannel*	psChannel;
	size		uiAdded;

	if (IsChanging())
	{
		uiAdded = mpsChangingDesc->asAddedChannels.NumElements();
		for (i = 0; i < uiAdded; i++)
		{
			psChannel = mpsChangingDesc->asAddedChannels.Get(i);
			if ((psChannel->eType >= PRIMTIVE_OPERATOR_END) && (psChannel->eType < PRIMTIVE_CONVERTER_END))
			{
				psChannel->eType = PT_uint8;
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
	size	iSize;

	iSize = CalculateByteSize(miBitStride, miSize);
	FreeData();
	mabData.Init();
	mabData.Resize(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::IsUserAllocated(void)
{
	return (mpvUserData != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::GetNumChannels(void)
{
	return masChannelOffsets.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CChannels::GetChannelAtIndex(size iIndex)
{
	return masChannelOffsets.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::GetIndexOfChannel(CChannel* pcChannel)
{
	return masChannelOffsets.GetIndex(pcChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CChannels::GetChannel(size iChannel)
{
	size	iIndex;

	iIndex = masChannelOffsets.FindWithIntKey(iChannel, 0);
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	return masChannelOffsets.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::HasChannel(size iChannel)
{
	CChannel*	psChannel;

	psChannel = GetChannel(iChannel);
	return (psChannel != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::HasChannel(size iChannel1, size iChannel2)
{
	if (HasChannel(iChannel1))
	{
		return HasChannel(iChannel2);
	}
	return false;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::HasChannel(size iChannel1, size iChannel2, size iChannel3)
{
	if (HasChannel(iChannel1))
	{
		if (HasChannel(iChannel2))
		{
			return HasChannel(iChannel3);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::HasChannels(size iFirst, ...)
{
	va_list		vaMarker;
	size		eIC;
	bool		bResult;

	eIC = iFirst;
	bResult = true;

	va_start(vaMarker, iFirst);
	while (eIC != ARRAY_ELEMENT_NOT_FOUND)
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
bool CChannels::IsSameFormat(CChannels* psOther)
{
	size		iIndex;
	CChannel*	psSourceChannel;
	CChannel*	psThisChannel;
	size		uiThisChannels;

	uiThisChannels = GetNumChannels();
	if (uiThisChannels == psOther->GetNumChannels())
	{
		for (iIndex = 0; iIndex < uiThisChannels; iIndex++)
		{
			psSourceChannel = psOther->GetChannelAtIndex(iIndex);
			psThisChannel = GetChannelAtIndex(iIndex);
			if (!(	(psSourceChannel->eType == psThisChannel->eType) &&
					(psSourceChannel->iChannel == psThisChannel->iChannel) &&
					(psSourceChannel->bReverse == psThisChannel->bReverse)))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::IsIgnored(void)
{
	if (IsChanging())
	{
		return false;
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
	//This assumes channels is not initialised.

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
bool CChannels::IsValid(size iPos)
{
	if ((iPos >= miSize) || (iPos == SIZE_MAX))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::GetSize(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::GetByteSize(void)
{
	if (!IsUserAllocated())
	{
		return mabData.NumElements();
	}
	else
	{
		if (miByteStride != SIZE_MAX)
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
size CChannels::GetByteStride(void)
{
	return miByteStride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::GetBitStride(void)
{
	return miBitStride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetAllChannels(CArrayInt* paiChannels)
{
	size		j;
	CChannel*	psChannel;
	size		uiOffsets;

	paiChannels->Init();

	uiOffsets = masChannelOffsets.NumElements();
	for (j = 0; j < uiOffsets; j++)
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
	size		j;
	CChannel*	psChannelSource;
	SChannel*	psChannelDest;
	size		uiOffsets;

	uiOffsets = masChannelOffsets.NumElements();

	pasChannels->Init();
	pasChannels->Resize(uiOffsets);

	for (j = 0; j < uiOffsets; j++)
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
EPrimitiveType CChannels::GetPrimitiveType(void)
{
	size			j;
	CChannel*		psChannel;
	EPrimitiveType	eType;
	size			uiOffsets;

	uiOffsets = masChannelOffsets.NumElements();

	eType = PT_Undefined;
	for (j = 0; j < uiOffsets; j++)
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
	size		j;
	CChannel*	psChannel;
	size		uiOffsets;

	uiOffsets = masChannelOffsets.NumElements();

	paiPrimitiveTypes->Init();
	for (j = 0; j < uiOffsets; j++)
	{
		psChannel = masChannelOffsets.Get(j);
		paiPrimitiveTypes->AddIfUnique((int)psChannel->eType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetChannelsForType(EPrimitiveType eType, CArrayInt* paiChannels)
{
	size		j;
	CChannel*	psChannel;
	size		uiOffsets;

	uiOffsets = masChannelOffsets.NumElements();

	paiChannels->Init();

	for (j = 0; j < uiOffsets; j++)
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
bool CChannels::IsOnlyBasicTypes(void)
{
	return mbOnlyBasicTypes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetByteStride(size iByteStride)
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
void CChannels::Dump(size iLineLength)
{
	size		i;
	CChannel*	psChannel;
	char*		szTypeName;
	CChars		c;
	size		iRemain;
	size		uiData;
	size		uiOffsets;
	size		uiNum;

	uiOffsets = masChannelOffsets.NumElements();

	c.Init();

	for (i = 0; i < uiOffsets; i++)
	{
		psChannel = masChannelOffsets.Get(i);
		szTypeName = (char*)gcTypeNames.GetPrettyName(psChannel->eType);
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
		uiData = mabData.NumElements();
		uiNum = uiData / iLineLength;
		for (i = 0; i <= uiNum; i++)
		{
			if ((uiData - (iLineLength * i)) < iLineLength)
			{
				iRemain = uiData - iLineLength * i;
			}
			else
			{
				iRemain = iLineLength;
			}
			c.AppendHexLoHi(mabData.Get(i * iLineLength), iRemain);
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
EPrimitiveType CChannels::GetLargestPrimitiveType(void)
{
	CChannel*		pcChannel;
	size			i;
	EPrimitiveType	eLargeType;
	size			iLargeSize;
	size			iSize;
	size			uiOffsets;

	//This (correctly) doesn't handle embedded structs.  An external channel should only contain primitive types.

	eLargeType = PT_Undefined;
	iLargeSize = SIZE_MAX;
	uiOffsets = masChannelOffsets.NumElements();
	for (i = 0; i < uiOffsets; i++)
	{
		pcChannel = masChannelOffsets.Get(i);
		iSize = pcChannel->miByteSize;
		if (((iSize > iLargeSize) || (iLargeSize == SIZE_MAX)) && (iSize != CHANNEL_NON_ALIGNED_BYTES))
		{
			iLargeSize = iSize;
			eLargeType = pcChannel->eType;
		}
	}
	return eLargeType;
}

