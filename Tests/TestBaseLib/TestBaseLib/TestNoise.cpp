#include <math.h>
#include "BaseLib/FloatHelper.h"
#include "BaseLib/Noise.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Points spread through space, none on a lattice plane, the same on
// every build.
//
//////////////////////////////////////////////////////////////////////////
static void TestNoisePoint(SFloat32Vec3* psPoint, int i)
{
	psPoint->Init(((float32)i * 0.37f) + 0.11f, ((float32)i * 0.73f) - 5.0f, ((float32)i * 0.19f) + 2.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A hash is the same wherever it's worked out, changes with each
// coordinate and with the seed, and its bits are set about half the time.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseHash(void)
{
	int		i;
	int		iOnes;

	AssertIntHex((uint32)0x00000000, NoiseHash(0, 0, 0, 0));
	AssertIntHex((uint32)0x464923EA, NoiseHash(1, 2, 3, 4));
	AssertIntHex((uint32)0xDE188AF8, NoiseHash(-1, -2, -3, 4));
	AssertIntHex((uint32)0xCF4C759A, NoiseHash(1, 2, 3, 5));
	AssertTrue(NoiseHash(2, 2, 3, 4) != NoiseHash(1, 2, 3, 4));
	AssertTrue(NoiseHash(1, 3, 3, 4) != NoiseHash(1, 2, 3, 4));
	AssertTrue(NoiseHash(1, 2, 4, 4) != NoiseHash(1, 2, 3, 4));

	iOnes = 0;
	for (i = 0; i < 1000; i++)
	{
		iOnes += (int)(NoiseHash(i, -i, i * 3, 9) & 1);
	}
	AssertInt(488, iOnes);
}


//////////////////////////////////////////////////////////////////////////
//
// All of a noise is kept at a point, half at half the widest width, none
// from the widest, and less the wider.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseKept(void)
{
	float32	fWidth;

	AssertFloat(1.0f, NoiseKept(0.0f), 6);
	AssertFloat(1.0f, NoiseKept(-1.0f), 6);
	AssertFloat(0.5f, NoiseKept(1.25f), 6);
	AssertFloat(0.0f, NoiseKept(2.5f), 6);
	AssertFloat(0.0f, NoiseKept(100.0f), 6);
	for (fWidth = 0.0f; fWidth < 2.5f; fWidth += 0.125f)
	{
		AssertTrue(NoiseKept(fWidth + 0.125f) < NoiseKept(fWidth));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gradient noise is 0 at lattice points, the same on every build, within
// -1 to 1, 0 on average and spread as it's known to be (a root mean
// square near 0.27, an absolute value near 0.22).
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseGradientValues(void)
{
	SFloat32Vec3	sPoint;
	float32			fNoise;
	float32			fSum;
	float32			fSquares;
	float32			fAbsolutes;
	float32			fRidges;
	int				i;

	sPoint.Init(0, 0, 0);
	AssertFloat(0.0f, GradientNoise(&sPoint, 1), 6);
	sPoint.Init(3, -4, 17);
	AssertFloat(0.0f, GradientNoise(&sPoint, 1), 6);
	sPoint.Init(-250, 99, -1);
	AssertFloat(0.0f, GradientNoise(&sPoint, 77), 6);

	sPoint.Init(0.5f, 0.5f, 0.5f);
	AssertFloat(0.125f, GradientNoise(&sPoint, 1), 5);
	sPoint.Init(1.25f, -2.75f, 3.5f);
	AssertFloat(-0.013316f, GradientNoise(&sPoint, 1), 5);
	sPoint.Init(-7.3f, 4.1f, 0.6f);
	AssertFloat(0.178822f, GradientNoise(&sPoint, 1), 5);
	AssertTrue(fabsf(GradientNoise(&sPoint, 2) - 0.178822f) > 0.001f);

	fSum = 0.0f;
	fSquares = 0.0f;
	fAbsolutes = 0.0f;
	fRidges = 0.0f;
	for (i = 0; i < 1000; i++)
	{
		TestNoisePoint(&sPoint, i);
		fNoise = GradientNoise(&sPoint, 2);
		AssertTrue(fabsf(fNoise) <= 1.0f);
		fSum += fNoise;
		fSquares += fNoise * fNoise;
		fAbsolutes += fabsf(fNoise);
		fRidges += (1.0f - fabsf(fNoise)) * (1.0f - fabsf(fNoise));
	}
	AssertTrue(fabsf(fSum / 1000.0f) < 0.03f);
	AssertTrue(fabsf(sqrtf(fSquares / 1000.0f) - 0.27f) < 0.03f);
	AssertTrue(fabsf((fAbsolutes / 1000.0f) - 0.2204f) < 0.02f);
	AssertTrue(fabsf((fRidges / 1000.0f) - 0.6322f) < 0.02f);
}


//////////////////////////////////////////////////////////////////////////
//
// How far a slope is from the noise's change between two points a
// thousandth either side along each axis, the worst axis.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestNoiseSlopeError(EFractalNoise eKind, size uiOctaves, SFloat32Vec3* psPoint, uint32 uiSeed)
{
	SFloat32Vec3	sGradient;
	SFloat32Vec3	sBefore;
	SFloat32Vec3	sAfter;
	float32			fWorst;
	float32			fSlope;
	int				iAxis;

	if (uiOctaves == 0)
	{
		GradientNoise(psPoint, uiSeed, &sGradient);
	}
	else
	{
		FractalNoise(eKind, psPoint, uiSeed, uiOctaves, 0.0f, &sGradient);
	}

	fWorst = 0.0f;
	for (iAxis = 0; iAxis < 3; iAxis++)
	{
		sBefore = *psPoint;
		sAfter = *psPoint;
		(&sBefore.x)[iAxis] -= 0.001f;
		(&sAfter.x)[iAxis] += 0.001f;
		if (uiOctaves == 0)
		{
			fSlope = (GradientNoise(&sAfter, uiSeed) - GradientNoise(&sBefore, uiSeed)) / 0.002f;
		}
		else
		{
			fSlope = (FractalNoise(eKind, &sAfter, uiSeed, uiOctaves, 0.0f) - FractalNoise(eKind, &sBefore, uiSeed, uiOctaves, 0.0f)) / 0.002f;
		}
		fWorst = maxf(fWorst, fabsf(fSlope - (&sGradient.x)[iAxis]));
	}
	return fWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// The gradient is the noise's slope, and both carry on smoothly across
// a lattice plane.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseGradientSlope(void)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sGradient;
	SFloat32Vec3	sAcross;
	float32			fNoise;
	int				i;

	sPoint.Init(0.5f, 0.5f, 0.5f);
	GradientNoise(&sPoint, 1, &sGradient);
	AssertFloat(-0.21875f, sGradient.x, 5);
	AssertFloat(0.46875f, sGradient.y, 5);
	AssertFloat(0.21875f, sGradient.z, 5);

	for (i = 0; i < 20; i++)
	{
		TestNoisePoint(&sPoint, i);
		AssertTrue(TestNoiseSlopeError(FN_Sum, 0, &sPoint, 5) < 0.02f);
	}

	sPoint.Init(2.9999f, 0.3f, -1.6f);
	fNoise = GradientNoise(&sPoint, 3, &sGradient);
	sPoint.x = 3.0001f;
	AssertFloat(fNoise, GradientNoise(&sPoint, 3, &sAcross), 3);
	AssertFloat(sGradient.x, sAcross.x, 2);
	AssertFloat(sGradient.y, sAcross.y, 2);
	AssertFloat(sGradient.z, sAcross.z, 2);
}


//////////////////////////////////////////////////////////////////////////
//
// One octave is the gradient noise; more are the same on every build and
// within their ranges; none is 0.  A sum's gradient is its slope, in each
// kind.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseFractalValues(void)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sGradient;
	float32			fNoise;
	int				i;

	sPoint.Init(1.25f, -2.75f, 3.5f);
	AssertFloat(GradientNoise(&sPoint, 1), FractalNoise(FN_Sum, &sPoint, 1, 1, 0.0f), 6);
	AssertFloat(fabsf(GradientNoise(&sPoint, 1)), FractalNoise(FN_Turbulence, &sPoint, 1, 1, 0.0f), 6);
	AssertFloat(0.0f, FractalNoise(FN_Sum, &sPoint, 1, 0, 0.0f, &sGradient), 6);
	AssertFloat(0.0f, sGradient.Magnitude(), 6);

	AssertFloat(0.040292f, FractalNoise(FN_Sum, &sPoint, 1, 4, 0.0f), 4);
	AssertFloat(0.143157f, FractalNoise(FN_Turbulence, &sPoint, 1, 4, 0.0f), 4);
	AssertFloat(0.755206f, FractalNoise(FN_Ridged, &sPoint, 1, 4, 0.0f), 4);

	for (i = 0; i < 100; i++)
	{
		TestNoisePoint(&sPoint, i);
		AssertTrue(fabsf(FractalNoise(FN_Sum, &sPoint, 4, 5, 0.0f)) <= 1.0f);
		fNoise = FractalNoise(FN_Turbulence, &sPoint, 4, 5, 0.0f);
		AssertTrue((fNoise >= 0.0f) && (fNoise <= 1.0f));
		fNoise = FractalNoise(FN_Ridged, &sPoint, 4, 5, 0.0f);
		AssertTrue((fNoise >= 0.0f) && (fNoise <= 1.0f));
	}

	for (i = 0; i < 10; i++)
	{
		TestNoisePoint(&sPoint, i);
		AssertTrue(TestNoiseSlopeError(FN_Sum, 3, &sPoint, 5) < 0.05f);
		AssertTrue(TestNoiseSlopeError(FN_Turbulence, 3, &sPoint, 5) < 0.05f);
		AssertTrue(TestNoiseSlopeError(FN_Ridged, 3, &sPoint, 5) < 0.05f);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How far, as a root mean square over forty points, a sum at half a unit's
// width is from the average of thirty six of its values across a square
// that wide.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestNoiseFilterError(EFractalNoise eKind)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sIn;
	float32			fAverage;
	float32			fError;
	float32			fSquares;
	int				i;
	int				x;
	int				y;

	fSquares = 0.0f;
	for (i = 0; i < 40; i++)
	{
		TestNoisePoint(&sPoint, i);
		fAverage = 0.0f;
		for (y = 0; y < 6; y++)
		{
			for (x = 0; x < 6; x++)
			{
				sIn.Init(sPoint.x + (((((float32)x + 0.5f) / 6.0f) - 0.5f) * 0.5f), sPoint.y + (((((float32)y + 0.5f) / 6.0f) - 0.5f) * 0.5f), sPoint.z);
				fAverage += FractalNoise(eKind, &sIn, 3, 4, 0.0f);
			}
		}
		fError = FractalNoise(eKind, &sPoint, 3, 4, 0.5f) - (fAverage / 36.0f);
		fSquares += fError * fError;
	}
	return sqrtf(fSquares / 40.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A sum over a width is near its average across the width; over a great
// width it's the kind's mean, with no slope, and no octave is worked out.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseFractalFiltered(void)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sGradient;

	AssertTrue(TestNoiseFilterError(FN_Sum) < 0.04f);
	AssertTrue(TestNoiseFilterError(FN_Turbulence) < 0.05f);
	AssertTrue(TestNoiseFilterError(FN_Ridged) < 0.07f);

	sPoint.Init(1.25f, -2.75f, 3.5f);
	AssertFloat(0.0f, FractalNoise(FN_Sum, &sPoint, 1, 4, 10.0f, &sGradient), 6);
	AssertFloat(0.0f, sGradient.Magnitude(), 6);
	AssertFloat(0.2204f, FractalNoise(FN_Turbulence, &sPoint, 1, 4, 10.0f), 5);
	AssertFloat(0.6322f, FractalNoise(FN_Ridged, &sPoint, 1, 4, 10.0f), 5);

	//At a width of one and a half the first octave is partly kept and the second (three wide in its own units) isn't.
	AssertFloat(GradientNoise(&sPoint, 1) * NoiseKept(1.5f) / 1.5f, FractalNoise(FN_Sum, &sPoint, 1, 2, 1.5f), 6);
}


//////////////////////////////////////////////////////////////////////////
//
// The nearest two of the points of every lattice cube within three cubes
// of the position's, placed as CellularNoise places them.
//
//////////////////////////////////////////////////////////////////////////
static void TestNoiseCellularSearch(SFloat32Vec3* psPoint, uint32 uiSeed, float32* pfNearest, float32* pfSecond)
{
	SFloat32Vec3	sOffset;
	float32			fDistance;
	uint32			uiHash;
	int32			x;
	int32			y;
	int32			z;

	*pfNearest = 1e30f;
	*pfSecond = 1e30f;
	for (z = (int32)floorf(psPoint->z) - 3; z <= (int32)floorf(psPoint->z) + 3; z++)
	{
		for (y = (int32)floorf(psPoint->y) - 3; y <= (int32)floorf(psPoint->y) + 3; y++)
		{
			for (x = (int32)floorf(psPoint->x) - 3; x <= (int32)floorf(psPoint->x) + 3; x++)
			{
				uiHash = NoiseHash(x, y, z, uiSeed);
				sOffset.Init((float32)x + ((float32)(uiHash & 0x3FF) / 1024.0f) - psPoint->x, (float32)y + ((float32)((uiHash >> 10) & 0x3FF) / 1024.0f) - psPoint->y, (float32)z + ((float32)((uiHash >> 20) & 0x3FF) / 1024.0f) - psPoint->z);
				fDistance = sqrtf(Float32Vec3Dot(&sOffset, &sOffset));
				if (fDistance < *pfNearest)
				{
					*pfSecond = *pfNearest;
					*pfNearest = fDistance;
				}
				else if (fDistance < *pfSecond)
				{
					*pfSecond = fDistance;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The nearest and second nearest distances are the true ones (against a
// search of every point nearby), the same on every build, and average what
// they're known to.  At a cube's point the nearest is 0; a cell's number
// is the same close by.  Over a great width each is its mean.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseCellular(void)
{
	SCellularNoise	sNoise;
	SCellularNoise	sBeside;
	SFloat32Vec3	sPoint;
	float32			fNearest;
	float32			fSecond;
	float32			fNearests;
	float32			fSeconds;
	float32			fCells;
	uint32			uiHash;
	int				i;

	sPoint.Init(1.25f, -2.75f, 3.5f);
	CellularNoise(&sNoise, &sPoint, 1, 0.0f);
	AssertFloat(0.608032f, sNoise.fNearest, 5);
	AssertFloat(0.820229f, sNoise.fSecond, 5);
	AssertFloat(0.710576f, sNoise.fCell, 5);

	fNearests = 0.0f;
	fSeconds = 0.0f;
	fCells = 0.0f;
	for (i = 0; i < 300; i++)
	{
		TestNoisePoint(&sPoint, i);
		CellularNoise(&sNoise, &sPoint, 2, 0.0f);
		AssertTrue((sNoise.fNearest >= 0.0f) && (sNoise.fNearest <= sNoise.fSecond));
		AssertTrue((sNoise.fCell >= 0.0f) && (sNoise.fCell < 1.0f));
		fNearests += sNoise.fNearest;
		fSeconds += sNoise.fSecond;
		fCells += sNoise.fCell;
		if (i < 30)
		{
			TestNoiseCellularSearch(&sPoint, 2, &fNearest, &fSecond);
			AssertFloat(fNearest, sNoise.fNearest, 5);
			AssertFloat(fSecond, sNoise.fSecond, 5);
		}
	}
	AssertTrue(fabsf((fNearests / 300.0f) - 0.5201f) < 0.03f);
	AssertTrue(fabsf((fSeconds / 300.0f) - 0.7167f) < 0.03f);
	AssertTrue(fabsf((fCells / 300.0f) - 0.5f) < 0.06f);

	uiHash = NoiseHash(4, -2, 7, 9);
	sPoint.Init(4.0f + ((float32)(uiHash & 0x3FF) / 1024.0f), -2.0f + ((float32)((uiHash >> 10) & 0x3FF) / 1024.0f), 7.0f + ((float32)((uiHash >> 20) & 0x3FF) / 1024.0f));
	CellularNoise(&sNoise, &sPoint, 9, 0.0f);
	AssertFloat(0.0f, sNoise.fNearest, 5);
	sPoint.x += 0.01f;
	CellularNoise(&sBeside, &sPoint, 9, 0.0f);
	AssertFloat(0.01f, sBeside.fNearest, 5);
	AssertFloat(sNoise.fCell, sBeside.fCell, 6);

	CellularNoise(&sNoise, &sPoint, 9, 10.0f);
	AssertFloat(0.5201f, sNoise.fNearest, 5);
	AssertFloat(0.7167f, sNoise.fSecond, 5);
	AssertFloat(0.5f, sNoise.fCell, 5);
}


//////////////////////////////////////////////////////////////////////////
//
// The greatest change in a cell's number between one point and the next
// of five hundred along a line, two thousandths of a unit apart, at a
// width.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestNoiseCellularStep(float32 fWidth)
{
	SCellularNoise	sNoise;
	SFloat32Vec3	sPoint;
	float32			fLast;
	float32			fGreatest;
	int				i;

	fGreatest = 0.0f;
	fLast = 0.0f;
	for (i = 0; i < 500; i++)
	{
		sPoint.Init(0.3f + ((float32)i * 0.0016f), 1.7f + ((float32)i * 0.0010f), -2.2f + ((float32)i * 0.0006f));
		CellularNoise(&sNoise, &sPoint, 5, fWidth);
		if (i > 0)
		{
			fGreatest = maxf(fGreatest, fabsf(sNoise.fCell - fLast));
		}
		fLast = sNoise.fCell;
	}
	return fGreatest;
}


//////////////////////////////////////////////////////////////////////////
//
// At a point a cell's number steps at the cell's edge.  Over a width it
// doesn't: a texel on the edge between two cells is partly each, so the
// number crosses from one's to the other's in the texel's width.
//
//////////////////////////////////////////////////////////////////////////
void TestNoiseCellularEdges(void)
{
	AssertTrue(TestNoiseCellularStep(0.0f) > 0.15f);
	AssertTrue(TestNoiseCellularStep(0.05f) < 0.03f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNoise(void)
{
	BeginTests();

	TestNoiseHash();
	TestNoiseKept();
	TestNoiseGradientValues();
	TestNoiseGradientSlope();
	TestNoiseFractalValues();
	TestNoiseFractalFiltered();
	TestNoiseCellular();
	TestNoiseCellularEdges();

	TestStatistics();
}

