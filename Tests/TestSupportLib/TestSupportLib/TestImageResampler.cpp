#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageResampler.h"
#include "SupportLib/ImageResampleChannel.h"
#include "SupportLib/ImageResampleBilinear.h"
#include "SupportLib/ImageResampleBicubic.h"
#include "SupportLib/ImageResampleSmart.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "AssertImage.h"
#include "TestLib/Assert.h"


#define TEST_IMAGE_RESAMPLER_PI		3.14159265358979323846


//////////////////////////////////////////////////////////////////////////
//
// A dest image with pcSource's channels.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> MakeResampleDest(Ptr<CImage> pcSource, uint32 uiWidth, uint32 uiHeight)
{
	return OMalloc<CImage>(uiWidth, uiHeight, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetResampleIntegers(Ptr<CImage> pcImage, size uiChannelIndex, int32* paiValues)
{
	CImageResampleChannel	cChannel;

	AssertTrue(cChannel.Init(pcImage, uiChannelIndex));
	cChannel.SetIntegers(paiValues);
	cChannel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GetResampleIntegers(Ptr<CImage> pcImage, size uiChannelIndex, int32* paiValues)
{
	CImageResampleChannel	cChannel;

	AssertTrue(cChannel.Init(pcImage, uiChannelIndex));
	cChannel.GetIntegers(paiValues);
	cChannel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetResampleFloats(Ptr<CImage> pcImage, size uiChannelIndex, float64* pafValues)
{
	CImageResampleChannel	cChannel;

	AssertTrue(cChannel.Init(pcImage, uiChannelIndex));
	cChannel.SetFloats(pafValues);
	cChannel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GetResampleFloats(Ptr<CImage> pcImage, size uiChannelIndex, float64* pafValues)
{
	CImageResampleChannel	cChannel;

	AssertTrue(cChannel.Init(pcImage, uiChannelIndex));
	cChannel.GetFloats(pafValues);
	cChannel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Doubling puts dest pixels a quarter of the way between source pixels.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBilinearDouble(void)
{
	int32	aiSource[4] = { 0, 100,
							200, 40 };
	int32	aiDest[16];
	int32	aiRow[4];

	Ptr<CImage> pcSource = OMalloc<CImage>(2, 1, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 4, 1);
	AssertTrue(ResampleBilinear(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiRow);
	AssertInt((int32)0, aiRow[0]);
	AssertInt((int32)25, aiRow[1]);
	AssertInt((int32)75, aiRow[2]);
	AssertInt((int32)100, aiRow[3]);

	pcSource = OMalloc<CImage>(2, 2, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	pcDest = MakeResampleDest(pcSource, 4, 4);
	AssertTrue(ResampleBilinear(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiDest);

	//The corners are the source pixels.
	AssertInt((int32)0, aiDest[0]);
	AssertInt((int32)100, aiDest[3]);
	AssertInt((int32)200, aiDest[12]);
	AssertInt((int32)40, aiDest[15]);

	//(0 * 0.75 + 100 * 0.25) * 0.75 + (200 * 0.75 + 40 * 0.25) * 0.25 = 58.75.
	AssertInt((int32)59, aiDest[5]);
	//(0 * 0.25 + 100 * 0.75) * 0.25 + (200 * 0.25 + 40 * 0.75) * 0.75 = 78.75.
	AssertInt((int32)79, aiDest[10]);
	//Column 0, row 1: 0 * 0.75 + 200 * 0.25.
	AssertInt((int32)50, aiDest[4]);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Halving puts dest pixels half way between pairs of source pixels so
// each is the average of a 2 x 2 block.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBilinearHalve(void)
{
	int32	aiSource[16] = {  10,  20,  30,  40,
							  50,  60,  70,  80,
							 100, 101, 200, 201,
							 102, 104, 202, 203 };
	int32	aiDest[4];

	Ptr<CImage> pcSource = OMalloc<CImage>(4, 4, PT_uint16, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 2, 2);
	AssertTrue(ResampleBilinear(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiDest);
	AssertInt((int32)35, aiDest[0]);
	AssertInt((int32)55, aiDest[1]);
	AssertInt((int32)102, aiDest[2]);		//101.75
	AssertInt((int32)202, aiDest[3]);		//201.5 rounds up.

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Every channel of a flat image stays flat at any size.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBilinearFlat(void)
{
	int32*	pai;
	int		i;
	int		iMismatches;
	size	c;
	int32	aiValues[3] = { 17, 200, 255 };

	Ptr<CImage> pcSource = OMalloc<CImage>(7, 5, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	pai = (int32*)malloc(sizeof(int32) * 13 * 3);
	for (c = 0; c < 3; c++)
	{
		for (i = 0; i < 35; i++)
		{
			pai[i] = aiValues[c];
		}
		SetResampleIntegers(pcSource, c, pai);
	}

	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 13, 3);
	AssertTrue(ResampleBilinear(pcSource, pcDest));

	iMismatches = 0;
	for (c = 0; c < 3; c++)
	{
		GetResampleIntegers(pcDest, c, pai);
		for (i = 0; i < 39; i++)
		{
			if (pai[i] != aiValues[c])
			{
				iMismatches++;
			}
		}
	}
	AssertInt(0, iMismatches);
	free(pai);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The integer maths gives the same answer as float64 maths, rounded.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBilinearMatchesFloat(void)
{
	int32		aiSource[13 * 7];
	int32		aiDest[29 * 17];
	float64		afSource[13 * 7];
	float64		afDest[29 * 17];
	int			i;
	int			iMismatches;
	int			iFarOff;
	float64		fDifference;

	for (i = 0; i < 13 * 7; i++)
	{
		aiSource[i] = (i * 7919 + 13) % 65536;
		afSource[i] = (float64)aiSource[i];
	}

	Ptr<CImage> pcSource = OMalloc<CImage>(13, 7, PT_uint16, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 29, 17);
	AssertTrue(ResampleBilinear(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiDest);

	Ptr<CImage> pcFloatSource = OMalloc<CImage>(13, 7, PT_float64, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleFloats(pcFloatSource, 0, afSource);
	Ptr<CImage> pcFloatDest = MakeResampleDest(pcFloatSource, 29, 17);
	AssertTrue(ResampleBilinear(pcFloatSource, pcFloatDest));
	GetResampleFloats(pcFloatDest, 0, afDest);

	iMismatches = 0;
	iFarOff = 0;
	for (i = 0; i < 29 * 17; i++)
	{
		fDifference = fabs((float64)aiDest[i] - afDest[i]);
		if (fDifference > 0.5 + 1e-6)
		{
			iFarOff++;
		}
		if (aiDest[i] != (int32)floor(afDest[i] + 0.5))
		{
			iMismatches++;
		}
	}
	AssertInt(0, iFarOff);
	AssertInt(0, iMismatches);

	pcSource = NULL;
	pcDest = NULL;
	pcFloatSource = NULL;
	pcFloatDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Bit sized channels are not byte aligned.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBilinearNybbles(void)
{
	int32	aiRed[2] = { 0, 15 };
	int32	aiGreen[2] = { 15, 0 };
	int32	aiBlue[2] = { 6, 6 };
	int32	aiRow[4];

	Ptr<CImage> pcSource = OMalloc<CImage>(2, 1, PT_nybble, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiRed);
	SetResampleIntegers(pcSource, 1, aiGreen);
	SetResampleIntegers(pcSource, 2, aiBlue);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 4, 1);
	AssertTrue(ResampleBilinear(pcSource, pcDest));

	GetResampleIntegers(pcDest, 0, aiRow);
	AssertInt((int32)0, aiRow[0]);
	AssertInt((int32)4, aiRow[1]);		//3.75
	AssertInt((int32)11, aiRow[2]);		//11.25
	AssertInt((int32)15, aiRow[3]);
	GetResampleIntegers(pcDest, 1, aiRow);
	AssertInt((int32)15, aiRow[0]);
	AssertInt((int32)11, aiRow[1]);
	AssertInt((int32)4, aiRow[2]);
	AssertInt((int32)0, aiRow[3]);
	GetResampleIntegers(pcDest, 2, aiRow);
	AssertInt((int32)6, aiRow[0]);
	AssertInt((int32)6, aiRow[3]);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBilinearFloat(void)
{
	float64	afSource[2] = { 0.0, 1.0 };
	float64	afRow[4];

	Ptr<CImage> pcSource = OMalloc<CImage>(2, 1, PT_float32, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleFloats(pcSource, 0, afSource);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 4, 1);
	AssertTrue(ResampleBilinear(pcSource, pcDest));
	GetResampleFloats(pcDest, 0, afRow);
	AssertDouble(0.0, afRow[0], 12);
	AssertDouble(0.25, afRow[1], 12);
	AssertDouble(0.75, afRow[2], 12);
	AssertDouble(1.0, afRow[3], 12);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Catmull-Rom reproduces straight lines away from the edges.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBicubicRamp(void)
{
	float64		afSource[10 * 3];
	float64		afDest[25 * 3];
	uint32		x;
	uint32		y;
	int			iMismatches;
	float64		fPosition;
	float64		fDifference;

	for (y = 0; y < 3; y++)
	{
		for (x = 0; x < 10; x++)
		{
			afSource[y * 10 + x] = x * 0.1 - 0.3;
		}
	}

	Ptr<CImage> pcSource = OMalloc<CImage>(10, 3, PT_float64, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleFloats(pcSource, 0, afSource);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 25, 3);
	AssertTrue(ResampleBicubic(pcSource, pcDest));
	GetResampleFloats(pcDest, 0, afDest);

	iMismatches = 0;
	for (y = 0; y < 3; y++)
	{
		for (x = 0; x < 25; x++)
		{
			fPosition = (x + 0.5) * 10.0 / 25.0 - 0.5;
			if ((fPosition < 1.0) || (fPosition > 7.0))
			{
				continue;
			}
			fDifference = afDest[y * 25 + x] - (fPosition * 0.1 - 0.3);
			if (fabs(fDifference) > 1e-12)
			{
				iMismatches++;
			}
		}
	}
	AssertInt(0, iMismatches);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The cubic overshoots at a hard edge.  Float channels keep the overshoot
// and integer channels are clamped.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleBicubicOvershoot(void)
{
	int32	aiSource[4] = { 0, 0, 255, 255 };
	float64	afSource[4] = { 0.0, 0.0, 1.0, 1.0 };
	int32	aiRow[8];
	float64	afRow[8];
	int		i;
	float64	fMinimum;
	float64	fMaximum;

	Ptr<CImage> pcSource = OMalloc<CImage>(4, 1, PT_float32, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleFloats(pcSource, 0, afSource);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 8, 1);
	AssertTrue(ResampleBicubic(pcSource, pcDest));
	GetResampleFloats(pcDest, 0, afRow);
	fMinimum = 0.0;
	fMaximum = 1.0;
	for (i = 0; i < 8; i++)
	{
		fMinimum = afRow[i] < fMinimum ? afRow[i] : fMinimum;
		fMaximum = afRow[i] > fMaximum ? afRow[i] : fMaximum;
	}
	AssertTrue(fMinimum < -0.01);
	AssertTrue(fMaximum > 1.01);

	pcSource = OMalloc<CImage>(4, 1, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	pcDest = MakeResampleDest(pcSource, 8, 1);
	AssertTrue(ResampleBicubic(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiRow);
	AssertInt((int32)0, aiRow[0]);
	AssertInt((int32)0, aiRow[2]);			//Would be below zero.
	AssertInt((int32)52, aiRow[3]);		//255 * 0.203125.
	AssertInt((int32)203, aiRow[4]);
	AssertInt((int32)255, aiRow[5]);		//Would be above 255.
	AssertInt((int32)255, aiRow[7]);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The RMS difference from a smooth image.
//
//////////////////////////////////////////////////////////////////////////
float64 ResampledWaveError(EImageResample eResample)
{
	CImageResampler	cResampler;
	float64			afSource[16 * 16];
	float64			afDest[64 * 64];
	uint32			x;
	uint32			y;
	float64			fSum;
	float64			fExpected;
	float64			fDifference;
	size			uiCount;

	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			afSource[y * 16 + x] = 0.5 + 0.4 * sin(2.0 * TEST_IMAGE_RESAMPLER_PI * (x + 0.5) / 8.0) * cos(2.0 * TEST_IMAGE_RESAMPLER_PI * (y + 0.5) / 16.0);
		}
	}
	Ptr<CImage> pcSource = OMalloc<CImage>(16, 16, PT_float64, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleFloats(pcSource, 0, afSource);

	cResampler.Init(eResample, 64, 64);
	Ptr<CImage> pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();
	AssertInt((int32)64, pcDest->GetWidth());
	AssertInt((int32)64, pcDest->GetHeight());
	GetResampleFloats(pcDest, 0, afDest);

	//Away from the edges, which are held.
	fSum = 0.0;
	uiCount = 0;
	for (y = 8; y < 56; y++)
	{
		for (x = 8; x < 56; x++)
		{
			fExpected = 0.5 + 0.4 * sin(2.0 * TEST_IMAGE_RESAMPLER_PI * (x + 0.5) / 32.0) * cos(2.0 * TEST_IMAGE_RESAMPLER_PI * (y + 0.5) / 64.0);
			fDifference = afDest[y * 64 + x] - fExpected;
			fSum += fDifference * fDifference;
			uiCount++;
		}
	}

	pcSource = NULL;
	pcDest = NULL;
	return sqrt(fSum / uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageResamplerMethods(void)
{
	float64	fNearest;
	float64	fBilinear;
	float64	fBicubic;

	fNearest = ResampledWaveError(IR_NearestNeighbour);
	fBilinear = ResampledWaveError(IR_Bilinear);
	fBicubic = ResampledWaveError(IR_Bicubic);
	AssertTrue(fBilinear < fNearest);
	AssertTrue(fBicubic < fBilinear);
	AssertTrue(fBicubic < 0.01);
}


//////////////////////////////////////////////////////////////////////////
//
// CImageResampler keeps the channels.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResamplerModify(void)
{
	CImageResampler	cResampler;
	int32			aiSource[6 * 4];
	int				i;

	for (i = 0; i < 6 * 4; i++)
	{
		aiSource[i] = ((int32)i * 37) & 0xFF;
	}
	Ptr<CImage> pcSource = OMalloc<CImage>(6, 4, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	for (i = 0; i < 4; i++)
	{
		SetResampleIntegers(pcSource, i, aiSource);
	}

	cResampler.Init(IR_Bilinear, 9, 5);
	Ptr<CImage> pcBilinear = cResampler.Modify(pcSource);
	cResampler.Kill();
	AssertInt((int32)9, pcBilinear->GetWidth());
	AssertInt((int32)5, pcBilinear->GetHeight());
	AssertTrue(pcBilinear->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP));
	AssertInt(PT_uint8, pcBilinear->GetPrimitiveType());

	cResampler.Init(IR_Bicubic, 3, 11);
	Ptr<CImage> pcBicubic = cResampler.Modify(pcSource);
	cResampler.Kill();
	AssertInt((int32)3, pcBicubic->GetWidth());
	AssertInt((int32)11, pcBicubic->GetHeight());
	AssertTrue(pcBicubic->IsSameFormat(pcSource));

	cResampler.Init(IR_Smart, 9, 5);
	Ptr<CImage> pcSmart = cResampler.Modify(pcSource);
	cResampler.Kill();
	AssertInt((int32)9, pcSmart->GetWidth());
	AssertInt((int32)5, pcSmart->GetHeight());
	AssertTrue(pcSmart->IsSameFormat(pcSource));

	pcSource = NULL;
	pcBilinear = NULL;
	pcBicubic = NULL;
	pcSmart = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Counts the values of channel 0 that are neither iLow nor iHigh.
//
//////////////////////////////////////////////////////////////////////////
int CountBlended(Ptr<CImage> pcImage, int32 iLow, int32 iHigh)
{
	int32*	pai;
	lsize	i;
	lsize	uiNumPixels;
	int		iCount;

	uiNumPixels = (lsize)pcImage->GetWidth() * pcImage->GetHeight();
	pai = (int32*)malloc(sizeof(int32) * uiNumPixels);
	GetResampleIntegers(pcImage, 0, pai);
	iCount = 0;
	for (i = 0; i < uiNumPixels; i++)
	{
		if ((pai[i] != iLow) && (pai[i] != iHigh))
		{
			iCount++;
		}
	}
	free(pai);
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Where neighbouring pixels are similar smart is bilinear.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleSmartSmooth(void)
{
	float64		afSource[9 * 6];
	float64		afSmart[20 * 11];
	float64		afBilinear[20 * 11];
	int			i;
	int			iMismatches;

	for (i = 0; i < 9 * 6; i++)
	{
		afSource[i] = 0.2 + (i % 9) * 0.03 + (i / 9) * 0.05;
	}
	Ptr<CImage> pcSource = OMalloc<CImage>(9, 6, PT_float64, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleFloats(pcSource, 0, afSource);

	Ptr<CImage> pcSmart = MakeResampleDest(pcSource, 20, 11);
	AssertTrue(ResampleSmart(pcSource, pcSmart));
	GetResampleFloats(pcSmart, 0, afSmart);
	Ptr<CImage> pcBilinear = MakeResampleDest(pcSource, 20, 11);
	AssertTrue(ResampleBilinear(pcSource, pcBilinear));
	GetResampleFloats(pcBilinear, 0, afBilinear);

	iMismatches = 0;
	for (i = 0; i < 20 * 11; i++)
	{
		if (fabs(afSmart[i] - afBilinear[i]) > 1e-12)
		{
			iMismatches++;
		}
	}
	AssertInt(0, iMismatches);

	pcSource = NULL;
	pcSmart = NULL;
	pcBilinear = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A hard edge stays hard, at the same place as nearest neighbour puts it.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleSmartEdge(void)
{
	int32	aiSource[16] = { 0, 0, 255, 255,
							 0, 0, 255, 255,
							 0, 0, 255, 255,
							 0, 0, 255, 255 };
	int32	aiDest[16 * 16];
	int		x;
	int		y;
	int		iMismatches;

	Ptr<CImage> pcSource = OMalloc<CImage>(4, 4, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	Ptr<CImage> pcSmart = MakeResampleDest(pcSource, 16, 16);
	AssertTrue(ResampleSmart(pcSource, pcSmart));
	GetResampleIntegers(pcSmart, 0, aiDest);

	iMismatches = 0;
	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			if (aiDest[y * 16 + x] != (x < 8 ? 0 : 255))
			{
				iMismatches++;
			}
		}
	}
	AssertInt(0, iMismatches);

	Ptr<CImage> pcBilinear = MakeResampleDest(pcSource, 16, 16);
	AssertTrue(ResampleBilinear(pcSource, pcBilinear));
	AssertInt(64, CountBlended(pcBilinear, 0, 255));

	pcSource = NULL;
	pcSmart = NULL;
	pcBilinear = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A staircase (the lower left triangle of an 8 x 8 image) is drawn closer
// to the straight line it stands for than nearest neighbour draws it,
// without blending.
//
//////////////////////////////////////////////////////////////////////////
int StaircaseMismatches(Ptr<CImage> pcImage)
{
	int32	aiDest[32 * 32];
	int		x;
	int		y;
	float64	fX;
	float64	fY;
	int		iMismatches;
	int32	iExpected;

	GetResampleIntegers(pcImage, 0, aiDest);
	iMismatches = 0;
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			fX = (x + 0.5) / 4.0 - 0.5;
			fY = (y + 0.5) / 4.0 - 0.5;
			if (fabs(fX - fY - 0.5) < 1e-9)
			{
				//On the line; either is right.
				continue;
			}
			iExpected = fX < fY + 0.5 ? 255 : 0;
			if (aiDest[y * 32 + x] != iExpected)
			{
				iMismatches++;
			}
		}
	}
	return iMismatches;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleSmartStaircase(void)
{
	CImageResampler	cResampler;
	int32			aiSource[8 * 8];
	int				x;
	int				y;
	int				iSmart;
	int				iNearest;

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			aiSource[y * 8 + x] = x <= y ? 255 : 0;
		}
	}
	Ptr<CImage> pcSource = OMalloc<CImage>(8, 8, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);

	cResampler.Init(IR_Smart, 32, 32);
	Ptr<CImage> pcSmart = cResampler.Modify(pcSource);
	cResampler.Kill();
	cResampler.Init(IR_NearestNeighbour, 32, 32);
	Ptr<CImage> pcNearest = cResampler.Modify(pcSource);
	cResampler.Kill();

	AssertInt(0, CountBlended(pcSmart, 0, 255));
	iSmart = StaircaseMismatches(pcSmart);
	iNearest = StaircaseMismatches(pcNearest);
	AssertTrue(iSmart * 4 < iNearest);

	pcSource = NULL;
	pcSmart = NULL;
	pcNearest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A lone pixel becomes a rounded blob: the 4 x 4 square nearest neighbour
// would draw without its corners.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleSmartLonePixel(void)
{
	int32	aiSource[5 * 5];
	int32	aiDest[20 * 20];
	int		i;
	int		x;
	int		y;
	int		iWhite;
	int		iOutside;

	for (i = 0; i < 25; i++)
	{
		aiSource[i] = 0;
	}
	aiSource[12] = 255;
	Ptr<CImage> pcSource = OMalloc<CImage>(5, 5, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiSource);
	Ptr<CImage> pcSmart = MakeResampleDest(pcSource, 20, 20);
	AssertTrue(ResampleSmart(pcSource, pcSmart));
	GetResampleIntegers(pcSmart, 0, aiDest);

	iWhite = 0;
	iOutside = 0;
	for (y = 0; y < 20; y++)
	{
		for (x = 0; x < 20; x++)
		{
			if (aiDest[y * 20 + x] == 255)
			{
				iWhite++;
				if ((x < 8) || (x > 11) || (y < 8) || (y > 11))
				{
					iOutside++;
				}
			}
		}
	}
	AssertInt(12, iWhite);
	AssertInt(0, iOutside);
	AssertInt((int32)0, aiDest[8 * 20 + 8]);
	AssertInt((int32)255, aiDest[8 * 20 + 9]);
	AssertInt(0, CountBlended(pcSmart, 0, 255));

	pcSource = NULL;
	pcSmart = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Colours are compared in YUV.  Red and blue aren't blended; colours a
// little apart are.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleSmartColours(void)
{
	int32	aiRed[2] = { 255, 0 };
	int32	aiGreen[2] = { 0, 0 };
	int32	aiBlue[2] = { 0, 255 };
	int32	aiClose[2] = { 100, 103 };
	int32	aiSame[2] = { 100, 100 };
	int32	aiRow[4];

	Ptr<CImage> pcSource = OMalloc<CImage>(2, 1, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiRed);
	SetResampleIntegers(pcSource, 1, aiGreen);
	SetResampleIntegers(pcSource, 2, aiBlue);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 4, 1);
	AssertTrue(ResampleSmart(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiRow);
	AssertInt((int32)255, aiRow[0]);
	AssertInt((int32)255, aiRow[1]);
	AssertInt((int32)0, aiRow[2]);
	AssertInt((int32)0, aiRow[3]);
	GetResampleIntegers(pcDest, 2, aiRow);
	AssertInt((int32)0, aiRow[1]);
	AssertInt((int32)255, aiRow[2]);

	SetResampleIntegers(pcSource, 0, aiClose);
	SetResampleIntegers(pcSource, 1, aiSame);
	SetResampleIntegers(pcSource, 2, aiSame);
	AssertTrue(ResampleSmart(pcSource, pcDest));
	GetResampleIntegers(pcDest, 0, aiRow);
	AssertInt((int32)100, aiRow[0]);
	AssertInt((int32)101, aiRow[1]);		//100.75
	AssertInt((int32)102, aiRow[2]);		//102.25
	AssertInt((int32)103, aiRow[3]);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Other channels, like opacity, are compared on their own.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampleSmartOpacity(void)
{
	int32	aiGrey[2] = { 80, 80 };
	int32	aiOpacity[2] = { 255, 0 };
	int32	aiRow[4];

	Ptr<CImage> pcSource = OMalloc<CImage>(2, 1, PT_uint8, IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_STOP);
	SetResampleIntegers(pcSource, 0, aiGrey);
	SetResampleIntegers(pcSource, 1, aiOpacity);
	Ptr<CImage> pcDest = MakeResampleDest(pcSource, 4, 1);
	AssertTrue(ResampleSmart(pcSource, pcDest));
	GetResampleIntegers(pcDest, 1, aiRow);
	AssertInt((int32)255, aiRow[0]);
	AssertInt((int32)255, aiRow[1]);
	AssertInt((int32)0, aiRow[2]);
	AssertInt((int32)0, aiRow[3]);
	GetResampleIntegers(pcDest, 0, aiRow);
	AssertInt((int32)80, aiRow[1]);
	AssertInt((int32)80, aiRow[2]);

	pcSource = NULL;
	pcDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Red, green and blue of pcImage, each an array of pixels.
//
//////////////////////////////////////////////////////////////////////////
void GetRealImageRGB(Ptr<CImage> pcImage, int32** papaiRGB)
{
	lsize	uiNumPixels;
	size	c;
	EChannel aeChannels[3] = { IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE };

	uiNumPixels = (lsize)pcImage->GetWidth() * pcImage->GetHeight();
	for (c = 0; c < 3; c++)
	{
		papaiRGB[c] = (int32*)malloc(sizeof(int32) * uiNumPixels);
		GetResampleIntegers(pcImage, pcImage->GetChannels()->GetIndexOfChannel(aeChannels[c]), papaiRGB[c]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The RMS difference (0 to 255) between two images the same size.
//
//////////////////////////////////////////////////////////////////////////
float64 RealImageError(Ptr<CImage> pcImage, Ptr<CImage> pcTruth)
{
	int32*	apaiImage[3];
	int32*	apaiTruth[3];
	lsize	uiNumPixels;
	lsize	i;
	size	c;
	float64	fSum;
	float64	fDifference;

	uiNumPixels = (lsize)pcImage->GetWidth() * pcImage->GetHeight();
	GetRealImageRGB(pcImage, apaiImage);
	GetRealImageRGB(pcTruth, apaiTruth);
	fSum = 0.0;
	for (c = 0; c < 3; c++)
	{
		for (i = 0; i < uiNumPixels; i++)
		{
			fDifference = (float64)(apaiImage[c][i] - apaiTruth[c][i]);
			fSum += fDifference * fDifference;
		}
		free(apaiImage[c]);
		free(apaiTruth[c]);
	}
	return sqrt(fSum / (uiNumPixels * 3));
}


//////////////////////////////////////////////////////////////////////////
//
// The fraction of neighbouring pixel pairs that differ by a lot (more
// than 48 in red, green or blue): how many hard edges there are.
//
//////////////////////////////////////////////////////////////////////////
float64 RealImageHardEdges(Ptr<CImage> pcImage)
{
	int32*	apaiRGB[3];
	uint32	uiWidth;
	uint32	uiHeight;
	uint32	x;
	uint32	y;
	size	c;
	lsize	uiPixel;
	size	uiHard;
	size	uiPairs;
	bool	bHorizontal;
	bool	bVertical;

	uiWidth = pcImage->GetWidth();
	uiHeight = pcImage->GetHeight();
	GetRealImageRGB(pcImage, apaiRGB);
	uiHard = 0;
	uiPairs = 0;
	for (y = 0; y + 1 < uiHeight; y++)
	{
		for (x = 0; x + 1 < uiWidth; x++)
		{
			uiPixel = (lsize)y * uiWidth + x;
			bHorizontal = false;
			bVertical = false;
			for (c = 0; c < 3; c++)
			{
				bHorizontal |= abs(apaiRGB[c][uiPixel] - apaiRGB[c][uiPixel + 1]) > 48;
				bVertical |= abs(apaiRGB[c][uiPixel] - apaiRGB[c][uiPixel + uiWidth]) > 48;
			}
			uiHard += (bHorizontal ? 1 : 0) + (bVertical ? 1 : 0);
			uiPairs += 2;
		}
	}
	for (c = 0; c < 3; c++)
	{
		free(apaiRGB[c]);
	}
	return (float64)uiHard / (float64)uiPairs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> ResampleRealImage(Ptr<CImage> pcSource, EImageResample eResample, const char* szName)
{
	CImageResampler	cResampler;
	CChars			szFile;
	Ptr<CImage>		pcDest;

	cResampler.Init(eResample, 640, 512);
	pcDest = cResampler.Modify(pcSource);
	cResampler.Kill();

	AssertInt((int32)640, pcDest->GetWidth());
	AssertInt((int32)512, pcDest->GetHeight());
	AssertTrue(pcDest->IsSameFormat(pcSource));

	szFile.Init("Output" _FS_ "RevoyLofiCyberpunk-");
	szFile.Append(szName);
	szFile.Append(".png");
	AssertTrue(WriteImage(pcDest, szFile.Text()));
	szFile.Kill();
	return pcDest;
}


//////////////////////////////////////////////////////////////////////////
//
// A 160 x 128 crop of David Revoy's "lofi cyberpunk" (CC BY 4.0, see
// Input/Resample/RevoyLofiCyberpunk.txt) resampled 4x by every method and
// compared with the same crop made at 640 x 512 from the original.  The
// results are written to Output/RevoyLofiCyberpunk-*.png.
//
//////////////////////////////////////////////////////////////////////////
void TestImageResamplerRealImage(void)
{
	float64		fNearestError;
	float64		fBilinearError;
	float64		fBicubicError;
	float64		fSmartError;
	float64		fTruthEdges;
	float64		fNearestEdges;
	float64		fBilinearEdges;
	float64		fBicubicEdges;
	float64		fSmartEdges;

	Ptr<CImage> pcSource = ReadImage("Input" _FS_ "Resample" _FS_ "RevoyLofiCyberpunk.png");
	Ptr<CImage> pcTruth = ReadImage("Input" _FS_ "Resample" _FS_ "RevoyLofiCyberpunk-640.png");
	AssertTrue(pcSource.IsNotNull());
	AssertTrue(pcTruth.IsNotNull());
	if (pcSource.IsNull() || pcTruth.IsNull())
	{
		return;
	}
	AssertInt((int32)160, pcSource->GetWidth());
	AssertInt((int32)128, pcSource->GetHeight());

	Ptr<CImage> pcNearest = ResampleRealImage(pcSource, IR_NearestNeighbour, "Nearest");
	Ptr<CImage> pcBilinear = ResampleRealImage(pcSource, IR_Bilinear, "Bilinear");
	Ptr<CImage> pcBicubic = ResampleRealImage(pcSource, IR_Bicubic, "Bicubic");
	Ptr<CImage> pcSmart = ResampleRealImage(pcSource, IR_Smart, "Smart");

	fNearestError = RealImageError(pcNearest, pcTruth);
	fBilinearError = RealImageError(pcBilinear, pcTruth);
	fBicubicError = RealImageError(pcBicubic, pcTruth);
	fSmartError = RealImageError(pcSmart, pcTruth);

	fTruthEdges = RealImageHardEdges(pcTruth);
	fNearestEdges = RealImageHardEdges(pcNearest);
	fBilinearEdges = RealImageHardEdges(pcBilinear);
	fBicubicEdges = RealImageHardEdges(pcBicubic);
	fSmartEdges = RealImageHardEdges(pcSmart);

	//Measured: nearest 9.5, bilinear 8.6, bicubic 8.3 and smart 9.0.  The original is painted with soft strokes, which bilinear and
	//bicubic suit, but smart is still closer than nearest neighbour.
	AssertTrue(fBicubicError < fBilinearError);
	AssertTrue(fBilinearError < fSmartError);
	AssertTrue(fSmartError < fNearestError);

	//Measured: 1.1% of neighbouring pixels in the original are hard edges, nearest 0.54%, smart 0.48% and bilinear and bicubic none.
	//Smart keeps most of the hard edges nearest neighbour keeps.
	AssertTrue(fNearestEdges < fTruthEdges);
	AssertTrue(fSmartEdges > fNearestEdges * 0.8);
	AssertTrue(fBilinearEdges < fSmartEdges * 0.1);
	AssertTrue(fBicubicEdges < fSmartEdges * 0.1);

	Ptr<CImage> pcExpected = ReadImage("Input" _FS_ "Resample" _FS_ "RevoyLofiCyberpunk-Smart-expected.png");
#ifdef _DEBUG
	AssertSameImage(pcExpected, pcSmart);
#else
	//Release is /fp:fast.  A pixel right on one of smart's similarity thresholds can fall the other way and change a
	//2 x 2 block (27 values of the million in the 64 bit build).  The error and edge checks above still hold.
	AssertSameImage(pcExpected, pcSmart, 100);
#endif // _DEBUG
	pcExpected = NULL;

	pcSource = NULL;
	pcTruth = NULL;
	pcNearest = NULL;
	pcBilinear = NULL;
	pcBicubic = NULL;
	pcSmart = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageResampler(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageResampleBilinearDouble();
	TestImageResampleBilinearHalve();
	TestImageResampleBilinearFlat();
	TestImageResampleBilinearMatchesFloat();
	TestImageResampleBilinearNybbles();
	TestImageResampleBilinearFloat();
	TestImageResampleBicubicRamp();
	TestImageResampleBicubicOvershoot();
	TestImageResampleSmartSmooth();
	TestImageResampleSmartEdge();
	TestImageResampleSmartStaircase();
	TestImageResampleSmartLonePixel();
	TestImageResampleSmartColours();
	TestImageResampleSmartOpacity();
	TestImageResamplerMethods();
	TestImageResamplerModify();
	TestImageResamplerRealImage();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

