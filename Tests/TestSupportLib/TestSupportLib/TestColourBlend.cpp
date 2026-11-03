#include "SupportLib/ColourBlend.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// True if uiResult is uiNumerator / uiDenominator rounded to the nearest,
// worked out without dividing.
//
//////////////////////////////////////////////////////////////////////////
static bool IsNearest(uint32 uiResult, uint32 uiNumerator, uint32 uiDenominator)
{
	int32	iDifference;

	iDifference = (int32)(2 * uiResult * uiDenominator) - (int32)(2 * uiNumerator);
	return (iDifference <= (int32)uiDenominator) && (iDifference >= -(int32)uiDenominator);
}


//////////////////////////////////////////////////////////////////////////
//
// d (1 - c) + s c, to the nearest.  No coverage leaves the destination,
// all of it is the source, and a colour lerped with itself is unchanged.
//
//////////////////////////////////////////////////////////////////////////
void TestColourBlendLerp(void)
{
	size	d;
	size	s;
	size	c;
	bool	bValid;

	bValid = true;
	for (d = 0; d <= 255; d += 3)
	{
		for (s = 0; s <= 255; s += 3)
		{
			bValid &= (BlendLerpByte(d, s, 0) == d);
			bValid &= (BlendLerpByte(d, s, 255) == s);
		}
	}
	AssertTrue(bValid);

	bValid = true;
	for (d = 0; d <= 255; d++)
	{
		for (c = 0; c <= 255; c += 5)
		{
			bValid &= (BlendLerpByte(d, d, c) == d);
		}
	}
	AssertTrue(bValid);

	bValid = true;
	for (d = 0; d <= 255; d += 17)
	{
		for (s = 0; s <= 255; s += 17)
		{
			for (c = 0; c <= 255; c += 15)
			{
				bValid &= IsNearest(BlendLerpByte(d, s, c), (uint32)(d * (255 - c) + s * c), 255);
			}
		}
	}
	AssertTrue(bValid);

	AssertInt(128, BlendLerpByte(0, 255, 128));
	AssertInt(127, BlendLerpByte(255, 0, 128));
	AssertInt(255, BlendLerpByte(255, 255, 255));
}


//////////////////////////////////////////////////////////////////////////
//
// s + d (1 - s), to the nearest.
//
//////////////////////////////////////////////////////////////////////////
void TestColourBlendOver(void)
{
	size	d;
	size	s;
	bool	bValid;

	bValid = true;
	for (d = 0; d <= 255; d++)
	{
		bValid &= (BlendOverByte(d, 0) == d);
		bValid &= (BlendOverByte(d, 255) == 255);
		bValid &= (BlendOverByte(255, d) == 255);
		bValid &= (BlendOverByte(0, d) == d);
	}
	AssertTrue(bValid);

	bValid = true;
	for (d = 0; d <= 255; d += 5)
	{
		for (s = 0; s <= 255; s += 5)
		{
			bValid &= IsNearest(BlendOverByte(d, s) - (uint32)s, (uint32)(d * (255 - s)), 255);
		}
	}
	AssertTrue(bValid);
	AssertInt(192, BlendOverByte(128, 128));
}


//////////////////////////////////////////////////////////////////////////
//
// d (1 - c), to the nearest.
//
//////////////////////////////////////////////////////////////////////////
void TestColourBlendErase(void)
{
	size	d;
	size	c;
	bool	bValid;

	bValid = true;
	for (d = 0; d <= 255; d++)
	{
		bValid &= (BlendEraseByte(d, 0) == d);
		bValid &= (BlendEraseByte(d, 255) == 0);
		bValid &= (BlendEraseByte(0, d) == 0);
	}
	AssertTrue(bValid);

	bValid = true;
	for (d = 0; d <= 255; d += 5)
	{
		for (c = 0; c <= 255; c += 5)
		{
			bValid &= IsNearest(BlendEraseByte(d, c), (uint32)(d * (255 - c)), 255);
		}
	}
	AssertTrue(bValid);
	AssertInt(64, BlendEraseByte(128, 128));
}


