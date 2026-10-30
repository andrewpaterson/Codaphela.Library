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

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <math.h>
#include "BaseLib/TypeNames.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "Sound.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsSampleType(EPrimitiveType eType)
{
	switch (eType)
	{
	case PT_int8:
	case PT_int16:
	case PT_int32:
	case PT_uint8:
	case PT_uint16:
	case PT_uint32:
	case PT_float32:
	case PT_float64:
		return true;
	default:
		return IsBittySampleType(eType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsBittySampleType(EPrimitiveType eType)
{
	return GetBittySampleBits(eType) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits in a bit sized sample type (bit, crumb, tribble, nybble,
// nickle or sixbits), zero for other types.
//
//////////////////////////////////////////////////////////////////////////
int GetBittySampleBits(EPrimitiveType eType)
{
	if ((eType < PT_bit) || (eType > PT_sixbits))
	{
		return 0;
	}
	return (int)gcTypeNames.GetBitSize(eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 ConvertSampleToFloat(void* pvSample, EPrimitiveType eType)
{
	switch (eType)
	{
	case PT_int8:
		return (float64)*((int8*)pvSample) / 128.0;
	case PT_int16:
		return (float64)*((int16*)pvSample) / 32768.0;
	case PT_int32:
		return (float64)*((int32*)pvSample) / 2147483648.0;
	case PT_uint8:
		return ((float64)*((uint8*)pvSample) - 128.0) / 128.0;
	case PT_uint16:
		return ((float64)*((uint16*)pvSample) - 32768.0) / 32768.0;
	case PT_uint32:
		return ((float64)*((uint32*)pvSample) - 2147483648.0) / 2147483648.0;
	case PT_float32:
		return (float64)*((float32*)pvSample);
	case PT_float64:
		return *((float64*)pvSample);
	case PT_bit:
		return (*((uint8*)pvSample) & 1) ? 1.0 : -1.0;
	default:
		return ConvertBittySampleToFloat(*((uint8*)pvSample), GetBittySampleBits(eType));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Bit sized samples are offset binary, like uint8: the middle code is
// silence.  iBits must be 2 or more.
//
//////////////////////////////////////////////////////////////////////////
float64 ConvertBittySampleToFloat(uint8 uiCode, int iBits)
{
	float64	fHalf;

	if (iBits < 2)
	{
		return 0.0;
	}
	fHalf = (float64)(1 << (iBits - 1));
	return ((float64)(uiCode & ((1 << iBits) - 1)) - fHalf) / fHalf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 ConvertBittySampleFromFloat(float64 fSample, int iBits)
{
	float64	fHalf;

	if (iBits < 2)
	{
		return 0;
	}
	fHalf = (float64)(1 << (iBits - 1));
	return (uint8)(QuantiseSample(fSample, fHalf, -fHalf, fHalf - 1.0) + fHalf);
}


//////////////////////////////////////////////////////////////////////////
//
// Rounds fSample * fScale to the nearest integer in [fMin, fMax].
//
//////////////////////////////////////////////////////////////////////////
float64 QuantiseSample(float64 fSample, float64 fScale, float64 fMin, float64 fMax)
{
	float64	f;

	f = floor(fSample * fScale + 0.5);
	if (f < fMin)
	{
		return fMin;
	}
	if (f > fMax)
	{
		return fMax;
	}
	if (f != f)
	{
		//NaN.
		return 0.0;
	}
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ConvertSampleFromFloat(void* pvSample, EPrimitiveType eType, float64 fSample)
{
	switch (eType)
	{
	case PT_int8:
		*((int8*)pvSample) = (int8)QuantiseSample(fSample, 128.0, -128.0, 127.0);
		break;
	case PT_int16:
		*((int16*)pvSample) = (int16)QuantiseSample(fSample, 32768.0, -32768.0, 32767.0);
		break;
	case PT_int32:
		*((int32*)pvSample) = (int32)QuantiseSample(fSample, 2147483648.0, -2147483648.0, 2147483647.0);
		break;
	case PT_uint8:
		*((uint8*)pvSample) = (uint8)(QuantiseSample(fSample, 128.0, -128.0, 127.0) + 128.0);
		break;
	case PT_uint16:
		*((uint16*)pvSample) = (uint16)(QuantiseSample(fSample, 32768.0, -32768.0, 32767.0) + 32768.0);
		break;
	case PT_uint32:
		*((uint32*)pvSample) = (uint32)(QuantiseSample(fSample, 2147483648.0, -2147483648.0, 2147483647.0) + 2147483648.0);
		break;
	case PT_float32:
		*((float32*)pvSample) = (float32)fSample;
		break;
	case PT_float64:
		*((float64*)pvSample) = fSample;
		break;
	case PT_bit:
		*((uint8*)pvSample) = fSample >= 0.0 ? 1 : 0;
		break;
	default:
		if (IsBittySampleType(eType))
		{
			*((uint8*)pvSample) = ConvertBittySampleFromFloat(fSample, GetBittySampleBits(eType));
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::PrivateInit(void)
{
	mcChannels.Init();
	muiSampleRate = 0;
	mpsSoundChangingDesc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Init(void)
{
	PreInit();
	PrivateInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Init(uint32 uiSampleRate, lsize uiNumFrames, EPrimitiveType eType, EChannel eFirst, ...)
{
	va_list		vaMarker;
	EChannel	eIC;

	PreInit();

	PrivateInit();
	eIC = eFirst;

	BeginChange();
	va_start(vaMarker, eFirst);
	while (eIC != CHANNEL_STOP)
	{
		AddChannel(eIC, eType);
		eIC = VA_ARG_SIZE(vaMarker);
	}
	va_end(vaMarker);

	SetSampleRate(uiSampleRate);
	SetNumFrames(uiNumFrames);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Init(uint32 uiSampleRate, lsize uiNumFrames, void* pvUserData, EPrimitiveType eType, EChannel eFirst, ...)
{
	va_list		vaMarker;
	EChannel	eIC;

	PreInit();

	PrivateInit();
	eIC = eFirst;

	BeginChange();
	va_start(vaMarker, eFirst);
	while (eIC != CHANNEL_STOP)
	{
		AddChannel(eIC, eType);
		eIC = VA_ARG_SIZE(vaMarker);
	}
	va_end(vaMarker);

	SetSampleRate(uiSampleRate);
	SetNumFrames(uiNumFrames);
	SetData(pvUserData);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Init(uint32 uiSampleRate, lsize uiNumFrames, CArrayChannel* pasNewChannels)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pasNewChannels);
	SetSampleRate(uiSampleRate);
	SetNumFrames(uiNumFrames);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Init(uint32 uiSampleRate, lsize uiNumFrames, Ptr<CSound> pcChannelsSource)
{
	PreInit();

	PrivateInit();
	BeginChange();
	AddChannels(pcChannelsSource);
	SetSampleRate(uiSampleRate);
	SetNumFrames(uiNumFrames);
	EndChange();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Init(Ptr<CSound> pcChannelsSource)
{
	Init(pcChannelsSource->GetSampleRate(), pcChannelsSource->GetNumFrames(), pcChannelsSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Class(void)
{
	M_Embedded(mcChannels);
	U_UInt32(muiSampleRate);
	U_Pointer(mpsSoundChangingDesc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Free(void)
{
	SafeFree(mpsSoundChangingDesc);
	muiSampleRate = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::ReInit(void)
{
	Free();
	mcChannels.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::Load(CObjectReader* pcFile)
{
	mpsSoundChangingDesc = NULL;

	ReturnOnFalse(pcFile->ReadInt32(&muiSampleRate));

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt32(muiSampleRate));

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::BeginChange(void)
{
	mcChannels.BeginChange();
	mpsSoundChangingDesc = (SSoundChangingDesc*)malloc(sizeof(SSoundChangingDesc));
	mpsSoundChangingDesc->uiSampleRate = muiSampleRate;
	mpsSoundChangingDesc->uiNumFrames = mcChannels.GetSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::AddChannel(SChannel* psChannel)
{
	mcChannels.AddChannel(psChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::AddChannel(size iChannel, EPrimitiveType eType, bool bReverse)
{
	mcChannels.AddChannel(iChannel, eType, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::AddChannel(size iChannel1, size iChannel2, EPrimitiveType eType, bool bReverse)
{
	AddChannel(iChannel1, eType, bReverse);
	AddChannel(iChannel2, eType, bReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::AddChannels(CArrayChannel* pasChannels)
{
	mcChannels.AddChannels(pasChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::AddChannels(Ptr<CSound> pcSourceChannels)
{
	mcChannels.AddChannels(&pcSourceChannels->mcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::RemoveChannel(size iChannel)
{
	return mcChannels.RemoveChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::RenameChannel(size iOldName, size iNewName)
{
	return mcChannels.RenameChannel(iOldName, iNewName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::SetNumFrames(lsize uiNumFrames)
{
	if (IsChanging())
	{
		mpsSoundChangingDesc->uiNumFrames = uiNumFrames;
		mcChannels.SetSize(uiNumFrames);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::SetSampleRate(uint32 uiSampleRate)
{
	if (IsChanging())
	{
		mpsSoundChangingDesc->uiSampleRate = uiSampleRate;
	}
	else
	{
		muiSampleRate = uiSampleRate;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::ByteAlignChannels(void)
{
	mcChannels.ByteAlign();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::EndChange(void)
{
	bool	bResult;
	lsize	uiNumFrames;

	if (IsChanging())
	{
		muiSampleRate = mpsSoundChangingDesc->uiSampleRate;
		uiNumFrames = mpsSoundChangingDesc->uiNumFrames;
		SafeFree(mpsSoundChangingDesc);
		bResult = mcChannels.EndChange();

		if ((uiNumFrames > 0) && (mcChannels.GetChannelOffsets()->NumElements() > 0) && (mcChannels.GetData() == NULL))
		{
			//The data could not be allocated so the sound is empty rather than the wrong length.
			gcLogger.Error2(__METHOD__, " Could not allocate sound of [", LongToString((uint64)uiNumFrames), "] frames.", NULL);
			mcChannels.BeginChange();
			mcChannels.SetSize(0);
			mcChannels.EndChange();
			return false;
		}
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::IsChanging(void)
{
	return mcChannels.IsChanging();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSound::GetNumFrames(void)
{
	if (IsChanging())
	{
		return mpsSoundChangingDesc->uiNumFrames;
	}
	return mcChannels.GetSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CSound::GetSampleRate(void)
{
	if (IsChanging())
	{
		return mpsSoundChangingDesc->uiSampleRate;
	}
	return muiSampleRate;
}


//////////////////////////////////////////////////////////////////////////
//
// The length of the sound in seconds.
//
//////////////////////////////////////////////////////////////////////////
float64 CSound::GetDuration(void)
{
	if (muiSampleRate == 0)
	{
		return 0.0;
	}
	return (float64)mcChannels.GetSize() / (float64)muiSampleRate;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Copy(Ptr<CSound> pcSource)
{
	//This assumes Sound is not initialised.
	PreInit();

	muiSampleRate = pcSource->muiSampleRate;
	if (pcSource->mpsSoundChangingDesc)
	{
		mpsSoundChangingDesc = (SSoundChangingDesc*)malloc(sizeof(SSoundChangingDesc));
		mpsSoundChangingDesc->uiSampleRate = pcSource->mpsSoundChangingDesc->uiSampleRate;
		mpsSoundChangingDesc->uiNumFrames = pcSource->mpsSoundChangingDesc->uiNumFrames;
	}
	else
	{
		mpsSoundChangingDesc = NULL;
	}

	mcChannels.Copy(&pcSource->mcChannels);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::CopyIntoInitialised(Ptr<CSound> pcSource)
{
	//This assumes Sound IS initialised.

	muiSampleRate = pcSource->muiSampleRate;
	if (pcSource->mpsSoundChangingDesc)
	{
		mpsSoundChangingDesc = (SSoundChangingDesc*)malloc(sizeof(SSoundChangingDesc));
		mpsSoundChangingDesc->uiSampleRate = pcSource->mpsSoundChangingDesc->uiSampleRate;
		mpsSoundChangingDesc->uiNumFrames = pcSource->mpsSoundChangingDesc->uiNumFrames;
	}
	else
	{
		mpsSoundChangingDesc = NULL;
	}

	mcChannels.CopyIntoInitialised(&pcSource->mcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
// Sets every sample to zero.  Unsigned samples are set to the middle of
// their range and padding to zero.
//
//////////////////////////////////////////////////////////////////////////
void CSound::Silence(void)
{
	size		uiNumChannels;
	size		uiIndex;
	lsize		uiFrame;
	lsize		uiNumFrames;
	CChannel*	pcChannel;
	uint64		ulliRaw;

	mcChannels.Black();

	uiNumChannels = NumChannels();
	uiNumFrames = GetNumFrames();
	for (uiIndex = 0; uiIndex < uiNumChannels; uiIndex++)
	{
		pcChannel = GetChannelAtIndex(uiIndex);
		if ((pcChannel->eType == PT_uint8) || (pcChannel->eType == PT_uint16) || (pcChannel->eType == PT_uint32) || IsBittySampleType(pcChannel->eType))
		{
			if (!pcChannel->bReverse)
			{
				ulliRaw = 0;
				ConvertSampleFromFloat(&ulliRaw, pcChannel->eType, 0.0);
				for (uiFrame = 0; uiFrame < uiNumFrames; uiFrame++)
				{
					mcChannels.SetNative(uiFrame, pcChannel, &ulliRaw);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Samples can be read and written when the channel is one of the sample
// types and isn't reversed.  It needn't be byte aligned.
//
//////////////////////////////////////////////////////////////////////////
CChannel* CSound::GetSampleChannel(size uiChannelIndex, lsize uiFirstFrame, lsize uiNumFrames)
{
	CChannel*	pcChannel;

	if (IsChanging())
	{
		gcLogger.Error2(__METHOD__, " Cannot access samples while the sound is changing.", NULL);
		return NULL;
	}
	if (uiChannelIndex >= NumChannels())
	{
		gcLogger.Error2(__METHOD__, " Channel index [", SizeToString(uiChannelIndex), "] is not less than the number of channels [", SizeToString(NumChannels()), "].", NULL);
		return NULL;
	}
	if ((uiFirstFrame > GetNumFrames()) || (uiNumFrames > GetNumFrames() - uiFirstFrame))
	{
		gcLogger.Error2(__METHOD__, " Frames [", LongToString((uint64)uiFirstFrame), "] to [", LongToString((uint64)(uiFirstFrame + uiNumFrames)), "] are past the end of the sound [", LongToString((uint64)(GetNumFrames())), "].", NULL);
		return NULL;
	}

	pcChannel = GetChannelAtIndex(uiChannelIndex);
	if (!IsSampleType(pcChannel->eType) || pcChannel->bReverse)
	{
		gcLogger.Error2(__METHOD__, " Channel [", SizeToString(uiChannelIndex), "] of type [", gcTypeNames.GetPrettyName(pcChannel->eType), "] is not a sample.", NULL);
		return NULL;
	}
	return pcChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CSound::GetSample(lsize uiFrame, size uiChannelIndex)
{
	CChannel*	pcChannel;

	uint64		ulliRaw;

	pcChannel = GetSampleChannel(uiChannelIndex, uiFrame, 1);
	if (pcChannel == NULL)
	{
		return 0.0;
	}
	mcChannels.GetNative(uiFrame, pcChannel, &ulliRaw);
	return ConvertSampleToFloat(&ulliRaw, pcChannel->eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::SetSample(lsize uiFrame, size uiChannelIndex, float64 fSample)
{
	CChannel*	pcChannel;

	uint64		ulliRaw;

	pcChannel = GetSampleChannel(uiChannelIndex, uiFrame, 1);
	if (pcChannel != NULL)
	{
		ulliRaw = 0;
		ConvertSampleFromFloat(&ulliRaw, pcChannel->eType, fSample);
		mcChannels.SetNative(uiFrame, pcChannel, &ulliRaw);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::GetSamples(size uiChannelIndex, lsize uiFirstFrame, lsize uiNumFrames, float64* pfDest)
{
	CChannel*		pcChannel;
	uint8*			pui;
	size			uiStride;
	lsize			i;
	EPrimitiveType	eType;
	uint64			ulliRaw;

	pcChannel = GetSampleChannel(uiChannelIndex, uiFirstFrame, uiNumFrames);
	if (pcChannel == NULL)
	{
		return false;
	}

	eType = pcChannel->eType;
	if (pcChannel->Is8BitAligned() && (GetFrameByteStride() != 0))
	{
		uiStride = GetFrameByteStride();
		pui = (uint8*)GetData() + uiFirstFrame * uiStride + pcChannel->miByteOffset;
		for (i = 0; i < uiNumFrames; i++)
		{
			pfDest[i] = ConvertSampleToFloat(pui, eType);
			pui += uiStride;
		}
	}
	else
	{
		for (i = 0; i < uiNumFrames; i++)
		{
			mcChannels.GetNative(uiFirstFrame + i, pcChannel, &ulliRaw);
			pfDest[i] = ConvertSampleToFloat(&ulliRaw, eType);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::SetSamples(size uiChannelIndex, lsize uiFirstFrame, lsize uiNumFrames, float64* pfSource)
{
	CChannel*		pcChannel;
	uint8*			pui;
	size			uiStride;
	lsize			i;
	EPrimitiveType	eType;
	uint64			ulliRaw;

	pcChannel = GetSampleChannel(uiChannelIndex, uiFirstFrame, uiNumFrames);
	if (pcChannel == NULL)
	{
		return false;
	}

	eType = pcChannel->eType;
	if (pcChannel->Is8BitAligned() && (GetFrameByteStride() != 0))
	{
		uiStride = GetFrameByteStride();
		pui = (uint8*)GetData() + uiFirstFrame * uiStride + pcChannel->miByteOffset;
		for (i = 0; i < uiNumFrames; i++)
		{
			ConvertSampleFromFloat(pui, eType, pfSource[i]);
			pui += uiStride;
		}
	}
	else
	{
		for (i = 0; i < uiNumFrames; i++)
		{
			ulliRaw = 0;
			ConvertSampleFromFloat(&ulliRaw, eType, pfSource[i]);
			mcChannels.SetNative(uiFirstFrame + i, pcChannel, &ulliRaw);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::SetData(void* pvData)
{
	mcChannels.SetData(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSound::GetData(void)
{
	return mcChannels.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSound::GetByteSize(void)
{
	return mcChannels.GetByteSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannels* CSound::GetChannels(void)
{
	return &mcChannels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSound::GetFrameByteStride(void)
{
	return mcChannels.GetByteStride();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSound::GetFrameBitStride(void)
{
	return mcChannels.GetBitStride();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::IsSameFormat(Ptr<CSound> pcOther)
{
	return (muiSampleRate == pcOther->muiSampleRate) && mcChannels.IsSameFormat(&pcOther->mcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::IsValid(lsize uiFrame)
{
	return uiFrame < mcChannels.GetSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CSound::GetChannel(size iChannel)
{
	return mcChannels.GetChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CSound::GetChannelAtIndex(size iIndex)
{
	return mcChannels.GetChannelAtIndex(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSound::GetIndexOfChannel(size iChannel)
{
	return mcChannels.GetIndexOfChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::GetAllChannels(CArraySize* paiChannels)
{
	mcChannels.GetAllChannels(paiChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::GetAllChannels(CArrayChannel* pasChannels)
{
	mcChannels.GetAllChannels(pasChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CSound::GetPrimitiveType(void)
{
	return mcChannels.GetPrimitiveType();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSound::NumChannels(void)
{
	return mcChannels.NumChannels();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::HasChannel(size iChannel)
{
	return mcChannels.HasChannel(iChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSound::HasChannels(size iFirst, ...)
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
		eIC = VA_ARG_SIZE(vaMarker);
	}
	va_end(vaMarker);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// True if every channel's samples can be read and written as float64.
//
//////////////////////////////////////////////////////////////////////////
bool CSound::CanAccessSamples(void)
{
	size		uiNumChannels;
	size		uiIndex;
	CChannel*	pcChannel;

	if (IsChanging())
	{
		return false;
	}

	uiNumChannels = NumChannels();
	for (uiIndex = 0; uiIndex < uiNumChannels; uiIndex++)
	{
		pcChannel = GetChannelAtIndex(uiIndex);
		if (!IsSampleType(pcChannel->eType) || pcChannel->bReverse)
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
void CSound::Print(CChars* psz)
{
	size		uiNumChannels;
	size		uiIndex;
	lsize		uiFrame;
	lsize		uiNumFrames;
	CChannel*	pcChannel;
	bool		bSamples;

	psz->Append("Object[");
	PrintIdentifier(psz);
	psz->Append("]  ");
	psz->Append(muiSampleRate);
	psz->Append("Hz  ");
	psz->Append(GetNumFrames());
	psz->Append(" frames  Channels");

	uiNumChannels = NumChannels();
	for (uiIndex = 0; uiIndex < uiNumChannels; uiIndex++)
	{
		pcChannel = GetChannelAtIndex(uiIndex);
		psz->Append("[(");
		PrintPrimitiveType(pcChannel->eType, psz);
		psz->Append(")\"");
		PrintSoundChannel(pcChannel->iChannel, psz);
		psz->Append("\"]");
	}
	psz->AppendNewLine();

	bSamples = CanAccessSamples();
	uiNumFrames = GetNumFrames();
	for (uiFrame = 0; bSamples && (uiFrame < uiNumFrames); uiFrame++)
	{
		for (uiIndex = 0; uiIndex < uiNumChannels; uiIndex++)
		{
			//Whole numbers (the sample times 32768) because printing floats needs the number control initialised.
			psz->Append(" ");
			psz->Append((int32)floor(GetSample(uiFrame, uiIndex) * 32768.0 + 0.5));
		}
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSound::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

