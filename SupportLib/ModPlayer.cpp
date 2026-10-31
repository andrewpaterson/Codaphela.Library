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
#include <math.h>
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "ModPlayer.h"


#define MOD_NUM_NOTES			36
#define MOD_NUM_FINETUNES		16
#define MOD_RENDER_BLOCK		4096
#define MOD_MAX_SONG_MINUTES	30


//ProTracker's period table: C-1 to B-3 for each finetune, in the order ProTracker keeps them (0 to +7 then -8 to -1).  The
//values are ProTracker's own (as also used by Olav Sorensen's pt2-clone), not the nearest periods to equal temperament.
static const int16 gaaiModPeriods[MOD_NUM_FINETUNES][MOD_NUM_NOTES] =
{
	//Finetune 0.
	{ 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	  428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	  214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113 },
	//Finetune +1.
	{ 850, 802, 757, 715, 674, 637, 601, 567, 535, 505, 477, 450,
	  425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 239, 225,
	  213, 201, 189, 179, 169, 159, 150, 142, 134, 126, 119, 113 },
	//Finetune +2.
	{ 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474, 447,
	  422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237, 224,
	  211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118, 112 },
	//Finetune +3.
	{ 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470, 444,
	  419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235, 222,
	  209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118, 111 },
	//Finetune +4.
	{ 832, 785, 741, 699, 660, 623, 588, 555, 524, 495, 467, 441,
	  416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233, 220,
	  208, 196, 185, 175, 165, 156, 147, 139, 131, 124, 117, 110 },
	//Finetune +5.
	{ 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463, 437,
	  413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232, 219,
	  206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116, 109 },
	//Finetune +6.
	{ 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460, 434,
	  410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230, 217,
	  205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115, 109 },
	//Finetune +7.
	{ 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457, 431,
	  407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228, 216,
	  204, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114, 108 },
	//Finetune -8.
	{ 907, 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480,
	  453, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,
	  226, 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120 },
	//Finetune -7.
	{ 900, 850, 802, 757, 715, 675, 636, 601, 567, 535, 505, 477,
	  450, 425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 238,
	  225, 212, 200, 189, 179, 169, 159, 150, 142, 134, 126, 119 },
	//Finetune -6.
	{ 894, 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474,
	  447, 422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237,
	  223, 211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118 },
	//Finetune -5.
	{ 887, 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470,
	  444, 419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235,
	  222, 209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118 },
	//Finetune -4.
	{ 881, 832, 785, 741, 699, 660, 623, 588, 555, 524, 494, 467,
	  441, 416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233,
	  220, 208, 196, 185, 175, 165, 156, 147, 139, 131, 123, 117 },
	//Finetune -3.
	{ 875, 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463,
	  437, 413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232,
	  219, 206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116 },
	//Finetune -2.
	{ 868, 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460,
	  434, 410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230,
	  217, 205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115 },
	//Finetune -1.
	{ 862, 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457,
	  431, 407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228,
	  216, 203, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114 },
};


//ProTracker's vibrato and tremolo sine: half a cycle.
static const uint8 gauiModSine[32] =
{
	0x00, 0x18, 0x31, 0x4A, 0x61, 0x78, 0x8D, 0xA1,
	0xB4, 0xC5, 0xD4, 0xE0, 0xEB, 0xF4, 0xFA, 0xFD,
	0xFF, 0xFD, 0xFA, 0xF4, 0xEB, 0xE0, 0xD4, 0xC5,
	0xB4, 0xA1, 0x8D, 0x78, 0x61, 0x4A, 0x31, 0x18
};


