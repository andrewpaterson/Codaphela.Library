#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "ProceduralLib/SolidAdjust.h"
#include "ProceduralLib/SolidCells.h"
#include "ProceduralLib/SolidFlat.h"
#include "ProceduralLib/SolidGradient.h"
#include "ProceduralLib/SolidMask.h"
#include "ProceduralLib/SolidMerge.h"
#include "ProceduralLib/SolidNoise.h"
#include "ProceduralLib/SolidRamp.h"
#include "TestLib/Assert.h"
#include "TextureGallery.h"


//////////////////////////////////////////////////////////////////////////
//
// A sample of eight numbers that are all different.
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidSample(SSolidSample* psSample, float32 fFirst, float32 fStep)
{
	float32*	pafChannels;
	size		i;

	pafChannels = psSample->Channels();
	for (i = 0; i < SOLID_SAMPLE_CHANNELS; i++)
	{
		pafChannels[i] = fFirst + ((float32)i * fStep);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSolidSample(SSolidSample* psExpected, SSolidSample* psActual, int iDecimals)
{
	float32*	pafExpected;
	float32*	pafActual;
	size		i;

	pafExpected = psExpected->Channels();
	pafActual = psActual->Channels();
	for (i = 0; i < SOLID_SAMPLE_CHANNELS; i++)
	{
		AssertFloat(pafExpected[i], pafActual[i], iDecimals);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidSampleAt(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z, SSolidSample* psSample)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNormal;

	sPoint.Init(x, y, z);
	sNormal.Init(0, 0, 1);
	pTexture->Sample(&sPoint, &sNormal, 0.0f, psSample);
}


//////////////////////////////////////////////////////////////////////////
//
// A ramp's keys are kept in order of their values however they're added.
// At a key's value it's the key's sample; between two, theirs blended in
// every channel; before the first and after the last, that key's.  Two
// keys at one value are a hard edge: the first's up to the value, the
// second's from it.  Its input's value chooses the place.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidRamp(void)
{
	Ptr<CSolidRamp>		pRamp;
	Ptr<CSolidGradient>	pGradient;
	SSolidSample		sLow;
	SSolidSample		sHigh;
	SSolidSample		sMiddle;
	SSolidSample		sResult;
	SSolidSample		sBlack;

	pGradient = OMalloc<CSolidGradient>(SG_Plane, SR_Once);
	pRamp = OMalloc<CSolidRamp>(pGradient);
	sBlack.Init();
	pRamp->At(0.5f, &sResult);
	AssertSolidSample(&sBlack, &sResult, 6);

	TestSolidSample(&sLow, 0.1f, 0.1f);
	TestSolidSample(&sHigh, 1.0f, -0.05f);
	TestSolidSample(&sMiddle, 0.3f, 0.2f);
	pRamp->AddKey(0.75f, &sHigh);
	pRamp->AddKey(0.25f, &sLow);
	pRamp->AddKey(0.5f, &sMiddle);
	AssertSize(3, pRamp->masKeys.NumElements());
	AssertFloat(0.25f, pRamp->masKeys.Get(0)->fValue, 6);
	AssertFloat(0.5f, pRamp->masKeys.Get(1)->fValue, 6);
	AssertFloat(0.75f, pRamp->masKeys.Get(2)->fValue, 6);

	pRamp->At(-1.0f, &sResult);
	AssertSolidSample(&sLow, &sResult, 6);
	pRamp->At(0.25f, &sResult);
	AssertSolidSample(&sLow, &sResult, 6);
	pRamp->At(0.5f, &sResult);
	AssertSolidSample(&sMiddle, &sResult, 6);
	pRamp->At(0.75f, &sResult);
	AssertSolidSample(&sHigh, &sResult, 6);
	pRamp->At(3.0f, &sResult);
	AssertSolidSample(&sHigh, &sResult, 6);

	pRamp->At(0.3f, &sResult);
	AssertFloat(0.1f + (0.2f * 0.2f), sResult.sColour.x, 5);  //A fifth of the way from 0.1 to 0.3.
	AssertFloat(0.8f + (0.2f * 0.9f), sResult.fGlow, 5);  //And from 0.8 to 1.7.
	pRamp->At(0.625f, &sResult);
	AssertFloat((0.3f + 1.0f) * 0.5f, sResult.sColour.x, 5);
	AssertFloat((1.1f + 0.8f) * 0.5f, sResult.fHeight, 5);

	TestSolidSampleAt(pRamp, 0.5f, 9.0f, -4.0f, &sResult);
	AssertSolidSample(&sMiddle, &sResult, 5);
	TestSolidSampleAt(pRamp, 0.3f, 9.0f, -4.0f, &sResult);
	AssertFloat(0.1f + (0.2f * 0.2f), sResult.sColour.x, 5);

	pRamp = OMalloc<CSolidRamp>(pGradient);
	pRamp->AddKey(0.0f, Set32BitColour((uint8)255, (uint8)0, (uint8)0));
	pRamp->AddKey(0.5f, Set32BitColour((uint8)255, (uint8)255, (uint8)0));
	pRamp->AddKey(0.5f, Set32BitColour((uint8)0, (uint8)0, (uint8)255));
	pRamp->AddKey(1.0f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
	pRamp->At(0.4999f, &sResult);
	AssertFloat(1.0f, sResult.sColour.y, 3);
	AssertFloat(0.0f, sResult.sColour.z, 3);
	pRamp->At(0.5f, &sResult);
	AssertFloat(0.0f, sResult.sColour.y, 6);
	AssertFloat(1.0f, sResult.sColour.z, 6);
	pRamp->At(0.75f, &sResult);
	AssertFloat(0.5f, sResult.sColour.x, 6);
	AssertFloat(1.0f, sResult.sColour.z, 6);

	//With no input its value is black's, 0.
	pRamp->mpInput = NULL;
	TestSolidSampleAt(pRamp, 0.5f, 9.0f, -4.0f, &sResult);
	AssertFloat(1.0f, sResult.sColour.x, 6);
	AssertFloat(0.0f, sResult.sColour.y, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// An adjust starts changing nothing.  Inverted, each of red, green and
// blue is 1 less itself; a third of a turn of hue takes red to green and
// keeps a grey; no saturation is the colour's brightness as a grey;
// contrast is about a half; brightness is added.  Alpha and the other
// channels are the input's throughout.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidAdjust(void)
{
	Ptr<CSolidAdjust>	pAdjust;
	Ptr<CSolidFlat>		pFlat;
	SSolidSample		sSample;
	SSolidSample		sResult;

	TestSolidSample(&sSample, 0.1f, 0.1f);
	pFlat = OMalloc<CSolidFlat>(&sSample);
	pAdjust = OMalloc<CSolidAdjust>(pFlat);
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertSolidSample(&sSample, &sResult, 6);

	pAdjust->mbInvert = true;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(0.9f, sResult.sColour.x, 6);
	AssertFloat(0.8f, sResult.sColour.y, 6);
	AssertFloat(0.7f, sResult.sColour.z, 6);
	AssertFloat(0.4f, sResult.sColour.w, 6);
	AssertFloat(0.5f, sResult.fHeight, 6);
	AssertFloat(0.8f, sResult.fGlow, 6);
	pAdjust->mbInvert = false;

	pAdjust->mfSaturation = 0.0f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(0.2f, sResult.sColour.x, 6);
	AssertFloat(0.2f, sResult.sColour.y, 6);
	AssertFloat(0.2f, sResult.sColour.z, 6);
	pAdjust->mfSaturation = 2.0f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(0.0f, sResult.sColour.x, 6);
	AssertFloat(0.2f, sResult.sColour.y, 6);
	AssertFloat(0.4f, sResult.sColour.z, 6);
	pAdjust->mfSaturation = 1.0f;

	pAdjust->mfContrast = 2.0f;
	pAdjust->mfBrightness = 0.25f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(-0.05f, sResult.sColour.x, 6);  //0.1 is 0.4 below a half: 0.8 below, and a quarter.
	AssertFloat(0.15f, sResult.sColour.y, 6);
	AssertFloat(0.35f, sResult.sColour.z, 6);
	AssertFloat(0.4f, sResult.sColour.w, 6);

	pAdjust = OMalloc<CSolidAdjust>(OMalloc<CSolidFlat>(Set32BitColour((uint8)255, (uint8)0, (uint8)0)));
	pAdjust->mfHue = 1.0f / 3.0f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(0.0f, sResult.sColour.x, 5);
	AssertFloat(1.0f, sResult.sColour.y, 5);
	AssertFloat(0.0f, sResult.sColour.z, 5);
	pAdjust->mfHue = 2.0f / 3.0f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(0.0f, sResult.sColour.x, 5);
	AssertFloat(0.0f, sResult.sColour.y, 5);
	AssertFloat(1.0f, sResult.sColour.z, 5);
	pAdjust->mfHue = 0.5f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertFloat(1.0f / 3.0f, sResult.Value(), 5);  //Cyan, a little dark: as bright as the red was.
	AssertFloat(sResult.sColour.y, sResult.sColour.z, 5);
	AssertTrue(sResult.sColour.y > sResult.sColour.x);

	sSample.InitValue(0.6f);
	pAdjust = OMalloc<CSolidAdjust>(OMalloc<CSolidFlat>(&sSample));
	pAdjust->mfHue = 0.37f;
	TestSolidSampleAt(pAdjust, 1, 2, 3, &sResult);
	AssertSolidSample(&sSample, &sResult, 5);
}


//////////////////////////////////////////////////////////////////////////
//
// A merge is its way of putting two numbers together, in every channel.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidMerge(void)
{
	Ptr<CSolidMerge>	pMerge;
	Ptr<CSolidFlat>		pFirst;
	Ptr<CSolidFlat>		pSecond;
	SSolidSample		sFirst;
	SSolidSample		sSecond;
	SSolidSample		sResult;
	SSolidSample		sExpected;
	float32*			pafExpected;
	size				i;

	TestSolidSample(&sFirst, 0.1f, 0.1f);  //0.1 to 0.8.
	TestSolidSample(&sSecond, 0.75f, -0.1f);  //0.75 to 0.05.
	pFirst = OMalloc<CSolidFlat>(&sFirst);
	pSecond = OMalloc<CSolidFlat>(&sSecond);
	pafExpected = sExpected.Channels();

	pMerge = OMalloc<CSolidMerge>(SM_Add, pFirst, pSecond);
	TestSolidSampleAt(pMerge, 1, 2, 3, &sResult);
	for (i = 0; i < SOLID_SAMPLE_CHANNELS; i++)
	{
		pafExpected[i] = 0.85f;
	}
	AssertSolidSample(&sExpected, &sResult, 5);

	pMerge->meMerge = SM_Multiply;
	TestSolidSampleAt(pMerge, 1, 2, 3, &sResult);
	for (i = 0; i < SOLID_SAMPLE_CHANNELS; i++)
	{
		pafExpected[i] = sFirst.Channels()[i] * sSecond.Channels()[i];
	}
	AssertSolidSample(&sExpected, &sResult, 6);
	AssertFloat(0.075f, sResult.sColour.x, 6);

	pMerge->meMerge = SM_Lesser;
	TestSolidSampleAt(pMerge, 1, 2, 3, &sResult);
	AssertFloat(0.1f, sResult.sColour.x, 6);
	AssertFloat(0.4f, sResult.sColour.w, 6);
	AssertFloat(0.35f, sResult.fHeight, 6);
	AssertFloat(0.05f, sResult.fGlow, 6);

	pMerge->meMerge = SM_Greater;
	TestSolidSampleAt(pMerge, 1, 2, 3, &sResult);
	AssertFloat(0.75f, sResult.sColour.x, 6);
	AssertFloat(0.45f, sResult.sColour.w, 6);
	AssertFloat(0.5f, sResult.fHeight, 6);
	AssertFloat(0.8f, sResult.fGlow, 6);

	pMerge->meMerge = SM_Difference;
	TestSolidSampleAt(pMerge, 1, 2, 3, &sResult);
	AssertFloat(0.65f, sResult.sColour.x, 6);
	AssertFloat(0.05f, sResult.sColour.w, 6);
	AssertFloat(0.15f, sResult.fHeight, 6);
	AssertFloat(0.75f, sResult.fGlow, 6);

	//An input that isn't there is opaque black.
	pMerge->meMerge = SM_Add;
	pMerge->mpSecond = NULL;
	TestSolidSampleAt(pMerge, 1, 2, 3, &sResult);
	AssertFloat(0.1f, sResult.sColour.x, 6);
	AssertFloat(1.4f, sResult.sColour.w, 6);
	AssertFloat(0.5f, sResult.fHeight, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A mask is its first input where its mask's value is 0 or less, its
// second where it's 1 or more, and between, that much of the way from
// the first to the second in every channel.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidMask(void)
{
	Ptr<CSolidMask>	pMask;
	SSolidSample	sFirst;
	SSolidSample	sSecond;
	SSolidSample	sResult;
	SSolidSample	sExpected;

	TestSolidSample(&sFirst, 0.1f, 0.1f);
	TestSolidSample(&sSecond, 0.75f, -0.1f);
	pMask = OMalloc<CSolidMask>(OMalloc<CSolidFlat>(&sFirst), OMalloc<CSolidFlat>(&sSecond), OMalloc<CSolidGradient>(SG_Plane, SR_Once));

	TestSolidSampleAt(pMask, -2.0f, 0, 0, &sResult);
	AssertSolidSample(&sFirst, &sResult, 6);
	TestSolidSampleAt(pMask, 0.0f, 0, 0, &sResult);
	AssertSolidSample(&sFirst, &sResult, 6);
	TestSolidSampleAt(pMask, 1.0f, 0, 0, &sResult);
	AssertSolidSample(&sSecond, &sResult, 6);
	TestSolidSampleAt(pMask, 5.0f, 0, 0, &sResult);
	AssertSolidSample(&sSecond, &sResult, 6);

	TestSolidSampleAt(pMask, 0.25f, 0, 0, &sResult);
	sExpected = sFirst;
	sExpected.Blend(&sSecond, 0.25f);
	AssertSolidSample(&sExpected, &sResult, 6);
	AssertFloat(0.1f + (0.65f * 0.25f), sResult.sColour.x, 6);
	AssertFloat(0.8f - (0.75f * 0.25f), sResult.fGlow, 6);

	pMask->mpMask = NULL;
	TestSolidSampleAt(pMask, 0.25f, 0, 0, &sResult);
	AssertSolidSample(&sFirst, &sResult, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// Coloured clouds through cells' edges: blue to white by a noise through
// a ramp, darkened along the edges of cells (a merge), and a red where
// another noise is high (a mask).
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CSolidTexture> TestSolidGraphMake(const char* szName)
{
	Ptr<CSolidMask>		pMask;
	Ptr<CSolidRamp>		pSky;
	Ptr<CSolidRamp>		pEdges;
	Ptr<CSolidRamp>		pPatches;
	Ptr<CSolidCells>	pCells;
	Ptr<CSolidNoise>	pNoise;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	sOrigin.Init(0, 0, 0);
	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);

	pNoise = OMalloc<CSolidNoise>(FN_Sum, 5, 11);
	pNoise->Place(&sOrigin, &sX, &sY, 0.6f);
	pSky = OMalloc<CSolidRamp>(pNoise);
	pSky->AddKey(0.35f, Set32BitColour((uint8)40, (uint8)90, (uint8)200));
	pSky->AddKey(0.65f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

	pCells = OMalloc<CSolidCells>(SC_Edge, 3);
	pCells->Place(&sOrigin, &sX, &sY, 0.3f);
	pEdges = OMalloc<CSolidRamp>(pCells);
	pEdges->AddKey(0.0f, Set32BitColour((uint8)40, (uint8)40, (uint8)40));
	pEdges->AddKey(0.08f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

	pNoise = OMalloc<CSolidNoise>(FN_Sum, 3, 29);
	pPatches = OMalloc<CSolidRamp>(pNoise);
	pPatches->AddKey(0.5f, Set32BitColour((uint8)0, (uint8)0, (uint8)0));
	pPatches->AddKey(0.56f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

	pMask = gcObjects.Malloc<CSolidMask>(szName);
	pMask->Init(OMalloc<CSolidMerge>(SM_Multiply, pSky, pEdges), OMalloc<CSolidFlat>(Set32BitColour((uint8)200, (uint8)50, (uint8)40)), pPatches);
	return pMask;
}


//////////////////////////////////////////////////////////////////////////
//
// A graph of textures written to a file and read back in a fresh set of
// objects gives the samples it gave before, its inputs, enums, numbers,
// placements and a ramp's keys all kept.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidGraphSaveLoad(void)
{
	char			szDirectory[] = "Output" _FS_ "TestSolidGraph";
	CFileUtil		cFileUtil;
	SSolidSample	asSamples[12];
	SSolidSample	sResult;
	int				i;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CSolidTexture>			pGraph;
		CExternalObjectSerialiser	cSerialiser;
		CChunkFileObjectWriter		cWriter;

		pGraph = TestSolidGraphMake("Graph");
		for (i = 0; i < 12; i++)
		{
			TestSolidSampleAt(pGraph, ((float32)i * 0.37f) - 2.0f, (float32)i * 0.21f, ((float32)i * -0.13f) + 0.4f, &asSamples[i]);
		}
		TextureGallery("Graph", pGraph);

		cWriter.Init(szDirectory, "", pGraph->GetName());
		cSerialiser.Init(&cWriter);
		AssertTrue(cSerialiser.Write(&pGraph));
		cSerialiser.Kill();
		cWriter.Kill();
		pGraph = NULL;
	}
	ObjectsDiscard();
	ObjectsKill();

	ObjectsInit();
	{
		Ptr<CSolidTexture>				pGraph;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader	cReader;

		gcObjects.AddConstructor<CSolidMask>();
		gcObjects.AddConstructor<CSolidMerge>();
		gcObjects.AddConstructor<CSolidRamp>();
		gcObjects.AddConstructor<CSolidFlat>();
		gcObjects.AddConstructor<CSolidNoise>();
		gcObjects.AddConstructor<CSolidCells>();

		cReader.Init(szDirectory, "Graph");
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pGraph = cDeserialiser.Read("Graph");
		cDeserialiser.Kill();
		cReader.Kill();
		AssertNotNull(&pGraph);
		for (i = 0; i < 12; i++)
		{
			TestSolidSampleAt(pGraph, ((float32)i * 0.37f) - 2.0f, (float32)i * 0.21f, ((float32)i * -0.13f) + 0.4f, &sResult);
			AssertSolidSample(&asSamples[i], &sResult, 6);
		}
		pGraph = NULL;
	}
	ObjectsDiscard();
	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidOperators(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestSolidRamp();
	TestSolidAdjust();
	TestSolidMerge();
	TestSolidMask();
	ObjectsDiscard();
	ObjectsKill(false);

	TestSolidGraphSaveLoad();
	DataIOKill();

	TestStatistics();
}

