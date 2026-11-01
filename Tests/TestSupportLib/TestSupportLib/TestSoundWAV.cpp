#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Sound.h"
#include "SupportLib/WAVReader.h"
#include "SupportLib/WAVWriter.h"
#include "TestLib/Assert.h"


//The sample patterns written by Input/WAV/MakeWAV.py.
//The arithmetic is in 32 bits (as in Python) so a 16 bit size doesn't wrap it.
int32 ExpectedWAV8(lsize i, size c)		{ return (int32)(((uint32)i * 7 + (uint32)c * 50) & 0xFF); }
int32 ExpectedWAV16(lsize i, size c)		{ return (int32)(((uint32)i * 1237 + (uint32)c * 9001) & 0xFFFF) - 32768; }
int32 ExpectedWAV24(lsize i, size c)		{ return (int32)(((uint32)i * 76543 + (uint32)c * 1000003) & 0xFFFFFF) - 0x800000; }
int32 ExpectedWAV32(lsize i, size c)		{ return (int32)((uint32)(((uint64)i * 123456789 + (uint64)c * 987654321) & 0xFFFFFFFF) ^ 0x80000000); }
float64 ExpectedWAVFloat32(lsize i, size c)	{ return (float64)((int32)(((uint32)i * 37 + (uint32)c * 11) % 200) - 100) / 128.0; }
float64 ExpectedWAVFloat64(lsize i, size c)	{ return (float64)((int32)(((uint32)i * 12345 + (uint32)c * 777) % 100000) - 50000) / 65536.0; }


