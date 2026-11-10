#include <stdlib.h>
#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/NativeThread.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Sound.h"
#include "SupportLib/SoundCel.h"
#include "WindowLib/Window.h"
#include "WindowLib/NativeSoundFactory.h"
#include "WindowLib/SoundMixer.h"
#include "WindowLib/SoundCelVoice.h"
#include "WindowLib/SoundStreamVoice.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "WaitTestRefWindow.h"


//The host's sound output for the tests: it keeps every frame written and says as many are queued as it is told to.
class CTestNativeSoundMixer : public CNativeSoundMixer
{
CONSTRUCTABLE(CTestNativeSoundMixer);
public:
	int16*	mpaiWritten;
	lsize	muiWrittenFrames;
	lsize	muiQueuedFrames;

	void Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory)
	{
		CNativeSoundMixer::Init(pcMixer, pcSoundFactory);
		mpaiWritten = NULL;
		muiWrittenFrames = 0;
		muiQueuedFrames = 0;
	}

	void Kill(void) override
	{
		free(mpaiWritten);
		mpaiWritten = NULL;
		CNativeSoundMixer::Kill();
	}

	bool Open(uint32 uiSampleRate) override { muiSampleRate = uiSampleRate; return true; }
	void Close(void) override { muiSampleRate = 0; }
	lsize NumQueuedFrames(void) override { return muiQueuedFrames; }

	void Write(int16* paiFrames, lsize uiFrames) override
	{
		mpaiWritten = (int16*)realloc(mpaiWritten, (size_t)((muiWrittenFrames + uiFrames) * 2 * sizeof(int16)));
		memcpy(&mpaiWritten[muiWrittenFrames * 2], paiFrames, (size_t)(uiFrames * 2 * sizeof(int16)));
		muiWrittenFrames += uiFrames;
	}

	int16 Left(lsize uiFrame) { return mpaiWritten[uiFrame * 2]; }
	int16 Right(lsize uiFrame) { return mpaiWritten[uiFrame * 2 + 1]; }
};


class CTestSoundFactory : public CNativeSoundFactory
{
CONSTRUCTABLE(CTestSoundFactory);
public:
	CNativeSoundMixer* CreateNativeSoundMixer(CSoundMixer* pcMixer) override
	{
		CTestNativeSoundMixer* pcNativeMixer;

		pcNativeMixer = NativeMalloc<CTestNativeSoundMixer>();
		pcNativeMixer->Init(pcMixer, this);
		return pcNativeMixer;
	}
};


//A stream that gives the frames it is handed, mpuiFrames at a time, as unsigned 8 bit left and right pairs.
class CTestSoundStreamVoice : public CSoundStreamVoice
{
CONSTRUCTABLE(CTestSoundStreamVoice);
DESTRUCTABLE(CTestSoundStreamVoice);
public:
	lsize	muiPending;
	uint8	muiLeft;
	uint8	muiRight;

	void Init(uint32 uiSampleRate, lsize uiBufferFrames)
	{
		PreInit();
		CSoundStreamVoice::Init(uiSampleRate, uiBufferFrames, PT_uint8, true);
		muiPending = 0;
		muiLeft = 128;
		muiRight = 128;
		PostInit();
	}

	void Class(void) override
	{
		CSoundStreamVoice::Class();
		U_LSize(muiPending);
		U_UInt8(muiLeft);
		U_UInt8(muiRight);
	}