//////////////////////////////////////////////////////////////////////////
//
// s + d (1 - a), the destination rounded to the nearest and the sum held
// at 255.
//
//////////////////////////////////////////////////////////////////////////
void TestColourBlendPremultiplied(void)
{
	size	d;
	size	s;
	size	a;
	uint32	uiExpected;
	bool	bValid;

	bValid = true;
	for (d = 0; d <= 255; d += 15)
	{
		for (s = 0; s <= 255; s += 15)
		{
			for (a = 0; a <= 255; a += 15)
			{
				uiExpected = (uint32)s + ((uint32)(d * a) + 127) / 255;
				if (uiExpected > 255)
				{
					uiExpected = 255;
				}
				bValid &= (BlendPremultipliedByte(s, d, a) == uiExpected);
			}
		}
	}
	AssertTrue(bValid);
	AssertInt(255, BlendPremultipliedByte(200, 255, 255));
	AssertInt(200, BlendPremultipliedByte(200, 255, 0));
	AssertInt(100, BlendPremultipliedByte(0, 100, 255));
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels are opacity, red, green then blue and not multiplied by their
// opacity.
//
//////////////////////////////////////////////////////////////////////////
static void SetPixel(uint8* puiPixel, size uiOpacity, size uiRed, size uiGreen, size uiBlue)
{
	puiPixel[0] = (uint8)uiOpacity;
	puiPixel[1] = (uint8)uiRed;
	puiPixel[2] = (uint8)uiGreen;
	puiPixel[3] = (uint8)uiBlue;
}


//////////////////////////////////////////////////////////////////////////
//
// The source's coverage of 255ths over the destination.
//
//////////////////////////////////////////////////////////////////////////
void TestColourBlendStraightOver(void)
{
	uint8	auiSource[4];
	uint8	auiDest[4];
	size	uiSourceOpacity;
	size	uiDestOpacity;
	size	uiCoverage;
	size	uiColour;
	size	i;
	uint32	uiSourceShown;
	uint32	uiDestShown;
	uint32	uiOpacity;
	bool	bValid;

	//Over nothing the source is copied, its opacity scaled by the coverage.
	SetPixel(auiSource, 200, 10, 20, 30);
	SetPixel(auiDest, 0, 99, 99, 99);
	BlendStraightOverPixel(auiDest, auiSource, 128);
	AssertInt(100, auiDest[0]);
	AssertInt(10, auiDest[1]);
	AssertInt(20, auiDest[2]);
	AssertInt(30, auiDest[3]);

	//Opaque and fully covering, the source replaces the destination.
	SetPixel(auiSource, 255, 10, 20, 30);
	SetPixel(auiDest, 128, 99, 99, 99);
	BlendStraightOverPixel(auiDest, auiSource, 255);
	AssertInt(255, auiDest[0]);
	AssertInt(10, auiDest[1]);
	AssertInt(20, auiDest[2]);
	AssertInt(30, auiDest[3]);

	//No coverage leaves the destination as it was.
	SetPixel(auiSource, 255, 10, 20, 30);
	SetPixel(auiDest, 255, 40, 50, 60);
	BlendStraightOverPixel(auiDest, auiSource, 0);
	AssertInt(255, auiDest[0]);
	AssertInt(40, auiDest[1]);
	AssertInt(50, auiDest[2]);
	AssertInt(60, auiDest[3]);

	//Half covering red over opaque blue.
	SetPixel(auiSource, 255, 255, 0, 0);
	SetPixel(auiDest, 255, 0, 0, 255);
	BlendStraightOverPixel(auiDest, auiSource, 128);
	AssertInt(255, auiDest[0]);
	AssertInt(128, auiDest[1]);
	AssertInt(0, auiDest[2]);
	AssertInt(127, auiDest[3]);

	//Half opaque over half opaque.
	SetPixel(auiSource, 128, 100, 100, 100);
	SetPixel(auiDest, 128, 100, 100, 100);
	BlendStraightOverPixel(auiDest, auiSource, 255);
	AssertInt(192, auiDest[0]);

	//A colour drawn over itself doesn't change it, whatever the opacities and coverage.
	bValid = true;
	for (uiColour = 0; uiColour <= 255; uiColour += 15)
	{
		for (uiSourceOpacity = 0; uiSourceOpacity <= 255; uiSourceOpacity += 51)
		{
			for (uiDestOpacity = 1; uiDestOpacity <= 255; uiDestOpacity += 51)
			{
				for (uiCoverage = 0; uiCoverage <= 255; uiCoverage += 51)
				{
					SetPixel(auiSource, uiSourceOpacity, uiColour, 255 - uiColour, uiColour / 2);
					SetPixel(auiDest, uiDestOpacity, uiColour, 255 - uiColour, uiColour / 2);
					BlendStraightOverPixel(auiDest, auiSource, uiCoverage);
					bValid &= (auiDest[1] == uiColour) && (auiDest[2] == 255 - uiColour) && (auiDest[3] == uiColour / 2);
				}
			}
		}
	}
	AssertTrue(bValid);

	//Otherwise each colour is the source and destination weighted by how much of each shows, to the nearest.
	bValid = true;
	for (uiSourceOpacity = 0; uiSourceOpacity <= 255; uiSourceOpacity += 51)
	{
		for (uiDestOpacity = 1; uiDestOpacity <= 255; uiDestOpacity += 51)
		{
			for (uiCoverage = 0; uiCoverage <= 255; uiCoverage += 51)
			{
				for (i = 0; i <= 255; i += 85)
				{
					SetPixel(auiSource, uiSourceOpacity, i, 255 - i, 255);
					SetPixel(auiDest, uiDestOpacity, 255 - i, i, 0);
					BlendStraightOverPixel(auiDest, auiSource, uiCoverage);

					uiSourceShown = ((uint32)(uiSourceOpacity * uiCoverage) + 127) / 255;
					if (uiSourceShown == 255)
					{
						bValid &= (auiDest[0] == 255) && (auiDest[1] == i) && (auiDest[3] == 255);
						continue;
					}
					uiDestShown = ((uint32)(uiDestOpacity * (255 - uiSourceShown)) + 127) / 255;
					uiOpacity = uiSourceShown + uiDestShown;
					bValid &= (auiDest[0] == uiOpacity);
					if (uiOpacity > 0)
					{
						bValid &= IsNearest(auiDest[1], (uint32)(i * uiSourceShown + (255 - i) * uiDestShown), uiOpacity);
						bValid &= IsNearest(auiDest[2], (uint32)((255 - i) * uiSourceShown + i * uiDestShown), uiOpacity);
						bValid &= IsNearest(auiDest[3], (uint32)(255 * uiSourceShown), uiOpacity);
					}
				}
			}
		}
	}
	AssertTrue(bValid);
}


//////////////////////////////////////////////////////////////////////////
//
// The byte blends are worked out in size, 16 bits on the W65C816, so
// these run on it too (TestSupportLib65).
//
//////////////////////////////////////////////////////////////////////////
void TestColourBlend(void)
{
	BeginTests();

	TestColourBlendLerp();
	TestColourBlendOver();
	TestColourBlendErase();
	TestColourBlendPremultiplied();
	TestColourBlendStraightOver();

	TestStatistics();
}
