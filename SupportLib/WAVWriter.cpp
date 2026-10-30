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
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/Logger.h"
#include "WAVCommon.h"
#include "WAVWriter.h"


//The frames converted at a time.
#define WAV_WRITE_BLOCK_FRAMES		4096


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EWAVSampleFormat WAVGetDefaultFormat(Ptr<CSound> pcSound)
{
	size		uiIndex;
	CChannel*	pcChannel;
	size		uiBits;
	bool		bFloat64;
	bool		bFloat32;

	//CChannels::GetLargestPrimitiveType skips channels that aren't whole bytes, so the bit sized types are counted here.
	uiBits = 0;
	bFloat32 = false;
	bFloat64 = false;
	for (uiIndex = 0; uiIndex < pcSound->NumChannels(); uiIndex++)
	{
		pcChannel = pcSound->GetChannelAtIndex(uiIndex);
		if (pcChannel->iChannel == SOUND_IGNORED)
		{
			continue;
		}
		bFloat32 |= pcChannel->eType == PT_float32;
		bFloat64 |= pcChannel->eType == PT_float64;
		if (pcChannel->miBitSize > uiBits)
		{
			uiBits = pcChannel->miBitSize;
		}
	}

	if (bFloat64)
	{
		return WSF_Float64;
	}
	if (bFloat32)
	{
		return WSF_Float32;
	}
	if (uiBits <= 8)
	{
		return WSF_PCM8;
	}
	if (uiBits <= 16)
	{
		return WSF_PCM16;
	}
	return WSF_PCM32;
}