	lsize Fill(CSound* pcBuffer) override
	{
		lsize	uiFrames;
		uint8*	pui;
		lsize	i;

		uiFrames = muiPending < pcBuffer->GetNumFrames() ? muiPending : pcBuffer->GetNumFrames();
		pui = (uint8*)pcBuffer->GetData();
		for (i = 0; i < uiFrames; i++)
		{
			pui[i * 2] = muiLeft;
			pui[i * 2 + 1] = muiRight;
		}
		muiPending -= uiFrames;
		return uiFrames;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> MakeTestMonoSound(uint32 uiSampleRate, lsize uiFrames, int16* paiSamples)
{
	Ptr<CSound>	pSound;
	lsize		i;
	int16*		pai;

	pSound = OMalloc<CSound>(uiSampleRate, uiFrames, PT_int16, SOUND_MONO, CHANNEL_STOP);
	pai = (int16*)pSound->GetData();
	for (i = 0; i < uiFrames; i++)
	{
		pai[i] = paiSamples[i];
	}
	return pSound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixerSilence(void)
{
	CTestSoundFactory		cSoundFactory;
	Ptr<CSoundMixer>		pMixer;
	CTestNativeSoundMixer*	pcNative;
	lsize					i;
	bool					bSilent;

	cSoundFactory.Init(&gcMemoryAllocator);
	{
		pMixer = OMalloc<CSoundMixer>(&cSoundFactory, (uint32)48000);
		pcNative = (CTestNativeSoundMixer*)pMixer->GetNativeMixer();
		AssertTrue(pMixer->IsOpen());
		AssertInt(48000, pcNative->GetSampleRate());

		//The lead is 100ms: with nothing queued that's 4800 frames.
		pMixer->Mix();
		AssertLSize(4800, pcNative->muiWrittenFrames);
		bSilent = true;
		for (i = 0; i < 4800; i++)
		{
			bSilent &= (pcNative->Left(i) == 0) && (pcNative->Right(i) == 0);
		}
		AssertTrue(bSilent);

		//With 4000 still queued only 800 more are needed.
		pcNative->muiQueuedFrames = 4000;
		pMixer->Mix();
		AssertLSize(5600, pcNative->muiWrittenFrames);

		//Enough queued: nothing is mixed.
		pcNative->muiQueuedFrames = 4800;
		pMixer->Mix();
		AssertLSize(5600, pcNative->muiWrittenFrames);
		AssertLong(5600, pMixer->NumFramesMixed());

		pMixer = NULL;
	}
	cSoundFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixerCelVoice(void)
{
	CTestSoundFactory		cSoundFactory;
	Ptr<CSoundMixer>		pMixer;
	CTestNativeSoundMixer*	pcNative;
	Ptr<CSoundCelVoice>		pVoice;
	int16					aiSamples[1000];
	lsize					i;
	bool					bSame;

	cSoundFactory.Init(&gcMemoryAllocator);
	{
		for (i = 0; i < 1000; i++)
		{
			aiSamples[i] = 16384;
		}
		pMixer = OMalloc<CSoundMixer>(&cSoundFactory, (uint32)48000);
		pcNative = (CTestNativeSoundMixer*)pMixer->GetNativeMixer();

		pVoice = OMalloc<CSoundCelVoice>(MakeTestMonoSound(48000, 1000, aiSamples), (lsize)SOUND_CEL_NO_LOOP);
		pMixer->AddVoice(pVoice);
		AssertSize(1, pMixer->NumVoices());

		//A mono sound plays on both sides.
		pMixer->Mix(600);
		bSame = true;
		for (i = 0; i < 600; i++)
		{
			bSame &= (pcNative->Left(i) == 16384) && (pcNative->Right(i) == 16384);
		}
		AssertTrue(bSame);
		AssertSize(1, pMixer->NumVoices());
		AssertFalse(pVoice->IsFinished());

		//It ends 400 frames into the next mix and is taken out.
		pMixer->Mix(600);
		AssertShort(16384, pcNative->Left(999));
		AssertShort(0, pcNative->Left(1000));
		AssertShort(0, pcNative->Right(1199));
		AssertTrue(pVoice->IsFinished());
		AssertSize(0, pMixer->NumVoices());

		//Half volume.
		pVoice = OMalloc<CSoundCelVoice>(MakeTestMonoSound(48000, 1000, aiSamples), (lsize)SOUND_CEL_NO_LOOP);
		pVoice->SetVolume(0.5);
		pMixer->AddVoice(pVoice);
		pMixer->Mix(10);
		AssertShort(8192, pcNative->Left(1200));
		AssertTrue(pMixer->RemoveVoice(&pVoice));
		AssertSize(0, pMixer->NumVoices());

		pVoice = NULL;
		pMixer = NULL;
	}
	cSoundFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixerLoopAndResample(void)
{
	CTestSoundFactory		cSoundFactory;
	Ptr<CSoundMixer>		pMixer;
	CTestNativeSoundMixer*	pcNative;
	Ptr<CSoundCelVoice>		pVoice;
	int16					aiSamples[4] = { 0, 1000, 2000, 3000 };
	int16					aiLooped[10] = { 0, 1000, 2000, 3000, 2000, 3000, 2000, 3000, 2000, 3000 };
	int16					aiResampled[8] = { 0, 500, 1000, 1500, 2000, 2500, 3000, 1500 };
	lsize					i;

	cSoundFactory.Init(&gcMemoryAllocator);
	{
		pMixer = OMalloc<CSoundMixer>(&cSoundFactory, (uint32)48000);
		pcNative = (CTestNativeSoundMixer*)pMixer->GetNativeMixer();

		//Going back to frame 2 each time it gets to the end; split over two mixes.
		pVoice = OMalloc<CSoundCelVoice>(MakeTestMonoSound(48000, 4, aiSamples), (lsize)2);
		pMixer->AddVoice(pVoice);
		pMixer->Mix(3);
		pMixer->Mix(7);
		for (i = 0; i < 10; i++)
		{
			AssertShort(aiLooped[i], pcNative->Left(i));
		}
		AssertFalse(pVoice->IsFinished());
		pMixer->RemoveAllVoices();

		//At half the mixer's rate each frame is followed by one halfway to the next.  The last goes halfway to the
		//silence after the sound.
		pVoice = OMalloc<CSoundCelVoice>(MakeTestMonoSound(24000, 4, aiSamples), (lsize)SOUND_CEL_NO_LOOP);
		pMixer->AddVoice(pVoice);
		pMixer->Mix(8);
		for (i = 0; i < 8; i++)
		{
			AssertShort(aiResampled[i], pcNative->Left(10 + i));
		}
		AssertTrue(pVoice->IsFinished());
		AssertSize(0, pMixer->NumVoices());

		pVoice = NULL;
		pMixer = NULL;
	}
	cSoundFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixerStreamVoice(void)
{
	CTestSoundFactory			cSoundFactory;
	Ptr<CSoundMixer>			pMixer;
	CTestNativeSoundMixer*		pcNative;
	Ptr<CTestSoundStreamVoice>	pVoice;

	cSoundFactory.Init(&gcMemoryAllocator);
	{
		pMixer = OMalloc<CSoundMixer>(&cSoundFactory, (uint32)48000);
		pcNative = (CTestNativeSoundMixer*)pMixer->GetNativeMixer();
		pVoice = OMalloc<CTestSoundStreamVoice>((uint32)48000, (lsize)256);
		pVoice->SetLatency(10, 20);  //480 and 960 frames.
		pVoice->muiLeft = 192;  //0.5
		pVoice->muiRight = 64;  //-0.5
		pMixer->AddVoice(&pVoice);

		//Not far enough ahead to start: silence, and the frames wait.
		pVoice->muiPending = 400;
		pMixer->Mix(100);
		AssertShort(0, pcNative->Left(0));
		AssertLSize(400, pVoice->NumWaitingFrames());
		AssertLong(0, pVoice->NumFramesPlayed());

		//Far enough ahead: it plays.
		pVoice->muiPending = 200;
		pMixer->Mix(100);
		AssertShort(16384, pcNative->Left(100));
		AssertShort(-16384, pcNative->Right(100));
		AssertShort(16384, pcNative->Left(199));
		AssertLong(100, pVoice->NumFramesPlayed());
		AssertLSize(500, pVoice->NumWaitingFrames());

		//Runs out: the last waiting frame is kept to play from, and it starts again once far enough ahead.
		pMixer->Mix(600);
		AssertLong(1, pVoice->NumUnderruns());
		AssertLong(599, pVoice->NumFramesPlayed());
		AssertLSize(1, pVoice->NumWaitingFrames());
		AssertShort(16384, pcNative->Left(200 + 498));
		AssertShort(0, pcNative->Left(200 + 499));

		//Too far ahead: dropped back to the latency.
		pVoice->muiPending = 1000;
		pMixer->Mix(10);
		AssertLong(521, pVoice->NumDroppedFrames());
		AssertLSize(470, pVoice->NumWaitingFrames());
		AssertShort(16384, pcNative->Left(800));
		AssertSize(1, pMixer->NumVoices());

		pVoice = NULL;
		pMixer = NULL;
	}
	cSoundFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How many frames the mixer has written to the test's host, which its
// thread writes under its lock.
//
//////////////////////////////////////////////////////////////////////////
static lsize NumTestFramesWritten(CSoundMixer* pcMixer)
{
	lsize	uiFrames;

	pcMixer->Lock();
	uiFrames = ((CTestNativeSoundMixer*)pcMixer->GetNativeMixer())->muiWrittenFrames;
	pcMixer->Unlock();
	return uiFrames;
}


//////////////////////////////////////////////////////////////////////////
//
// While a window is shown its mixer keeps the host a lead ahead on a
// thread of its own: the first lead is mixed as it starts and, as the
// test's output never plays, a whole lead each time the thread wakes.  It
// stops with the window.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixerWindow(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CTestSoundFactory		cSoundFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "SoundMixerWindow";

	//The reference window writes each frame it paints here.
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	cNativeFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory);
	cSoundFactory.Init(&gcMemoryAllocator);
	{
		Ptr<CWindow>				pWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		lsize						uiWritten;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 2);
		pWindow = OMalloc<CWindow>("Sound Window", &cNativeFactory, &cSoundFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		AssertTrue(pWindow->GetSoundMixer().IsNotNull());
		AssertInt(SOUND_MIXER_DEFAULT_SAMPLE_RATE, pWindow->GetSoundMixer()->GetSampleRate());
		AssertFalse(pWindow->GetSoundMixer()->IsRunning());

		pWindow->Show();
		AssertInt(2, cData.iTick);
		AssertFalse(pWindow->GetSoundMixer()->IsRunning());
		uiWritten = NumTestFramesWritten(&pWindow->GetSoundMixer());
		AssertTrue(uiWritten >= 4800);
		AssertLSize(0, uiWritten % 4800);

		//Stopped, it mixes no more.
		ThreadSleep(SOUND_MIXER_DEFAULT_LEAD_MS);
		AssertLSize(uiWritten, NumTestFramesWritten(&pWindow->GetSoundMixer()));

		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;

		//A window without a sound factory has no mixer.
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Quiet Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		AssertTrue(pWindow->GetSoundMixer().IsNull());
		pWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cSoundFactory.Kill();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//What the window's thread saw of the mixer while it was waiting for events.
struct STestSoundWaiting
{
	lsize	uiBefore;
	lsize	uiAfter;
};


//////////////////////////////////////////////////////////////////////////
//
// The window's thread sits in its wait for most of a second's lead, as a
// busy or blocked thread would, and stops the window.
//
//////////////////////////////////////////////////////////////////////////
static bool WaitSoundWindow(Ptr<CWindow> pWindow, size uiWait, int64 iMilliseconds, void* pvContext)
{
	STestSoundWaiting*	psWaiting;

	psWaiting = (STestSoundWaiting*)pvContext;
	psWaiting->uiBefore = NumTestFramesWritten(&pWindow->GetSoundMixer());
	ThreadSleep(SOUND_MIXER_DEFAULT_LEAD_MS * 3);
	psWaiting->uiAfter = NumTestFramesWritten(&pWindow->GetSoundMixer());
	pWindow->Stop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A window waiting for events doesn't wake to mix its sound, even while a
// voice is playing: its mixer's thread mixes while it waits.
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixerWindowWaits(void)
{
	CWaitTestRefWindowFactory	cNativeFactory;
	CTestSoundFactory			cSoundFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "SoundMixerWindowWaits";
	int16						aiSamples[1000];
	lsize						i;
	STestSoundWaiting			sWaiting;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	sWaiting.uiBefore = 0;
	sWaiting.uiAfter = 0;
	cNativeFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, WaitSoundWindow, &sWaiting);
	cSoundFactory.Init(&gcMemoryAllocator);
	{
		Ptr<CWindow>				pWindow;
		Ptr<CCountingTick>			pTick;
		CPointer					pNull;
		CWaitTestRefWindow*			pcNativeWindow;

		for (i = 0; i < 1000; i++)
		{
			aiSamples[i] = 1000;
		}
		pTick = OMalloc<CCountingTick>(0, 0, -1);
		pWindow = OMalloc<CWindow>("Sound Window", &cNativeFactory, &cSoundFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);

		//A looping voice never finishes.
		pWindow->GetSoundMixer()->AddVoice(OMalloc<CSoundCelVoice>(MakeTestMonoSound(48000, 1000, aiSamples), (lsize)0));
		AssertTrue(pWindow->Show());
		pcNativeWindow = (CWaitTestRefWindow*)pWindow->GetNativeWindow();
		AssertSize(1, pcNativeWindow->NumWaits());
		AssertLong(-1, pcNativeWindow->GetWait(0));
		AssertTrue(sWaiting.uiBefore >= 4800);
		AssertTrue(sWaiting.uiAfter > sWaiting.uiBefore);

		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cSoundFactory.Kill();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSoundMixer(void)
{
	BeginTests();
	ObjectsInit();

	TestSoundMixerSilence();
	TestSoundMixerCelVoice();
	TestSoundMixerLoopAndResample();
	TestSoundMixerStreamVoice();
	TestSoundMixerWindow();
	TestSoundMixerWindowWaits();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

