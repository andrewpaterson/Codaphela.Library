#include "SupportLib/ColourARGB32.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The six colours round the hues and back; a grey has no hue or
// saturation; a hue of a whole turn, or less than none, is the hue it
// comes round to.
//
//////////////////////////////////////////////////////////////////////////
void TestColourHSVConversion(void)
{
	float32	afRGB[6][3] = { { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 }, { 0, 1, 1 }, { 0, 0, 1 }, { 1, 0, 1 } };
	float32	fHue;
	float32	fSaturation;
	float32	fValue;
	float32	fRed;
	float32	fGreen;
	float32	fBlue;
	size	i;

	for (i = 0; i < 6; i++)
	{
		ColourRGBToHSV(afRGB[i][0], afRGB[i][1], afRGB[i][2], &fHue, &fSaturation, &fValue);
		AssertFloat((float32)i / 6.0f, fHue, 6);
		AssertFloat(1.0f, fSaturation, 6);
		AssertFloat(1.0f, fValue, 6);
		ColourHSVToRGB(fHue, fSaturation, fValue, &fRed, &fGreen, &fBlue);
		AssertFloat(afRGB[i][0], fRed, 5);
		AssertFloat(afRGB[i][1], fGreen, 5);
		AssertFloat(afRGB[i][2], fBlue, 5);
	}

	ColourRGBToHSV(0.4f, 0.4f, 0.4f, &fHue, &fSaturation, &fValue);
	AssertFloat(0.0f, fHue, 6);
	AssertFloat(0.0f, fSaturation, 6);
	AssertFloat(0.4f, fValue, 6);
	ColourRGBToHSV(0.0f, 0.0f, 0.0f, &fHue, &fSaturation, &fValue);
	AssertFloat(0.0f, fSaturation, 6);
	AssertFloat(0.0f, fValue, 6);

	//A dull orange and back.
	ColourRGBToHSV(0.8f, 0.5f, 0.2f, &fHue, &fSaturation, &fValue);
	AssertFloat(0.0833333f, fHue, 5);
	AssertFloat(0.75f, fSaturation, 5);
	AssertFloat(0.8f, fValue, 6);
	ColourHSVToRGB(fHue, fSaturation, fValue, &fRed, &fGreen, &fBlue);
	AssertFloat(0.8f, fRed, 5);
	AssertFloat(0.5f, fGreen, 5);
	AssertFloat(0.2f, fBlue, 5);

	ColourHSVToRGB(1.0f, 1.0f, 1.0f, &fRed, &fGreen, &fBlue);
	AssertFloat(1.0f, fRed, 6);
	AssertFloat(0.0f, fGreen, 6);
	AssertFloat(0.0f, fBlue, 6);
	ColourHSVToRGB(-1.0f / 3.0f, 1.0f, 0.5f, &fRed, &fGreen, &fBlue);
	AssertFloat(0.0f, fRed, 5);
	AssertFloat(0.0f, fGreen, 5);
	AssertFloat(0.5f, fBlue, 5);
	ColourHSVToRGB(0.25f, 0.0f, 0.3f, &fRed, &fGreen, &fBlue);
	AssertFloat(0.3f, fRed, 6);
	AssertFloat(0.3f, fGreen, 6);
	AssertFloat(0.3f, fBlue, 6);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestColourHSV(void)
{
	BeginTests();

	TestColourHSVConversion();

	TestStatistics();
}

