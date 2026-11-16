#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "BaseLib/FloatHelper.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/SolidChecker.h"
#include "ProceduralLib/SolidCells.h"
#include "ProceduralLib/SolidGradient.h"
#include "ProceduralLib/SolidMaterials.h"
#include "ProceduralLib/SolidNoise.h"
#include "TestLib/Assert.h"


#define TEST_SOLID_FILTER_POINTS	12		//Each way across the plane.
#define TEST_SOLID_FILTER_SAMPLES	8		//Each way across a texel, for its truth.
#define TEST_SOLID_FILTER_WIDTHS	6		//A 64th of a unit, then each twice the one before, to half a unit.
#define TEST_SOLID_FILTER_BAKES		3		//Of one sample a texel, two each way and four each way.


//What a texture is measured to do at each width (TexturePlan.txt, Filtering): how far a bake of it is from the truth, and
//how much the truth itself varies, for scale.
struct STestSolidFilter
{
	float32	aafError[TEST_SOLID_FILTER_BAKES][TEST_SOLID_FILTER_WIDTHS];
	float32	afSpread[TEST_SOLID_FILTER_WIDTHS];
	float32	fMicroseconds;  //A sample at the finest width.
};


//////////////////////////////////////////////////////////////////////////
//
// The mean colour of iSamples by iSamples samples across a texel fWidth
// wide at psPoint on the plane z = psPoint->z, each sample as wide as its
// share of the texel: what a bake of that many samples makes of it.
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidFilterTexel(SFloat32Vec3* psColour, Ptr<CSolidTexture> pTexture, SFloat32Vec3* psPoint, float32 fWidth, int iSamples)
{
	SSolidSample	sSample;
	SFloat32Vec3	sIn;
	SFloat32Vec3	sNormal;
	float32			fStep;
	int				x;
	int				y;

	sNormal.Init(0, 0, 1);
	fStep = fWidth / (float32)iSamples;
	psColour->Init(0, 0, 0);
	for (y = 0; y < iSamples; y++)
	{
		for (x = 0; x < iSamples; x++)
		{
			sIn.Init(psPoint->x + (((float32)x + 0.5f) * fStep) - (fWidth * 0.5f), psPoint->y + (((float32)y + 0.5f) * fStep) - (fWidth * 0.5f), psPoint->z);
			pTexture->Sample(&sIn, &sNormal, fStep, &sSample);
			psColour->x += sSample.sColour.x;
			psColour->y += sSample.sColour.y;
			psColour->z += sSample.sColour.z;
		}
	}
	*psColour /= (float32)(iSamples * iSamples);
}


//////////////////////////////////////////////////////////////////////////
//
// Texels fWidth across at points over a plane.  Each texel's truth is the
// mean colour of sixty four samples across it.  *pfError is the root mean
// square, over the texels and red, green and blue, of how far the mean of
// iSamples by iSamples samples is from that; *pfSpread of how far the
// truths are from their own mean.  A texture that filters perfectly has
// no error from one sample; one that doesn't filter at all has about the
// spread of the level below.
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidFilterMeasure(Ptr<CSolidTexture> pTexture, float32 fWidth, int iSamples, float32* pfError, float32* pfSpread)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sColour;
	SFloat32Vec3	sMean;
	SFloat32Vec3	sOff;
	SFloat32Vec3	asTruths[TEST_SOLID_FILTER_POINTS * TEST_SOLID_FILTER_POINTS];
	float32			fErrors;
	float32			fSpreads;
	int				i;

	fErrors = 0.0f;
	sMean.Init(0, 0, 0);
	for (i = 0; i < TEST_SOLID_FILTER_POINTS * TEST_SOLID_FILTER_POINTS; i++)
	{
		sPoint.Init(((float32)(i % TEST_SOLID_FILTER_POINTS) * 0.371f) - 2.0f, ((float32)(i / TEST_SOLID_FILTER_POINTS) * 0.293f) - 1.5f, 0.37f);
		TestSolidFilterTexel(&asTruths[i], pTexture, &sPoint, fWidth, TEST_SOLID_FILTER_SAMPLES);
		sMean += asTruths[i];

		TestSolidFilterTexel(&sColour, pTexture, &sPoint, fWidth, iSamples);
		sOff = sColour - asTruths[i];
		fErrors += Float32Vec3Dot(&sOff, &sOff);
	}

	sMean /= (float32)(TEST_SOLID_FILTER_POINTS * TEST_SOLID_FILTER_POINTS);
	fSpreads = 0.0f;
	for (i = 0; i < TEST_SOLID_FILTER_POINTS * TEST_SOLID_FILTER_POINTS; i++)
	{
		sOff = asTruths[i] - sMean;
		fSpreads += Float32Vec3Dot(&sOff, &sOff);
	}
	*pfError = sqrtf(fErrors / (float32)(TEST_SOLID_FILTER_POINTS * TEST_SOLID_FILTER_POINTS * 3));
	*pfSpread = sqrtf(fSpreads / (float32)(TEST_SOLID_FILTER_POINTS * TEST_SOLID_FILTER_POINTS * 3));
}


