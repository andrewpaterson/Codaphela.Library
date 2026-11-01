#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MODReader.h"
#include "SupportLib/ModPlayer.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "SupportLib/WAVWriter.h"
#include "TestLib/AssertFile.h"
#include "TestLib/Assert.h"


//The modules are made by Input/MOD/MakeMOD.py, which documents what each row does.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestModReader(void)
{
	Ptr<CModSong>	pcSong;
	Ptr<CSoundCel>	pcCel;
	SLogConfig		sLogConfig;

	pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Effects.mod");
	AssertTrue(pcSong.IsNotNull());
	if (pcSong.IsNull())
	{
		return;
	}
	AssertString("Effects", pcSong->GetTitle());
	AssertSize(4, pcSong->NumChannels());
	AssertSize(31, pcSong->NumSamples());
	AssertSize(2, pcSong->NumPatterns());
	AssertSize(2, pcSong->GetSongLength());
	AssertSize(0, pcSong->GetRestart());
	AssertInt(1, pcSong->GetOrder(1));
	AssertString("Square", pcSong->GetSample(1)->szName);
	AssertInt(64, pcSong->GetSample(1)->uiVolume);

	//The samples are cels in one signed 8 bit bank.
	AssertInt(PT_int8, pcSong->GetSampleBank()->GetSound()->GetPrimitiveType());
	AssertLSize(32 + 512, pcSong->GetSampleBank()->GetSound()->GetNumFrames());
	pcCel = pcSong->GetSampleCel(1);
	AssertLSize(0, pcCel->GetFirstFrame());
	AssertLSize(32, pcCel->GetNumFrames());
	AssertLSize(0, pcCel->GetLoopFrame());
	AssertDouble(0.5, pcCel->GetSample(0, 0), 12);
	AssertDouble(-0.5, pcCel->GetSample(16, 0), 12);
	pcCel = pcSong->GetSampleCel(2);
	AssertLSize(32, pcCel->GetFirstFrame());
	AssertLSize(512, pcCel->GetNumFrames());
	AssertFalse(pcCel->HasLoop());
	AssertTrue(pcSong->GetSampleCel(3).IsNull());

	AssertInt(428, pcSong->GetNote(0, 0, 0)->uiPeriod);
	AssertInt(1, pcSong->GetNote(0, 0, 0)->uiSample);
	AssertInt(0xC, pcSong->GetNote(0, 0, 0)->uiEffect);
	AssertInt(0x20, pcSong->GetNote(0, 0, 0)->uiParam);
	AssertInt(0x8, pcSong->GetNote(0, 14, 1)->uiEffect);

	pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Eight.mod");
	AssertSize(8, pcSong->NumChannels());
	pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Soundtracker.mod");
	AssertTrue(pcSong.IsNotNull());
	AssertSize(15, pcSong->NumSamples());
	AssertSize(4, pcSong->NumChannels());

	sLogConfig = gcLogger.SetSilent();
	AssertTrue(LoadMOD("Input" _FS_ "MOD" _FS_ "Truncated.mod").IsNull());
	AssertTrue(LoadMOD("Input" _FS_ "MOD" _FS_ "NoSong.mod").IsNull());
	AssertTrue(LoadMOD("Input" _FS_ "MOD" _FS_ "Garbage.mod").IsNull());
	AssertTrue(LoadMOD("Input" _FS_ "MOD" _FS_ "Missing.mod").IsNull());
	gcLogger.SetConfig(&sLogConfig);

	pcSong = NULL;
	pcCel = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayerPeriods(void)
{
	AssertInt(428, CModPlayer::GetFineTunedPeriod(428, 0));
	AssertInt(437, CModPlayer::GetFineTunedPeriod(428, -3));
	AssertInt(407, CModPlayer::GetFineTunedPeriod(428, 7));
	AssertInt(907, CModPlayer::GetFineTunedPeriod(856, -8));
	AssertInt(320, CModPlayer::GetArpeggioPeriod(404, 0, 4));
	AssertInt(269, CModPlayer::GetArpeggioPeriod(404, 0, 7));
	AssertInt(113, CModPlayer::GetArpeggioPeriod(120, 0, 7));

	//ProTracker moves periods onto its table: the first note at or below.  Periods outside the table are left alone.
	AssertInt(381, CModPlayer::GetFineTunedPeriod(400, 0));
	AssertInt(856, CModPlayer::GetFineTunedPeriod(870, 0));
	AssertInt(100, CModPlayer::GetFineTunedPeriod(100, 0));
	AssertInt(1712, CModPlayer::GetFineTunedPeriod(1712, 0));
}


//////////////////////////////////////////////////////////////////////////
//
// A looped square wave note on channel 0 for 64 rows of 6 ticks.
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayerBasic(void)
{
	CModPlayer		cPlayer;
	Ptr<CSound>		pcSound;
	size			i;
	int				iCrossings;
	float64			fPrevious;
	float64			fSample;
	float64			fRightLevel;
	float64			fExpectedCrossings;

	Ptr<CModSong> pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Basic.mod");

	cPlayer.Init(pcSong, 44100);
	AssertLSize(882, cPlayer.SkipTick());
	AssertTrue(cPlayer.GetChannel(0)->bPlaying);
	AssertInt(428, cPlayer.GetChannel(0)->iPeriod);
	AssertInt(64, cPlayer.GetChannel(0)->iVolume);
	AssertFalse(cPlayer.GetChannel(1)->bPlaying);
	cPlayer.Kill();

	cPlayer.Init(pcSong, 44100);
	pcSound = cPlayer.RenderSong(PT_float32);
	AssertTrue(cPlayer.IsFinished());
	cPlayer.Kill();
	AssertLSize((lsize)64 * 6 * 882, pcSound->GetNumFrames());
	AssertInt((int32)44100, pcSound->GetSampleRate());
	AssertTrue(pcSound->HasChannels(SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP));

	//Channel 0 is hard left.  The 32 frame square at period 428 plays at 3546894.6 / 428 / 32 = 259 Hz.
	iCrossings = 0;
	fRightLevel = 0.0;
	fPrevious = pcSound->GetSample(0, 0);
	for (i = 1; i < 44100; i++)
	{
		fSample = pcSound->GetSample(i, 0);
		if ((fSample >= 0.0) != (fPrevious >= 0.0))
		{
			iCrossings++;
		}
		fPrevious = fSample;
		fRightLevel += fabs(pcSound->GetSample(i, 1));
	}
	fExpectedCrossings = 2.0 * MOD_PAULA_CLOCK_PAL / (2.0 * 428.0) / 32.0;
	AssertTrue(fabs(iCrossings - fExpectedCrossings) <= 2.0);
	AssertDouble(0.0, fRightLevel, 12);

	//Full volume on one of two channels a side: the square is at half scale, 0.5 * 0.5.
	AssertDouble(0.25, fabs(pcSound->GetSample(1000, 0)), 4);

	pcSong = NULL;
	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Plays one tick (rendering it so samples move) and checks it was the
// expected order, row and tick.
//
//////////////////////////////////////////////////////////////////////////
SModChannel* PlayModTick(CModPlayer* pcPlayer, size uiOrder, size uiRow, size uiTick, size uiFrames)
{
	float64	afStereo[882 * 2];

	AssertLSize(uiFrames, pcPlayer->Render(afStereo, uiFrames));
	AssertSize(uiOrder, pcPlayer->GetOrder());
	AssertSize(uiRow, pcPlayer->GetRow());
	AssertSize(uiTick, pcPlayer->GetTick());
	return pcPlayer->GetChannel(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayerEffects(void)
{
	CModPlayer		cPlayer;
	SModChannel*	psChannel;
	size			uiTick;
	size			uiRow;
	int				aiVibrato[5] = { 0, 6, 11, 14, 15 };
	int				aiTremolo[5] = { 12, 24, 34, 41, 43 };
	lsize			uiFramesBefore;

	Ptr<CModSong> pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Effects.mod");
	cPlayer.Init(pcSong, 44100);

	//Row 0: C20.
	psChannel = PlayModTick(&cPlayer, 0, 0, 0, 882);
	AssertInt(32, psChannel->iVolume);
	AssertInt(428, psChannel->iPeriod);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		PlayModTick(&cPlayer, 0, 0, uiTick, 882);
	}

	//Row 1: A04.
	psChannel = PlayModTick(&cPlayer, 0, 1, 0, 882);
	AssertInt(32, psChannel->iVolume);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 1, uiTick, 882);
		AssertInt(32 - 4 * (int)uiTick, psChannel->iVolume);
	}

	//Row 2: 102.
	for (uiTick = 0; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 2, uiTick, 882);
		AssertInt(428 - 2 * (int)uiTick, psChannel->iPeriod);
	}

	//Row 3: 203.
	for (uiTick = 0; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 3, uiTick, 882);
		AssertInt(418 + 3 * (int)uiTick, psChannel->iPeriod);
	}

	//Rows 4 and 5: tone portamento to C#2 (404) by 4, without restarting the note.
	psChannel = PlayModTick(&cPlayer, 0, 4, 0, 882);
	AssertInt(433, psChannel->iPeriod);
	AssertInt(404, psChannel->iPortaTarget);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 4, uiTick, 882);
		AssertInt(433 - 4 * (int)uiTick, psChannel->iPeriod);
	}
	PlayModTick(&cPlayer, 0, 5, 0, 882);
	psChannel = PlayModTick(&cPlayer, 0, 5, 1, 882);
	AssertInt(409, psChannel->iPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 5, 2, 882);
	AssertInt(405, psChannel->iPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 5, 3, 882);
	AssertInt(404, psChannel->iPeriod);
	PlayModTick(&cPlayer, 0, 5, 4, 882);
	psChannel = PlayModTick(&cPlayer, 0, 5, 5, 882);
	AssertInt(404, psChannel->iPeriod);

	//Row 6: arpeggio 047 on C#2.
	psChannel = PlayModTick(&cPlayer, 0, 6, 0, 882);
	AssertInt(404, psChannel->iOutPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 6, 1, 882);
	AssertInt(320, psChannel->iOutPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 6, 2, 882);
	AssertInt(269, psChannel->iOutPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 6, 3, 882);
	AssertInt(404, psChannel->iOutPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 6, 4, 882);
	AssertInt(320, psChannel->iOutPeriod);
	AssertInt(404, psChannel->iPeriod);
	PlayModTick(&cPlayer, 0, 6, 5, 882);

	//Row 7: E13.
	psChannel = PlayModTick(&cPlayer, 0, 7, 0, 882);
	AssertInt(401, psChannel->iPeriod);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		PlayModTick(&cPlayer, 0, 7, uiTick, 882);
	}

	//Row 8: vibrato 448.  The period itself doesn't change.
	psChannel = PlayModTick(&cPlayer, 0, 8, 0, 882);
	AssertInt(401, psChannel->iOutPeriod);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 8, uiTick, 882);
		AssertInt(401 + aiVibrato[uiTick - 1], psChannel->iOutPeriod);
		AssertInt(401, psChannel->iPeriod);
	}

	//Row 9: tremolo 748 on volume 12.
	PlayModTick(&cPlayer, 0, 9, 0, 882);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 9, uiTick, 882);
		AssertInt(aiTremolo[uiTick - 1], psChannel->iOutVolume);
		AssertInt(12, psChannel->iVolume);
	}

	//Row 10: EC3.
	for (uiTick = 0; uiTick < 6; uiTick++)
	{
		psChannel = PlayModTick(&cPlayer, 0, 10, uiTick, 882);
		AssertInt(uiTick < 3 ? 12 : 0, psChannel->iVolume);
	}

	//Row 11: as in ProTracker the sample, volume and period change on tick 0 but the sample only restarts on tick 2.
	psChannel = PlayModTick(&cPlayer, 0, 11, 0, 882);
	AssertInt(64, psChannel->iVolume);
	AssertInt(428, psChannel->iPeriod);
	psChannel = PlayModTick(&cPlayer, 0, 11, 1, 882);
	AssertTrue((psChannel->uiPosition > 0) || (psChannel->uiFraction >= 0x8000));
	AssertLSize(882, cPlayer.SkipTick());
	psChannel = cPlayer.GetChannel(0);
	AssertSize(2, cPlayer.GetTick());
	AssertLSize(0, psChannel->uiPosition);
	AssertInt(0, (int)psChannel->uiFraction);
	for (uiTick = 3; uiTick < 6; uiTick++)
	{
		PlayModTick(&cPlayer, 0, 11, uiTick, 882);
	}

	//Row 12: retrigger on tick 3.  A tick plays 882 / 44100 * 8287 = 166 frames of the sample, which loops every 32.
	PlayModTick(&cPlayer, 0, 12, 0, 882);
	PlayModTick(&cPlayer, 0, 12, 1, 882);
	psChannel = PlayModTick(&cPlayer, 0, 12, 2, 882);
	AssertTrue((psChannel->uiPosition > 0) || (psChannel->uiFraction >= 0x8000));
	cPlayer.SkipTick();
	AssertSize(3, cPlayer.GetTick());
	AssertLSize(0, cPlayer.GetChannel(0)->uiPosition);
	AssertInt(0, (int)cPlayer.GetChannel(0)->uiFraction);
	PlayModTick(&cPlayer, 0, 12, 4, 882);
	PlayModTick(&cPlayer, 0, 12, 5, 882);

	//Row 13: sample 2 from 256.
	cPlayer.SkipTick();
	AssertSize(13, cPlayer.GetRow());
	psChannel = cPlayer.GetChannel(0);
	AssertSize(2, psChannel->uiSample);
	AssertLSize(256, psChannel->uiPosition);
	AssertInt(0, (int)psChannel->uiFraction);
	for (uiTick = 1; uiTick < 6; uiTick++)
	{
		PlayModTick(&cPlayer, 0, 13, uiTick, 882);
	}

	//Row 14: speed 3, and channel 1 panned.
	PlayModTick(&cPlayer, 0, 14, 0, 882);
	AssertSize(3, cPlayer.GetSpeed());
	AssertDouble(-0.5, cPlayer.GetChannel(1)->fPan, 12);
	PlayModTick(&cPlayer, 0, 14, 1, 882);
	PlayModTick(&cPlayer, 0, 14, 2, 882);

	//Row 15: tempo 250 halves the ticks.
	PlayModTick(&cPlayer, 0, 15, 0, 441);
	AssertSize(250, cPlayer.GetTempo());
	PlayModTick(&cPlayer, 0, 15, 1, 441);
	PlayModTick(&cPlayer, 0, 15, 2, 441);

	//Row 16: pattern delay; 9 ticks.
	for (uiTick = 0; uiTick < 9; uiTick++)
	{
		PlayModTick(&cPlayer, 0, 16, uiTick, 441);
	}

	//Rows 17 and 18 three times.
	for (uiRow = 0; uiRow < 6; uiRow++)
	{
		for (uiTick = 0; uiTick < 3; uiTick++)
		{
			PlayModTick(&cPlayer, 0, 17 + uiRow % 2, uiTick, 441);
		}
	}

	//Row 19 breaks to order 1 row 5, which jumps to order 0 row 0 and ends the song.
	for (uiTick = 0; uiTick < 3; uiTick++)
	{
		PlayModTick(&cPlayer, 0, 19, uiTick, 441);
	}
	for (uiTick = 0; uiTick < 3; uiTick++)
	{
		PlayModTick(&cPlayer, 1, 5, uiTick, 441);
	}
	AssertFalse(cPlayer.IsFinished());
	uiFramesBefore = cPlayer.GetFramesPlayed();
	AssertLSize(0, cPlayer.SkipTick());
	AssertTrue(cPlayer.IsFinished());
	AssertLSize(uiFramesBefore, cPlayer.GetFramesPlayed());
	cPlayer.Kill();

	//The whole song: 14 rows of 6 x 882, 1 of 3 x 882, 1 of 3 x 441 then 9 + 18 + 3 + 3 ticks of 441.
	cPlayer.Init(pcSong, 44100);
	Ptr<CSound> pcSound = cPlayer.RenderSong(PT_float32);
	cPlayer.Kill();
	AssertLSize((lsize)14 * 6 * 882 + 3 * 882 + (3 + 9 + 18 + 3 + 3) * 441, pcSound->GetNumFrames());
	AssertTrue(SaveWAV(pcSound, "Output" _FS_ "Effects.wav"));

	pcSong = NULL;
	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayerFineTune(void)
{
	CModPlayer	cPlayer;
	size		uiTick;

	Ptr<CModSong> pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "FineTune.mod");
	AssertInt(-3, pcSong->GetSample(1)->iFineTune);
	AssertInt(7, pcSong->GetSample(2)->iFineTune);

	cPlayer.Init(pcSong, 44100);
	cPlayer.SkipTick();
	AssertInt(437, cPlayer.GetChannel(0)->iPeriod);
	for (uiTick = 0; uiTick < 6; uiTick++)
	{
		cPlayer.SkipTick();
	}
	AssertInt(407, cPlayer.GetChannel(0)->iPeriod);
	for (uiTick = 0; uiTick < 6; uiTick++)
	{
		cPlayer.SkipTick();
	}
	AssertInt(413, cPlayer.GetChannel(0)->iPeriod);
	AssertInt(5, cPlayer.GetChannel(0)->iFineTune);

	//F00 on row 3 ends the song before the row plays.
	for (uiTick = 0; uiTick < 5; uiTick++)
	{
		cPlayer.SkipTick();
	}
	AssertFalse(cPlayer.IsFinished());
	AssertLSize(0, cPlayer.SkipTick());
	AssertTrue(cPlayer.IsFinished());
	cPlayer.Kill();

	pcSong = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Channels are left, right, right, left.  Stereo separation 0 is mono.
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayerStereo(void)
{
	CModPlayer	cPlayer;
	float64		afStereo[882 * 2];

	Ptr<CModSong> pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Eight.mod");

	cPlayer.Init(pcSong, 44100);
	AssertLSize(882, cPlayer.Render(afStereo, 882));
	AssertDouble(-1.0, cPlayer.GetChannel(0)->fPan, 12);
	AssertDouble(1.0, cPlayer.GetChannel(1)->fPan, 12);
	AssertDouble(1.0, cPlayer.GetChannel(2)->fPan, 12);
	AssertDouble(-1.0, cPlayer.GetChannel(3)->fPan, 12);
	AssertDouble(-1.0, cPlayer.GetChannel(4)->fPan, 12);
	AssertDouble(1.0, cPlayer.GetChannel(6)->fPan, 12);

	//Eight channels at full volume, four a side, each at a quarter: the square's 0.5 sums to 0.5 a side.
	AssertDouble(0.5, afStereo[0], 6);
	AssertDouble(0.5, afStereo[1], 6);
	cPlayer.Kill();

	cPlayer.Init(pcSong, 44100, MI_Nearest, 0.0);
	AssertLSize(882, cPlayer.Render(afStereo, 882));
	AssertDouble(afStereo[100], afStereo[101], 12);
	cPlayer.Kill();

	pcSong = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The RMS level of each side for every row: a small fingerprint of a
// render that changes if the player plays anything differently.
//
//////////////////////////////////////////////////////////////////////////
void PrintModRowLevels(Ptr<CSound> pcSound, size uiRowFrames, CChars* psz)
{
	size	uiRow;
	size	uiNumRows;
	lsize	i;
	size	c;
	float64	fSample;
	float64	afSum[2];

	uiNumRows = (size)(pcSound->GetNumFrames() / uiRowFrames);
	for (uiRow = 0; uiRow < uiNumRows; uiRow++)
	{
		afSum[0] = 0.0;
		afSum[1] = 0.0;
		for (i = (lsize)uiRow * uiRowFrames; i < ((lsize)uiRow + 1) * uiRowFrames; i++)
		{
			for (c = 0; c < 2; c++)
			{
				fSample = pcSound->GetSample(i, c);
				afSum[c] += fSample * fSample;
			}
		}
		//Millionths, as integers.
		psz->Append((int32)uiRow);
		psz->Append(" ");
		psz->Append((int32)floor(sqrt(afSum[0] / uiRowFrames) * 1000000.0 + 0.5));
		psz->Append(" ");
		psz->Append((int32)floor(sqrt(afSum[1] / uiRowFrames) * 1000000.0 + 0.5));
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The original tune in Input/MOD/MakeMOD.py: 4 orders of 64 rows at speed
// 6 and 125 BPM, 30.72 seconds.  Written to Output/Tune.wav.
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayerTune(void)
{
	CModPlayer	cPlayer;
	CChars		sz;
	CFileBasic	cFile;
	lsize		i;
	size		c;
	float64		fPeak;
	float64		fSample;
	float64		afSum[2];

	Ptr<CModSong> pcSong = LoadMOD("Input" _FS_ "MOD" _FS_ "Tune.mod");
	AssertString("Codaphela Tune", pcSong->GetTitle());
	AssertSize(4, pcSong->GetSongLength());
	AssertSize(3, pcSong->NumPatterns());

	//32000 Hz 16 bit keeps the sound under the tests' 4MB allocation limit.  A tick is 640 frames.
	cPlayer.Init(pcSong, 32000);
	Ptr<CSound> pcSound = cPlayer.RenderSong(PT_int16);
	AssertTrue(cPlayer.IsFinished());
	cPlayer.Kill();
	AssertLSize((lsize)4 * 64 * 6 * 640, pcSound->GetNumFrames());

	//No clipping, and both sides are playing.
	fPeak = 0.0;
	afSum[0] = 0.0;
	afSum[1] = 0.0;
	for (i = 0; i < pcSound->GetNumFrames(); i++)
	{
		for (c = 0; c < 2; c++)
		{
			fSample = fabs(pcSound->GetSample(i, c));
			fPeak = fSample > fPeak ? fSample : fPeak;
			afSum[c] += fSample;
		}
	}
	AssertTrue(fPeak < 1.0);
	AssertTrue(afSum[0] / pcSound->GetNumFrames() > 0.02);
	AssertTrue(afSum[1] / pcSound->GetNumFrames() > 0.02);

	AssertTrue(SaveWAV(pcSound, "Output" _FS_ "Tune.wav"));

	//Checked against libopenmpt when written: each channel alone matched (drums and bass to 0.99999 correlation, chords to
	//0.9986, the lead's pitch and level tick by tick), allowing for libopenmpt's 75% panning and quieter mix.
	sz.Init();
	PrintModRowLevels(pcSound, 6 * 640, &sz);
	cFile.Init(DiskFile("Output" _FS_ "Tune-levels.txt"));
	if (cFile.Open(EFM_Write_Create))
	{
		cFile.WriteData(sz.Text(), sz.Length());
		cFile.Close();
	}
	cFile.Kill();
	AssertFileString("Input" _FS_ "MOD" _FS_ "Tune-levels-expected.txt", sz.Text());
	sz.Kill();

	pcSong = NULL;
	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestModPlayer(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestModReader();
	TestModPlayerPeriods();
	TestModPlayerBasic();
	TestModPlayerEffects();
	TestModPlayerFineTune();
	TestModPlayerStereo();
	TestModPlayerTune();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

