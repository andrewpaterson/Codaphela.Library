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
#ifndef __SOUND_H__
#define __SOUND_H__
#include "StandardLib/Channels.h"
#include "StandardLib/Array.h"
#include "SoundChannel.h"


struct SSoundChangingDesc
{
	uint32	uiSampleRate;
	size	uiNumFrames;
};


//A sound is a run of frames played at muiSampleRate frames per second.  A frame holds one sample for each channel (like a pixel
//in an image) and the channels can be any primitive type.
//
//Samples are read and written as float64 in [-1, 1).  Signed integer samples are scaled by their range (int16 -32768 is -1.0) and
//unsigned integer samples are offset by half their range first (uint8 128 is silence).  Floating point samples are not scaled.
class CSound : public CObject
{
CONSTRUCTABLE(CSound);
DESTRUCTABLE(CSound);
public:
	CChannels				mcChannels;
	uint32					muiSampleRate;
	SSoundChangingDesc*		mpsSoundChangingDesc;

public:
	void					Init(void);
	void					Init(uint32 uiSampleRate, size uiNumFrames, EPrimitiveType eType, EChannel eFirst, ...);
	void					Init(uint32 uiSampleRate, size uiNumFrames, void* pvUserData, EPrimitiveType eType, EChannel eFirst, ...);
	void					Init(uint32 uiSampleRate, size uiNumFrames, CArrayChannel* pasNewChannels);
	void					Init(uint32 uiSampleRate, size uiNumFrames, Ptr<CSound> pcChannelsSource);
	void					Init(Ptr<CSound> pcChannelsSource);  //This only sets up channels, sample rate and length.
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					BeginChange(void);
	void					AddChannel(SChannel* psChannel);
	void 					AddChannel(size iChannel, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel1, size iChannel2, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannels(CArrayChannel* pasChannels);
	void 					AddChannels(Ptr<CSound> pcSourceChannels);
	bool					RemoveChannel(size iChannel);
	bool					RenameChannel(size iOldName, size iNewName);
	void					SetNumFrames(size uiNumFrames);
	void					SetSampleRate(uint32 uiSampleRate);
	void 					ByteAlignChannels(void);
	bool 					EndChange(void);
	bool					IsChanging(void);

	size					GetNumFrames(void);
	uint32					GetSampleRate(void);
	float64					GetDuration(void);

	void					Copy(Ptr<CSound> pcSource);
	void					Silence(void);

	float64					GetSample(size uiFrame, size uiChannelIndex);
	void					SetSample(size uiFrame, size uiChannelIndex, float64 fSample);
	bool					GetSamples(size uiChannelIndex, size uiFirstFrame, size uiNumFrames, float64* pfDest);
	bool					SetSamples(size uiChannelIndex, size uiFirstFrame, size uiNumFrames, float64* pfSource);

	void					SetData(void* pvData);
	void*					GetData(void);
	size					GetByteSize(void);
	CChannels*				GetChannels(void);
	size					GetFrameByteStride(void);
	size					GetFrameBitStride(void);

	bool					IsSameFormat(Ptr<CSound> pcOther);
	bool					IsValid(size uiFrame);
	CChannel*				GetChannel(size iChannel);
	CChannel*				GetChannelAtIndex(size iIndex);
	size					GetIndexOfChannel(size iChannel);
	void					GetAllChannels(CArraySize* paiChannels);
	void					GetAllChannels(CArrayChannel* pasChannels);
	EPrimitiveType			GetPrimitiveType(void);  //Returns PT_Undefined if more than one.
	size					NumChannels(void);
	bool					HasChannel(size iChannel);
	bool					HasChannels(size iFirst, ...);
	bool					CanAccessSamples(void);

	void					Print(CChars* psz);
	void					Dump(void);

protected:
	void					PrivateInit(void);
	CChannel*				GetSampleChannel(size uiChannelIndex, size uiFirstFrame, size uiNumFrames);
};


typedef CArray<CSound> CArraySound;


float64	ConvertSampleToFloat(void* pvSample, EPrimitiveType eType);
float64	QuantiseSample(float64 fSample, float64 fScale, float64 fMin, float64 fMax);
void	ConvertSampleFromFloat(void* pvSample, EPrimitiveType eType, float64 fSample);
bool	IsSampleType(EPrimitiveType eType);


#endif // __SOUND_H__