//////////////////////////////////////////////////////////////////////////
//
// The row of the period table for a finetune of -8 to 7.
//
//////////////////////////////////////////////////////////////////////////
const int16* ModGetPeriods(int iFineTune)
{
	return gaaiModPeriods[iFineTune & 0x0F];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int ModClampVolume(int iVolume)
{
	return iVolume < 0 ? 0 : (iVolume > 64 ? 64 : iVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// ProTracker's slides only stop in the direction they move: up (a
// smaller period) at B-3 and down at C-1.
//
//////////////////////////////////////////////////////////////////////////
int ModSlidePeriodUp(int iPeriod, int iAmount)
{
	iPeriod -= iAmount;
	return iPeriod < MOD_MIN_PERIOD ? MOD_MIN_PERIOD : iPeriod;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int ModSlidePeriodDown(int iPeriod, int iAmount)
{
	iPeriod += iAmount;
	return iPeriod > MOD_MAX_PERIOD ? MOD_MAX_PERIOD : iPeriod;
}


//////////////////////////////////////////////////////////////////////////
//
// Vibrato and tremolo keep their last speed (high nibble) and depth (low
// nibble); a zero nibble keeps the one before.
//
//////////////////////////////////////////////////////////////////////////
void ModSetSpeedAndDepth(uint8 uiParam, int* piSpeed, int* piDepth)
{
	if ((uiParam >> 4) != 0)
	{
		*piSpeed = uiParam >> 4;
	}
	if ((uiParam & 0x0F) != 0)
	{
		*piDepth = uiParam & 0x0F;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The note of a period as ProTracker finds it: the first note in the
// finetune 0 row at or below the period.  -1 for periods outside the
// table (higher or lower octaves from other trackers).
//
//////////////////////////////////////////////////////////////////////////
int ModGetNote(int iPeriod)
{
	size	i;

	if ((iPeriod < MOD_MIN_PERIOD) || (iPeriod > gaaiModPeriods[8][0]))
	{
		return -1;
	}
	for (i = 0; i < MOD_NUM_NOTES; i++)
	{
		if (iPeriod >= gaaiModPeriods[0][i])
		{
			return i;
		}
	}
	return MOD_NUM_NOTES - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The period a note plays at with a finetune.  As in ProTracker a period
// in the table's range is moved onto the table, even with no finetune.
// Periods outside it (from trackers with more octaves) are scaled.
//
//////////////////////////////////////////////////////////////////////////
int CModPlayer::GetFineTunedPeriod(int iPeriod, int iFineTune)
{
	int		iNote;

	iNote = ModGetNote(iPeriod);
	if (iNote < 0)
	{
		if (iFineTune == 0)
		{
			return iPeriod;
		}
		return (int)floor(iPeriod * pow(2.0, -iFineTune / 96.0) + 0.5);
	}
	return ModGetPeriods(iFineTune)[iNote];
}


//////////////////////////////////////////////////////////////////////////
//
// As ProTracker: the first note in the finetune's row at or below the
// period, moved up iSemitones.
//
//////////////////////////////////////////////////////////////////////////
int CModPlayer::GetArpeggioPeriod(int iPeriod, int iFineTune, int iSemitones)
{
	const int16*	paiPeriods;
	size			i;
	int				iNote;

	paiPeriods = ModGetPeriods(iFineTune);
	for (i = 0; i < MOD_NUM_NOTES; i++)
	{
		if (iPeriod >= paiPeriods[i])
		{
			iNote = (int)i + iSemitones;
			if (iNote >= MOD_NUM_NOTES)
			{
				iNote = MOD_NUM_NOTES - 1;
			}
			return paiPeriods[iNote];
		}
	}
	return iPeriod;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::Init(Ptr<CModSong> pcSong, uint32 uiSampleRate, EModInterpolation eInterpolation, float64 fStereoSeparation)
{
	size			uiChannel;
	size			uiNumChannels;
	Ptr<CSound>		pcBank;
	lsize			uiBankFrames;
	lsize			uiFrame;
	float64			fSample;
	CChannel*		pcChannel;
	SModChannel*	psChannel;

	mpSong = pcSong;
	muiSampleRate = uiSampleRate;
	meInterpolation = eInterpolation;
	mfStereoSeparation = fStereoSeparation;

	//Half the channels are on each side, so this keeps each side in -1 to 1.
	uiNumChannels = pcSong->NumChannels();
	mfGain = 2.0 / (float64)(uiNumChannels < 2 ? 2 : uiNumChannels);

	//A MOD's 8 bit mono samples are mixed from the bank itself; other banks are copied as 16 bit samples.
	mpiBank8 = NULL;
	mpiBank16 = NULL;
	if (pcSong->GetSampleBank().IsNotNull())
	{
		pcBank = pcSong->GetSampleBank()->GetSound();
		uiBankFrames = pcBank->GetNumFrames();
		pcChannel = pcBank->GetChannelAtIndex(0);
		if ((pcChannel != NULL) && (pcChannel->eType == PT_int8) && pcChannel->Is8BitAligned() && (pcBank->GetFrameByteStride() == 1))
		{
			mpiBank8 = (int8*)pcBank->GetData() + pcChannel->miByteOffset;
		}
		else
		{
			mpiBank16 = (int16*)malloc((lsize)sizeof(int16) * (uiBankFrames + 1));
			for (uiFrame = 0; uiFrame < uiBankFrames; uiFrame++)
			{
				fSample = pcBank->GetSample(uiFrame, 0) * 32768.0;
				fSample = (fSample > 32767.0) ? 32767.0 : ((fSample < -32768.0) ? -32768.0 : fSample);
				mpiBank16[uiFrame] = (int16)fSample;
			}
		}
	}
	mpsMix = (SModMix*)malloc(sizeof(SModMix));
	mpsGainTables = (SModGainTable*)malloc((lsize)sizeof(SModGainTable) * 2 * (uiNumChannels > 0 ? uiNumChannels : 1));
	muiStepNumerator = (uint32)(MOD_PAULA_CLOCK_PAL / 2.0 * 65536.0 / (float64)muiSampleRate + 0.5);

	memset(masChannels, 0, sizeof(masChannels));
	for (uiChannel = 0; uiChannel < MOD_MAX_CHANNELS; uiChannel++)
	{
		psChannel = &masChannels[uiChannel];
		psChannel->uiLoopFrame = SOUND_CEL_NO_LOOP;
		psChannel->fPan = ((uiChannel % 4 == 0) || (uiChannel % 4 == 3)) ? -1.0 : 1.0;
		psChannel->fMixPan = 2.0;
		psChannel->iMixVolume = -1;
		psChannel->iTableLeft = -1;
		psChannel->iTableRight = -1;
	}

	muiOrder = 0;
	muiRow = 0;
	muiTick = 0;
	muiSpeed = MOD_DEFAULT_SPEED;
	muiTempo = MOD_DEFAULT_TEMPO;
	muiPatternDelay = 0;
	mbBreak = false;
	muiBreakRow = 0;
	mbPositionJump = false;
	muiJumpOrder = 0;
	mbLoopJump = false;
	muiLoopRow = 0;
	mbFinished = pcSong->GetSongLength() == 0;
	mbStarted = false;
	mpuiPlayedRows = (uint8*)malloc(MOD_MAX_ORDERS * MOD_ROWS_PER_PATTERN);
	memset(mpuiPlayedRows, 0, MOD_MAX_ORDERS * MOD_ROWS_PER_PATTERN);

	mfFramesPerTick = (float64)muiSampleRate * 2.5 / (float64)muiTempo;
	muiTickFrames = 0;
	mfFrameRemainder = 0.0;
	muiFramesPlayed = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::Kill(void)
{
	SafeFree(mpiBank16);
	SafeFree(mpsMix);
	SafeFree(mpsGainTables);
	mpiBank8 = NULL;
	SafeFree(mpuiPlayedRows);
	mpSong = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes uiNumFrames (at most MOD_MIX_BLOCK) stereo frames into the mix,
// in units of MOD_MIX_ONE (16 bits).  Returns the frames mixed, fewer if the song ended.
//
//////////////////////////////////////////////////////////////////////////
lsize CModPlayer::RenderMix(SModMix* psMix, lsize uiNumFrames)
{
	lsize	uiWritten;
	lsize	uiCount;

	uiWritten = 0;
	while (uiWritten < uiNumFrames)
	{
		if (muiTickFrames == 0)
		{
			StartTick();
			if (mbFinished)
			{
				break;
			}
			if (muiTickFrames == 0)
			{
				continue;
			}
		}

		uiCount = muiTickFrames;
		if (uiCount > uiNumFrames - uiWritten)
		{
			uiCount = uiNumFrames - uiWritten;
		}
		Mix(psMix, (uint16)uiWritten, (uint16)uiCount);
		muiTickFrames -= uiCount;
		uiWritten += uiCount;
	}
	muiFramesPlayed += uiWritten;
	return uiWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes uiNumFrames stereo frames (left, right) into pfStereo, 1.0 full
// scale.  Returns the frames mixed, fewer if the song ended.
//
//////////////////////////////////////////////////////////////////////////
lsize CModPlayer::Render(float64* pfStereo, lsize uiNumFrames)
{
	lsize	uiWritten;
	lsize	uiCount;
	lsize	uiMixed;
	lsize	i;

	uiWritten = 0;
	while (uiWritten < uiNumFrames)
	{
		uiCount = (uiNumFrames - uiWritten < MOD_MIX_BLOCK) ? (uiNumFrames - uiWritten) : MOD_MIX_BLOCK;
		uiMixed = RenderMix(mpsMix, uiCount);
		for (i = 0; i < uiMixed * 2; i++)
		{
			pfStereo[uiWritten * 2 + i] = (float64)((i & 1) ? mpsMix->sRight.aiFrames[i / 2] : mpsMix->sLeft.aiFrames[i / 2]) / (float64)MOD_MIX_ONE;
		}
		uiWritten += uiMixed;
		if (uiMixed < uiCount)
		{
			break;
		}
	}
	return uiWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes uiNumFrames stereo frames (left, right) into piStereo, 16 bit
// full scale (clipped).  Returns the frames mixed, fewer if the song
// ended.
//
//////////////////////////////////////////////////////////////////////////
lsize CModPlayer::Render(int16* piStereo, lsize uiNumFrames)
{
	lsize	uiWritten;
	lsize	uiCount;
	lsize	uiMixed;
	lsize	i;
	int32	iSample;

	uiWritten = 0;
	while (uiWritten < uiNumFrames)
	{
		uiCount = (uiNumFrames - uiWritten < MOD_MIX_BLOCK) ? (uiNumFrames - uiWritten) : MOD_MIX_BLOCK;
		uiMixed = RenderMix(mpsMix, uiCount);
		for (i = 0; i < uiMixed * 2; i++)
		{
			iSample = (int32)((i & 1) ? mpsMix->sRight.aiFrames[i / 2] : mpsMix->sLeft.aiFrames[i / 2]) * 2;
			piStereo[uiWritten * 2 + i] = (int16)((iSample > 32767) ? 32767 : ((iSample < -32768) ? -32768 : iSample));
		}
		uiWritten += uiMixed;
		if (uiMixed < uiCount)
		{
			break;
		}
	}
	return uiWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes uiNumFrames (at most MOD_MIX_BLOCK) frames as the HCP65's sound
// board plays them: unsigned 8 bit samples ($80 the middle, clipped),
// left and right apart.  The same as Render's 16 bit samples' high bytes
// + $80.  Returns the frames mixed, fewer if the song ended.
//
//////////////////////////////////////////////////////////////////////////
lsize CModPlayer::Render8(SModUnsigned8* psOut, lsize uiNumFrames)
{
	lsize	uiMixed;

	if (uiNumFrames > MOD_MIX_BLOCK)
	{
		uiNumFrames = MOD_MIX_BLOCK;
	}
	uiMixed = RenderMix(mpsMix, uiNumFrames);
	ModMixToUnsigned8(psOut, mpsMix, (uint16)uiMixed);
	return uiMixed;
}


//////////////////////////////////////////////////////////////////////////
//
// Plays the whole song (or uiMaxFrames, zero for up to 30 minutes) into a
// stereo sound.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> CModPlayer::RenderSong(EPrimitiveType eType, lsize uiMaxFrames)
{
	float64*	pfStereo;
	lsize		uiAllocated;
	lsize		uiFrames;
	lsize		uiCount;
	size		uiRequest;
	float64*	pfChannel;
	lsize		i;
	size		c;
	Ptr<CSound>	pcSound;

	if (uiMaxFrames == 0)
	{
		uiMaxFrames = (lsize)muiSampleRate * 60 * MOD_MAX_SONG_MINUTES;
	}

	uiAllocated = MOD_RENDER_BLOCK;
	pfStereo = (float64*)malloc((lsize)sizeof(float64) * 2 * uiAllocated);
	uiFrames = 0;
	while ((uiFrames < uiMaxFrames) && !mbFinished)
	{
		if (uiFrames + MOD_RENDER_BLOCK > uiAllocated)
		{
			uiAllocated *= 2;
			pfStereo = (float64*)realloc(pfStereo, (lsize)sizeof(float64) * 2 * uiAllocated);
		}
		uiRequest = uiMaxFrames - uiFrames < MOD_RENDER_BLOCK ? (size)(uiMaxFrames - uiFrames) : MOD_RENDER_BLOCK;
		uiCount = Render(&pfStereo[uiFrames * 2], uiRequest);
		uiFrames += uiCount;
		if (uiCount < uiRequest)
		{
			break;
		}
	}

	pcSound = OMalloc<CSound>(muiSampleRate, uiFrames, eType, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	pfChannel = (float64*)malloc((lsize)sizeof(float64) * (uiFrames + 1));
	for (c = 0; c < 2; c++)
	{
		for (i = 0; i < uiFrames; i++)
		{
			pfChannel[i] = pfStereo[i * 2 + c];
		}
		pcSound->SetSamples(c, 0, uiFrames, pfChannel);
	}
	free(pfChannel);
	free(pfStereo);
	return pcSound;
}


//////////////////////////////////////////////////////////////////////////
//
// Plays the next tick without mixing it.  Returns its frames, zero if the
// song has ended.
//
//////////////////////////////////////////////////////////////////////////
lsize CModPlayer::SkipTick(void)
{
	lsize	uiFrames;

	StartTick();
	if (mbFinished)
	{
		return 0;
	}
	uiFrames = muiTickFrames;
	muiTickFrames = 0;
	muiFramesPlayed += uiFrames;
	return uiFrames;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves on to the next tick (unless this is the first) and plays it:
// the notes and tick 0 effects of a new row, otherwise the other ticks'
// effects.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::StartTick(void)
{
	float64	fFrames;
	float64	fTickFrames;

	if (mbFinished)
	{
		return;
	}

	if (mbStarted)
	{
		muiTick++;
		if (muiTick >= muiSpeed * (1 + muiPatternDelay))
		{
			NextRow();
		}
	}
	mbStarted = true;

	if (muiTick == 0)
	{
		ProcessRow();
		if (mbFinished)
		{
			return;
		}
	}
	else
	{
		ProcessTickEffects();
	}

	fFrames = mfFramesPerTick + mfFrameRemainder;
	fTickFrames = floor(fFrames);
	muiTickFrames = (lsize)fTickFrames;
	mfFrameRemainder = fFrames - fTickFrames;
}


//////////////////////////////////////////////////////////////////////////
//
// Goes to the row after the current one, or to where a pattern loop,
// break or position jump says.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::NextRow(void)
{
	size	uiOrder;
	size	uiRow;

	uiOrder = muiOrder;
	uiRow = muiRow;
	if (mbLoopJump)
	{
		uiRow = muiLoopRow;
	}
	else if (mbPositionJump || mbBreak)
	{
		uiOrder = mbPositionJump ? muiJumpOrder : muiOrder + 1;
		uiRow = mbBreak ? muiBreakRow : 0;
	}
	else
	{
		uiRow++;
		if (uiRow >= MOD_ROWS_PER_PATTERN)
		{
			uiRow = 0;
			uiOrder++;
		}
	}

	if (uiOrder >= mpSong->GetSongLength())
	{
		uiOrder = mpSong->GetRestart();
	}

	muiOrder = uiOrder;
	muiRow = uiRow;
	muiTick = 0;
	muiPatternDelay = 0;
	mbLoopJump = false;
	mbPositionJump = false;
	mbBreak = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Tick 0 of a row: notes and the effects that happen once a row.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::ProcessRow(void)
{
	size			uiChannel;
	size			uiPattern;
	SModNote*		psNote;
	SModChannel*	psChannel;
	lsize			uiPlayed;
	int				iX;
	int				iY;
	size			uiRow;

	uiPlayed = muiOrder * MOD_ROWS_PER_PATTERN + muiRow;
	uiPattern = mpSong->GetOrder(muiOrder);
	if (mpuiPlayedRows[uiPlayed] || (uiPattern >= mpSong->NumPatterns()))
	{
		mbFinished = true;
		return;
	}
	mpuiPlayedRows[uiPlayed] = 1;

	for (uiChannel = 0; uiChannel < mpSong->NumChannels(); uiChannel++)
	{
		psChannel = &masChannels[uiChannel];
		psNote = mpSong->GetNote(uiPattern, muiRow, uiChannel);
		psChannel->uiEffect = psNote->uiEffect;
		psChannel->uiParam = psNote->uiParam;
		iX = psNote->uiParam >> 4;
		iY = psNote->uiParam & 0x0F;

		//A note delay sets the sample, volume and period now but only starts the sample on its tick, as ProTracker does.
		psChannel->sDelayedNote = *psNote;
		PlayNote(psChannel, psNote, !((psNote->uiEffect == 0xE) && (iX == 0xD) && (iY != 0)));

		switch (psNote->uiEffect)
		{
		case 0x3:
			if (psNote->uiParam != 0)
			{
				psChannel->iPortaSpeed = psNote->uiParam;
			}
			break;
		case 0x4:
			ModSetSpeedAndDepth(psNote->uiParam, &psChannel->iVibratoSpeed, &psChannel->iVibratoDepth);
			break;
		case 0x7:
			ModSetSpeedAndDepth(psNote->uiParam, &psChannel->iTremoloSpeed, &psChannel->iTremoloDepth);
			break;
		case 0x8:
			psChannel->fPan = ((float64)psNote->uiParam - 128.0) / 128.0;
			break;
		case 0xB:
			mbPositionJump = true;
			muiJumpOrder = psNote->uiParam;
			break;
		case 0xC:
			psChannel->iVolume = ModClampVolume(psNote->uiParam);
			break;
		case 0xD:
			uiRow = (size)(iX * 10 + iY);
			mbBreak = true;
			muiBreakRow = uiRow < MOD_ROWS_PER_PATTERN ? uiRow : 0;
			break;
		case 0xE:
			ExtendedTickZero(psChannel, iX, iY);
			if ((iX == 0x9) && (iY != 0) && (psNote->uiPeriod == 0))
			{
				StartSample(psChannel, 0);
			}
			break;
		case 0xF:
			if (psNote->uiParam == 0)
			{
				mbFinished = true;
				return;
			}
			else if (psNote->uiParam < 32)
			{
				muiSpeed = psNote->uiParam;
			}
			else
			{
				muiTempo = psNote->uiParam;
				mfFramesPerTick = (float64)muiSampleRate * 2.5 / (float64)muiTempo;
			}
			break;
		}

		psChannel->iOutPeriod = psChannel->iPeriod;
		psChannel->iOutVolume = psChannel->iVolume;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::ExtendedTickZero(SModChannel* psChannel, int iCommand, int iValue)
{
	size	uiRow;

	switch (iCommand)
	{
	case 0x1:
		psChannel->iPeriod = ModSlidePeriodUp(psChannel->iPeriod, iValue);
		break;
	case 0x2:
		psChannel->iPeriod = ModSlidePeriodDown(psChannel->iPeriod, iValue);
		break;
	case 0x4:
		psChannel->iVibratoWave = iValue;
		break;
	case 0x5:
		psChannel->iFineTune = (iValue & 0x08) ? iValue - 16 : iValue;
		break;
	case 0x6:
		if (iValue == 0)
		{
			psChannel->iLoopRow = (int)muiRow;
		}
		else
		{
			if (psChannel->iLoopCount == 0)
			{
				psChannel->iLoopCount = iValue;
			}
			else
			{
				psChannel->iLoopCount--;
			}

			if (psChannel->iLoopCount != 0)
			{
				//The rows of the loop are played again so they don't end the song.
				mbLoopJump = true;
				muiLoopRow = (size)psChannel->iLoopRow;
				for (uiRow = muiLoopRow; uiRow <= muiRow; uiRow++)
				{
					mpuiPlayedRows[muiOrder * MOD_ROWS_PER_PATTERN + uiRow] = 0;
				}
			}
		}
		break;
	case 0x7:
		psChannel->iTremoloWave = iValue;
		break;
	case 0x8:
		psChannel->fPan = ((float64)iValue - 7.5) / 7.5;
		break;
	case 0xA:
		psChannel->iVolume = ModClampVolume(psChannel->iVolume + iValue);
		break;
	case 0xB:
		psChannel->iVolume = ModClampVolume(psChannel->iVolume - iValue);
		break;
	case 0xC:
		if (iValue == 0)
		{
			psChannel->iVolume = 0;
		}
		break;
	case 0xE:
		if (muiPatternDelay == 0)
		{
			muiPatternDelay = (size)iValue;
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A sample number sets the sample (for the next note), its volume and
// finetune.  A period starts the note, unless it is the target of a tone
// portamento or bStart is false (a note delay).
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::PlayNote(SModChannel* psChannel, SModNote* psNote, bool bStart)
{
	int		iX;
	int		iY;
	int		iPeriod;

	iX = psNote->uiParam >> 4;
	iY = psNote->uiParam & 0x0F;

	if (psNote->uiSample != 0)
	{
		SetSample(psChannel, psNote->uiSample);
	}

	if (psNote->uiPeriod != 0)
	{
		if ((psNote->uiEffect == 0xE) && (iX == 0x5))
		{
			psChannel->iFineTune = (iY & 0x08) ? iY - 16 : iY;
		}
		iPeriod = GetFineTunedPeriod(psNote->uiPeriod, psChannel->iFineTune);

		if ((psNote->uiEffect == 0x3) || (psNote->uiEffect == 0x5))
		{
			psChannel->iPortaTarget = iPeriod;
		}
		else if (!bStart)
		{
			psChannel->iPeriod = iPeriod;
		}
		else
		{
			psChannel->iPeriod = iPeriod;
			if (psNote->uiEffect == 0x9)
			{
				if (psNote->uiParam != 0)
				{
					psChannel->uiSampleOffset = (size)psNote->uiParam * 256;
				}
				StartSample(psChannel, psChannel->uiSampleOffset);
			}
			else
			{
				StartSample(psChannel, 0);
			}

			if ((psChannel->iVibratoWave & 0x04) == 0)
			{
				psChannel->iVibratoPosition = 0;
			}
			if ((psChannel->iTremoloWave & 0x04) == 0)
			{
				psChannel->iTremoloPosition = 0;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::SetSample(SModChannel* psChannel, size uiSample)
{
	SModSample*		psSample;

	psSample = mpSong->GetSample(uiSample);
	if (psSample == NULL)
	{
		return;
	}
	psChannel->uiSample = uiSample;
	psChannel->iVolume = ModClampVolume(psSample->uiVolume);
	psChannel->iFineTune = psSample->iFineTune;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts the channel's sample uiOffset frames in.  An offset past the
// end plays the loop, or nothing.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::StartSample(SModChannel* psChannel, lsize uiOffset)
{
	Ptr<CSoundCel>	pcCel;

	pcCel = mpSong->GetSampleCel(psChannel->uiSample);
	if (pcCel.IsNull() || ((mpiBank8 == NULL) && (mpiBank16 == NULL)) || (pcCel->GetNumFrames() == 0))
	{
		psChannel->bPlaying = false;
		psChannel->bHasData = false;
		return;
	}

	psChannel->bHasData = true;
	psChannel->uiFirstFrame = pcCel->GetFirstFrame();
	psChannel->uiLength = pcCel->GetNumFrames();
	psChannel->uiLoopFrame = pcCel->GetLoopFrame();
	psChannel->bPlaying = true;
	psChannel->uiFraction = 0;
	if (uiOffset < psChannel->uiLength)
	{
		psChannel->uiPosition = uiOffset;
	}
	else if (psChannel->uiLoopFrame != SOUND_CEL_NO_LOOP)
	{
		psChannel->uiPosition = psChannel->uiLoopFrame;
	}
	else
	{
		psChannel->bPlaying = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ticks after the first of a row.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::ProcessTickEffects(void)
{
	size			uiChannel;
	SModChannel*	psChannel;
	int				iX;
	int				iY;
	int				iTick;
	int				iSemitones;

	//Pattern delay repeats the row's ticks; the first tick of each repeat plays no notes.
	iTick = (int)(muiTick % muiSpeed);

	for (uiChannel = 0; uiChannel < mpSong->NumChannels(); uiChannel++)
	{
		psChannel = &masChannels[uiChannel];
		iX = psChannel->uiParam >> 4;
		iY = psChannel->uiParam & 0x0F;
		psChannel->iOutPeriod = psChannel->iPeriod;
		psChannel->iOutVolume = psChannel->iVolume;
		if (iTick == 0)
		{
			continue;
		}

		switch (psChannel->uiEffect)
		{
		case 0x0:
			if (psChannel->uiParam != 0)
			{
				iSemitones = (iTick % 3 == 0) ? 0 : ((iTick % 3 == 1) ? iX : iY);
				psChannel->iOutPeriod = GetArpeggioPeriod(psChannel->iPeriod, psChannel->iFineTune, iSemitones);
			}
			break;
		case 0x1:
			psChannel->iPeriod = ModSlidePeriodUp(psChannel->iPeriod, psChannel->uiParam);
			psChannel->iOutPeriod = psChannel->iPeriod;
			break;
		case 0x2:
			psChannel->iPeriod = ModSlidePeriodDown(psChannel->iPeriod, psChannel->uiParam);
			psChannel->iOutPeriod = psChannel->iPeriod;
			break;
		case 0x3:
			TonePortamento(psChannel);
			break;
		case 0x4:
			Vibrato(psChannel);
			break;
		case 0x5:
			TonePortamento(psChannel);
			VolumeSlide(psChannel, psChannel->uiParam);
			break;
		case 0x6:
			Vibrato(psChannel);
			VolumeSlide(psChannel, psChannel->uiParam);
			break;
		case 0x7:
			Tremolo(psChannel);
			break;
		case 0xA:
			VolumeSlide(psChannel, psChannel->uiParam);
			break;
		case 0xE:
			if ((iX == 0x9) && (iY != 0) && (iTick % iY == 0))
			{
				StartSample(psChannel, 0);
			}
			else if ((iX == 0xC) && (iTick == iY))
			{
				psChannel->iVolume = 0;
			}
			else if ((iX == 0xD) && (iTick == iY) && (psChannel->sDelayedNote.uiPeriod != 0))
			{
				StartSample(psChannel, 0);
			}
			break;
		}

		if (psChannel->uiEffect != 0x7)
		{
			psChannel->iOutVolume = psChannel->iVolume;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::VolumeSlide(SModChannel* psChannel, int iParam)
{
	int		iX;
	int		iY;

	iX = iParam >> 4;
	iY = iParam & 0x0F;
	if (iX != 0)
	{
		psChannel->iVolume = ModClampVolume(psChannel->iVolume + iX);
	}
	else
	{
		psChannel->iVolume = ModClampVolume(psChannel->iVolume - iY);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::TonePortamento(SModChannel* psChannel)
{
	if (psChannel->iPortaTarget != 0)
	{
		if (psChannel->iPeriod < psChannel->iPortaTarget)
		{
			psChannel->iPeriod += psChannel->iPortaSpeed;
			if (psChannel->iPeriod > psChannel->iPortaTarget)
			{
				psChannel->iPeriod = psChannel->iPortaTarget;
			}
		}
		else if (psChannel->iPeriod > psChannel->iPortaTarget)
		{
			psChannel->iPeriod -= psChannel->iPortaSpeed;
			if (psChannel->iPeriod < psChannel->iPortaTarget)
			{
				psChannel->iPeriod = psChannel->iPortaTarget;
			}
		}
	}
	psChannel->iOutPeriod = psChannel->iPeriod;
}


//////////////////////////////////////////////////////////////////////////
//
// 0 to 255 for a position of 0 to 63; positions 32 to 63 are the
// negative half.
//
//////////////////////////////////////////////////////////////////////////
int CModPlayer::GetWave(int iWave, int iPosition)
{
	int		iValue;

	switch (iWave & 0x03)
	{
	case 1:
		//Ramp down.
		iValue = (iPosition & 0x1F) * 8;
		if (iPosition & 0x20)
		{
			iValue = 255 - iValue;
		}
		return iValue;
	case 2:
		return 255;
	default:
		//Sine, and random which ProTracker plays as sine.
		return gauiModSine[iPosition & 0x1F];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::Vibrato(SModChannel* psChannel)
{
	int		iDelta;

	iDelta = (GetWave(psChannel->iVibratoWave, psChannel->iVibratoPosition) * psChannel->iVibratoDepth) >> 7;
	if (psChannel->iVibratoPosition & 0x20)
	{
		iDelta = -iDelta;
	}
	psChannel->iOutPeriod = psChannel->iPeriod + iDelta;
	psChannel->iVibratoPosition = (psChannel->iVibratoPosition + psChannel->iVibratoSpeed) & 0x3F;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::Tremolo(SModChannel* psChannel)
{
	int		iDelta;

	iDelta = (GetWave(psChannel->iTremoloWave, psChannel->iTremoloPosition) * psChannel->iTremoloDepth) >> 6;
	if (psChannel->iTremoloPosition & 0x20)
	{
		iDelta = -iDelta;
	}
	psChannel->iOutVolume = ModClampVolume(psChannel->iVolume + iDelta);
	psChannel->iTremoloPosition = (psChannel->iTremoloPosition + psChannel->iTremoloSpeed) & 0x3F;
}


//////////////////////////////////////////////////////////////////////////
//
// The sample (Q8) uiFrame frames into the channel's cel, following the
// loop past the end.  Silence past the end of a sample that doesn't loop.
//
//////////////////////////////////////////////////////////////////////////
int32 CModPlayer::GetSampleAt(SModChannel* psChannel, lsize uiFrame)
{
	uiFrame = CSoundCel::GetLoopedFrame(uiFrame, psChannel->uiLength, psChannel->uiLoopFrame);
	if (uiFrame == SOUND_CEL_NO_LOOP)
	{
		return 0;
	}
	return GetBankSample(psChannel, uiFrame);
}


//////////////////////////////////////////////////////////////////////////
//
// The sample (Q8) uiFrame frames into the channel's cel (within it).
//
//////////////////////////////////////////////////////////////////////////
int32 CModPlayer::GetBankSample(SModChannel* psChannel, lsize uiFrame)
{
	uiFrame += psChannel->uiFirstFrame;
	if (mpiBank8)
	{
		return (int32)mpiBank8[uiFrame] * 256;
	}
	return mpiBank16[uiFrame];
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiNumFrames frames of each channel playing to piStereo.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::Mix(SModMix* psMix, uint16 uiFirstFrame, uint16 uiNumFrames)
{
	size			uiChannel;
	SModChannel*	psChannel;

	memset(&psMix->sLeft.aiFrames[uiFirstFrame], 0, sizeof(int16) * uiNumFrames);
	memset(&psMix->sRight.aiFrames[uiFirstFrame], 0, sizeof(int16) * uiNumFrames);
	for (uiChannel = 0; uiChannel < mpSong->NumChannels(); uiChannel++)
	{
		psChannel = &masChannels[uiChannel];
		if (!psChannel->bPlaying || !psChannel->bHasData || (psChannel->iOutPeriod <= 0) || (psChannel->iOutVolume == 0))
		{
			continue;
		}

		UpdateMix(psChannel);
		if (mpiBank8 && (meInterpolation == MI_Nearest))
		{
			MixNearest8(psChannel, psMix, uiFirstFrame, uiNumFrames);
		}
		else
		{
			MixSamples(psChannel, psMix, uiFirstFrame, uiNumFrames);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The channel's step and gains for its period, volume and pan, worked out
// again only when they change: a step is the step numerator / the period
// and a gain the side's gain for a volume of 1 (Q8, from the pan) * the
// volume.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::UpdateMix(SModChannel* psChannel)
{
	float64	fPan;
	uint32	uiPeriod;

	if (psChannel->iMixPeriod != psChannel->iOutPeriod)
	{
		uiPeriod = (uint32)psChannel->iOutPeriod;
		psChannel->uiStep = (muiStepNumerator + uiPeriod / 2) / uiPeriod;
		psChannel->iMixPeriod = psChannel->iOutPeriod;
	}

	if (psChannel->fMixPan != psChannel->fPan)
	{
		fPan = psChannel->fPan * mfStereoSeparation;
		psChannel->uiPanLeft = (uint32)((1.0 - fPan) * 0.5 * mfGain / 64.0 * MOD_GAIN_ONE * 256.0 + 0.5);
		psChannel->uiPanRight = (uint32)((1.0 + fPan) * 0.5 * mfGain / 64.0 * MOD_GAIN_ONE * 256.0 + 0.5);
		psChannel->fMixPan = psChannel->fPan;
		psChannel->iMixVolume = -1;
	}

	if (psChannel->iMixVolume != psChannel->iOutVolume)
	{
		psChannel->iLeft = (int16)((psChannel->uiPanLeft * (uint32)psChannel->iOutVolume + 128) >> 8);
		psChannel->iRight = (int16)((psChannel->uiPanRight * (uint32)psChannel->iOutVolume + 128) >> 8);
		psChannel->iMixVolume = psChannel->iOutVolume;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiNumFrames frames of the channel's 8 bit samples, played nearest,
// through its gain tables (made again when its gains change), to each
// side of the mix it has a gain on: a channel panned to one side (as all
// are with full stereo separation) is mixed once.  At the end of its
// sample it loops, or stops if it doesn't loop.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::MixNearest8(SModChannel* psChannel, SModMix* psMix, uint16 uiFirstFrame, uint16 uiNumFrames)
{
	SModGainTable*	psLeft;
	SModGainTable*	psRight;
	lsize			uiPosition;
	uint16			uiFraction;
	bool			bPlaying;
	size			uiChannel;
	bool			bLeft;
	bool			bRight;

	uiChannel = (size)(psChannel - masChannels);
	psLeft = &mpsGainTables[uiChannel * 2];
	psRight = &mpsGainTables[uiChannel * 2 + 1];
	bRight = psChannel->iRight != 0;
	bLeft = (psChannel->iLeft != 0) || !bRight;

	bPlaying = true;
	if (bLeft)
	{
		if (psChannel->iTableLeft != psChannel->iLeft)
		{
			ModBuildGainTable(psLeft, psChannel->iLeft);
			psChannel->iTableLeft = psChannel->iLeft;
		}
		uiPosition = psChannel->uiPosition;
		uiFraction = psChannel->uiFraction;
		MixNearest8Side(psChannel, &psMix->sLeft, psLeft, uiFirstFrame, uiNumFrames, &uiPosition, &uiFraction, &bPlaying);
	}
	if (bRight)
	{
		if (psChannel->iTableRight != psChannel->iRight)
		{
			ModBuildGainTable(psRight, psChannel->iRight);
			psChannel->iTableRight = psChannel->iRight;
		}
		uiPosition = psChannel->uiPosition;
		uiFraction = psChannel->uiFraction;
		MixNearest8Side(psChannel, &psMix->sRight, psRight, uiFirstFrame, uiNumFrames, &uiPosition, &uiFraction, &bPlaying);
	}
	psChannel->uiPosition = uiPosition;
	psChannel->uiFraction = uiFraction;
	if (!bPlaying)
	{
		psChannel->bPlaying = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One side of MixNearest8, from *puiPosition and *puiFraction (left where
// it stopped; *pbPlaying false if the sample ended without a loop).  The
// sample is given to ModMixNearest8 at most MOD_SAMPLE_WINDOW frames at a
// time.  A position past the end (a sample offset) plays a frame and
// wraps, as each frame past the end does.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::MixNearest8Side(SModChannel* psChannel, SModMixSide* psMix, SModGainTable* psGains, uint16 uiFirstFrame, uint16 uiNumFrames, lsize* puiPosition, uint16* puiFraction, bool* pbPlaying)
{
	int8*	piBase;
	lsize	uiPosition;
	lsize	uiLength;
	lsize	uiLeft;
	uint16	uiFraction;
	uint16	uiFrame;
	uint16	uiOffset;
	uint16	uiEnd;

	piBase = &mpiBank8[psChannel->uiFirstFrame];
	uiPosition = *puiPosition;
	uiFraction = *puiFraction;
	uiLength = psChannel->uiLength;
	uiFrame = 0;
	while (uiFrame < uiNumFrames)
	{
		if (uiPosition >= uiLength)
		{
			uiEnd = 1;
		}
		else
		{
			uiLeft = uiLength - uiPosition;
			uiEnd = (uiLeft > MOD_SAMPLE_WINDOW) ? (uint16)MOD_SAMPLE_WINDOW : (uint16)uiLeft;
		}
		uiOffset = 0;
		uiFrame = (uint16)(uiFrame + ModMixNearest8(psMix, (uint16)(uiFirstFrame + uiFrame), (uint16)(uiNumFrames - uiFrame), (SModSampleWindow*)&piBase[uiPosition], &uiOffset, &uiFraction, uiEnd, psChannel->uiStep, psGains));
		uiPosition += uiOffset;
		if (uiPosition >= uiLength)
		{
			if (psChannel->uiLoopFrame == SOUND_CEL_NO_LOOP)
			{
				*pbPlaying = false;
				break;
			}
			uiPosition = psChannel->uiLoopFrame + (uiPosition - uiLength) % (uiLength - psChannel->uiLoopFrame);
		}
	}
	*puiPosition = uiPosition;
	*puiFraction = uiFraction;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiNumFrames frames of the channel's samples (16 bit, or played
// with linear interpolation): each sample (Q8) * gain >> MOD_MIX_SHIFT.
//
//////////////////////////////////////////////////////////////////////////
void CModPlayer::MixSamples(SModChannel* psChannel, SModMix* psMix, uint16 uiFirstFrame, uint16 uiNumFrames)
{
	uint16	uiFrame;
	uint16	uiMix;
	lsize	uiStepFrames;
	uint16	uiStepFraction;
	uint16	uiFraction;
	int32	iSample;
	int32	iNext;
	lsize	uiPosition;
	lsize	uiLength;
	lsize	uiLoopFrame;

	uiStepFrames = psChannel->uiStep >> 16;
	uiStepFraction = (uint16)psChannel->uiStep;
	uiPosition = psChannel->uiPosition;
	uiFraction = psChannel->uiFraction;
	uiLength = psChannel->uiLength;
	uiLoopFrame = psChannel->uiLoopFrame;
	uiMix = uiFirstFrame;
	for (uiFrame = 0; uiFrame < uiNumFrames; uiFrame++)
	{
		iSample = GetBankSample(psChannel, uiPosition);
		if (meInterpolation == MI_Linear)
		{
			iNext = GetSampleAt(psChannel, uiPosition + 1);
			iSample += ((iNext - iSample) >> 8) * (int32)uiFraction >> 8;
		}
		psMix->sLeft.aiFrames[uiMix] = (int16)(psMix->sLeft.aiFrames[uiMix] + ((iSample * psChannel->iLeft) >> MOD_MIX_SHIFT));
		psMix->sRight.aiFrames[uiMix] = (int16)(psMix->sRight.aiFrames[uiMix] + ((iSample * psChannel->iRight) >> MOD_MIX_SHIFT));
		uiMix++;

		uiPosition += uiStepFrames;
		uiFraction += uiStepFraction;
		if (uiFraction < uiStepFraction)
		{
			uiPosition++;
		}
		if (uiPosition >= uiLength)
		{
			if (uiLoopFrame == SOUND_CEL_NO_LOOP)
			{
				psChannel->bPlaying = false;
				break;
			}
			uiPosition = uiLoopFrame + (uiPosition - uiLength) % (uiLength - uiLoopFrame);
		}
	}
	psChannel->uiPosition = uiPosition;
	psChannel->uiFraction = uiFraction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CModPlayer::IsFinished(void)
{
	return mbFinished;
}


//////////////////////////////////////////////////////////////////////////
//
// The order, row and tick last played.
//
//////////////////////////////////////////////////////////////////////////
size CModPlayer::GetOrder(void)
{
	return muiOrder;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CModPlayer::GetRow(void)
{
	return muiRow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CModPlayer::GetTick(void)
{
	return muiTick;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CModPlayer::GetSpeed(void)
{
	return muiSpeed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CModPlayer::GetTempo(void)
{
	return muiTempo;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CModPlayer::GetFramesPlayed(void)
{
	return muiFramesPlayed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SModChannel* CModPlayer::GetChannel(size uiChannel)
{
	if (uiChannel >= MOD_MAX_CHANNELS)
	{
		return NULL;
	}
	return &masChannels[uiChannel];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CModPlayer::NumChannels(void)
{
	return mpSong->NumChannels();
}