//////////////////////////////////////////////////////////////////////////
//
// The sample for frame i, channel c in the files made by MakeWAV.py.
//
//////////////////////////////////////////////////////////////////////////
float64 ExpectedWAVSample(int iBits, bool bFloat, lsize i, size c)
{
	if (bFloat)
	{
		return iBits == 32 ? ExpectedWAVFloat32(i, c) : ExpectedWAVFloat64(i, c);
	}
	switch (iBits)
	{
	case 8:
		return (ExpectedWAV8(i, c) - 128) / 128.0;
	case 16:
		return ExpectedWAV16(i, c) / 32768.0;
	case 24:
		return ExpectedWAV24(i, c) / 8388608.0;
	default:
		return ExpectedWAV32(i, c) / 2147483648.0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertWAV(const char* szName, uint32 uiSampleRate, lsize uiNumFrames, EPrimitiveType eType, int iBits, bool bFloat, EChannel eFirst, ...)
{
	CChars		szFile;
	va_list		vaMarker;
	EChannel	eChannel;
	size		uiNumChannels;
	lsize		i;
	size		c;
	int			iMismatches;

	szFile.Init("Input" _FS_ "WAV" _FS_);
	szFile.Append(szName);

	Ptr<CSound> pcSound = LoadWAV(szFile.Text());
	szFile.Kill();
	AssertTrue(pcSound.IsNotNull());
	if (pcSound.IsNull())
	{
		return;
	}

	AssertInt((int32)uiSampleRate, pcSound->GetSampleRate());
	AssertLSize(uiNumFrames, pcSound->GetNumFrames());
	AssertInt(eType, pcSound->GetPrimitiveType());

	uiNumChannels = 0;
	eChannel = eFirst;
	va_start(vaMarker, eFirst);
	while (eChannel != CHANNEL_STOP)
	{
		if (uiNumChannels < pcSound->NumChannels())
		{
			AssertSize(eChannel, pcSound->GetChannelAtIndex(uiNumChannels)->iChannel);
		}
		uiNumChannels++;
		eChannel = VA_ARG_SIZE(vaMarker);
	}
	va_end(vaMarker);
	AssertSize(uiNumChannels, pcSound->NumChannels());

	iMismatches = 0;
	for (i = 0; i < pcSound->GetNumFrames(); i++)
	{
		for (c = 0; c < pcSound->NumChannels(); c++)
		{
			if (pcSound->GetSample(i, c) != ExpectedWAVSample(iBits, bFloat, i, c))
			{
				iMismatches++;
			}
		}
	}
	AssertInt(0, iMismatches);

	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAVReader(void)
{
	AssertWAV("pcm8mono.wav", 8000, 101, PT_uint8, 8, false, SOUND_MONO, CHANNEL_STOP);
	AssertWAV("pcm16stereo.wav", 44100, 1000, PT_int16, 16, false, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	AssertWAV("pcm24stereo.wav", 48000, 500, PT_int32, 24, false, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	AssertWAV("pcm32centre.wav", 96000, 300, PT_int32, 32, false, SOUND_FRONT_CENTRE, CHANNEL_STOP);
	AssertWAV("float32stereo.wav", 22050, 200, PT_float32, 32, true, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	AssertWAV("float64mono.wav", 11025, 150, PT_float64, 64, true, SOUND_MONO, CHANNEL_STOP);
	AssertWAV("pcm16surround51.wav", 48000, 100, PT_int16, 16, false, SOUND_FRONT_LEFT, SOUND_FRONT_RIGHT, SOUND_FRONT_CENTRE, SOUND_LOW_FREQUENCY, SOUND_BACK_LEFT, SOUND_BACK_RIGHT, CHANNEL_STOP);
	AssertWAV("pcm24in32.wav", 48000, 100, PT_int32, 24, false, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	AssertWAV("pcm16quad.wav", 44100, 50, PT_int16, 16, false, SOUND_FRONT_LEFT, SOUND_FRONT_RIGHT, SOUND_BACK_LEFT, SOUND_BACK_RIGHT, CHANNEL_STOP);
	AssertWAV("pcm16discrete.wav", 44100, 60, PT_int16, 16, false, SOUND_LEFT, SOUND_RIGHT, SOUND_DISCRETE(0), CHANNEL_STOP);
	AssertWAV("streamed.wav", 16000, 77, PT_int16, 16, false, SOUND_MONO, CHANNEL_STOP);
	AssertWAV("wavemodule.wav", 32000, 400, PT_int16, 16, false, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertWAVRejected(const char* szName)
{
	CChars	szFile;

	szFile.Init("Input" _FS_ "WAV" _FS_ "Bad" _FS_);
	szFile.Append(szName);

	Ptr<CSound> pcSound = LoadWAV(szFile.Text());
	AssertTrue(pcSound.IsNull());

	pcSound = NULL;
	szFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Malformed files must be rejected without reading past the file.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAVBadFiles(void)
{
	SLogConfig	sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertWAVRejected("empty.wav");
	AssertWAVRejected("tooshort.wav");
	AssertWAVRejected("badriff.wav");
	AssertWAVRejected("badwave.wav");
	AssertWAVRejected("nofmt.wav");
	AssertWAVRejected("nodata.wav");
	AssertWAVRejected("shortfmt.wav");
	AssertWAVRejected("fmtpastend.wav");
	AssertWAVRejected("badbits.wav");
	AssertWAVRejected("badblockalign.wav");
	AssertWAVRejected("zerochannels.wav");
	AssertWAVRejected("zerorate.wav");
	AssertWAVRejected("adpcm.wav");
	AssertWAVRejected("float16.wav");
	AssertWAVRejected("badguid.wav");
	AssertWAVRejected("shortextensible.wav");
	AssertWAVRejected("twofmt.wav");
	AssertWAVRejected("missing.wav");
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
// A sound whose samples are exactly representable in 8 bits and up.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> MakeWAVTestSound(uint32 uiSampleRate, size uiNumFrames, EPrimitiveType eType, size uiNumChannels)
{
	Ptr<CSound>	pcSound;
	size		i;
	size		c;

	pcSound = OMalloc<CSound>();
	pcSound->BeginChange();
	for (c = 0; c < uiNumChannels; c++)
	{
		pcSound->AddChannel(SOUND_SPEAKER(SS_FrontLeft + c), eType);
	}
	pcSound->SetSampleRate(uiSampleRate);
	pcSound->SetNumFrames(uiNumFrames);
	pcSound->EndChange();

	for (i = 0; i < uiNumFrames; i++)
	{
		for (c = 0; c < uiNumChannels; c++)
		{
			pcSound->SetSample(i, c, (float64)((int)((i * 31 + c * 17) % 255) - 127) / 128.0);
		}
	}
	return pcSound;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes pcSound, reads it back and checks the samples are within
// fTolerance and the channels are the same.
//
//////////////////////////////////////////////////////////////////////////
void AssertWAVRoundTrip(Ptr<CSound> pcSound, const char* szName, EWAVSampleFormat eFormat, EPrimitiveType eReadType, float64 fTolerance)
{
	CChars	szFile;
	lsize	i;
	size	c;
	int		iMismatches;
	int		iChannelMismatches;
	float64	fDifference;

	szFile.Init("Output" _FS_);
	szFile.Append(szName);
	AssertTrue(SaveWAV(pcSound, szFile.Text(), eFormat));
	Ptr<CSound> pcRead = LoadWAV(szFile.Text());
	szFile.Kill();
	AssertTrue(pcRead.IsNotNull());
	if (pcRead.IsNull())
	{
		return;
	}

	AssertInt((int32)pcSound->GetSampleRate(), pcRead->GetSampleRate());
	AssertLSize(pcSound->GetNumFrames(), pcRead->GetNumFrames());
	AssertSize(pcSound->NumChannels(), pcRead->NumChannels());
	AssertInt(eReadType, pcRead->GetPrimitiveType());

	iChannelMismatches = 0;
	for (c = 0; c < pcSound->NumChannels(); c++)
	{
		if (pcSound->GetChannelAtIndex(c)->iChannel != pcRead->GetChannelAtIndex(c)->iChannel)
		{
			iChannelMismatches++;
		}
	}
	AssertInt(0, iChannelMismatches);

	iMismatches = 0;
	for (i = 0; i < pcSound->GetNumFrames(); i++)
	{
		for (c = 0; c < pcSound->NumChannels(); c++)
		{
			fDifference = pcSound->GetSample(i, c) - pcRead->GetSample(i, c);
			if ((fDifference > fTolerance) || (fDifference < -fTolerance))
			{
				iMismatches++;
			}
		}
	}
	AssertInt(0, iMismatches);

	pcRead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAVWriter(void)
{
	Ptr<CSound>	pcSound;
	size		i;

	pcSound = MakeWAVTestSound(8000, 333, PT_uint8, 1);
	pcSound->RenameChannel(SOUND_FRONT_LEFT, SOUND_MONO);
	AssertWAVRoundTrip(pcSound, "mono8.wav", WSF_Default, PT_uint8, 0.0);

	pcSound = MakeWAVTestSound(44100, 1000, PT_int16, 2);
	AssertWAVRoundTrip(pcSound, "stereo16.wav", WSF_Default, PT_int16, 0.0);

	pcSound = MakeWAVTestSound(48000, 500, PT_int32, 2);
	AssertWAVRoundTrip(pcSound, "stereo24.wav", WSF_PCM24, PT_int32, 0.0);
	AssertWAVRoundTrip(pcSound, "stereo32.wav", WSF_Default, PT_int32, 0.0);

	pcSound = MakeWAVTestSound(96000, 250, PT_float32, 6);
	AssertWAVRoundTrip(pcSound, "surround32f.wav", WSF_Default, PT_float32, 0.0);

	pcSound = MakeWAVTestSound(22050, 77, PT_float64, 1);
	pcSound->RenameChannel(SOUND_FRONT_LEFT, SOUND_MONO);
	AssertWAVRoundTrip(pcSound, "mono64f.wav", WSF_Default, PT_float64, 0.0);

	//Float written as 16 bit is rounded to the nearest step.
	pcSound = OMalloc<CSound>(44100, 1000, PT_float32, SOUND_MONO, CHANNEL_STOP);
	for (i = 0; i < 1000; i++)
	{
		pcSound->SetSample(i, 0, sin(i * 0.05) * 0.99);
	}
	AssertWAVRoundTrip(pcSound, "float16.wav", WSF_PCM16, PT_int16, 0.5 / 32768.0 + 1e-9);

	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Speakers are written in channel mask order, then the other channels.
// Ignored channels are left out.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAVWriterChannelOrder(void)
{
	size	i;
	int		iMismatches;
	bool	bResult;

	Ptr<CSound> pcSound = OMalloc<CSound>(44100, 64, PT_int16, SOUND_DISCRETE(0), SOUND_IGNORED, SOUND_RIGHT, SOUND_LOW_FREQUENCY, SOUND_LEFT, CHANNEL_STOP);
	for (i = 0; i < 64; i++)
	{
		pcSound->SetSample(i, 0, 0.5);
		pcSound->SetSample(i, 1, 0.75);
		pcSound->SetSample(i, 2, -0.25);
		pcSound->SetSample(i, 3, i / 128.0);
		pcSound->SetSample(i, 4, -0.5);
	}
	bResult = SaveWAV(pcSound, "Output" _FS_ "order.wav");
	AssertTrue(bResult);

	Ptr<CSound> pcRead = LoadWAV("Output" _FS_ "order.wav");
	AssertSize(4, pcRead->NumChannels());
	AssertSize(SOUND_LEFT, pcRead->GetChannelAtIndex(0)->iChannel);
	AssertSize(SOUND_RIGHT, pcRead->GetChannelAtIndex(1)->iChannel);
	AssertSize(SOUND_LOW_FREQUENCY, pcRead->GetChannelAtIndex(2)->iChannel);
	AssertSize(SOUND_DISCRETE(0), pcRead->GetChannelAtIndex(3)->iChannel);

	iMismatches = 0;
	for (i = 0; i < 64; i++)
	{
		if ((pcRead->GetSample(i, 0) != -0.5) || (pcRead->GetSample(i, 1) != -0.25) || (pcRead->GetSample(i, 2) != i / 128.0) || (pcRead->GetSample(i, 3) != 0.5))
		{
			iMismatches++;
		}
	}
	AssertInt(0, iMismatches);

	pcSound = NULL;
	pcRead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAVWriterBad(void)
{
	SLogConfig	sLogConfig;
	Ptr<CSound>	pcSound;

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(SaveWAV(pcSound, "Output" _FS_ "null.wav"));

	pcSound = OMalloc<CSound>(8000, 10, PT_int16, SOUND_IGNORED, CHANNEL_STOP);
	AssertFalse(SaveWAV(pcSound, "Output" _FS_ "ignored.wav"));

	pcSound = OMalloc<CSound>(8000, 10, PT_int64, SOUND_MONO, CHANNEL_STOP);
	AssertFalse(SaveWAV(pcSound, "Output" _FS_ "int64.wav"));
	gcLogger.SetConfig(&sLogConfig);

	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Bit sized sounds are written as 8 bit PCM.  Every code is exact in 8
// bits (a nybble's n is 16n).
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAVWriterBitty(void)
{
	size		i;
	uint8*		pui;
	int			iMismatches;

	Ptr<CSound> pcSound = OMalloc<CSound>(16000, 16, PT_nybble, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	for (i = 0; i < 16; i++)
	{
		pcSound->SetSample(i, 0, ((float64)i - 8.0) / 8.0);
		pcSound->SetSample(i, 1, (7.0 - (float64)i) / 8.0);
	}
	AssertWAVRoundTrip(pcSound, "nybble.wav", WSF_Default, PT_uint8, 0.0);

	Ptr<CSound> pcRead = LoadWAV("Output" _FS_ "nybble.wav");
	pui = (uint8*)pcRead->GetData();
	iMismatches = 0;
	for (i = 0; i < 16; i++)
	{
		iMismatches += pui[i * 2] != i * 16 ? 1 : 0;
		iMismatches += pui[i * 2 + 1] != (15 - i) * 16 ? 1 : 0;
	}
	AssertInt(0, iMismatches);

	pcSound = MakeWAVTestSound(8000, 100, PT_sixbits, 1);
	for (i = 0; i < 100; i++)
	{
		pcSound->SetSample(i, 0, (float64)((int)(i % 64) - 32) / 32.0);
	}
	AssertWAVRoundTrip(pcSound, "sixbits.wav", WSF_Default, PT_uint8, 0.0);
	AssertWAVRoundTrip(pcSound, "sixbits16.wav", WSF_PCM16, PT_int16, 0.0);

	pcSound = NULL;
	pcRead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundWAV(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestSoundWAVReader();
	TestSoundWAVBadFiles();
	TestSoundWAVWriter();
	TestSoundWAVWriterChannelOrder();
	TestSoundWAVWriterBad();
	TestSoundWAVWriterBitty();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

