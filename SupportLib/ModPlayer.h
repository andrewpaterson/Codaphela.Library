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
#ifndef __MOD_PLAYER_H__
#define __MOD_PLAYER_H__
#include "ModSong.h"


#define MOD_PAULA_CLOCK_PAL			7093789.2	//The PAL Amiga's clock.  A sample at period p plays at clock / (2 * p) Hz.
#define MOD_DEFAULT_SPEED			6			//Ticks a row.
#define MOD_DEFAULT_TEMPO			125			//Beats a minute; a tick is 2.5 / tempo seconds.
#define MOD_MIN_PERIOD				113			//B-3, the highest note ProTracker slides to.
#define MOD_MAX_PERIOD				856			//C-1, the lowest.


enum EModInterpolation
{
	MI_Nearest,		//What the Amiga does.
	MI_Linear,
};


struct SModChannel
{
	//The sample playing.
	size		uiSample;			//1 to 31, zero for none.
	float64*	pfData;				//The sample's cel in the player's copy of the bank.
	lsize		uiLength;
	lsize		uiLoopFrame;		//SOUND_CEL_NO_LOOP if the sample plays once.
	bool		bPlaying;
	float64		fPosition;			//Frames into the cel.

	//The note.
	int			iPeriod;
	int			iVolume;			//0 to 64.
	int			iFineTune;			//-8 to 7.
	float64		fPan;				//-1 left to 1 right.

	//What this tick plays, after arpeggio, vibrato and tremolo.
	int			iOutPeriod;
	int			iOutVolume;

	//Effects.
	uint8		uiEffect;
	uint8		uiParam;
	int			iPortaTarget;
	int			iPortaSpeed;
	int			iVibratoPosition;
	int			iVibratoSpeed;
	int			iVibratoDepth;
	int			iVibratoWave;
	int			iTremoloPosition;
	int			iTremoloSpeed;
	int			iTremoloDepth;
	int			iTremoloWave;
	lsize		uiSampleOffset;
	int			iLoopRow;
	int			iLoopCount;
	SModNote	sDelayedNote;
};


//Plays a ProTracker module the way ProTracker 2 does: speed and tempo (CIA) timing, the ProTracker period table and every
//effect (arpeggio, slides, tone portamento, vibrato, tremolo, sample offset, volume slides, jumps, breaks, pattern loops and
//delays, retriggers, note cuts and delays, fine slides and finetune), plus 8xx and E8x panning which ProTracker ignores.
//Channels are panned left, right, right, left as on the Amiga, scaled by the stereo separation.  The song ends when it
//would play a row it has already played (outside a pattern loop), or at F00.
class CModPlayer
{
protected:
	Ptr<CModSong>		mpSong;
	uint32				muiSampleRate;
	EModInterpolation	meInterpolation;
	float64				mfStereoSeparation;
	float64				mfGain;

	float64*			mpfBank;			//The song's sample bank as floats.
	SModChannel			masChannels[MOD_MAX_CHANNELS];

	size				muiOrder;
	size				muiRow;
	size				muiTick;			//Within the row, including pattern delay rows.
	size				muiSpeed;
	size				muiTempo;
	size				muiPatternDelay;
	bool				mbBreak;
	size				muiBreakRow;
	bool				mbPositionJump;
	size				muiJumpOrder;
	bool				mbLoopJump;
	size				muiLoopRow;
	bool				mbFinished;
	bool				mbStarted;
	uint8*				mpuiPlayedRows;		//One for each order and row played.

	float64				mfFramesPerTick;
	float64				mfTickFrames;		//Frames of the current tick still to mix.
	float64				mfFrameRemainder;
	lsize				muiFramesPlayed;

public:
	void			Init(Ptr<CModSong> pcSong, uint32 uiSampleRate = 44100, EModInterpolation eInterpolation = MI_Linear, float64 fStereoSeparation = 1.0);
	void			Kill(void);

	lsize			Render(float64* pfStereo, lsize uiNumFrames);
	Ptr<CSound>		RenderSong(EPrimitiveType eType = PT_int16, lsize uiMaxFrames = 0);
	lsize			SkipTick(void);

	bool			IsFinished(void);
	size			GetOrder(void);
	size			GetRow(void);
	size			GetTick(void);
	size			GetSpeed(void);
	size			GetTempo(void);
	lsize			GetFramesPlayed(void);
	SModChannel*	GetChannel(size uiChannel);
	size			NumChannels(void);

	static int		GetFineTunedPeriod(int iPeriod, int iFineTune);
	static int		GetArpeggioPeriod(int iPeriod, int iFineTune, int iSemitones);

protected:
	void			StartTick(void);
	void			ProcessRow(void);
	void			ProcessTickEffects(void);
	void			NextRow(void);
	void			PlayNote(SModChannel* psChannel, SModNote* psNote, bool bStart);
	void			StartSample(SModChannel* psChannel, lsize uiOffset);
	void			SetSample(SModChannel* psChannel, size uiSample);
	void			ExtendedTickZero(SModChannel* psChannel, int iCommand, int iValue);
	void			VolumeSlide(SModChannel* psChannel, int iParam);
	void			TonePortamento(SModChannel* psChannel);
	void			Vibrato(SModChannel* psChannel);
	void			Tremolo(SModChannel* psChannel);
	int				GetWave(int iWave, int iPosition);
	void			Mix(float64* pfStereo, lsize uiNumFrames);
	float64			GetSampleAt(SModChannel* psChannel, lsize uiFrame);
};


#endif // __MOD_PLAYER_H__

