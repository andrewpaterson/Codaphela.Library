#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Sound.h"
#include "SupportLib/SoundCel.h"
#include "SupportLib/SoundCelGroup.h"
#include "SupportLib/WAVWriter.h"
#include "SupportLib/WAVReader.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A mono int8 sound whose frame i is i / 128.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> MakeSoundCelRamp(size uiNumFrames)
{
	Ptr<CSound>	pcSound;
	size		i;

	pcSound = OMalloc<CSound>(8000, uiNumFrames, PT_int8, SOUND_MONO, CHANNEL_STOP);
	for (i = 0; i < uiNumFrames; i++)
	{
		pcSound->SetSample(i, 0, (float64)i / 128.0);
	}
	return pcSound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundCelInit(void)
{
	float64		af[4];
	SLogConfig	sLogConfig;

	Ptr<CSound> pcSound = MakeSoundCelRamp(100);

	Ptr<CSoundCel> pcWhole = OMalloc<CSoundCel>(pcSound);
	AssertLSize(0, pcWhole->GetFirstFrame());
	AssertLSize(100, pcWhole->GetNumFrames());
	AssertFalse(pcWhole->HasLoop());
	AssertLSize(SOUND_CEL_NO_LOOP, pcWhole->GetLoopFrame());
	AssertLSize(0, pcWhole->GetLoopNumFrames());
	AssertTrue(pcWhole->IsValid());
	AssertTrue(pcWhole->GetSourceSound() == pcSound);

	//Frames are counted from the start of the cel.
	Ptr<CSoundCel> pcCel = OMalloc<CSoundCel>(pcSound, 40, 20, 5);
	AssertTrue(pcCel->HasLoop());
	AssertLSize(5, pcCel->GetLoopFrame());
	AssertLSize(15, pcCel->GetLoopNumFrames());
	AssertDouble(40.0 / 128.0, pcCel->GetSample(0, 0), 12);
	AssertDouble(59.0 / 128.0, pcCel->GetSample(19, 0), 12);
	AssertTrue(pcCel->GetSamples(0, 2, 4, af));
	AssertDouble(42.0 / 128.0, af[0], 12);
	AssertDouble(45.0 / 128.0, af[3], 12);

	//A loop frame outside the cel means no loop.
	pcCel->SetLoopFrame(20);
	AssertFalse(pcCel->HasLoop());

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcCel->GetSamples(0, 18, 4, af));
	AssertFalse(pcCel->GetSamples(1, 0, 4, af));

	//A cel past the end of its sound.
	Ptr<CSoundCel> pcBad = OMalloc<CSoundCel>(pcSound, 90, 20);
	AssertFalse(pcBad->IsValid());
	AssertFalse(pcBad->GetSamples(0, 0, 4, af));
	gcLogger.SetConfig(&sLogConfig);

	pcSound = NULL;
	pcWhole = NULL;
	pcCel = NULL;
	pcBad = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A cel without a loop stops at its end; one with a loop goes back to its
// loop frame, as the SNES does.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundCelPlay(void)
{
	float64		af[24];
	size		i;
	int			iMismatches;
	float64		fExpected;

	Ptr<CSound> pcSound = MakeSoundCelRamp(30);

	//Frames 10 to 19, no loop.
	Ptr<CSoundCel> pcOnce = OMalloc<CSoundCel>(pcSound, 10, 10);
	AssertLSize(10, pcOnce->Play(0, 0, 24, af));
	iMismatches = 0;
	for (i = 0; i < 24; i++)
	{
		fExpected = i < 10 ? (float64)(10 + i) / 128.0 : 0.0;
		iMismatches += af[i] != fExpected ? 1 : 0;
	}
	AssertInt(0, iMismatches);
	AssertLSize(3, pcOnce->Play(0, 7, 5, af));
	AssertDouble(19.0 / 128.0, af[2], 12);
	AssertDouble(0.0, af[3], 12);
	AssertLSize(0, pcOnce->Play(0, 50, 5, af));

	//Frames 10 to 19, looping back to the cel's frame 6: 0 to 9 then 6 7 8 9 6 7 8 9...
	Ptr<CSoundCel> pcLoop = OMalloc<CSoundCel>(pcSound, 10, 10, 6);
	AssertLSize(24, pcLoop->Play(0, 0, 24, af));
	iMismatches = 0;
	for (i = 0; i < 24; i++)
	{
		fExpected = (float64)(10 + (i < 10 ? i : 6 + (i - 6) % 4)) / 128.0;
		iMismatches += af[i] != fExpected ? 1 : 0;
	}
	AssertInt(0, iMismatches);

	//Starting long after the cel started.
	AssertLSize(3, pcLoop->Play(0, 1001, 3, af));
	AssertDouble((10.0 + 6.0 + (1001 - 6) % 4) / 128.0, af[0], 12);
	AssertDouble((10.0 + 6.0 + (1002 - 6) % 4) / 128.0, af[1], 12);

	//Looping the whole cel.
	Ptr<CSoundCel> pcWhole = OMalloc<CSoundCel>(pcSound, 25, 5, 0);
	AssertLSize(12, pcWhole->Play(0, 0, 12, af));
	iMismatches = 0;
	for (i = 0; i < 12; i++)
	{
		iMismatches += af[i] != (float64)(25 + i % 5) / 128.0 ? 1 : 0;
	}
	AssertInt(0, iMismatches);

	pcSound = NULL;
	pcOnce = NULL;
	pcLoop = NULL;
	pcWhole = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundCelToSound(void)
{
	size	i;
	int		iMismatches;

	Ptr<CSound> pcSound = MakeSoundCelRamp(50);
	Ptr<CSoundCel> pcCel = OMalloc<CSoundCel>(pcSound, 20, 7);
	Ptr<CSound> pcCopy = pcCel->ToSound();

	AssertLSize(7, pcCopy->GetNumFrames());
	AssertInt((int32)8000, pcCopy->GetSampleRate());
	AssertInt(PT_int8, pcCopy->GetPrimitiveType());
	iMismatches = 0;
	for (i = 0; i < 7; i++)
	{
		iMismatches += pcCopy->GetSample(i, 0) != (float64)(20 + i) / 128.0 ? 1 : 0;
	}
	AssertInt(0, iMismatches);
	AssertTrue(pcCopy != pcSound);

	pcSound = NULL;
	pcCel = NULL;
	pcCopy = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A bank built like SNES audio RAM: sounds added one after another, each
// found by its index.  Samples are converted to the bank's int8.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundCelGroup(void)
{
	CSoundCelGroup	cGroup;
	SLogConfig		sLogConfig;
	size			i;
	int				iMismatches;
	float64			af[8];

	Ptr<CSound> pcBank = OMalloc<CSound>(16000, 0, PT_int8, SOUND_MONO, CHANNEL_STOP);
	cGroup.Init(pcBank);

	Ptr<CSound> pcFloat = OMalloc<CSound>(16000, 10, PT_float32, SOUND_MONO, CHANNEL_STOP);
	for (i = 0; i < 10; i++)
	{
		pcFloat->SetSample(i, 0, (float64)i / 16.0 - 0.5);
	}
	Ptr<CSound> pcInt16 = OMalloc<CSound>(16000, 6, PT_int16, SOUND_LEFT, CHANNEL_STOP);
	for (i = 0; i < 6; i++)
	{
		pcInt16->SetSample(i, 0, -(float64)i / 8.0);
	}
	Ptr<CSound> pcInt8 = MakeSoundCelRamp(4);
	pcInt8->SetSampleRate(16000);

	AssertSize(0, cGroup.AddSound(pcFloat));
	AssertSize(1, cGroup.AddSound(pcInt16, 2));
	AssertSize(2, cGroup.AddSound(pcInt8, 0));
	AssertSize(3, cGroup.NumCels());

	AssertTrue(cGroup.GetSound() == pcBank);
	AssertLSize(20, pcBank->GetNumFrames());
	AssertInt(PT_int8, pcBank->GetPrimitiveType());

	AssertLSize(0, cGroup.GetCel(0)->GetFirstFrame());
	AssertLSize(10, cGroup.GetCel(1)->GetFirstFrame());
	AssertLSize(16, cGroup.GetCel(2)->GetFirstFrame());
	AssertFalse(cGroup.GetCel(0)->HasLoop());
	AssertLSize(2, cGroup.GetCel(1)->GetLoopFrame());
	AssertLSize(0, cGroup.GetCel(2)->GetLoopFrame());

	//Cels added before the bank grew still read their samples.
	iMismatches = 0;
	for (i = 0; i < 10; i++)
	{
		iMismatches += cGroup.GetCel(0)->GetSample(i, 0) != (float64)i / 16.0 - 0.5 ? 1 : 0;
	}
	for (i = 0; i < 6; i++)
	{
		iMismatches += cGroup.GetCel(1)->GetSample(i, 0) != -(float64)i / 8.0 ? 1 : 0;
	}
	for (i = 0; i < 4; i++)
	{
		iMismatches += cGroup.GetCel(2)->GetSample(i, 0) != (float64)i / 128.0 ? 1 : 0;
	}
	AssertInt(0, iMismatches);

	//Cel 1 is 0 -1/8 -2/8 -3/8 -4/8 -5/8 looping from its frame 2.
	AssertLSize(8, cGroup.GetCel(1)->Play(0, 4, 8, af));
	AssertDouble(-4.0 / 8.0, af[0], 12);
	AssertDouble(-5.0 / 8.0, af[1], 12);
	AssertDouble(-2.0 / 8.0, af[2], 12);
	AssertDouble(-5.0 / 8.0, af[5], 12);
	AssertDouble(-2.0 / 8.0, af[6], 12);

	//A cel can also be added for frames already in the bank.
	AssertSize(3, cGroup.AddCel(5, 10));
	AssertDouble(5.0 / 16.0 - 0.5, cGroup.GetCel(3)->GetSample(0, 0), 12);

	sLogConfig = gcLogger.SetSilent();
	pcInt8->SetSampleRate(8000);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cGroup.AddSound(pcInt8));
	Ptr<CSound> pcStereo = OMalloc<CSound>(16000, 4, PT_int8, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cGroup.AddSound(pcStereo));
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cGroup.AddCel(15, 10));
	Ptr<CSoundCel> pcOther = OMalloc<CSoundCel>(pcStereo);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cGroup.AddCel(pcOther));
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(4, cGroup.NumCels());
	AssertLSize(20, pcBank->GetNumFrames());

	cGroup.Kill();
	pcBank = NULL;
	pcFloat = NULL;
	pcInt16 = NULL;
	pcInt8 = NULL;
	pcStereo = NULL;
	pcOther = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A bank of packed nybbles, and a bank saved as one WAV file.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundCelGroupBitty(void)
{
	CSoundCelGroup	cGroup;
	size			i;
	int				iMismatches;

	Ptr<CSound> pcBank = OMalloc<CSound>(8000, 0, PT_nybble, SOUND_MONO, CHANNEL_STOP);
	cGroup.Init(pcBank);
	AssertSize(0, cGroup.AddSound(MakeSoundCelRamp(5)));
	AssertSize(1, cGroup.AddSound(MakeSoundCelRamp(3)));
	AssertLSize(8, pcBank->GetNumFrames());
	AssertLSize(4, pcBank->GetByteSize());

	//i / 128 rounds to the nearest sixteenth: all of these are the middle code.
	iMismatches = 0;
	for (i = 0; i < 5; i++)
	{
		iMismatches += cGroup.GetCel(0)->GetSample(i, 0) != 0.0 ? 1 : 0;
	}
	AssertInt(0, iMismatches);

	AssertTrue(SaveWAV(pcBank, "Output" _FS_ "bank.wav"));
	Ptr<CSound> pcRead = LoadWAV("Output" _FS_ "bank.wav");
	AssertLSize(8, pcRead->GetNumFrames());

	cGroup.Kill();
	pcBank = NULL;
	pcRead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundCel(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestSoundCelInit();
	TestSoundCelPlay();
	TestSoundCelToSound();
	TestSoundCelGroup();
	TestSoundCelGroupBitty();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

