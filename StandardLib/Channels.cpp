/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/Chars.h"
#include "BaseLib/Define.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "Channels.h"
#include "BaseLib/ErrorHandler.h"


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
	mpmicChannelDebugs = NULL;
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
void CChannels::Init(lsize iSize, EPrimitiveType eType, size iFirst, ...)
{
	Init();

	va_list		vaMarker;
	size		iCount;
	size		i;

	iCount = 0;
	i = iFirst;

	BeginChange();
	va_start(vaMarker, iFirst);
	while (i != ARRAY_ELEMENT_NOT_FOUND)
	{
		AddChannel(i, eType);
		iCount++;
		i = VA_ARG_SIZE(vaMarker);
	}
	va_end(vaMarker);

	SetSize(iSize);
	EndChange();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Init(lsize iSize, void* pvUserData, EPrimitiveType eType, size iFirst, ...)
{
	Init();

	va_list		vaMarker;
	size		iCount;
	size		i;

	iCount = 0;
	i = iFirst;

	BeginChange();
	va_start(vaMarker, iFirst);
	while (i != ARRAY_ELEMENT_NOT_FOUND)
	{
		AddChannel(i, eType);
		iCount++;
		i = VA_ARG_SIZE(vaMarker);
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
	Init();

	CChannel*	psSource;
	size		i;
	size		uiOffsets;

	BeginChange();

	uiOffsets = pcSource->masChannelOffsets.NumElements();
	for (i = 0; i < uiOffsets; i++)
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
void CChannels::Class(void)
{
	U_LSize(miSize);
	U_Size(miByteStride);
	U_Size(miBitStride);
	U_Bool(mbOnlyBasicTypes);

	U_Unknown(CArrayChannelOffset, masChannelOffsets);
	U_Unknown(CArrayLargeChar, mabData);
	U_Pointer(mpvUserData);
	U_Pointer(mpsChangingDesc);
	U_Pointer(mpvDataCache);
	U_Pointer(mpmicChannelDebugs);
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
	SafeKill(mpmicChannelDebugs);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::ReInit(void)
{
	Free();

	mabData.Init();
	masChannelOffsets.Init();
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
	ReturnOnFalse(pcFile->ReadSize(&miSize));
	ReturnOnFalse(pcFile->ReadSize(&miByteStride));
	ReturnOnFalse(pcFile->ReadSize(&miBitStride));
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
void CChannels::SetSize(lsize iSize)
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
void CChannels::AddChannel(SChannel* psChannel)
{
	AddChannel(psChannel->iChannel, psChannel->eType, psChannel->bReverse);
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
void CChannels::AddChannel(size iChannel, EPrimitiveType eType, const char* szShortName, const char* szLongName, bool bReverse)
{
	AddChannel(iChannel, eType, bReverse);
	SetChannelDebugNames(iChannel, szShortName, szLongName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetChannelDebugNames(size iChannel, const char* szShortName, const char* szLongName)
{
	CChannelDebugDescriptor*	psDesc;

	if ((szShortName != NULL) && (szLongName != NULL))
	{
		if (mpmicChannelDebugs == NULL)
		{
			mpmicChannelDebugs = NewMalloc<CMapIntChannelDescriptor>();
			mpmicChannelDebugs->Init();
		}

		psDesc = mpmicChannelDebugs->Put((int)iChannel);
		if (psDesc)
		{
			psDesc->Init(szLongName, szShortName);
		}
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
		return (mpsChangingDesc->asRemovedChannels.FindWithSizeKey(iChannel, 0) != ARRAY_ELEMENT_NOT_FOUND);
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
		return (mpsChangingDesc->asAddedChannels.FindWithSizeKey(iChannel, 0) != ARRAY_ELEMENT_NOT_FOUND);
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
	CChannel*		pcChannel;
	size			iSize;
	size			iOffset;
	lsize			uiByteSize;
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
			//Remove
			uiRemoved = mpsChangingDesc->asRemovedChannels.NumElements();
			for (i = 0; i < uiRemoved; i++)
			{
				psRemovedChannel = mpsChangingDesc->asRemovedChannels.Get(i);
				pcChannel = GetChannel(psRemovedChannel->iChannel);

				//Only data that has been allocated needs its bytes removed.  The channel is removed either way.
				if ((miBitStride != 0) && (miSize != 0))
				{
					if (pcChannel->Is8BitAligned())
					{
						iOffset = pcChannel->miByteOffset;
						iSize = pcChannel->miByteSize;
						mabData.RemoveBatch(iOffset, iSize, miSize, miByteStride-iSize);
					}
					else
					{
						//Deal with bitty removal.
					}
				}
				masChannelOffsets.RemoveAt(masChannelOffsets.GetIndex(pcChannel), true);
				Recalculate();

				if (mpmicChannelDebugs)
				{
					mpmicChannelDebugs->Remove((int)psRemovedChannel->iChannel);
				}
			}

			//Add.  If any channel can't be added none are.
			bAnyAdded = false;
			if (CanAddChannels())
			{
				uiAdded = mpsChangingDesc->asAddedChannels.NumElements();
				for (i = 0; i < uiAdded; i++)
				{
					psAddedChannel = mpsChangingDesc->asAddedChannels.Get(i);
					pcChannel = masChannelOffsets.Add();
					pcChannel->Init(psAddedChannel->iChannel, psAddedChannel->eType, psAddedChannel->bReverse);
					bAnyAdded = true;
				}
			}
			else
			{
				bResult = false;
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
				uiByteSize = CalculateByteSize(miBitStride, miSize);
				mabData.Resize(uiByteSize);
			}
		}
		else  //Using user allocated memory.
		{
			//This assumes the mpvUserData is *already* in the form being specified.
			uiRemoved = mpsChangingDesc->asRemovedChannels.NumElements();
			for (i = 0; i < uiRemoved; i++)
			{
				psRemovedChannel = mpsChangingDesc->asRemovedChannels.Get(i);
				pcChannel = GetChannel(psRemovedChannel->iChannel);
				masChannelOffsets.RemoveAt(masChannelOffsets.GetIndex(pcChannel), true);

				if (mpmicChannelDebugs)
				{
					mpmicChannelDebugs->Remove((int)psRemovedChannel->iChannel);
				}
			}

			//User data can describe the same channel more than once (bytes an image ignores, say), so these aren't
			//checked by CanAddChannels.
			uiAdded = mpsChangingDesc->asAddedChannels.NumElements();
			for (i = 0; i < uiAdded; i++)
			{
				psAddedChannel = mpsChangingDesc->asAddedChannels.Get(i);
				pcChannel = masChannelOffsets.Add();
				pcChannel->Init(psAddedChannel->iChannel, psAddedChannel->eType, psAddedChannel->bReverse);
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
// The channels a change adds to channels that own their data must not already be in the channels (after the
// change's removals) or be added twice.
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::CanAddChannels(void)
{
	size		i;
	size		j;
	size		uiAdded;
	SChannel*	psAddedChannel;

	uiAdded = mpsChangingDesc->asAddedChannels.NumElements();
	for (i = 0; i < uiAdded; i++)
	{
		psAddedChannel = mpsChangingDesc->asAddedChannels.Get(i);
		if (GetChannel(psAddedChannel->iChannel))
		{
			return gcLogger.Error2(__METHOD__, " Channel [", SizeToString(psAddedChannel->iChannel), "] already exists in Channels.", NULL);
		}
		for (j = 0; j < i; j++)
		{
			if (mpsChangingDesc->asAddedChannels.Get(j)->iChannel == psAddedChannel->iChannel)
			{
				return gcLogger.Error2(__METHOD__, " Channel [", SizeToString(psAddedChannel->iChannel), "] is added twice.", NULL);
			}
		}
	}
	return true;
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
	return MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CChannels::CalculateByteSize(size iBitStride, lsize iSize)
{
	uint64	ulliBitSize;
	uint64	ulliByteSize;

	//The size in bits overflows 32 bits at only 512MB of data so it is calculated in 64 bits.
	ulliBitSize = (uint64)iBitStride * (uint64)iSize;
	ulliByteSize = (ulliBitSize + 7) / 8;
	if (ulliByteSize > MAX_LSIZE)
	{
		gcLogger.Error2(__METHOD__, " Channels need [", LongToString(ulliByteSize), "] bytes, which is more than can be addressed.", NULL);
		return 0;
	}
	return (lsize)ulliByteSize;
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
	lsize	iSize;

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
// Copies pcChannel's value at iPos into pvDest (8 bytes) as it is
// stored, without converting it.  Channels bigger than 8 bytes are cut
// to their first 8.  A channel that isn't whole bytes at a whole byte is
// put in the low bits of pvDest, the rest zero.  A reversed channel is
// byte swapped (or bit reversed if it is bit sized).
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetNative(lsize iPos, CChannel* pcChannel, void* pvDest)
{
	size	uiByteSize;
	lsize	uiBitPos;
	size	uiNumBits;

	uiByteSize = pcChannel->miByteSize < sizeof(uint64) ? pcChannel->miByteSize : sizeof(uint64);
	if (pcChannel->Is8BitAligned() && (miByteStride != 0))
	{
		memcpy(pvDest, &mpvDataCache[iPos * miByteStride + pcChannel->miByteOffset], uiByteSize);
	}
	else
	{
		memset(pvDest, 0, sizeof(uint64));
		uiBitPos = iPos * miBitStride + pcChannel->miBitOffset;
		uiNumBits = pcChannel->miBitSize < 64 ? pcChannel->miBitSize : 64;
		if (pcChannel->bReverse && (pcChannel->miByteSize == CHANNEL_NON_ALIGNED_BYTES))
		{
			CopyBitsReverseHiLo(pvDest, 0, mpvDataCache, uiBitPos, uiNumBits, true);
			return;
		}
		CopyBits(pvDest, 0, mpvDataCache, uiBitPos, uiNumBits, true);
	}

	if (pcChannel->bReverse)
	{
		ReverseBytes(pvDest, uiByteSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The reverse of GetNative.  The other channels are left alone.
//
//////////////////////////////////////////////////////////////////////////
void CChannels::SetNative(lsize iPos, CChannel* pcChannel, void* pvSource)
{
	uint64	ulliSwapped;
	size	uiByteSize;
	lsize	uiBitPos;
	size	uiNumBits;

	uiByteSize = pcChannel->miByteSize < sizeof(uint64) ? pcChannel->miByteSize : sizeof(uint64);
	if (pcChannel->bReverse && (pcChannel->miByteSize != CHANNEL_NON_ALIGNED_BYTES))
	{
		ulliSwapped = 0;
		memcpy(&ulliSwapped, pvSource, uiByteSize);
		ReverseBytes(&ulliSwapped, uiByteSize);
		pvSource = &ulliSwapped;
	}

	if (pcChannel->Is8BitAligned() && (miByteStride != 0))
	{
		memcpy(&mpvDataCache[iPos * miByteStride + pcChannel->miByteOffset], pvSource, uiByteSize);
		return;
	}

	uiBitPos = iPos * miBitStride + pcChannel->miBitOffset;
	uiNumBits = pcChannel->miBitSize < 64 ? pcChannel->miBitSize : 64;
	if (pcChannel->bReverse && (pcChannel->miByteSize == CHANNEL_NON_ALIGNED_BYTES))
	{
		CopyBitsReverseHiLo(mpvDataCache, uiBitPos, pvSource, 0, uiNumBits, false);
	}
	else
	{
		CopyBits(mpvDataCache, uiBitPos, pvSource, 0, uiNumBits, false);
	}
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
size CChannels::GetIndexOfChannel(size iChannel)
{
	return masChannelOffsets.FindWithSizeKey(iChannel, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CChannels::GetChannel(size iChannel)
{
	size	iIndex;

	iIndex = masChannelOffsets.FindWithSizeKey(iChannel, 0);
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
		eIC = VA_ARG_SIZE(vaMarker);
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
void CChannels::Black(void)
{
	if (!IsChanging())
	{
		if (IsUserAllocated())
		{
			//mabData is empty when the data belongs to the user.
			memset(mpvUserData, 0, GetByteSize());
		}
		else
		{
			mabData.Zero();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::White(void)
{
	if (!IsChanging())
	{
		if (IsUserAllocated())
		{
			//mabData is empty when the data belongs to the user.
			memset(mpvUserData, 0xff, GetByteSize());
		}
		else
		{
			mabData.One();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::Copy(CChannels* pcSource)
{
	//This assumes Channels is not initialised.
	PreInit();
	masChannelOffsets.Init();
	mabData.Init();

	CopyIntoInitialised(pcSource);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::CopyIntoInitialised(CChannels* pcSource)
{
	//This assumes Image IS initialised.

	masChannelOffsets.Copy(&pcSource->masChannelOffsets);
	miSize = pcSource->miSize;
	miByteStride = pcSource->miByteStride;
	miBitStride = pcSource->miBitStride;
	mbOnlyBasicTypes = pcSource->mbOnlyBasicTypes;
	mabData.Copy(&pcSource->mabData);
	mpvUserData = pcSource->mpvUserData;

	if (IsUserAllocated())
	{
		mpvDataCache = mpvUserData;
	}
	else
	{
		mpvDataCache = mabData.GetData();
	}

	if (pcSource->IsChanging())
	{
		BeginChange();
		mpsChangingDesc->iSize = pcSource->mpsChangingDesc->iSize;
		mpsChangingDesc->pvUserData = pcSource->mpsChangingDesc->pvUserData;
		mpsChangingDesc->asAddedChannels.Copy(&pcSource->mpsChangingDesc->asAddedChannels);
		mpsChangingDesc->asRemovedChannels.Copy(&pcSource->mpsChangingDesc->asRemovedChannels);
	}

	if (pcSource->mpmicChannelDebugs != NULL)
	{
		CopyChannelDebugs(pcSource->mpmicChannelDebugs);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::CopyChannelDebugs(CMapIntChannelDescriptor* mpmicSourceChannelDebugs)
{
	SMapIterator				sIter;
	uint*						puiChannel;
	CChannelDebugDescriptor*	pcDesc;
	bool						bHasNext;

	bHasNext = mpmicSourceChannelDebugs->StartIteration(&sIter, (void**)&puiChannel, NULL, (void**)&pcDesc, NULL);
	while (bHasNext)
	{
		SetChannelDebugNames(*puiChannel, pcDesc->mszShortName, pcDesc->mszLongName);
		bHasNext = mpmicSourceChannelDebugs->Iterate(&sIter, (void**)&puiChannel, NULL, (void**)&pcDesc, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannels::IsValid(lsize iPos)
{
	if ((iPos >= miSize) || (iPos == MAX_LSIZE))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CChannels::GetSize(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CChannels::GetByteSize(void)
{
	if (!IsUserAllocated())
	{
		return mabData.NumElements();
	}
	else
	{
		return CalculateByteSize(miBitStride, miSize);
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
const char* CChannels::GetChannelLongName(size iChannel)
{
	CChannelDebugDescriptor*	psDesc;

	if (mpmicChannelDebugs)
	{
		psDesc = mpmicChannelDebugs->Get((int)iChannel);
		if (psDesc)
		{
			return psDesc->mszLongName;
		}
	}
	
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CChannels::GetChannelShortName(size iChannel)
{
	CChannelDebugDescriptor* psDesc;

	if (mpmicChannelDebugs)
	{
		psDesc = mpmicChannelDebugs->Get((int)iChannel);
		if (psDesc)
		{
			return psDesc->mszShortName;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannels::GetAllChannels(CArraySize* paiChannels)
{
	size		j;
	CChannel*	psChannel;
	size		uiOffsets;

	uiOffsets = masChannelOffsets.NumElements();
	for (j = 0; j < uiOffsets; j++)
	{
		psChannel = masChannelOffsets.Get(j);
		paiChannels->Add((int)psChannel->iChannel);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannels::NumChannels(void)
{
	return masChannelOffsets.NumElements();
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
void CChannels::GetChannelsForType(EPrimitiveType eType, CArraySize* paiChannels)
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
			paiChannels->Add((int)psChannel->iChannel);
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
void CChannels::Dump(lsize iLineLength)
{
	size		i;
	lsize		uiLine;
	CChannel*	psChannel;
	char*		szTypeName;
	CChars		c;
	lsize		iRemain;
	lsize		uiData;
	size		uiOffsets;
	lsize		uiNum;

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
		for (uiLine = 0; uiLine <= uiNum; uiLine++)
		{
			if ((uiData - (iLineLength * uiLine)) < iLineLength)
			{
				iRemain = uiData - iLineLength * uiLine;
			}
			else
			{
				iRemain = iLineLength;
			}
			c.AppendHexLoHi(mabData.Get(uiLine * iLineLength), (size)iRemain);
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
	iLargeSize = MAX_SIZE;
	uiOffsets = masChannelOffsets.NumElements();
	for (i = 0; i < uiOffsets; i++)
	{
		pcChannel = masChannelOffsets.Get(i);
		iSize = pcChannel->miByteSize;
		if (((iSize > iLargeSize) || (iLargeSize == MAX_SIZE)) && (iSize != CHANNEL_NON_ALIGNED_BYTES))
		{
			iLargeSize = iSize;
			eLargeType = pcChannel->eType;
		}
	}
	return eLargeType;
}

