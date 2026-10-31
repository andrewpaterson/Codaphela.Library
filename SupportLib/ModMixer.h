#ifndef __MOD_MIXER_H__
#define __MOD_MIXER_H__
#include "BaseLib/PrimitiveTypes.h"


//CModPlayer's inner loops: mixing a channel's 8 bit samples through a gain table into a side of the 16 bit mix, the
//gain tables, and the mix to the HCP65 sound board's unsigned 8 bit samples.
//
//Written for the W65C816's code generator: the tables, blocks and samples are arrays (in structs) indexed with 16 bit
//subscripts, so they are indexed with Y (a pointer indexed would add 32 bit addresses), the sums are 16 bits and there
//are no multiplies.
//
//The mix is the sum of each channel's sample (Q8: an 8 bit sample * 256) * gain >> MOD_MIX_SHIFT.  A gain table holds
//that for each 8 bit sample: (sample * gain) >> (MOD_MIX_SHIFT - 8), by the sample's byte.
#define MOD_MIX_SHIFT				13
#define MOD_GAIN_TABLE_SIZE			256
#define MOD_MIX_BLOCK				256			//Frames mixed at a time.
#define MOD_SAMPLE_WINDOW			0xF000		//The most frames ModMixNearest8 is given of a sample at once (the steps from its end
												//stay inside 64KB).


//A channel side's gain table: (sample * gain) >> (MOD_MIX_SHIFT - 8) by the sample's byte.
struct SModGainTable
{
	int16	aiGains[MOD_GAIN_TABLE_SIZE];
};


//MOD_MIX_BLOCK frames of one side of the mix.
struct SModMixSide
{
	int16	aiFrames[MOD_MIX_BLOCK];
};


//MOD_MIX_BLOCK stereo frames of the mix.
struct SModMix
{
	SModMixSide	sLeft;
	SModMixSide	sRight;
};


//Up to 64KB of a sample's 8 bit frames, from where it is mixed (only those up to the window's end are read).
struct SModSampleWindow
{
	int8	aiFrames[0xFFFF];
};


//MOD_MIX_BLOCK frames as the HCP65's sound board plays them: unsigned 8 bit samples, $80 the middle, left and right
//apart (its RAM A and RAM B).
struct SModUnsigned8
{
	uint8	auiLeft[MOD_MIX_BLOCK];
	uint8	auiRight[MOD_MIX_BLOCK];
};


uint16	ModMixNearest8(SModMixSide* psMix, uint16 uiFirstFrame, uint16 uiFrames, SModSampleWindow* psSamples, uint16* puiOffset, uint16* puiFraction, uint16 uiEnd, uint32 uiStep, SModGainTable* psGains);
void	ModBuildGainTable(SModGainTable* psTable, int16 iGain);
void	ModMixToUnsigned8(SModUnsigned8* psOut, SModMix* psMix, uint16 uiFrames);


#endif // __MOD_MIXER_H__
