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
#include "BaseLib/MemoryFile.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "MODReader.h"


#define MOD_SAMPLE_HEADER_SIZE		30
#define MOD_NOTE_SIZE				4
#define MOD_BANK_SAMPLE_RATE		8363	//The rate of an untuned sample played at C-2.  Each note plays it at its own rate.


//////////////////////////////////////////////////////////////////////////
//
// The number of channels for a 31 sample signature, zero if the
// signature isn't known.
//
//////////////////////////////////////////////////////////////////////////
size MODGetSignatureChannels(uint8* puiSignature)
{
	char	sz[5];

	memcpy(sz, puiSignature, 4);
	sz[4] = '\0';
	if ((StringCompare(sz, "M.K.") == 0) || (StringCompare(sz, "M!K!") == 0) || (StringCompare(sz, "M&K!") == 0) || (StringCompare(sz, "N.T.") == 0) || (StringCompare(sz, "FLT4") == 0))
	{
		return 4;
	}
	if ((StringCompare(sz, "CD81") == 0) || (StringCompare(sz, "OKTA") == 0))
	{
		return 8;
	}
	if ((sz[0] >= '1') && (sz[0] <= '9') && (StringCompare(&sz[1], "CHN") == 0))
	{
		return (size)(sz[0] - '0');
	}
	if ((sz[0] >= '1') && (sz[0] <= '3') && (sz[1] >= '0') && (sz[1] <= '9') && (sz[2] == 'C') && (sz[3] == 'H'))
	{
		return (size)((sz[0] - '0') * 10 + (sz[1] - '0'));
	}
	if ((sz[0] == 'T') && (sz[1] == 'D') && (sz[2] == 'Z') && (sz[3] >= '1') && (sz[3] <= '9'))
	{
		return (size)(sz[3] - '0');
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The sizes from the sample headers, in bytes.
//
//////////////////////////////////////////////////////////////////////////
struct SMODSampleHeader
{
	char	szName[23];
	uint8	uiFineTune;
	uint8	uiVolume;
	size	uiLength;
	size	uiLoopStart;
	size	uiLoopLength;
};


//////////////////////////////////////////////////////////////////////////
//
// Reads the 22 byte name, the length, finetune, volume and loop of a
// sample.  Lengths are in words in the file.
//
//////////////////////////////////////////////////////////////////////////
bool MODReadSampleHeader(CFileBasic* pcFile, SMODSampleHeader* psHeader)
{
	uint16	uiLength;
	uint16	uiLoopStart;
	uint16	uiLoopLength;

	if (!pcFile->ReadData(psHeader->szName, 22) || !pcFile->ReadBigEndianInt16(&uiLength) || !pcFile->ReadInt8(&psHeader->uiFineTune) ||
		!pcFile->ReadInt8(&psHeader->uiVolume) || !pcFile->ReadBigEndianInt16(&uiLoopStart) || !pcFile->ReadBigEndianInt16(&uiLoopLength))
	{
		return false;
	}
	psHeader->szName[22] = '\0';
	psHeader->uiLength = (size)uiLength * 2;
	psHeader->uiLoopStart = (size)uiLoopStart * 2;
	psHeader->uiLoopLength = (size)uiLoopLength * 2;

	//Every tracker writes the finetune in the low four bits and volumes up to 64.  Anything else is most likely not a module.
	return ((psHeader->uiFineTune & 0xF0) == 0) && (psHeader->uiVolume <= 64);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the patterns into pcSong.  A note is 4 bytes: sample high
// nibble and period high nibble, period low byte, sample low nibble and
// effect, parameter.
//
//////////////////////////////////////////////////////////////////////////
bool MODReadPatterns(CFileBasic* pcFile, Ptr<CModSong> pcSong)
{
	size		uiPattern;
	size		uiRow;
	size		uiChannel;
	uint8		auiNote[MOD_NOTE_SIZE];
	SModNote*	psNote;

	for (uiPattern = 0; uiPattern < pcSong->NumPatterns(); uiPattern++)
	{
		for (uiRow = 0; uiRow < MOD_ROWS_PER_PATTERN; uiRow++)
		{
			for (uiChannel = 0; uiChannel < pcSong->NumChannels(); uiChannel++)
			{
				if (!pcFile->ReadData(auiNote, MOD_NOTE_SIZE))
				{
					return false;
				}
				psNote = pcSong->GetNote(uiPattern, uiRow, uiChannel);
				psNote->uiSample = (uint8)((auiNote[0] & 0xF0) | (auiNote[2] >> 4));
				psNote->uiPeriod = (uint16)(((auiNote[0] & 0x0F) << 8) | auiNote[1]);
				psNote->uiEffect = (uint8)(auiNote[2] & 0x0F);
				psNote->uiParam = auiNote[3];
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts every sample in one signed 8 bit bank, a cel each.  Sample data
// cut off by the end of the file is left silent.
//
//////////////////////////////////////////////////////////////////////////
bool MODReadSamples(CFileBasic* pcFile, size uiFileSize, Ptr<CModSong> pcSong, SMODSampleHeader* pasHeaders)
{
	size					i;
	size					uiTotalFrames;
	size					uiBankFrame;
	size					uiAvailable;
	size					uiCelFrames;
	size					uiLoop;
	Ptr<CSound>				pcBank;
	Ptr<CSoundCelGroup>		pcGroup;
	SModSample*				psSample;
	SMODSampleHeader*		psHeader;
	uint8*					puiBank;

	uiTotalFrames = 0;
	for (i = 0; i < pcSong->NumSamples(); i++)
	{
		uiTotalFrames += pasHeaders[i].uiLength;
	}

	pcBank = OMalloc<CSound>(MOD_BANK_SAMPLE_RATE, uiTotalFrames, PT_int8, SOUND_MONO, CHANNEL_STOP);
	puiBank = (uint8*)pcBank->GetData();
	if ((uiTotalFrames > 0) && (puiBank == NULL))
	{
		return false;
	}
	pcBank->Silence();
	pcGroup = OMalloc<CSoundCelGroup>(pcBank);

	uiBankFrame = 0;
	for (i = 0; i < pcSong->NumSamples(); i++)
	{
		psHeader = &pasHeaders[i];
		psSample = pcSong->GetSample(i + 1);
		StrCpySafe(psSample->szName, psHeader->szName, sizeof(psSample->szName));
		psSample->iFineTune = (int8)((psHeader->uiFineTune & 0x08) ? (int)psHeader->uiFineTune - 16 : (int)psHeader->uiFineTune);
		psSample->uiVolume = psHeader->uiVolume;

		uiAvailable = (size)(uiFileSize - pcFile->Tell());
		pcFile->ReadData(&puiBank[uiBankFrame], uiAvailable < psHeader->uiLength ? uiAvailable : psHeader->uiLength);

		if (psHeader->uiLength > 0)
		{
			//A loop of one word (or none) means the sample plays once.  A loop past the end is cut at the end.
			uiCelFrames = psHeader->uiLength;
			uiLoop = SOUND_CEL_NO_LOOP;
			if ((psHeader->uiLoopLength > 2) && (psHeader->uiLoopStart < psHeader->uiLength))
			{
				uiLoop = psHeader->uiLoopStart;
				if (psHeader->uiLoopStart + psHeader->uiLoopLength < psHeader->uiLength)
				{
					uiCelFrames = psHeader->uiLoopStart + psHeader->uiLoopLength;
				}
			}
			psSample->uiCel = pcGroup->AddCel(uiBankFrame, uiCelFrames, uiLoop);
		}
		uiBankFrame += psHeader->uiLength;
	}

	pcSong->SetSampleBank(pcGroup);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CModSong> MODRead(CFileBasic* pcFile, size uiFileSize)
{
	uint8					auiSignature[4];
	size					uiNumSamples;
	size					uiNumChannels;
	size					uiOrdersPos;
	uint8					uiSongLength;
	uint8					uiRestart;
	uint8					auiOrders[MOD_MAX_ORDERS];
	size					uiNumPatterns;
	size					uiPatternsSize;
	size					i;
	char					szTitle[21];
	SMODSampleHeader		asHeaders[MOD_MAX_SAMPLES];
	Ptr<CModSong>			pcSong;

	uiNumChannels = 0;
	if ((uiFileSize >= 1084) && pcFile->Seek(1080) && pcFile->ReadData(auiSignature, 4))
	{
		uiNumChannels = MODGetSignatureChannels(auiSignature);
	}
	if (uiNumChannels != 0)
	{
		uiNumSamples = 31;
		uiOrdersPos = 950;
	}
	else
	{
		//Soundtracker modules have 15 samples, 4 channels and no signature.
		uiNumSamples = 15;
		uiNumChannels = 4;
		uiOrdersPos = 470;
	}
	if (uiNumChannels > MOD_MAX_CHANNELS)
	{
		return NULL;
	}

	if (!pcFile->Seek(0) || !pcFile->ReadData(szTitle, 20))
	{
		return NULL;
	}
	szTitle[20] = '\0';
	for (i = 0; i < uiNumSamples; i++)
	{
		if (!MODReadSampleHeader(pcFile, &asHeaders[i]))
		{
			return NULL;
		}
	}

	if (!pcFile->Seek(uiOrdersPos) || !pcFile->ReadInt8(&uiSongLength) || !pcFile->ReadInt8(&uiRestart) || !pcFile->ReadData(auiOrders, MOD_MAX_ORDERS))
	{
		return NULL;
	}
	if ((uiSongLength == 0) || (uiSongLength > MOD_MAX_ORDERS))
	{
		return NULL;
	}

	//Every order counts towards the number of patterns, not just those in the song, as ProTracker does.
	uiNumPatterns = 0;
	for (i = 0; i < MOD_MAX_ORDERS; i++)
	{
		if (auiOrders[i] >= uiNumPatterns)
		{
			uiNumPatterns = auiOrders[i] + 1;
		}
	}
	if ((uiNumSamples == 15) && (uiNumPatterns > 64))
	{
		return NULL;
	}

	uiPatternsSize = uiNumPatterns * MOD_ROWS_PER_PATTERN * uiNumChannels * MOD_NOTE_SIZE;
	if ((uiNumSamples == 31) && !pcFile->Seek(1084))
	{
		return NULL;
	}
	if ((size)pcFile->Tell() + uiPatternsSize > uiFileSize)
	{
		return NULL;
	}

	pcSong = OMalloc<CModSong>(uiNumChannels, uiNumSamples, uiNumPatterns);
	pcSong->SetTitle(szTitle);
	pcSong->SetSongLength(uiSongLength, uiRestart);
	for (i = 0; i < MOD_MAX_ORDERS; i++)
	{
		pcSong->SetOrder(i, auiOrders[i]);
	}

	if (!MODReadPatterns(pcFile, pcSong) || !MODReadSamples(pcFile, uiFileSize, pcSong, asHeaders))
	{
		return NULL;
	}
	return pcSong;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CModSong> LoadMOD(void* pvFile, size uiFileSize)
{
	CFileBasic		cFile;
	Ptr<CModSong>	pcSong;

	cFile.Init(MemoryFile(pvFile, uiFileSize));
	if (cFile.Open(EFM_Read))
	{
		pcSong = MODRead(&cFile, uiFileSize);
		cFile.Close();
	}
	cFile.Kill();
	return pcSong;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CModSong> LoadMOD(char* szFilename)
{
	CNaiveFile		cNaiveFile;
	Ptr<CModSong>	pcSong;

	cNaiveFile.Init();
	if (!cNaiveFile.Read(szFilename))
	{
		cNaiveFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read [", szFilename, "].", NULL);
		return NULL;
	}

	pcSong = LoadMOD(cNaiveFile.Get(), (size)cNaiveFile.Size());
	cNaiveFile.Kill();

	if (pcSong.IsNull())
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported module.", NULL);
	}
	return pcSong;
}

