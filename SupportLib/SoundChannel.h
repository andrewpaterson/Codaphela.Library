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
#ifndef __SOUND_CHANNEL_H__
#define __SOUND_CHANNEL_H__
#include "BaseLib/Chars.h"
#include "ChannelTypes.h"


//Sound purposes start well above the image purposes so a sound channel never has the same number as an image channel.
enum ESoundPurpose
{
	SP_Speaker = 0x100,		//The channel feeds a speaker position (ESoundSpeaker).
	SP_Discrete,			//A numbered channel with no speaker position.
	SP_Ignored,				//Padding in the frame.
};


//The speakers are in WAVE channel mask order; speaker s is bit (s - SS_FrontLeft) of the mask.  Mono is not in the mask.
enum ESoundSpeaker
{
	SS_Mono,
	SS_FrontLeft,
	SS_FrontRight,
	SS_FrontCentre,
	SS_LowFrequency,
	SS_BackLeft,
	SS_BackRight,
	SS_FrontLeftOfCentre,
	SS_FrontRightOfCentre,
	SS_BackCentre,
	SS_SideLeft,
	SS_SideRight,
	SS_TopCentre,
	SS_TopFrontLeft,
	SS_TopFrontCentre,
	SS_TopFrontRight,
	SS_TopBackLeft,
	SS_TopBackCentre,
	SS_TopBackRight,

	NUM_SOUND_SPEAKERS
};


#define SOUND_SPEAKER(speaker)			CHANNEL(SP_Speaker, speaker)
#define SOUND_DISCRETE(index)			CHANNEL(SP_Discrete, index)

#define SOUND_MONO						SOUND_SPEAKER(SS_Mono)
#define SOUND_FRONT_LEFT				SOUND_SPEAKER(SS_FrontLeft)
#define SOUND_FRONT_RIGHT				SOUND_SPEAKER(SS_FrontRight)
#define SOUND_FRONT_CENTRE				SOUND_SPEAKER(SS_FrontCentre)
#define SOUND_LOW_FREQUENCY				SOUND_SPEAKER(SS_LowFrequency)
#define SOUND_BACK_LEFT					SOUND_SPEAKER(SS_BackLeft)
#define SOUND_BACK_RIGHT				SOUND_SPEAKER(SS_BackRight)
#define SOUND_FRONT_LEFT_OF_CENTRE		SOUND_SPEAKER(SS_FrontLeftOfCentre)
#define SOUND_FRONT_RIGHT_OF_CENTRE		SOUND_SPEAKER(SS_FrontRightOfCentre)
#define SOUND_BACK_CENTRE				SOUND_SPEAKER(SS_BackCentre)
#define SOUND_SIDE_LEFT					SOUND_SPEAKER(SS_SideLeft)
#define SOUND_SIDE_RIGHT				SOUND_SPEAKER(SS_SideRight)
#define SOUND_TOP_CENTRE				SOUND_SPEAKER(SS_TopCentre)
#define SOUND_TOP_FRONT_LEFT			SOUND_SPEAKER(SS_TopFrontLeft)
#define SOUND_TOP_FRONT_CENTRE			SOUND_SPEAKER(SS_TopFrontCentre)
#define SOUND_TOP_FRONT_RIGHT			SOUND_SPEAKER(SS_TopFrontRight)
#define SOUND_TOP_BACK_LEFT				SOUND_SPEAKER(SS_TopBackLeft)
#define SOUND_TOP_BACK_CENTRE			SOUND_SPEAKER(SS_TopBackCentre)
#define SOUND_TOP_BACK_RIGHT			SOUND_SPEAKER(SS_TopBackRight)
#define SOUND_IGNORED					CHANNEL(SP_Ignored, 0)

#define SOUND_LEFT						SOUND_FRONT_LEFT
#define SOUND_RIGHT						SOUND_FRONT_RIGHT


bool			IsSoundSpeaker(EChannel eChannel);
bool			IsSoundDiscrete(EChannel eChannel);
ESoundSpeaker	GetSoundSpeaker(EChannel eChannel);			//NUM_SOUND_SPEAKERS if eChannel is not a speaker.
size			GetSoundDiscreteIndex(EChannel eChannel);	//ARRAY_ELEMENT_NOT_FOUND if eChannel is not discrete.
uint32			GetSoundSpeakerMask(EChannel eChannel);		//The WAVE channel mask bit, zero for mono and non speakers.
EChannel		GetSoundSpeakerForMaskBit(size uiBit);
void			PrintSoundChannel(EChannel eChannel, CChars* psz);


#endif // __SOUND_CHANNEL_H__

