#include "ModMixer.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds uiFrames frames of an 8 bit sample to a side of the mix, from
// uiFirstFrame: each frame adds the gain table's entry for the sample's
// byte at *puiOffset in the window, which then steps on by uiStep (16.16
// frames, the fraction carried in *puiFraction).  Stops after the frame
// that steps to uiEnd (at most MOD_SAMPLE_WINDOW) or past it.  Returns
// the frames mixed; *puiOffset and *puiFraction are where it stopped.
//
//////////////////////////////////////////////////////////////////////////
uint16 ModMixNearest8(SModMixSide* psMix, uint16 uiFirstFrame, uint16 uiFrames, SModSampleWindow* psSamples, uint16* puiOffset, uint16* puiFraction, uint16 uiEnd, uint32 uiStep, SModGainTable* psGains)
{
	uint16	uiOffset;
	uint16	uiFraction;
	uint16	uiStepFraction;
	uint16	uiStepFrames;
	uint16	uiFrame;
	uint16	uiLast;
	uint16	uiIndex;

	uiOffset = *puiOffset;
	uiFraction = *puiFraction;
	uiStepFraction = (uint16)uiStep;
	uiStepFrames = (uint16)(uiStep >> 16);
	uiFrame = uiFirstFrame;
	uiLast = (uint16)(uiFirstFrame + uiFrames);
	while (uiFrame < uiLast)
	{
		uiIndex = (uint16)(uint8)psSamples->aiFrames[uiOffset];
		psMix->aiFrames[uiFrame] = (int16)(psMix->aiFrames[uiFrame] + psGains->aiGains[uiIndex]);
		uiFrame++;

		uiOffset = (uint16)(uiOffset + uiStepFrames);
		uiFraction = (uint16)(uiFraction + uiStepFraction);
		if (uiFraction < uiStepFraction)
		{
			uiOffset++;
		}
		if (uiOffset >= uiEnd)
		{
			break;
		}
	}
	*puiOffset = uiOffset;
	*puiFraction = uiFraction;
	return (uint16)(uiFrame - uiFirstFrame);
}


//////////////////////////////////////////////////////////////////////////
//
// The gain table for iGain (0 to 4096): (sample * iGain) >> (MOD_MIX_SHIFT
// - 8) for each 8 bit sample (-128 to 127), by the sample's byte.  With
// the gain split into its high bits and its low 5, (sample * gain) >> 5
// is sample * high + (sample * low) >> 5 exactly.  The samples are
// stepped through from -128 adding the parts, no multiplies: the low
// part's sum is kept as a quotient and a remainder (-128 * low is a
// multiple of 32, and low is less than 32 so the remainder carries at
// most once a step), so there are no shifts either.
//
//////////////////////////////////////////////////////////////////////////
void ModBuildGainTable(SModGainTable* psTable, int16 iGain)
{
	int16	iHigh;
	int16	iLow;
	int16	iSum;
	int16	iRemainder;
	uint16	uiByte;

	iHigh = (int16)(iGain >> (MOD_MIX_SHIFT - 8));
	iLow = (int16)(iGain & ((1 << (MOD_MIX_SHIFT - 8)) - 1));
	iSum = (int16)(-128 * iHigh - 4 * iLow);
	iRemainder = 0;
	for (uiByte = 0x80; uiByte < 0x100; uiByte++)
	{
		psTable->aiGains[uiByte] = iSum;
		iSum = (int16)(iSum + iHigh);
		iRemainder = (int16)(iRemainder + iLow);
		if (iRemainder >= (1 << (MOD_MIX_SHIFT - 8)))
		{
			iRemainder = (int16)(iRemainder - (1 << (MOD_MIX_SHIFT - 8)));
			iSum++;
		}
	}
	for (uiByte = 0; uiByte < 0x80; uiByte++)
	{
		psTable->aiGains[uiByte] = iSum;
		iSum = (int16)(iSum + iHigh);
		iRemainder = (int16)(iRemainder + iLow);
		if (iRemainder >= (1 << (MOD_MIX_SHIFT - 8)))
		{
			iRemainder = (int16)(iRemainder - (1 << (MOD_MIX_SHIFT - 8)));
			iSum++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiFrames frames of the mix to the sound board's unsigned 8 bit samples:
// mix >> 7, clipped to -128 to 127, + 128.  That is the high byte of the
// mix clipped to 15 bits, doubled and + $8000 (a shift by 8 is a byte
// swap on the W65C816, a shift by 7 seven shifts).
//
//////////////////////////////////////////////////////////////////////////
void ModMixToUnsigned8(SModUnsigned8* psOut, SModMix* psMix, uint16 uiFrames)
{
	uint16	i;
	int16	iSample;

	for (i = 0; i < uiFrames; i++)
	{
		iSample = psMix->sLeft.aiFrames[i];
		if (iSample > 16383)
		{
			iSample = 16383;
		}
		else if (iSample < -16384)
		{
			iSample = -16384;
		}
		psOut->auiLeft[i] = (uint8)((uint16)(iSample * 2 + 0x8000) >> 8);

		iSample = psMix->sRight.aiFrames[i];
		if (iSample > 16383)
		{
			iSample = 16383;
		}
		else if (iSample < -16384)
		{
			iSample = -16384;
		}
		psOut->auiRight[i] = (uint8)((uint16)(iSample * 2 + 0x8000) >> 8);
	}
}
