#include <string.h>
#include <stdlib.h>
#include "BaseLib/TypeNames.h"
#include "TestLib/Assert.h"
#include "AssertImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertChannel(EImagePurpose ePurpose, EChannelType eType, EPrimitiveType ePrimitive, SChannel* psChannel, size iLine, char* szFile)
{
	size			uiActualChannel;
	size			uiExpectedChannel;
	CChars			szExpected;
	CChars			szActual;
	EImagePurpose	eActualPurpose;
	EChannelType	eActualType;

	uiExpectedChannel = CHANNEL(ePurpose, eType);
	uiActualChannel = psChannel->iChannel;

	if (uiExpectedChannel != uiActualChannel)
	{
		eActualPurpose = (EImagePurpose)((psChannel->iChannel - 1) / NUM_CHANNEL_TYPES);
		eActualType = (EChannelType)((psChannel->iChannel - 1) % NUM_CHANNEL_TYPES);

		szExpected.Init("(");
		szActual.Init("(");
		PrintImagePurpose(ePurpose, &szExpected);
		PrintImagePurpose(eActualPurpose, &szActual);
		szExpected.Append(", ");
		szActual.Append(", ");
		PrintChannelType(eType, &szExpected);
		PrintChannelType(eActualType, &szActual);
		szExpected.Append(")");
		szActual.Append(")");

		Failed(szExpected.Text(), szActual.Text(), NULL, iLine, szFile, false);
		szExpected.Kill();
		szActual.Kill();
		return false;
	}

	if (ePrimitive != psChannel->eType)
	{
		szExpected.Init();
		szActual.Init();
		PrintPrimitiveType(ePrimitive, &szExpected);
		PrintPrimitiveType(psChannel->eType, &szActual);

		Failed(szExpected.Text(), szActual.Text(), NULL, iLine, szFile, false);
		szExpected.Kill();
		szActual.Kill();
		return false;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether one pixel's value for a channel is the same in both images.  In
// release, which is /fp:fast, the float maths that made the value can be
// reordered or fused and land it one step away, so 8 and 16 bit values can
// be off by one and float values by an ulp.
//
//////////////////////////////////////////////////////////////////////////
static bool SameChannelValue(uint8* puiExpected, uint8* puiActual, EPrimitiveType eType, size uiByteSize)
{
	if (memcmp(puiExpected, puiActual, uiByteSize) == 0)
	{
		return true;
	}

#ifndef _DEBUG
	uint8	ui8Expected;
	uint8	ui8Actual;
	uint16	ui16Expected;
	uint16	ui16Actual;
	int32	iExpected;
	int32	iActual;
	int64	lliExpected;
	int64	lliActual;

	if (eType == PT_uint8)
	{
		memcpy(&ui8Expected, puiExpected, 1);
		memcpy(&ui8Actual, puiActual, 1);
		return abs((int)ui8Expected - (int)ui8Actual) <= 1;
	}
	else if (eType == PT_uint16)
	{
		memcpy(&ui16Expected, puiExpected, 2);
		memcpy(&ui16Actual, puiActual, 2);
		return abs((int)ui16Expected - (int)ui16Actual) <= 1;
	}
	else if (eType == PT_float32)
	{
		//Signed magnitude bits in order, so neighbouring floats are one apart.
		memcpy(&iExpected, puiExpected, 4);
		memcpy(&iActual, puiActual, 4);
		lliExpected = (iExpected < 0) ? -(int64)(iExpected & 0x7FFFFFFF) : (int64)iExpected;
		lliActual = (iActual < 0) ? -(int64)(iActual & 0x7FFFFFFF) : (int64)iActual;
		return ((lliExpected - lliActual) <= 1) && ((lliActual - lliExpected) <= 1);
	}
	else if (eType == PT_float64)
	{
		memcpy(&lliExpected, puiExpected, 8);
		memcpy(&lliActual, puiActual, 8);
		lliExpected = (lliExpected < 0) ? (int64)0x8000000000000000ULL - lliExpected : lliExpected;
		lliActual = (lliActual < 0) ? (int64)0x8000000000000000ULL - lliActual : lliActual;
		return ((lliExpected - lliActual) <= 1) && ((lliActual - lliExpected) <= 1);
	}
#endif // _DEBUG

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Compares channel by channel so images with the same channels in a
// different order (LoadPNG puts opacity first) are the same.
//
//////////////////////////////////////////////////////////////////////////
void AssertSameImage(Ptr<CImage> pcExpected, Ptr<CImage> pcActual)
{
	AssertSameImage(pcExpected, pcActual, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// As above but up to iAllowedMismatches channel values may differ.
//
//////////////////////////////////////////////////////////////////////////
void AssertSameImage(Ptr<CImage> pcExpected, Ptr<CImage> pcActual, int iAllowedMismatches)
{
	CArraySize		aiExpectedChannels;
	CArraySize		aiActualChannels;
	size			i;
	size			uiChannel;
	CChannel*		pcExpectedChannel;
	CChannel*		pcActualChannel;
	lsize			uiNumPixels;
	size			uiExpectedStride;
	size			uiActualStride;
	uint8*			puiExpected;
	uint8*			puiActual;
	lsize			uiPixel;
	int32			iMismatches;

	AssertTrue(pcExpected.IsNotNull());
	AssertTrue(pcActual.IsNotNull());
	if (pcExpected.IsNull() || pcActual.IsNull())
	{
		return;
	}

	AssertInt((int32)pcExpected->GetWidth(), pcActual->GetWidth());
	AssertInt((int32)pcExpected->GetHeight(), pcActual->GetHeight());
	if ((pcExpected->GetWidth() != pcActual->GetWidth()) || (pcExpected->GetHeight() != pcActual->GetHeight()))
	{
		return;
	}

	aiExpectedChannels.Init();
	aiActualChannels.Init();
	pcExpected->GetAllChannels(&aiExpectedChannels);
	pcActual->GetAllChannels(&aiActualChannels);
	AssertSize(aiExpectedChannels.NumElements(), aiActualChannels.NumElements());

	uiNumPixels = (lsize)pcExpected->GetWidth() * pcExpected->GetHeight();
	uiExpectedStride = (size)(pcExpected->GetByteSize() / uiNumPixels);
	uiActualStride = (size)(pcActual->GetByteSize() / uiNumPixels);
	puiExpected = (uint8*)pcExpected->GetData();
	puiActual = (uint8*)pcActual->GetData();
	iMismatches = 0;
	for (i = 0; i < aiExpectedChannels.NumElements(); i++)
	{
		uiChannel = aiExpectedChannels.GetValue(i);
		pcExpectedChannel = pcExpected->GetChannel(uiChannel);
		pcActualChannel = pcActual->GetChannel(uiChannel);
		AssertNotNull(pcActualChannel);
		if (pcActualChannel == NULL)
		{
			continue;
		}
		AssertInt((int)pcExpectedChannel->eType, (int)pcActualChannel->eType);
		if (pcExpectedChannel->eType != pcActualChannel->eType)
		{
			continue;
		}
		for (uiPixel = 0; uiPixel < uiNumPixels; uiPixel++)
		{
			if (!SameChannelValue(&puiExpected[uiPixel * uiExpectedStride + pcExpectedChannel->miByteOffset], &puiActual[uiPixel * uiActualStride + pcActualChannel->miByteOffset], pcExpectedChannel->eType, pcExpectedChannel->miByteSize))
			{
				iMismatches++;
			}
		}
	}
	if (iMismatches <= iAllowedMismatches)
	{
		iMismatches = 0;
	}
	AssertInt((int32)0, iMismatches);

	aiExpectedChannels.Kill();
	aiActualChannels.Kill();
}


