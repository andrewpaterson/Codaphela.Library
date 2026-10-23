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
#ifndef __MOD_SONG_H__
#define __MOD_SONG_H__
#include "StandardLib/Object.h"
#include "SoundCelGroup.h"


#define MOD_ROWS_PER_PATTERN		64
#define MOD_MAX_ORDERS				128
#define MOD_MAX_SAMPLES				31
#define MOD_MAX_CHANNELS			32
#define MOD_NO_CEL					ARRAY_ELEMENT_NOT_FOUND


//One cell of a pattern.
struct SModNote
{
	uint16	uiPeriod;		//The Amiga period of the note (finetune 0), zero for no note.
	uint8	uiSample;		//1 to 31, zero for none.
	uint8	uiEffect;		//0 to F.
	uint8	uiParam;
};


struct SModSample
{
	char	szName[23];
	int8	iFineTune;		//-8 to 7.
	uint8	uiVolume;		//0 to 64.
	size	uiCel;			//The sample's cel in the song's sample bank, MOD_NO_CEL if the sample is empty.
};


//A ProTracker module: its samples (a cel each in a signed 8 bit sound bank), patterns and order list.
class CModSong : public CObject
{
CONSTRUCTABLE(CModSong);
DESTRUCTABLE(CModSong);
protected:
	char					mszTitle[21];
	size					muiNumChannels;
	size					muiNumSamples;		//15 or 31.
	SModSample				masSamples[MOD_MAX_SAMPLES];
	size					muiSongLength;
	size					muiRestart;
	uint8					mauiOrders[MOD_MAX_ORDERS];
	size					muiNumPatterns;
	SModNote*				mpsNotes;			//Pattern, row, channel.
	Ptr<CSoundCelGroup>		mpSamples;

public:
	void					Init(size uiNumChannels, size uiNumSamples, size uiNumPatterns);
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					SetTitle(char* szTitle);
	char*					GetTitle(void);
	size					NumChannels(void);
	size					NumSamples(void);
	size					NumPatterns(void);

	SModSample*				GetSample(size uiSample);  //1 to NumSamples.
	Ptr<CSoundCel>			GetSampleCel(size uiSample);
	Ptr<CSoundCelGroup>		GetSampleBank(void);
	void					SetSampleBank(Ptr<CSoundCelGroup> pSamples);

	void					SetSongLength(size uiSongLength, size uiRestart);
	size					GetSongLength(void);
	size					GetRestart(void);
	void					SetOrder(size uiOrder, uint8 uiPattern);
	uint8					GetOrder(size uiOrder);

	SModNote*				GetNote(size uiPattern, size uiRow, size uiChannel);
};


#endif // __MOD_SONG_H__

