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
#include "BaseLib/NaiveFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "StandardLib/Objects.h"
#include "WAVCommon.h"
#include "WAVReader.h"


//A frame can't be bigger than this (256 channels of 64 bits).
#define WAV_MAX_CHANNELS			256


struct SWAVInfo
{
	uint16	uiFormat;			//WAV_FORMAT_PCM or WAV_FORMAT_IEEE_FLOAT once the extensible sub format is read.
	uint16	uiNumChannels;
	uint32	uiSampleRate;
	uint16	uiBlockAlign;
	uint16	uiBitsPerSample;	//The container size.
	uint32	uiChannelMask;
	bool	bHasMask;
	uint8*	puiData;
	size	uiDataSize;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 WAVGetUInt16(uint8* pui)
{
	return (uint16)(pui[0] | (pui[1] << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 WAVGetUInt32(uint8* pui)
{
	return (uint32)pui[0] | ((uint32)pui[1] << 8) | ((uint32)pui[2] << 16) | ((uint32)pui[3] << 24);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the fmt chunk.  Returns false if the format can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool WAVReadFormat(uint8* puiChunk, size uiChunkSize, SWAVInfo* psInfo)
{
	uint16	uiCBSize;

	if (uiChunkSize < WAV_FMT_SIZE)
	{
		return false;
	}

	psInfo->uiFormat = WAVGetUInt16(&puiChunk[0]);
	psInfo->uiNumChannels = WAVGetUInt16(&puiChunk[2]);
	psInfo->uiSampleRate = WAVGetUInt32(&puiChunk[4]);
	psInfo->uiBlockAlign = WAVGetUInt16(&puiChunk[12]);
	psInfo->uiBitsPerSample = WAVGetUInt16(&puiChunk[14]);
	psInfo->uiChannelMask = 0;
	psInfo->bHasMask = false;

	if (psInfo->uiFormat == WAV_FORMAT_EXTENSIBLE)
	{
		if (uiChunkSize < WAV_FMT_EXTENSIBLE_SIZE)
		{
			return false;
		}
		uiCBSize = WAVGetUInt16(&puiChunk[16]);
		if (uiCBSize < WAV_EXTENSIBLE_CB_SIZE)
		{
			return false;
		}
		psInfo->uiChannelMask = WAVGetUInt32(&puiChunk[20]);
		psInfo->bHasMask = true;
		if (memcmp(&puiChunk[26], gauiWAVSubFormatGUID, 14) != 0)
		{
			return false;
		}
		psInfo->uiFormat = WAVGetUInt16(&puiChunk[24]);
	}

	if (psInfo->uiFormat == WAV_FORMAT_PCM)
	{
		if ((psInfo->uiBitsPerSample != 8) && (psInfo->uiBitsPerSample != 16) && (psInfo->uiBitsPerSample != 24) && (psInfo->uiBitsPerSample != 32))
		{
			return false;
		}
	}
	else if (psInfo->uiFormat == WAV_FORMAT_IEEE_FLOAT)
	{
		if ((psInfo->uiBitsPerSample != 32) && (psInfo->uiBitsPerSample != 64))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	if ((psInfo->uiNumChannels == 0) || (psInfo->uiNumChannels > WAV_MAX_CHANNELS) || (psInfo->uiSampleRate == 0))
	{
		return false;
	}
	if (psInfo->uiBlockAlign != psInfo->uiNumChannels * (psInfo->uiBitsPerSample / 8))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the fmt and data chunks.  Chunks are padded to an even size.
//
//////////////////////////////////////////////////////////////////////////
bool WAVReadInfo(uint8* puiFile, size uiFileSize, SWAVInfo* psInfo)
{
	size	uiPos;
	uint32	uiChunkSize;
	uint8*	puiChunk;
	bool	bFormat;
	bool	bData;

	if ((uiFileSize < 12) || (memcmp(puiFile, "RIFF", 4) != 0) || (memcmp(&puiFile[8], "WAVE", 4) != 0))
	{
		return false;
	}

	bFormat = false;
	bData = false;
	uiPos = 12;
	while (uiPos + 8 <= uiFileSize)
	{
		puiChunk = &puiFile[uiPos];
		uiChunkSize = WAVGetUInt32(&puiChunk[4]);
		uiPos += 8;

		if (memcmp(puiChunk, "data", 4) == 0)
		{
			psInfo->puiData = &puiFile[uiPos];
			if (uiChunkSize > uiFileSize - uiPos)
			{
				//Streamed files often have an unknown data size.  Everything to the end of the file is data.
				psInfo->uiDataSize = uiFileSize - uiPos;
				bData = true;
				break;
			}
			psInfo->uiDataSize = uiChunkSize;
			bData = true;
		}
		else if (memcmp(puiChunk, "fmt ", 4) == 0)
		{
			if (bFormat || (uiChunkSize > uiFileSize - uiPos) || !WAVReadFormat(&puiFile[uiPos], uiChunkSize, psInfo))
			{
				return false;
			}
			bFormat = true;
		}
		else if (uiChunkSize > uiFileSize - uiPos)
		{
			break;
		}

		uiPos += uiChunkSize;
		uiPos += uiChunkSize & 1;
	}
	return bFormat && bData;
}


//////////////////////////////////////////////////////////////////////////
//
// The channel for each sample in a frame.  Mask speakers are given in
// bit order; channels past the mask are discrete.
//
//////////////////////////////////////////////////////////////////////////
void WAVGetChannels(SWAVInfo* psInfo, EChannel* paeChannels)
{
	uint32	uiMask;
	size	uiBit;
	size	uiChannel;
	size	uiDiscrete;

	if (psInfo->bHasMask)
	{
		uiMask = psInfo->uiChannelMask;
	}
	else
	{
		uiMask = GetWAVDefaultChannelMask(psInfo->uiNumChannels);
	}

	if ((psInfo->uiNumChannels == 1) && (uiMask == 0))
	{
		paeChannels[0] = SOUND_MONO;
		return;
	}

	uiChannel = 0;
	for (uiBit = 0; (uiBit < 32) && (uiChannel < psInfo->uiNumChannels); uiBit++)
	{
		if ((uiMask & (1 << uiBit)) && (GetSoundSpeakerForMaskBit(uiBit) != CHANNEL_STOP))
		{
			paeChannels[uiChannel] = GetSoundSpeakerForMaskBit(uiBit);
			uiChannel++;
		}
	}

	uiDiscrete = 0;
	for (; uiChannel < psInfo->uiNumChannels; uiChannel++)
	{
		paeChannels[uiChannel] = SOUND_DISCRETE(uiDiscrete);
		uiDiscrete++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType WAVGetSampleType(SWAVInfo* psInfo)
{
	if (psInfo->uiFormat == WAV_FORMAT_IEEE_FLOAT)
	{
		return psInfo->uiBitsPerSample == 32 ? PT_float32 : PT_float64;
	}
	switch (psInfo->uiBitsPerSample)
	{
	case 8:
		return PT_uint8;
	case 16:
		return PT_int16;
	default:
		return PT_int32;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Copies one little endian sample from the file into the sound.
//
//////////////////////////////////////////////////////////////////////////
void WAVCopySample(uint8* puiSource, uint8* puiDest, uint16 uiBitsPerSample)
{
	uint32	ui;

	switch (uiBitsPerSample)
	{
	case 8:
		*puiDest = *puiSource;
		break;
	case 16:
		*((uint16*)puiDest) = WAVGetUInt16(puiSource);
		break;
	case 24:
		ui = ((uint32)puiSource[0] << 8) | ((uint32)puiSource[1] << 16) | ((uint32)puiSource[2] << 24);
		*((uint32*)puiDest) = ui;
		break;
	case 32:
		*((uint32*)puiDest) = WAVGetUInt32(puiSource);
		break;
	case 64:
		*((uint32*)puiDest) = WAVGetUInt32(puiSource);
		*((uint32*)&puiDest[4]) = WAVGetUInt32(&puiSource[4]);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> LoadWAV(void* pvFile, size uiFileSize)
{
	SWAVInfo		sInfo;
	EChannel		aeChannels[WAV_MAX_CHANNELS];
	EPrimitiveType	eType;
	Ptr<CSound>		pcSound;
	size			uiNumFrames;
	size			uiFrame;
	size			uiChannel;
	size			uiBytesPerSample;
	size			uiStride;
	size			auiOffsets[WAV_MAX_CHANNELS];
	uint8*			puiSource;
	uint8*			puiDest;

	if (!WAVReadInfo((uint8*)pvFile, uiFileSize, &sInfo))
	{
		return NULL;
	}

	WAVGetChannels(&sInfo, aeChannels);
	eType = WAVGetSampleType(&sInfo);
	uiNumFrames = sInfo.uiDataSize / sInfo.uiBlockAlign;

	pcSound = OMalloc<CSound>();
	pcSound->BeginChange();
	for (uiChannel = 0; uiChannel < sInfo.uiNumChannels; uiChannel++)
	{
		pcSound->AddChannel(aeChannels[uiChannel], eType);
	}
	pcSound->SetSampleRate(sInfo.uiSampleRate);
	pcSound->SetNumFrames(uiNumFrames);
	if (!pcSound->EndChange() && (uiNumFrames > 0))
	{
		return NULL;
	}

	for (uiChannel = 0; uiChannel < sInfo.uiNumChannels; uiChannel++)
	{
		auiOffsets[uiChannel] = pcSound->GetChannelAtIndex(uiChannel)->miByteOffset;
	}

	uiBytesPerSample = sInfo.uiBitsPerSample / 8;
	uiStride = pcSound->GetFrameByteStride();
	puiSource = sInfo.puiData;
	puiDest = (uint8*)pcSound->GetData();
	for (uiFrame = 0; uiFrame < uiNumFrames; uiFrame++)
	{
		for (uiChannel = 0; uiChannel < sInfo.uiNumChannels; uiChannel++)
		{
			WAVCopySample(puiSource, &puiDest[auiOffsets[uiChannel]], sInfo.uiBitsPerSample);
			puiSource += uiBytesPerSample;
		}
		puiDest += uiStride;
	}
	return pcSound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> LoadWAV(char* szFilename)
{
	CNaiveFile		cNaiveFile;
	Ptr<CSound>		pcSound;

	cNaiveFile.Init();
	if (!cNaiveFile.Read(szFilename))
	{
		cNaiveFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read [", szFilename, "].", NULL);
		return NULL;
	}

	pcSound = LoadWAV(cNaiveFile.Get(), (size)cNaiveFile.Size());
	cNaiveFile.Kill();

	if (pcSound.IsNull())
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported WAVE file.", NULL);
	}
	return pcSound;
}