//////////////////////////////////////////////////////////////////////////
//
// Speakers are sorted by their channel mask bit.  Mono, discrete and
// unknown channels follow in the order they are in the sound.
//
//////////////////////////////////////////////////////////////////////////
size WAVGetChannelOrder(Ptr<CSound> pcSound, size* puiOrder, uint32* puiMask)
{
	size		uiNumChannels;
	size		uiIndex;
	size		uiNumWritten;
	size		uiKey;
	size		i;
	size*		puiKeys;
	CChannel*	pcChannel;
	uint32		uiBit;

	uiNumChannels = pcSound->NumChannels();
	puiKeys = (size*)malloc((lsize)sizeof(size) * (uiNumChannels + 1));
	uiNumWritten = 0;
	*puiMask = 0;
	for (uiIndex = 0; uiIndex < uiNumChannels; uiIndex++)
	{
		pcChannel = pcSound->GetChannelAtIndex(uiIndex);
		if (pcChannel->iChannel == SOUND_IGNORED)
		{
			continue;
		}

		uiBit = GetSoundSpeakerMask(pcChannel->iChannel);
		if ((uiBit != 0) && ((*puiMask & uiBit) == 0))
		{
			*puiMask |= uiBit;
			uiKey = GetSoundSpeaker(pcChannel->iChannel);
		}
		else
		{
			uiKey = NUM_SOUND_SPEAKERS + uiIndex;
		}

		//Insertion sort.
		i = uiNumWritten;
		while ((i > 0) && (puiKeys[i - 1] > uiKey))
		{
			puiKeys[i] = puiKeys[i - 1];
			puiOrder[i] = puiOrder[i - 1];
			i--;
		}
		puiKeys[i] = uiKey;
		puiOrder[i] = uiIndex;
		uiNumWritten++;
	}
	free(puiKeys);
	return uiNumWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes one sample as little endian.
//
//////////////////////////////////////////////////////////////////////////
void WAVPutSample(uint8* puiDest, EWAVSampleFormat eFormat, float64 fSample)
{
	int32		i;
	uint32		ui;
	float32		f;
	float64		d;
	uint8		uiByte;
	int16		i16;

	switch (eFormat)
	{
	case WSF_PCM8:
		ConvertSampleFromFloat(&uiByte, PT_uint8, fSample);
		puiDest[0] = uiByte;
		break;
	case WSF_PCM16:
		ConvertSampleFromFloat(&i16, PT_int16, fSample);
		puiDest[0] = (uint8)i16;
		puiDest[1] = (uint8)(i16 >> 8);
		break;
	case WSF_PCM24:
		i = (int32)QuantiseSample(fSample, 8388608.0, -8388608.0, 8388607.0);
		puiDest[0] = (uint8)i;
		puiDest[1] = (uint8)(i >> 8);
		puiDest[2] = (uint8)(i >> 16);
		break;
	case WSF_PCM32:
		ConvertSampleFromFloat(&i, PT_int32, fSample);
		ui = (uint32)i;
		puiDest[0] = (uint8)ui;
		puiDest[1] = (uint8)(ui >> 8);
		puiDest[2] = (uint8)(ui >> 16);
		puiDest[3] = (uint8)(ui >> 24);
		break;
	case WSF_Float32:
		f = (float32)fSample;
		memcpy(puiDest, &f, 4);
		break;
	case WSF_Float64:
		d = fSample;
		memcpy(puiDest, &d, 8);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WAVWriteHeaders(CFileBasic* pcFile, Ptr<CSound> pcSound, size uiNumChannels, uint32 uiMask, EWAVSampleFormat eFormat, uint16 uiBitsPerSample, lsize uiDataSize)
{
	bool		bFloat;
	bool		bExtensible;
	uint16		uiBlockAlign;
	uint32		uiFmtSize;
	uint32		uiRIFFSize;
	bool		bResult;
	size		uiNumFrames;

	bFloat = (eFormat == WSF_Float32) || (eFormat == WSF_Float64);
	uiBlockAlign = (uint16)(uiNumChannels * (uiBitsPerSample / 8));
	uiNumFrames = pcSound->GetNumFrames();

	//The channel mask is needed when the channels aren't the default mono or stereo.
	bExtensible = (uiNumChannels > 2) || (uiBitsPerSample > 16) || bFloat || (uiMask != GetWAVDefaultChannelMask(uiNumChannels));
	uiFmtSize = bExtensible ? WAV_FMT_EXTENSIBLE_SIZE : WAV_FMT_SIZE;

	uiRIFFSize = 4 + (8 + uiFmtSize) + (8 + (uint32)uiDataSize + (uint32)(uiDataSize & 1));
	if (bFloat)
	{
		//Formats other than PCM need a fact chunk.
		uiRIFFSize += 8 + 4;
	}

	bResult =	pcFile->WriteData("RIFF", 4) &&
				pcFile->WriteLittleEndianInt32(uiRIFFSize) &&
				pcFile->WriteData("WAVE", 4) &&
				pcFile->WriteData("fmt ", 4) &&
				pcFile->WriteLittleEndianInt32(uiFmtSize) &&
				pcFile->WriteLittleEndianInt16((uint16)(bExtensible ? WAV_FORMAT_EXTENSIBLE : WAV_FORMAT_PCM)) &&
				pcFile->WriteLittleEndianInt16((uint16)uiNumChannels) &&
				pcFile->WriteLittleEndianInt32(pcSound->GetSampleRate()) &&
				pcFile->WriteLittleEndianInt32((uint32)(pcSound->GetSampleRate() * uiBlockAlign)) &&
				pcFile->WriteLittleEndianInt16(uiBlockAlign) &&
				pcFile->WriteLittleEndianInt16(uiBitsPerSample);

	if (bExtensible)
	{
		bResult =	bResult &&
					pcFile->WriteLittleEndianInt16((uint16)WAV_EXTENSIBLE_CB_SIZE) &&
					pcFile->WriteLittleEndianInt16(uiBitsPerSample) &&
					pcFile->WriteLittleEndianInt32(uiMask) &&
					pcFile->WriteLittleEndianInt16((uint16)(bFloat ? WAV_FORMAT_IEEE_FLOAT : WAV_FORMAT_PCM)) &&
					pcFile->WriteData(gauiWAVSubFormatGUID, 14);
	}

	if (bFloat)
	{
		bResult =	bResult &&
					pcFile->WriteData("fact", 4) &&
					pcFile->WriteLittleEndianInt32((uint32)4) &&
					pcFile->WriteLittleEndianInt32((uint32)uiNumFrames);
	}

	bResult =	bResult &&
				pcFile->WriteData("data", 4) &&
				pcFile->WriteLittleEndianInt32((uint32)uiDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveWAV(Ptr<CSound> pcSound, char* szPathName, EWAVSampleFormat eFormat)
{
	size*		puiOrder;
	size		uiNumChannels;
	uint32		uiMask;
	uint16		uiBitsPerSample;
	size		uiBytesPerSample;
	size		uiBlockAlign;
	size		uiNumFrames;
	uint64		ulliDataSize;
	lsize		uiDataSize;
	uint8*		puiData;
	float64*	pfBlock;
	size		uiFirst;
	size		uiCount;
	size		uiChannel;
	size		uiFrame;
	uint8*		puiDest;
	CFileBasic	cFile;
	bool		bResult;
	uint8		uiPad;

	if (pcSound.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Cannot write NULL sound [", szPathName, "].", NULL);
		return false;
	}
	if (!pcSound->CanAccessSamples() || (pcSound->GetSampleRate() == 0))
	{
		gcLogger.Error2(__METHOD__, " Sound samples cannot be read.  Cannot write [", szPathName, "].", NULL);
		return false;
	}

	puiOrder = (size*)malloc((lsize)sizeof(size) * (pcSound->NumChannels() + 1));
	uiNumChannels = WAVGetChannelOrder(pcSound, puiOrder, &uiMask);
	if ((uiNumChannels == 0) || (uiNumChannels > 0xFFFF))
	{
		free(puiOrder);
		gcLogger.Error2(__METHOD__, " Sound has no channels to write.  Cannot write [", szPathName, "].", NULL);
		return false;
	}

	if (eFormat == WSF_Default)
	{
		eFormat = WAVGetDefaultFormat(pcSound);
	}
	switch (eFormat)
	{
	case WSF_PCM8:
		uiBitsPerSample = 8;
		break;
	case WSF_PCM16:
		uiBitsPerSample = 16;
		break;
	case WSF_PCM24:
		uiBitsPerSample = 24;
		break;
	case WSF_Float64:
		uiBitsPerSample = 64;
		break;
	default:
		uiBitsPerSample = 32;
		break;
	}

	uiBytesPerSample = uiBitsPerSample / 8;
	uiBlockAlign = uiNumChannels * uiBytesPerSample;
	uiNumFrames = pcSound->GetNumFrames();
	ulliDataSize = (uint64)uiNumFrames * (uint64)uiBlockAlign;
	if ((uiBlockAlign > 0xFFFF) || (ulliDataSize > 0xFFFFFF00))
	{
		free(puiOrder);
		gcLogger.Error2(__METHOD__, " Sound is too big for a WAVE file.  Cannot write [", szPathName, "].", NULL);
		return false;
	}
	uiDataSize = (lsize)ulliDataSize;

	puiData = (uint8*)malloc(uiDataSize + 1);
	pfBlock = (float64*)malloc(sizeof(float64) * WAV_WRITE_BLOCK_FRAMES);
	for (uiFirst = 0; uiFirst < uiNumFrames; uiFirst += WAV_WRITE_BLOCK_FRAMES)
	{
		uiCount = uiNumFrames - uiFirst;
		if (uiCount > WAV_WRITE_BLOCK_FRAMES)
		{
			uiCount = WAV_WRITE_BLOCK_FRAMES;
		}
		for (uiChannel = 0; uiChannel < uiNumChannels; uiChannel++)
		{
			pcSound->GetSamples(puiOrder[uiChannel], uiFirst, uiCount, pfBlock);
			puiDest = &puiData[(lsize)uiFirst * uiBlockAlign + uiChannel * uiBytesPerSample];
			for (uiFrame = 0; uiFrame < uiCount; uiFrame++)
			{
				WAVPutSample(puiDest, eFormat, pfBlock[uiFrame]);
				puiDest += uiBlockAlign;
			}
		}
	}
	free(pfBlock);
	free(puiOrder);

	uiPad = 0;
	cFile.Init(DiskFile(szPathName));
	bResult = cFile.Open(EFM_Write_Create);
	if (bResult)
	{
		bResult = WAVWriteHeaders(&cFile, pcSound, uiNumChannels, uiMask, eFormat, uiBitsPerSample, uiDataSize) && cFile.WriteData(puiData, uiDataSize);
		if (bResult && (uiDataSize & 1))
		{
			bResult = cFile.WriteData(&uiPad, 1);
		}
		cFile.Close();
	}
	cFile.Kill();
	free(puiData);

	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not write [", szPathName, "].", NULL);
	}
	return bResult;
}