//////////////////////////////////////////////////////////////////////////
//
// A texture measured at every width, baked with one, two and four samples
// each way in a texel, and how long one of its samples takes.  With
// TEXTURE_FILTER_REPORT set in the environment they're printed, for the
// plan's tables.
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidFilter(const char* szName, Ptr<CSolidTexture> pTexture, STestSolidFilter* psFilter)
{
	SSolidSample	sSample;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNormal;
	float32			fWidth;
	clock_t			iStart;
	int				iBake;
	int				i;

	fWidth = 1.0f / 64.0f;
	for (i = 0; i < TEST_SOLID_FILTER_WIDTHS; i++)
	{
		for (iBake = 0; iBake < TEST_SOLID_FILTER_BAKES; iBake++)
		{
			TestSolidFilterMeasure(pTexture, fWidth, 1 << iBake, &psFilter->aafError[iBake][i], &psFilter->afSpread[i]);
		}
		fWidth *= 2.0f;
	}

	sNormal.Init(0, 0, 1);
	iStart = clock();
	for (i = 0; i < 20000; i++)
	{
		sPoint.Init((float32)(i % 200) / 64.0f, (float32)(i / 200) / 64.0f, 0.37f);
		pTexture->Sample(&sPoint, &sNormal, 1.0f / 64.0f, &sSample);
	}
	psFilter->fMicroseconds = (float32)(clock() - iStart) * (1000000.0f / (float32)CLOCKS_PER_SEC) / 20000.0f;

	if (getenv("TEXTURE_FILTER_REPORT"))
	{
		printf("%-12s spread", szName);
		for (i = 0; i < TEST_SOLID_FILTER_WIDTHS; i++)
		{
			printf(" %.3f", psFilter->afSpread[i]);
		}
		for (iBake = 0; iBake < TEST_SOLID_FILTER_BAKES; iBake++)
		{
			printf("  error x%d", 1 << iBake);
			for (i = 0; i < TEST_SOLID_FILTER_WIDTHS; i++)
			{
				printf(" %.3f", psFilter->aafError[iBake][i]);
			}
		}
		printf("  %.2f us\n", psFilter->fMicroseconds);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The worst error at any width of a bake of (1 << iBake) samples each way.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestSolidFilterWorst(STestSolidFilter* psFilter, int iBake)
{
	float32	fWorst;
	int		i;

	fWorst = 0.0f;
	for (i = 0; i < TEST_SOLID_FILTER_WIDTHS; i++)
	{
		fWorst = maxf(fWorst, psFilter->aafError[iBake][i]);
	}
	return fWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// The worst error at any width of a bake as CMeshBake makes its levels:
// one sample at the finest, two each way at the next, four from there on.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestSolidFilterWorstLevel(STestSolidFilter* psFilter)
{
	float32	fWorst;
	int		i;

	fWorst = 0.0f;
	for (i = 0; i < TEST_SOLID_FILTER_WIDTHS; i++)
	{
		fWorst = maxf(fWorst, psFilter->aafError[(i < TEST_SOLID_FILTER_BAKES) ? i : (TEST_SOLID_FILTER_BAKES - 1)][i]);
	}
	return fWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// The generators average themselves over a texel, so one sample is near
// the truth at every width: a checker and a gradient exactly, a noise
// nearly (it knows how much of each octave a texel keeps, not which
// part), and cells less nearly (their edges are softened by the texel's
// width; the distances only go to their means).  A colour is 0 to 1, so
// 0.004 is one step of 8 bits.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidFilteringGenerators(void)
{
	Ptr<CSolidTexture>	pTexture;
	STestSolidFilter	sFilter;

	pTexture = OMalloc<CSolidChecker>(0.25f, Set32BitColour((uint8)0, (uint8)0, (uint8)0), Set32BitColour((uint8)255, (uint8)255, (uint8)255));
	TestSolidFilter("Checker", pTexture, &sFilter);
	AssertTrue(TestSolidFilterWorst(&sFilter, 0) < 0.01f);

	pTexture = OMalloc<CSolidGradient>(SG_Plane, SR_Saw);
	pTexture->mfScale = 0.25f;
	TestSolidFilter("Saw", pTexture, &sFilter);
	AssertTrue(TestSolidFilterWorst(&sFilter, 0) < 0.002f);

	pTexture = OMalloc<CSolidNoise>(FN_Sum, 6, 3);
	pTexture->mfScale = 0.5f;
	TestSolidFilter("NoiseSum", pTexture, &sFilter);
	AssertTrue(TestSolidFilterWorst(&sFilter, 0) < 0.03f);

	pTexture = OMalloc<CSolidNoise>(FN_Ridged, 6, 3);
	pTexture->mfScale = 0.5f;
	TestSolidFilter("NoiseRidged", pTexture, &sFilter);
	AssertTrue(TestSolidFilterWorst(&sFilter, 0) < 0.07f);

	pTexture = OMalloc<CSolidCells>(SC_Edge, 3);
	pTexture->mfScale = 0.25f;
	TestSolidFilter("CellsEdge", pTexture, &sFilter);
	AssertTrue(TestSolidFilterWorst(&sFilter, 0) < 0.11f);

	pTexture = OMalloc<CSolidCells>(SC_Cell, 3);
	pTexture->mfScale = 0.25f;
	TestSolidFilter("CellsCell", pTexture, &sFilter);
	AssertTrue(sFilter.aafError[0][0] < 0.025f);  //0.046 before cells' edges were softened.
	AssertTrue(TestSolidFilterWorst(&sFilter, 0) < 0.12f);
}


//////////////////////////////////////////////////////////////////////////
//
// A material's ramps and masks put back edges its noises' averaging took
// out, and move its mean (a ramp of a mean isn't the mean of the ramp), so
// one sample is further from the truth the coarser the texel: up to a
// tenth of the whole range.  A bake of several samples a texel mends
// both: as CMeshBake makes its levels, no level is more than fWorst out.
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidFilteringMaterial(const char* szName, Ptr<CSolidTexture> pMaterial, float32 fWorst)
{
	STestSolidFilter	sFilter;

	TestSolidFilter(szName, pMaterial, &sFilter);
	AssertTrue(TestSolidFilterWorstLevel(&sFilter) < fWorst);
	AssertTrue(TestSolidFilterWorst(&sFilter, 2) < (TestSolidFilterWorst(&sFilter, 0) * 0.5f));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidFilteringMaterials(void)
{
	TestSolidFilteringMaterial("Marble", MakeSolidMarble(1), 0.02f);
	TestSolidFilteringMaterial("Wood", MakeSolidWood(1), 0.035f);
	TestSolidFilteringMaterial("Stone", MakeSolidStone(1), 0.03f);
	TestSolidFilteringMaterial("Rust", MakeSolidRust(1), 0.02f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidFiltering(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestSolidFilteringGenerators();
	TestSolidFilteringMaterials();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

