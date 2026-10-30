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
#include "BaseLib/ArrayTemplate.h"
#include "SoundChannel.h"


const char* gaszSoundSpeakerNames[NUM_SOUND_SPEAKERS] =
{
	"Mono",
	"Front Left",
	"Front Right",
	"Front Centre",
	"Low Frequency",
	"Back Left",
	"Back Right",
	"Front Left of Centre",
	"Front Right of Centre",
	"Back Centre",
	"Side Left",
	"Side Right",
	"Top Centre",
	"Top Front Left",
	"Top Front Centre",
	"Top Front Right",
	"Top Back Left",
	"Top Back Centre",
	"Top Back Right",
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsSoundSpeaker(EChannel eChannel)
{
	return GetSoundSpeaker(eChannel) != NUM_SOUND_SPEAKERS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsSoundDiscrete(EChannel eChannel)
{
	return GetSoundDiscreteIndex(eChannel) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ESoundSpeaker GetSoundSpeaker(EChannel eChannel)
{
	if ((eChannel >= SOUND_SPEAKER(0)) && (eChannel < SOUND_SPEAKER(NUM_SOUND_SPEAKERS)))
	{
		return (ESoundSpeaker)(eChannel - SOUND_SPEAKER(0));
	}
	return NUM_SOUND_SPEAKERS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size GetSoundDiscreteIndex(EChannel eChannel)
{
	if ((eChannel >= SOUND_DISCRETE(0)) && (eChannel < SOUND_DISCRETE(NUM_CHANNEL_TYPES - 1)))
	{
		return eChannel - SOUND_DISCRETE(0);
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 GetSoundSpeakerMask(EChannel eChannel)
{
	ESoundSpeaker	eSpeaker;

	eSpeaker = GetSoundSpeaker(eChannel);
	if ((eSpeaker == NUM_SOUND_SPEAKERS) || (eSpeaker == SS_Mono))
	{
		return 0;
	}
	return (uint32)1 << (eSpeaker - SS_FrontLeft);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EChannel GetSoundSpeakerForMaskBit(size uiBit)
{
	if (uiBit < NUM_SOUND_SPEAKERS - SS_FrontLeft)
	{
		return SOUND_SPEAKER(SS_FrontLeft + uiBit);
	}
	return CHANNEL_STOP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintSoundChannel(EChannel eChannel, CChars* psz)
{
	ESoundSpeaker	eSpeaker;
	size			uiIndex;

	eSpeaker = GetSoundSpeaker(eChannel);
	if (eSpeaker != NUM_SOUND_SPEAKERS)
	{
		psz->Append(gaszSoundSpeakerNames[eSpeaker]);
		return;
	}

	uiIndex = GetSoundDiscreteIndex(eChannel);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		psz->Append("Discrete ");
		psz->Append(uiIndex);
		return;
	}

	if (eChannel == SOUND_IGNORED)
	{
		psz->Append("Ignored");
		return;
	}
	psz->Append("Unknown");
}

