#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Sound.h"
#include "SupportLib/SoundResampler.h"
#include "SupportLib/SoundModifierStack.h"
#include "TestLib/Assert.h"


#define TEST_RESAMPLER_PI		3.14159265358979323846


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> MakeSine(uint32 uiSampleRate, lsize uiNumFrames, float64 fFrequency, float64 fAmplitude, EPrimitiveType eType)
{
	Ptr<CSound>	pcSound;
	lsize		i;

	pcSound = OMalloc<CSound>(uiSampleRate, uiNumFrames, eType, SOUND_MONO, CHANNEL_STOP);
	for (i = 0; i < uiNumFrames; i++)
	{
		pcSound->SetSample(i, 0, fAmplitude * sin(2.0 * TEST_RESAMPLER_PI * fFrequency * (float64)i / (float64)uiSampleRate));
	}
	return pcSound;
}


//////////////////////////////////////////////////////////////////////////
//
// The RMS difference between channel 0 and the sine wave it should be,
// away from the ends.
//
//////////////////////////////////////////////////////////////////////////
float64 SineError(Ptr<CSound> pcSound, float64 fFrequency, float64 fAmplitude)
{
	lsize	i;
	lsize	uiNumFrames;
	lsize	uiMargin;
	float64	fExpected;
	float64	fDifference;
	float64	fSum;
	lsize	uiCount;

	uiNumFrames = pcSound->GetNumFrames();
	uiMargin = uiNumFrames / 8;
	fSum = 0.0;
	uiCount = 0;
	for (i = uiMargin; i < uiNumFrames - uiMargin; i++)
	{
		fExpected = fAmplitude * sin(2.0 * TEST_RESAMPLER_PI * fFrequency * (float64)i / (float64)pcSound->GetSampleRate());
		fDifference = pcSound->GetSample(i, 0) - fExpected;
		fSum += fDifference * fDifference;
		uiCount++;
	}
	return sqrt(fSum / uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// The RMS level of channel 0 away from the ends.
//
//////////////////////////////////////////////////////////////////////////
float64 SoundLevel(Ptr<CSound> pcSound)
{
	return SineError(pcSound, 0.0, 0.0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 ResampledSineError(ESoundResample eResample, uint32 uiSourceRate, uint32 uiDestRate, float64 fFrequency)
{
	CSoundResampler	cResampler;
	float64			fError;

	Ptr<CSound> pcSource = MakeSine(uiSourceRate, uiSourceRate / 10, fFrequency, 0.8, PT_float64);
	cResampler.Init(eResample, uiDestRate);
	Ptr<CSound> pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();

	AssertInt((int32)uiDestRate, pcDest->GetSampleRate());
	AssertLSize(CSoundResampler::GetResampledNumFrames(uiSourceRate / 10, uiSourceRate, uiDestRate), pcDest->GetNumFrames());
	fError = SineError(pcDest, fFrequency, 0.8);

	pcSource = NULL;
	pcDest = NULL;
	return fError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerNumFrames(void)
{
	AssertLSize(48000, CSoundResampler::GetResampledNumFrames(44100, 44100, 48000));
	AssertLSize(44100, CSoundResampler::GetResampledNumFrames(48000, 48000, 44100));
	AssertLSize(3, CSoundResampler::GetResampledNumFrames(3, 48000, 44100));
	AssertLSize(1, CSoundResampler::GetResampledNumFrames(1, 48000, 8000));
	AssertLSize(0, CSoundResampler::GetResampledNumFrames(0, 48000, 8000));
	AssertLSize(0, CSoundResampler::GetResampledNumFrames(10, 0, 8000));
}


//////////////////////////////////////////////////////////////////////////
//
// The better the method the closer a resampled sine wave is to the sine
// wave at the new rate.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerSine(void)
{
	float64	fNearest;
	float64	fLinear;
	float64	fCubic;
	float64	fSinc;

	fNearest = ResampledSineError(SR_NearestNeighbour, 44100, 48000, 1000.0);
	fLinear = ResampledSineError(SR_Linear, 44100, 48000, 1000.0);
	fCubic = ResampledSineError(SR_Cubic, 44100, 48000, 1000.0);
	fSinc = ResampledSineError(SR_Sinc, 44100, 48000, 1000.0);
	AssertTrue(fNearest < 0.05);
	AssertTrue(fLinear < 2e-3);
	AssertTrue(fCubic < 1e-4);
	AssertTrue(fSinc < 1e-5);
	AssertTrue(fLinear < fNearest);
	AssertTrue(fCubic < fLinear);
	AssertTrue(fSinc < fCubic);

	//Down, with a tone the lower rate can hold.
	fSinc = ResampledSineError(SR_Sinc, 48000, 44100, 3000.0);
	AssertTrue(fSinc < 1e-5);
	fSinc = ResampledSineError(SR_Sinc, 48000, 16000, 3000.0);
	AssertTrue(fSinc < 1e-5);

	//Rates with no small ratio don't use a table of taps.
	fSinc = ResampledSineError(SR_Sinc, 44100, 44101, 1000.0);
	AssertTrue(fSinc < 1e-5);
}


//////////////////////////////////////////////////////////////////////////
//
// Going down, tones above the new Nyquist frequency are removed by sinc
// but fold back (alias) with linear.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerAntiAliasing(void)
{
	CSoundResampler	cResampler;

	Ptr<CSound> pcSource = MakeSine(48000, 4800, 15000.0, 0.8, PT_float32);

	cResampler.Init(SR_Sinc, 22050);
	Ptr<CSound> pcSinc = cResampler.Modify(pcSource);
	cResampler.Kill();

	cResampler.Init(SR_Linear, 22050);
	Ptr<CSound> pcLinear = cResampler.Modify(pcSource);
	cResampler.Kill();

	AssertTrue(SoundLevel(pcSinc) < 1e-4);
	AssertTrue(SoundLevel(pcLinear) > 0.1);

	pcSource = NULL;
	pcSinc = NULL;
	pcLinear = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Constant sound stays constant and a ramp is exact with linear.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerExact(void)
{
	CSoundResampler	cResampler;
	lsize			i;
	int				iMismatches;
	float64			fExpected;
	float64			fDifference;

	Ptr<CSound> pcSource = OMalloc<CSound>(8000, 800, PT_float64, SOUND_MONO, CHANNEL_STOP);
	for (i = 0; i < 800; i++)
	{
		pcSource->SetSample(i, 0, 0.25);
	}
	cResampler.Init(SR_Sinc, 11025);
	Ptr<CSound> pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();

	iMismatches = 0;
	for (i = 100; i < pcDest->GetNumFrames() - 100; i++)
	{
		fDifference = pcDest->GetSample(i, 0) - 0.25;
		if ((fDifference > 1e-12) || (fDifference < -1e-12))
		{
			iMismatches++;
		}
	}
	AssertInt(0, iMismatches);

	for (i = 0; i < 800; i++)
	{
		pcSource->SetSample(i, 0, (float64)i / 1024.0);
	}
	cResampler.Init(SR_Linear, 12000);
	pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();
	AssertLSize(1200, pcDest->GetNumFrames());

	iMismatches = 0;
	for (i = 0; i < 1198; i++)
	{
		fExpected = ((float64)i * 8000.0 / 12000.0) / 1024.0;
		fDifference = pcDest->GetSample(i, 0) - fExpected;
		if ((fDifference > 1e-12) || (fDifference < -1e-12))
		{
			iMismatches++;
		}
	}
	AssertInt(0, iMismatches);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The output has the source's channels and sample types.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerChannels(void)
{
	CSoundResampler	cResampler;
	size			i;
	float64			fError;
	float64			fExpected;
	float64			fDifference;

	Ptr<CSound> pcSource = OMalloc<CSound>(32000, 3200, PT_int16, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	for (i = 0; i < 3200; i++)
	{
		pcSource->SetSample(i, 0, 0.5 * sin(2.0 * TEST_RESAMPLER_PI * 440.0 * i / 32000.0));
		pcSource->SetSample(i, 1, -0.5);
	}

	cResampler.Init(SR_Sinc, 44100);
	Ptr<CSound> pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();

	AssertSize(2, pcDest->NumChannels());
	AssertTrue(pcDest->HasChannels(SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP));
	AssertInt(PT_int16, pcDest->GetPrimitiveType());
	AssertLSize(4410, pcDest->GetNumFrames());

	//16 bit samples are within a step or so of the sine wave.
	fError = 0.0;
	for (i = 500; i < 3900; i++)
	{
		fExpected = 0.5 * sin(2.0 * TEST_RESAMPLER_PI * 440.0 * i / 44100.0);
		fDifference = fabs(pcDest->GetSample(i, 0) - fExpected);
		if (fDifference > fError)
		{
			fError = fDifference;
		}
		fDifference = fabs(pcDest->GetSample(i, 1) + 0.5);
		if (fDifference > fError)
		{
			fError = fDifference;
		}
	}
	AssertTrue(fError < 2.0 / 32768.0);

	//The same rate returns the sound unchanged, as CImageResampler does for the same size.
	cResampler.Init(SR_Cubic, 32000);
	pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();
	AssertTrue(pcDest == pcSource);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerEdges(void)
{
	CSoundResampler	cResampler;
	SLogConfig		sLogConfig;
	Ptr<CSound>		pcDest;
	Ptr<CSound>		pcNull;

	Ptr<CSound> pcSource = OMalloc<CSound>(8000, 0, PT_int16, SOUND_MONO, CHANNEL_STOP);
	cResampler.Init(SR_Sinc, 16000);
	pcDest = cResampler.Modify(pcSource);
	AssertTrue(pcDest.IsNotNull());
	AssertLSize(0, pcDest->GetNumFrames());

	pcSource = OMalloc<CSound>(8000, 1, PT_int16, SOUND_MONO, CHANNEL_STOP);
	pcSource->SetSample(0, 0, 0.5);
	pcDest = cResampler.Modify(pcSource);
	AssertLSize(2, pcDest->GetNumFrames());
	cResampler.Kill();

	cResampler.Init(SR_Cubic, 16000);
	pcDest = cResampler.Modify(pcSource);
	AssertLSize(2, pcDest->GetNumFrames());
	AssertDouble(0.5, pcDest->GetSample(0, 0), 12);
	AssertDouble(0.5, pcDest->GetSample(1, 0), 12);
	cResampler.Kill();

	sLogConfig = gcLogger.SetSilent();
	cResampler.Init(SR_Linear, 0);
	AssertTrue(cResampler.Modify(pcSource).IsNull());
	cResampler.Kill();
	cResampler.Init(SR_Linear, 8000);
	AssertTrue(cResampler.Modify(pcNull).IsNull());
	cResampler.Kill();
	gcLogger.SetConfig(&sLogConfig);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The modifier stack resamples a sound in place, like an image's
// modifier stack.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerModifierStack(void)
{
	CSoundModifierStack		cStack;
	float64					fError;
	size					i;
	float64					fExpected;
	float64					fDifference;

	Ptr<CSound> pcSound = MakeSine(48000, 4800, 1000.0, 0.8, PT_int16);
	pcSound->RenameChannel(SOUND_MONO, SOUND_FRONT_CENTRE);
	Ptr<CSound> pcAlias = pcSound;

	cStack.Init(pcSound);
	cStack.AddModifier<CSoundResampler>(SR_Sinc, 32000);
	cStack.AddModifier<CSoundResampler>(SR_Cubic, 32000);	//Already at 32000; changes nothing.
	cStack.AddModifier<CSoundResampler>(SR_Sinc, 44100);
	AssertInt(3, cStack.NumModifiers());
	cStack.ApplyAll();

	//The sound itself changed, so everything pointing to it sees the new rate.
	AssertTrue(cStack.GetSound() == pcSound);
	AssertInt((int32)44100, pcAlias->GetSampleRate());
	AssertLSize(4410, pcAlias->GetNumFrames());
	AssertInt(PT_int16, pcAlias->GetPrimitiveType());
	AssertTrue(pcAlias->HasChannel(SOUND_FRONT_CENTRE));

	fError = 0.0;
	for (i = 500; i < 3900; i++)
	{
		fExpected = 0.8 * sin(2.0 * TEST_RESAMPLER_PI * 1000.0 * i / 44100.0);
		fDifference = fabs(pcAlias->GetSample(i, 0) - fExpected);
		if (fDifference > fError)
		{
			fError = fDifference;
		}
	}
	AssertTrue(fError < 3.0 / 32768.0);

	cStack.Kill();
	pcSound = NULL;
	pcAlias = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A modifier that fails leaves the sound as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerModifierStackFailure(void)
{
	CSoundModifierStack		cStack;
	SLogConfig				sLogConfig;

	Ptr<CSound> pcSound = OMalloc<CSound>(8000, 80, PT_int16, SOUND_MONO, CHANNEL_STOP);
	cStack.Init(pcSound);
	cStack.AddModifier<CSoundResampler>(SR_Linear, 16000);
	cStack.AddModifier<CSoundResampler>(SR_Linear, 0);

	sLogConfig = gcLogger.SetSilent();
	cStack.ApplyAll();
	gcLogger.SetConfig(&sLogConfig);

	AssertInt((int32)8000, pcSound->GetSampleRate());
	AssertLSize(80, pcSound->GetNumFrames());

	cStack.Kill();
	pcSound = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Bit sized sounds resample to the nearest code of their type.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResamplerBitty(void)
{
	CSoundResampler	cResampler;
	float64			afCodes[4] = { -1.0, 0.875, 0.0, 0.0 };
	float64			afRead[8];
	float64			afExpected[8] = { -1.0, -0.0625, 0.875, 0.4375, 0.0, 0.0, 0.0, 0.0 };
	size			i;
	int				iMismatches;

	Ptr<CSound> pcSource = OMalloc<CSound>(8000, 4, PT_nybble, SOUND_MONO, CHANNEL_STOP);
	AssertTrue(pcSource->SetSamples(0, 0, 4, afCodes));

	cResampler.Init(SR_Linear, 16000);
	Ptr<CSound> pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();

	AssertLSize(8, pcDest->GetNumFrames());
	AssertInt(PT_nybble, pcDest->GetPrimitiveType());
	AssertTrue(pcDest->GetSamples(0, 0, 8, afRead));

	//Every sample is a whole code and within half a code of the linear value.
	iMismatches = 0;
	for (i = 0; i < 8; i++)
	{
		if ((afRead[i] * 8.0 != floor(afRead[i] * 8.0)) || (fabs(afRead[i] - afExpected[i]) > 0.0625 + 1e-12))
		{
			iMismatches++;
		}
	}
	AssertInt(0, iMismatches);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundResampler(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestSoundResamplerNumFrames();
	TestSoundResamplerSine();
	TestSoundResamplerAntiAliasing();
	TestSoundResamplerExact();
	TestSoundResamplerChannels();
	TestSoundResamplerEdges();
	TestSoundResamplerModifierStack();
	TestSoundResamplerModifierStackFailure();
	TestSoundResamplerBitty();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

