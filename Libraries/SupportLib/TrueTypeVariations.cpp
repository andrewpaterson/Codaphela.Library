/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "BaseLib/FixedPoint.h"
#include "BaseLib/StringHelper.h"
#include "TrueTypeFont.h"
#include "TrueTypeVariations.h"


//fvar axis flags.
#define TRUE_TYPE_AXIS_HIDDEN				0x0001

//gvar tupleVariationCount flags.
#define TRUE_TYPE_SHARED_POINT_NUMBERS		0x8000
#define TRUE_TYPE_TUPLE_COUNT_MASK			0x0FFF

//gvar tupleIndex flags.
#define TRUE_TYPE_EMBEDDED_PEAK_TUPLE		0x8000
#define TRUE_TYPE_INTERMEDIATE_REGION		0x4000
#define TRUE_TYPE_PRIVATE_POINT_NUMBERS		0x2000
#define TRUE_TYPE_TUPLE_INDEX_MASK			0x0FFF

//Packed point numbers.
#define TRUE_TYPE_POINTS_ARE_WORDS			0x80
#define TRUE_TYPE_POINT_RUN_COUNT_MASK		0x7F

//Packed deltas.  Both size flags set are 32 bit deltas.
#define TRUE_TYPE_DELTAS_SIZE_MASK			0xC0
#define TRUE_TYPE_DELTAS_ARE_ZERO			0x80
#define TRUE_TYPE_DELTAS_ARE_WORDS			0x40
#define TRUE_TYPE_DELTAS_ARE_LONGS			0xC0
#define TRUE_TYPE_DELTA_RUN_COUNT_MASK		0x3F

//Item variation data wordDeltaCount.
#define TRUE_TYPE_LONG_WORDS				0x8000
#define TRUE_TYPE_WORD_DELTA_COUNT_MASK		0x7FFF


//The MVAR tag of each ETrueTypeMetric.
const char* gaszTrueTypeMetricTags[TTM_NumMetrics] =
{
	"hasc",
	"hdsc",
	"hlgp",
	"undo",
	"unds",
	"stro",
	"strs",
	"sbys",
	"sbyo",
	"spys",
	"spyo",
};


//////////////////////////////////////////////////////////////////////////
//
// FreeType rounds deltas half up (FT_fixedToInt) so 2.5 is 3 and -2.5 is
// -2.
//
//////////////////////////////////////////////////////////////////////////
int32 TrueTypeRoundDelta(int64 iDelta)
{
	return (int32)DivideFloor(iDelta + TRUE_TYPE_FIXED_ONE / 2, TRUE_TYPE_FIXED_ONE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::Init(CTrueTypeFont* pcFont)
{
	size	i;

	mpcFont = pcFont;
	masAxes.Init();
	masInstances.Init();
	maiAxisValues.Init();
	maiCoordinates.Init();
	mbVaried = false;

	muiAvarOffset = 0;
	muiAvarLength = 0;

	muiGvarOffset = 0;
	muiGvarLength = 0;
	muiGvarAxisCount = 0;
	muiSharedTupleCount = 0;
	muiSharedTuples = 0;
	muiGvarGlyphCount = 0;
	mbGvarLongOffsets = false;
	muiGvarOffsets = 0;
	muiGlyphVariationData = 0;

	InitItemVariationStore(&msHvarStore);
	muiHvarAdvanceMap = 0;

	InitItemVariationStore(&msMvarStore);
	for (i = 0; i < TTM_NumMetrics; i++)
	{
		maiMetricDeltas[i] = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::Kill(void)
{
	KillItemVariationStore(&msMvarStore);
	KillItemVariationStore(&msHvarStore);
	maiCoordinates.Kill();
	maiAxisValues.Kill();
	masInstances.Kill();
	masAxes.Kill();
	mpcFont = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the variation tables, if the font has them.  A font with a
// corrupt fvar table is taken not to be variable, and a corrupt gvar,
// HVAR or MVAR table is ignored.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::Parse(void)
{
	uint16	uiAxisCount;

	if (!ParseFvar())
	{
		masAxes.ReInit();
		masInstances.ReInit();
		return;
	}

	if (mpcFont->FindTable("avar", &muiAvarOffset, &muiAvarLength))
	{
		if ((muiAvarLength < 8) || !mpcFont->ReadUint16(muiAvarOffset + 6, &uiAxisCount) || (uiAxisCount != masAxes.NumElements()))
		{
			muiAvarOffset = 0;
			muiAvarLength = 0;
		}
	}
	else
	{
		muiAvarOffset = 0;
		muiAvarLength = 0;
	}

	ParseGvar();
	ParseHvar();
	ParseMvar();
	ResetAxes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::ParseFvar(void)
{
	lsize					uiOffset;
	lsize					uiLength;
	uint16					uiAxesOffset;
	uint16					uiAxisCount;
	uint16					uiAxisSize;
	uint16					uiInstanceCount;
	uint16					uiInstanceSize;
	uint16					i;
	lsize					uiRecord;
	uint32					uiMinimum;
	uint32					uiDefault;
	uint32					uiMaximum;
	uint16					uiFlags;
	STrueTypeAxis*			psAxis;
	STrueTypeNamedInstance*	psInstance;

	if (!mpcFont->FindTable("fvar", &uiOffset, &uiLength) || (uiLength < 16))
	{
		return false;
	}
	if (!mpcFont->ReadUint16(uiOffset + 4, &uiAxesOffset) ||
		!mpcFont->ReadUint16(uiOffset + 8, &uiAxisCount) ||
		!mpcFont->ReadUint16(uiOffset + 10, &uiAxisSize) ||
		!mpcFont->ReadUint16(uiOffset + 12, &uiInstanceCount) ||
		!mpcFont->ReadUint16(uiOffset + 14, &uiInstanceSize))
	{
		return false;
	}
	if ((uiAxisCount == 0) || (uiAxisSize < 20) || ((lsize)uiAxesOffset + (lsize)uiAxisCount * uiAxisSize > uiLength))
	{
		return false;
	}

	for (i = 0; i < uiAxisCount; i++)
	{
		uiRecord = uiOffset + uiAxesOffset + (lsize)i * uiAxisSize;
		psAxis = masAxes.Add();
		memcpy(psAxis->szTag, &mpcFont->GetData()[uiRecord], 4);
		psAxis->szTag[4] = '\0';
		mpcFont->ReadUint32(uiRecord + 4, &uiMinimum);
		mpcFont->ReadUint32(uiRecord + 8, &uiDefault);
		mpcFont->ReadUint32(uiRecord + 12, &uiMaximum);
		mpcFont->ReadUint16(uiRecord + 16, &uiFlags);
		mpcFont->ReadUint16(uiRecord + 18, &psAxis->uiNameID);
		psAxis->iMinimum = (int32)uiMinimum;
		psAxis->iDefault = (int32)uiDefault;
		psAxis->iMaximum = (int32)uiMaximum;
		psAxis->bHidden = (uiFlags & TRUE_TYPE_AXIS_HIDDEN) != 0;
		if ((psAxis->iMinimum > psAxis->iDefault) || (psAxis->iDefault > psAxis->iMaximum))
		{
			return false;
		}
	}

	//Instances follow the axes.  Each is a name ID, flags and a value for every axis (and maybe a PostScript name ID).
	if (uiInstanceSize >= 4 + (lsize)uiAxisCount * 4)
	{
		for (i = 0; i < uiInstanceCount; i++)
		{
			uiRecord = uiOffset + uiAxesOffset + (lsize)uiAxisCount * uiAxisSize + (lsize)i * uiInstanceSize;
			if ((uiRecord + uiInstanceSize > uiOffset + uiLength) || !mpcFont->IsInFile(uiRecord, uiInstanceSize))
			{
				break;
			}
			psInstance = masInstances.Add();
			mpcFont->ReadUint16(uiRecord, &psInstance->uiSubfamilyNameID);
			psInstance->uiCoordinates = uiRecord + 4;
		}
	}

	maiAxisValues.Resize(uiAxisCount);
	maiCoordinates.Resize(uiAxisCount);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The glyph variations header.  Glyph variation data is found through
// the offsets when a glyph is read.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::ParseGvar(void)
{
	lsize	uiOffset;
	lsize	uiLength;
	uint16	uiAxisCount;
	uint16	uiSharedTupleCount;
	uint32	uiSharedTuplesOffset;
	uint16	uiGlyphCount;
	uint16	uiFlags;
	uint32	uiDataOffset;
	lsize	uiOffsetsLength;

	if (!mpcFont->FindTable("gvar", &uiOffset, &uiLength) || (uiLength < 20))
	{
		return;
	}
	mpcFont->ReadUint16(uiOffset + 4, &uiAxisCount);
	mpcFont->ReadUint16(uiOffset + 6, &uiSharedTupleCount);
	mpcFont->ReadUint32(uiOffset + 8, &uiSharedTuplesOffset);
	mpcFont->ReadUint16(uiOffset + 12, &uiGlyphCount);
	mpcFont->ReadUint16(uiOffset + 14, &uiFlags);
	mpcFont->ReadUint32(uiOffset + 16, &uiDataOffset);

	if (uiAxisCount != masAxes.NumElements())
	{
		return;
	}
	uiOffsetsLength = ((lsize)uiGlyphCount + 1) * ((uiFlags & 1) ? 4 : 2);
	if ((20 + uiOffsetsLength > uiLength) ||
		((lsize)uiSharedTuplesOffset + (lsize)uiSharedTupleCount * uiAxisCount * 2 > uiLength) ||
		(uiDataOffset > uiLength))
	{
		return;
	}

	muiGvarOffset = uiOffset;
	muiGvarLength = uiLength;
	muiGvarAxisCount = uiAxisCount;
	muiSharedTupleCount = uiSharedTupleCount;
	muiSharedTuples = uiOffset + uiSharedTuplesOffset;
	muiGvarGlyphCount = uiGlyphCount;
	mbGvarLongOffsets = (uiFlags & 1) != 0;
	muiGvarOffsets = uiOffset + 20;
	muiGlyphVariationData = uiOffset + uiDataOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Horizontal metrics variations.  Only the advance widths are used.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::ParseHvar(void)
{
	lsize	uiOffset;
	lsize	uiLength;
	uint32	uiStoreOffset;
	uint32	uiAdvanceMapOffset;

	if (!mpcFont->FindTable("HVAR", &uiOffset, &uiLength) || (uiLength < 20))
	{
		return;
	}
	mpcFont->ReadUint32(uiOffset + 4, &uiStoreOffset);
	mpcFont->ReadUint32(uiOffset + 8, &uiAdvanceMapOffset);
	if ((uiStoreOffset == 0) || (uiStoreOffset >= uiLength) || (uiAdvanceMapOffset >= uiLength))
	{
		return;
	}

	ParseItemVariationStore(uiOffset + uiStoreOffset, uiOffset + uiLength, &msHvarStore);
	if (msHvarStore.uiOffset != 0)
	{
		muiHvarAdvanceMap = (uiAdvanceMapOffset != 0) ? uiOffset + uiAdvanceMapOffset : 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Font wide metrics variations.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::ParseMvar(void)
{
	lsize	uiOffset;
	lsize	uiLength;
	uint16	uiStoreOffset;

	if (!mpcFont->FindTable("MVAR", &uiOffset, &uiLength) || (uiLength < 12))
	{
		return;
	}
	mpcFont->ReadUint16(uiOffset + 10, &uiStoreOffset);
	if ((uiStoreOffset == 0) || (uiStoreOffset >= uiLength))
	{
		return;
	}
	ParseItemVariationStore(uiOffset + uiStoreOffset, uiOffset + uiLength, &msMvarStore);
}


//////////////////////////////////////////////////////////////////////////
//
// The store is left empty (uiOffset 0) if its region list doesn't fit
// the table or doesn't have the font's axes.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::ParseItemVariationStore(lsize uiOffset, lsize uiTableEnd, STrueTypeItemVariationStore* psStore)
{
	uint16	uiFormat;
	uint32	uiRegionListOffset;
	uint16	uiDataCount;
	uint16	uiAxisCount;
	uint16	uiRegionCount;
	lsize	uiRegionList;

	if ((uiOffset + 8 > uiTableEnd) ||
		!mpcFont->ReadUint16(uiOffset, &uiFormat) ||
		!mpcFont->ReadUint32(uiOffset + 2, &uiRegionListOffset) ||
		!mpcFont->ReadUint16(uiOffset + 6, &uiDataCount))
	{
		return;
	}
	if ((uiFormat != 1) || (uiOffset + 8 + (lsize)uiDataCount * 4 > uiTableEnd))
	{
		return;
	}

	uiRegionList = uiOffset + uiRegionListOffset;
	if ((uiRegionList + 4 > uiTableEnd) ||
		!mpcFont->ReadUint16(uiRegionList, &uiAxisCount) ||
		!mpcFont->ReadUint16(uiRegionList + 2, &uiRegionCount))
	{
		return;
	}
	if ((uiAxisCount != masAxes.NumElements()) || (uiRegionList + 4 + (lsize)uiRegionCount * uiAxisCount * 6 > uiTableEnd))
	{
		return;
	}

	psStore->uiOffset = uiOffset;
	psStore->uiRegionList = uiRegionList;
	psStore->uiRegionAxisCount = uiAxisCount;
	psStore->uiRegionCount = uiRegionCount;
	psStore->uiDataCount = uiDataCount;
	psStore->aiRegionScalars.Resize(uiRegionCount);
	psStore->aiRegionScalars.SetArrayValues(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::InitItemVariationStore(STrueTypeItemVariationStore* psStore)
{
	psStore->uiOffset = 0;
	psStore->uiRegionList = 0;
	psStore->uiRegionAxisCount = 0;
	psStore->uiRegionCount = 0;
	psStore->uiDataCount = 0;
	psStore->aiRegionScalars.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::KillItemVariationStore(STrueTypeItemVariationStore* psStore)
{
	psStore->aiRegionScalars.Kill();
	psStore->uiOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::IsVariable(void)
{
	return masAxes.NumElements() > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the axes aren't all at their defaults (after avar mapping), so
// outlines and metrics are varied.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::IsVaried(void)
{
	return mbVaried;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTrueTypeVariations::NumAxes(void)
{
	return masAxes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STrueTypeAxis* CTrueTypeVariations::GetAxis(size uiAxis)
{
	return masAxes.SafeGet(uiAxis);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns ARRAY_ELEMENT_NOT_FOUND if the font doesn't have the axis.
//
//////////////////////////////////////////////////////////////////////////
size CTrueTypeVariations::FindAxis(const char* szTag)
{
	size			i;
	STrueTypeAxis*	psAxis;

	for (i = 0; i < masAxes.NumElements(); i++)
	{
		psAxis = masAxes.Get(i);
		if (StringCompare(psAxis->szTag, szTag) == 0)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Sets an axis (16.16, clamped to the axis' range).  Returns false if
// the font doesn't have the axis.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::SetAxisValue(const char* szTag, int32 iValue)
{
	size			uiAxis;
	STrueTypeAxis*	psAxis;

	uiAxis = FindAxis(szTag);
	if (uiAxis == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	psAxis = masAxes.Get(uiAxis);
	if (iValue < psAxis->iMinimum)
	{
		iValue = psAxis->iMinimum;
	}
	else if (iValue > psAxis->iMaximum)
	{
		iValue = psAxis->iMaximum;
	}
	maiAxisValues.SetValue(uiAxis, iValue);
	UpdateCoordinates();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 16.16.  0 if the font doesn't have the axis.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::GetAxisValue(const char* szTag)
{
	size	uiAxis;

	uiAxis = FindAxis(szTag);
	if (uiAxis == ARRAY_ELEMENT_NOT_FOUND)
	{
		return 0;
	}
	return maiAxisValues.GetValue(uiAxis);
}


//////////////////////////////////////////////////////////////////////////
//
// Every axis at its default: the font's default instance.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::ResetAxes(void)
{
	size	i;

	for (i = 0; i < masAxes.NumElements(); i++)
	{
		maiAxisValues.SetValue(i, masAxes.Get(i)->iDefault);
	}
	UpdateCoordinates();
}


//////////////////////////////////////////////////////////////////////////
//
// 16.16, from -1 to 1.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::GetNormalisedCoordinate(size uiAxis)
{
	if (uiAxis >= maiCoordinates.NumElements())
	{
		return 0;
	}
	return maiCoordinates.GetValue(uiAxis);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTrueTypeVariations::NumNamedInstances(void)
{
	return masInstances.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the instance's subfamily name (Bold, Light ...) to psz.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::GetNamedInstanceName(size uiInstance, CChars* psz)
{
	STrueTypeNamedInstance*	psInstance;

	psInstance = masInstances.SafeGet(uiInstance);
	if (psInstance == NULL)
	{
		return false;
	}
	return mpcFont->GetName(psInstance->uiSubfamilyNameID, psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::SetNamedInstance(size uiInstance)
{
	STrueTypeNamedInstance*	psInstance;
	size					i;
	uint32					uiValue;
	int32					iValue;
	STrueTypeAxis*			psAxis;

	psInstance = masInstances.SafeGet(uiInstance);
	if (psInstance == NULL)
	{
		return false;
	}

	for (i = 0; i < masAxes.NumElements(); i++)
	{
		psAxis = masAxes.Get(i);
		mpcFont->ReadUint32(psInstance->uiCoordinates + (lsize)i * 4, &uiValue);
		iValue = (int32)uiValue;
		if (iValue < psAxis->iMinimum)
		{
			iValue = psAxis->iMinimum;
		}
		else if (iValue > psAxis->iMaximum)
		{
			iValue = psAxis->iMaximum;
		}
		maiAxisValues.SetValue(i, iValue);
	}
	UpdateCoordinates();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the instance with the subfamily name szName (Bold, Light ...).
// Returns false if the font doesn't have it.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::SetNamedInstance(const char* szName)
{
	size	i;
	CChars	sz;
	bool	bFound;

	for (i = 0; i < masInstances.NumElements(); i++)
	{
		sz.Init();
		GetNamedInstanceName(i, &sz);
		bFound = sz.Equals(szName);
		sz.Kill();
		if (bFound)
		{
			return SetNamedInstance(i);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Normalises the axis values (through avar) and works out everything that
// only depends on them: the region scalars of the item variation stores
// and the font wide metric deltas.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::UpdateCoordinates(void)
{
	size	i;
	int32	iNormalised;
	uint16	uiRecordSize;
	uint16	uiRecordCount;
	lsize	uiRecord;
	uint16	uiOuter;
	uint16	uiInner;
	size	uiMetric;
	lsize	uiOffset;
	lsize	uiLength;

	mbVaried = false;
	for (i = 0; i < masAxes.NumElements(); i++)
	{
		//Kept 16.16 (not rounded to 2.14) as FreeType and fontTools do, so advances round the same way.
		iNormalised = Normalise(i, maiAxisValues.GetValue(i));
		iNormalised = MapAvar(i, iNormalised);
		maiCoordinates.SetValue(i, iNormalised);
		if (iNormalised != 0)
		{
			mbVaried = true;
		}
	}

	UpdateRegionScalars(&msHvarStore);
	UpdateRegionScalars(&msMvarStore);

	for (i = 0; i < TTM_NumMetrics; i++)
	{
		maiMetricDeltas[i] = 0;
	}
	if (mbVaried && (msMvarStore.uiOffset != 0) && mpcFont->FindTable("MVAR", &uiOffset, &uiLength))
	{
		mpcFont->ReadUint16(uiOffset + 6, &uiRecordSize);
		mpcFont->ReadUint16(uiOffset + 8, &uiRecordCount);
		if (uiRecordSize >= 8)
		{
			for (i = 0; i < uiRecordCount; i++)
			{
				uiRecord = uiOffset + 12 + (lsize)i * uiRecordSize;
				if ((uiRecord + 8 > uiOffset + uiLength) || !mpcFont->IsInFile(uiRecord, 8))
				{
					break;
				}
				for (uiMetric = 0; uiMetric < TTM_NumMetrics; uiMetric++)
				{
					if (memcmp(&mpcFont->GetData()[uiRecord], gaszTrueTypeMetricTags[uiMetric], 4) == 0)
					{
						mpcFont->ReadUint16(uiRecord + 4, &uiOuter);
						mpcFont->ReadUint16(uiRecord + 6, &uiInner);
						maiMetricDeltas[uiMetric] = TrueTypeRoundDelta(GetItemDelta(&msMvarStore, uiOuter, uiInner));
						break;
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The default normalisation of an axis value: -1 at the minimum, 0 at
// the default and 1 at the maximum, linear in between.  16.16 in and out.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::Normalise(size uiAxis, int32 iValue)
{
	STrueTypeAxis*	psAxis;

	psAxis = masAxes.Get(uiAxis);
	if (iValue < psAxis->iDefault)
	{
		return (int32)-DivideRound(((int64)psAxis->iDefault - iValue) << TRUE_TYPE_FIXED_BITS, (int64)psAxis->iDefault - psAxis->iMinimum);
	}
	else if (iValue > psAxis->iDefault)
	{
		return (int32)DivideRound(((int64)iValue - psAxis->iDefault) << TRUE_TYPE_FIXED_BITS, (int64)psAxis->iMaximum - psAxis->iDefault);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The avar segment map of an axis (2.14 pairs of from and to, sorted by
// from) applied to a normalised coordinate.  16.16 in and out.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::MapAvar(size uiAxis, int32 iNormalised)
{
	lsize	uiPos;
	lsize	uiEnd;
	size	i;
	uint16	uiCount;
	int16	iFrom;
	int16	iTo;
	int32	iFrom1;
	int32	iTo1;
	int32	iFrom2;
	int32	iTo2;

	if (muiAvarOffset == 0)
	{
		return iNormalised;
	}

	uiPos = muiAvarOffset + 8;
	uiEnd = muiAvarOffset + muiAvarLength;
	for (i = 0; i < uiAxis; i++)
	{
		if ((uiPos + 2 > uiEnd) || !mpcFont->ReadUint16(uiPos, &uiCount))
		{
			return iNormalised;
		}
		uiPos += 2 + (lsize)uiCount * 4;
	}
	if ((uiPos + 2 > uiEnd) || !mpcFont->ReadUint16(uiPos, &uiCount) || (uiPos + 2 + (lsize)uiCount * 4 > uiEnd))
	{
		return iNormalised;
	}
	if (uiCount == 0)
	{
		return iNormalised;
	}
	uiPos += 2;

	//Before the first pair and after the last the map is a shift.
	mpcFont->ReadInt16(uiPos, &iFrom);
	mpcFont->ReadInt16(uiPos + 2, &iTo);
	iFrom1 = (int32)iFrom << 2;
	iTo1 = (int32)iTo << 2;
	if (iNormalised <= iFrom1)
	{
		return iNormalised + iTo1 - iFrom1;
	}

	for (i = 1; i < uiCount; i++)
	{
		mpcFont->ReadInt16(uiPos + (lsize)i * 4, &iFrom);
		mpcFont->ReadInt16(uiPos + (lsize)i * 4 + 2, &iTo);
		iFrom2 = (int32)iFrom << 2;
		iTo2 = (int32)iTo << 2;
		if (iNormalised <= iFrom2)
		{
			if (iFrom2 == iFrom1)
			{
				return iTo2;
			}
			return iTo1 + (int32)DivideRound((int64)(iNormalised - iFrom1) * (iTo2 - iTo1), (int64)iFrom2 - iFrom1);
		}
		iFrom1 = iFrom2;
		iTo1 = iTo2;
	}
	return iNormalised + iTo1 - iFrom1;
}


//////////////////////////////////////////////////////////////////////////
//
// Scales a region's (or tuple's) scalar by how much of it applies on one
// axis: all at the peak, falling to none at the start and the end.  The
// coordinate and scalar are 16.16, the start, peak and end 2.14 (as
// stored).  An axis with a peak of 0, or an invalid range, doesn't limit
// the region.  Multiplying and dividing in one step rounds as FreeType's
// FT_MulDiv does.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::ApplyAxis(int32 iScalar, int32 iCoordinate, int32 iStart, int32 iPeak, int32 iEnd)
{
	iStart *= 4;
	iPeak *= 4;
	iEnd *= 4;
	if ((iPeak == 0) || (iCoordinate == iPeak))
	{
		return iScalar;
	}
	if ((iStart > iPeak) || (iPeak > iEnd) || ((iStart < 0) && (iEnd > 0)))
	{
		return iScalar;
	}
	if ((iCoordinate <= iStart) || (iCoordinate >= iEnd))
	{
		return 0;
	}
	if (iCoordinate < iPeak)
	{
		return (int32)DivideRound((int64)iScalar * (iCoordinate - iStart), (int64)iPeak - iStart);
	}
	return (int32)DivideRound((int64)iScalar * (iEnd - iCoordinate), (int64)iEnd - iPeak);
}


//////////////////////////////////////////////////////////////////////////
//
// The scalar (16.16) of a gvar tuple.  uiPeak, uiStart and uiEnd are the
// file offsets of a 2.14 value per axis.  A tuple without an intermediate
// region (uiStart 0) runs from 0 to its peak.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::TupleScalar(lsize uiPeak, lsize uiStart, lsize uiEnd)
{
	int32	iScalar;
	size	i;
	int16	iPeak;
	int16	iStart;
	int16	iEnd;

	iScalar = TRUE_TYPE_FIXED_ONE;
	for (i = 0; i < muiGvarAxisCount; i++)
	{
		if (!mpcFont->ReadInt16(uiPeak + (lsize)i * 2, &iPeak))
		{
			return 0;
		}
		if (uiStart != 0)
		{
			if (!mpcFont->ReadInt16(uiStart + (lsize)i * 2, &iStart) || !mpcFont->ReadInt16(uiEnd + (lsize)i * 2, &iEnd))
			{
				return 0;
			}
		}
		else
		{
			iStart = (iPeak < 0) ? iPeak : 0;
			iEnd = (iPeak > 0) ? iPeak : 0;
		}

		iScalar = ApplyAxis(iScalar, maiCoordinates.GetValue(i), iStart, iPeak, iEnd);
		if (iScalar == 0)
		{
			return 0;
		}
	}
	return iScalar;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::UpdateRegionScalars(STrueTypeItemVariationStore* psStore)
{
	size	uiRegion;
	size	uiAxis;
	lsize	uiRecord;
	int16	iStart;
	int16	iPeak;
	int16	iEnd;
	int32	iScalar;

	if (psStore->uiOffset == 0)
	{
		return;
	}

	for (uiRegion = 0; uiRegion < psStore->uiRegionCount; uiRegion++)
	{
		iScalar = TRUE_TYPE_FIXED_ONE;
		for (uiAxis = 0; uiAxis < psStore->uiRegionAxisCount; uiAxis++)
		{
			uiRecord = psStore->uiRegionList + 4 + ((lsize)uiRegion * psStore->uiRegionAxisCount + uiAxis) * 6;
			mpcFont->ReadInt16(uiRecord, &iStart);
			mpcFont->ReadInt16(uiRecord + 2, &iPeak);
			mpcFont->ReadInt16(uiRecord + 4, &iEnd);
			iScalar = ApplyAxis(iScalar, maiCoordinates.GetValue(uiAxis), iStart, iPeak, iEnd);
			if (iScalar == 0)
			{
				break;
			}
		}
		psStore->aiRegionScalars.SetValue(uiRegion, iScalar);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The delta (16.16 font units) of an item at the current coordinates.
// Out of range indices have no delta.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::GetItemDelta(STrueTypeItemVariationStore* psStore, uint16 uiOuter, uint16 uiInner)
{
	uint32	uiDataOffset;
	lsize	uiData;
	uint16	uiItemCount;
	uint16	uiWordDeltaCount;
	uint16	uiRegionIndexCount;
	bool	bLongWords;
	lsize	uiWordCount;
	lsize	uiRowSize;
	lsize	uiRow;
	lsize	uiPos;
	size	i;
	uint16	uiRegion;
	int64	iSum;
	int32	iDelta;
	uint32	ui32;
	uint16	ui16;
	uint8	ui8;

	if ((psStore->uiOffset == 0) || (uiOuter >= psStore->uiDataCount))
	{
		return 0;
	}
	if (!mpcFont->ReadUint32(psStore->uiOffset + 8 + (lsize)uiOuter * 4, &uiDataOffset))
	{
		return 0;
	}
	uiData = psStore->uiOffset + uiDataOffset;
	if (!mpcFont->ReadUint16(uiData, &uiItemCount) ||
		!mpcFont->ReadUint16(uiData + 2, &uiWordDeltaCount) ||
		!mpcFont->ReadUint16(uiData + 4, &uiRegionIndexCount))
	{
		return 0;
	}
	if (uiInner >= uiItemCount)
	{
		return 0;
	}

	//The first uiWordCount deltas of a row are words (longs with bLongWords), the rest bytes (words).
	bLongWords = (uiWordDeltaCount & TRUE_TYPE_LONG_WORDS) != 0;
	uiWordCount = uiWordDeltaCount & TRUE_TYPE_WORD_DELTA_COUNT_MASK;
	if (uiWordCount > uiRegionIndexCount)
	{
		return 0;
	}
	if (bLongWords)
	{
		uiRowSize = uiWordCount * 4 + (uiRegionIndexCount - uiWordCount) * 2;
	}
	else
	{
		uiRowSize = uiWordCount * 2 + (uiRegionIndexCount - uiWordCount);
	}
	uiRow = uiData + 6 + (lsize)uiRegionIndexCount * 2 + (lsize)uiInner * uiRowSize;
	if (!mpcFont->IsInFile(uiRow, uiRowSize))
	{
		return 0;
	}

	iSum = 0;
	uiPos = uiRow;
	for (i = 0; i < uiRegionIndexCount; i++)
	{
		if (i < uiWordCount)
		{
			if (bLongWords)
			{
				mpcFont->ReadUint32(uiPos, &ui32);
				iDelta = (int32)ui32;
				uiPos += 4;
			}
			else
			{
				mpcFont->ReadUint16(uiPos, &ui16);
				iDelta = (int16)ui16;
				uiPos += 2;
			}
		}
		else
		{
			if (bLongWords)
			{
				mpcFont->ReadUint16(uiPos, &ui16);
				iDelta = (int16)ui16;
				uiPos += 2;
			}
			else
			{
				mpcFont->ReadUint8(uiPos, &ui8);
				iDelta = (int8)ui8;
				uiPos += 1;
			}
		}

		mpcFont->ReadUint16(uiData + 6 + (lsize)i * 2, &uiRegion);
		if (uiRegion < psStore->uiRegionCount)
		{
			iSum += (int64)iDelta * psStore->aiRegionScalars.GetValue(uiRegion);
		}
	}
	return (int32)iSum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::HasAdvanceDeltas(void)
{
	return msHvarStore.uiOffset != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The change to a glyph's advance width (whole font units) from HVAR.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::GetAdvanceDelta(int iGlyph)
{
	uint8	uiFormat;
	uint8	uiEntryFormat;
	uint16	uiCount16;
	uint32	uiCount;
	lsize	uiMapData;
	lsize	uiEntrySize;
	lsize	uiInnerBits;
	uint32	uiIndex;
	uint32	uiEntry;
	lsize	i;
	uint8	uiByte;
	uint16	uiOuter;
	uint16	uiInner;

	if (!mbVaried || (msHvarStore.uiOffset == 0) || (iGlyph < 0))
	{
		return 0;
	}

	if (muiHvarAdvanceMap == 0)
	{
		uiOuter = 0;
		uiInner = (uint16)iGlyph;
	}
	else
	{
		//A delta set index map: entries of 1 to 4 bytes, each an outer index above an inner index of 1 to 16 bits.
		if (!mpcFont->ReadUint8(muiHvarAdvanceMap, &uiFormat) || !mpcFont->ReadUint8(muiHvarAdvanceMap + 1, &uiEntryFormat))
		{
			return 0;
		}
		if (uiFormat == 0)
		{
			if (!mpcFont->ReadUint16(muiHvarAdvanceMap + 2, &uiCount16))
			{
				return 0;
			}
			uiCount = uiCount16;
			uiMapData = muiHvarAdvanceMap + 4;
		}
		else if (uiFormat == 1)
		{
			if (!mpcFont->ReadUint32(muiHvarAdvanceMap + 2, &uiCount))
			{
				return 0;
			}
			uiMapData = muiHvarAdvanceMap + 6;
		}
		else
		{
			return 0;
		}
		if (uiCount == 0)
		{
			return 0;
		}

		uiEntrySize = ((uiEntryFormat >> 4) & 3) + 1;
		uiInnerBits = (uiEntryFormat & 0x0F) + 1;
		uiIndex = ((uint32)iGlyph < uiCount) ? (uint32)iGlyph : uiCount - 1;
		uiEntry = 0;
		for (i = 0; i < uiEntrySize; i++)
		{
			if (!mpcFont->ReadUint8(uiMapData + (lsize)uiIndex * uiEntrySize + i, &uiByte))
			{
				return 0;
			}
			uiEntry = (uiEntry << 8) | uiByte;
		}
		uiOuter = (uint16)(uiEntry >> uiInnerBits);
		uiInner = (uint16)(uiEntry & ((1u << uiInnerBits) - 1));
	}

	return TrueTypeRoundDelta(GetItemDelta(&msHvarStore, uiOuter, uiInner));
}


//////////////////////////////////////////////////////////////////////////
//
// The change to a font wide metric (whole font units) from MVAR.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::GetMetricDelta(ETrueTypeMetric eMetric)
{
	if ((eMetric < 0) || (eMetric >= TTM_NumMetrics))
	{
		return 0;
	}
	return maiMetricDeltas[eMetric];
}


//////////////////////////////////////////////////////////////////////////
//
// Sums the gvar deltas of a glyph's points at the current coordinates
// into paiDeltaX and paiDeltaY (16.16 font units, uiNumPoints of each).
// pasPoints are the glyph's points in the default instance: the outline
// points then the four phantom points of a simple glyph, or a point per
// component then the phantom points of a composite glyph.  paiContourEnds
// (indices into pasPoints, inclusive) are the contours whose untouched
// points are inferred from their touched neighbours; points not in a
// contour that a tuple doesn't touch don't move.  Returns false if the
// glyph's variation data is corrupt (the deltas are then all 0).
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::GetGlyphDeltas(int iGlyph, STrueTypePoint* pasPoints, size uiNumPoints, int* paiContourEnds, size uiNumContours, int32* paiDeltaX, int32* paiDeltaY)
{
	uint32			uiStartOffset;
	uint32			uiEndOffset;
	uint16			uiStart16;
	uint16			uiEnd16;
	lsize			uiStart;
	lsize			uiEnd;
	uint16			uiTupleField;
	uint16			uiDataOffset;
	lsize			uiHeader;
	lsize			uiHeadersEnd;
	lsize			uiData;
	lsize			uiTupleEnd;
	lsize			uiPos;
	size			uiTuple;
	size			uiNumTuples;
	uint16			uiTupleSize;
	uint16			uiTupleIndex;
	lsize			uiPeak;
	lsize			uiIntermediateStart;
	lsize			uiIntermediateEnd;
	int32			iScalar;
	CArrayInt32		aiSharedPoints;
	bool			bSharedAll;
	CArrayInt32		aiPrivatePoints;
	bool			bPrivateAll;
	CArrayInt32*	paiPoints;
	bool			bAll;
	CArrayInt32		aiX;
	CArrayInt32		aiY;
	size			uiCount;
	size			i;
	int32			iPoint;
	bool*			pbTouched;
	int32*			paiTupleX;
	int32*			paiTupleY;
	bool			bResult;

	for (i = 0; i < uiNumPoints; i++)
	{
		paiDeltaX[i] = 0;
		paiDeltaY[i] = 0;
	}
	if (!mbVaried || (muiGvarOffset == 0) || (iGlyph < 0) || (iGlyph >= muiGvarGlyphCount) || (uiNumPoints == 0))
	{
		return true;
	}

	if (mbGvarLongOffsets)
	{
		if (!mpcFont->ReadUint32(muiGvarOffsets + (lsize)iGlyph * 4, &uiStartOffset) || !mpcFont->ReadUint32(muiGvarOffsets + (lsize)iGlyph * 4 + 4, &uiEndOffset))
		{
			return false;
		}
	}
	else
	{
		//Short offsets are stored halved.
		if (!mpcFont->ReadUint16(muiGvarOffsets + (lsize)iGlyph * 2, &uiStart16) || !mpcFont->ReadUint16(muiGvarOffsets + (lsize)iGlyph * 2 + 2, &uiEnd16))
		{
			return false;
		}
		uiStartOffset = (uint32)uiStart16 * 2;
		uiEndOffset = (uint32)uiEnd16 * 2;
	}
	uiStart = muiGlyphVariationData + uiStartOffset;
	uiEnd = muiGlyphVariationData + uiEndOffset;
	if ((uiEndOffset < uiStartOffset) || (uiEnd > muiGvarOffset + muiGvarLength))
	{
		return false;
	}
	if (uiEnd == uiStart)
	{
		return true;
	}

	//The tuple variation headers then the serialised data: shared point numbers then each tuple's points and deltas.
	if ((uiStart + 4 > uiEnd) || !mpcFont->ReadUint16(uiStart, &uiTupleField) || !mpcFont->ReadUint16(uiStart + 2, &uiDataOffset))
	{
		return false;
	}
	uiHeader = uiStart + 4;
	uiHeadersEnd = uiStart + uiDataOffset;
	uiData = uiHeadersEnd;
	if (uiData > uiEnd)
	{
		return false;
	}

	aiSharedPoints.Init();
	aiPrivatePoints.Init();
	aiX.Init();
	aiY.Init();
	pbTouched = (bool*)malloc(uiNumPoints * sizeof(bool));
	paiTupleX = (int32*)malloc(uiNumPoints * sizeof(int32));
	paiTupleY = (int32*)malloc(uiNumPoints * sizeof(int32));
	bResult = (pbTouched != NULL) && (paiTupleX != NULL) && (paiTupleY != NULL);

	bSharedAll = true;
	if (bResult && (uiTupleField & TRUE_TYPE_SHARED_POINT_NUMBERS))
	{
		bResult = ReadPackedPoints(&uiData, uiEnd, &aiSharedPoints, &bSharedAll);
	}

	uiNumTuples = uiTupleField & TRUE_TYPE_TUPLE_COUNT_MASK;
	for (uiTuple = 0; bResult && (uiTuple < uiNumTuples); uiTuple++)
	{
		if ((uiHeader + 4 > uiHeadersEnd) || !mpcFont->ReadUint16(uiHeader, &uiTupleSize) || !mpcFont->ReadUint16(uiHeader + 2, &uiTupleIndex))
		{
			bResult = false;
			break;
		}
		uiHeader += 4;

		if (uiTupleIndex & TRUE_TYPE_EMBEDDED_PEAK_TUPLE)
		{
			uiPeak = uiHeader;
			uiHeader += (lsize)muiGvarAxisCount * 2;
		}
		else
		{
			if ((uiTupleIndex & TRUE_TYPE_TUPLE_INDEX_MASK) >= muiSharedTupleCount)
			{
				bResult = false;
				break;
			}
			uiPeak = muiSharedTuples + (lsize)(uiTupleIndex & TRUE_TYPE_TUPLE_INDEX_MASK) * muiGvarAxisCount * 2;
		}
		if (uiTupleIndex & TRUE_TYPE_INTERMEDIATE_REGION)
		{
			uiIntermediateStart = uiHeader;
			uiIntermediateEnd = uiHeader + (lsize)muiGvarAxisCount * 2;
			uiHeader += (lsize)muiGvarAxisCount * 4;
		}
		else
		{
			uiIntermediateStart = 0;
			uiIntermediateEnd = 0;
		}
		if (uiHeader > uiHeadersEnd)
		{
			bResult = false;
			break;
		}

		uiPos = uiData;
		uiTupleEnd = uiData + uiTupleSize;
		if (uiTupleEnd > uiEnd)
		{
			bResult = false;
			break;
		}
		uiData = uiTupleEnd;

		iScalar = TupleScalar(uiPeak, uiIntermediateStart, uiIntermediateEnd);
		if (iScalar == 0)
		{
			continue;
		}

		if (uiTupleIndex & TRUE_TYPE_PRIVATE_POINT_NUMBERS)
		{
			if (!ReadPackedPoints(&uiPos, uiTupleEnd, &aiPrivatePoints, &bPrivateAll))
			{
				bResult = false;
				break;
			}
			paiPoints = &aiPrivatePoints;
			bAll = bPrivateAll;
		}
		else
		{
			paiPoints = &aiSharedPoints;
			bAll = bSharedAll;
		}

		uiCount = bAll ? uiNumPoints : paiPoints->NumElements();
		if (!ReadPackedDeltas(&uiPos, uiTupleEnd, uiCount, &aiX) || !ReadPackedDeltas(&uiPos, uiTupleEnd, uiCount, &aiY))
		{
			bResult = false;
			break;
		}

		if (bAll)
		{
			for (i = 0; i < uiNumPoints; i++)
			{
				paiDeltaX[i] += aiX.GetValue(i) * iScalar;
				paiDeltaY[i] += aiY.GetValue(i) * iScalar;
			}
		}
		else
		{
			//The deltas of the points the tuple doesn't touch are inferred, then the whole tuple is scaled.
			for (i = 0; i < uiNumPoints; i++)
			{
				pbTouched[i] = false;
				paiTupleX[i] = 0;
				paiTupleY[i] = 0;
			}
			for (i = 0; i < uiCount; i++)
			{
				iPoint = paiPoints->GetValue(i);
				if ((iPoint >= 0) && ((size)iPoint < uiNumPoints))
				{
					pbTouched[iPoint] = true;
					paiTupleX[iPoint] = aiX.GetValue(i) * TRUE_TYPE_FIXED_ONE;
					paiTupleY[iPoint] = aiY.GetValue(i) * TRUE_TYPE_FIXED_ONE;
				}
			}
			InferDeltas(pasPoints, uiNumPoints, paiContourEnds, uiNumContours, pbTouched, paiTupleX, paiTupleY);
			for (i = 0; i < uiNumPoints; i++)
			{
				paiDeltaX[i] += (int32)DivideRound((int64)paiTupleX[i] * iScalar, TRUE_TYPE_FIXED_ONE);
				paiDeltaY[i] += (int32)DivideRound((int64)paiTupleY[i] * iScalar, TRUE_TYPE_FIXED_ONE);
			}
		}
	}

	if (!bResult)
	{
		for (i = 0; i < uiNumPoints; i++)
		{
			paiDeltaX[i] = 0;
			paiDeltaY[i] = 0;
		}
	}

	free(paiTupleY);
	free(paiTupleX);
	free(pbTouched);
	aiY.Kill();
	aiX.Kill();
	aiPrivatePoints.Kill();
	aiSharedPoints.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Packed point numbers: a count (0 is every point) then runs of byte or
// word increments.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::ReadPackedPoints(lsize* puiPos, lsize uiEnd, CArrayInt32* paiPoints, bool* pbAll)
{
	lsize	uiPos;
	uint8	uiByte;
	uint8	uiLow;
	size	uiCount;
	uint8	uiControl;
	size	uiRun;
	int32	iPoint;
	uint16	uiWord;

	paiPoints->ReInit();
	uiPos = *puiPos;
	if ((uiPos + 1 > uiEnd) || !mpcFont->ReadUint8(uiPos, &uiByte))
	{
		return false;
	}
	uiPos++;
	if (uiByte == 0)
	{
		*pbAll = true;
		*puiPos = uiPos;
		return true;
	}

	if (uiByte & 0x80)
	{
		if ((uiPos + 1 > uiEnd) || !mpcFont->ReadUint8(uiPos, &uiLow))
		{
			return false;
		}
		uiPos++;
		uiCount = ((size)(uiByte & 0x7F) << 8) | uiLow;
	}
	else
	{
		uiCount = uiByte;
	}

	iPoint = 0;
	while (paiPoints->NumElements() < uiCount)
	{
		if ((uiPos + 1 > uiEnd) || !mpcFont->ReadUint8(uiPos, &uiControl))
		{
			return false;
		}
		uiPos++;
		for (uiRun = (uiControl & TRUE_TYPE_POINT_RUN_COUNT_MASK) + 1; (uiRun > 0) && (paiPoints->NumElements() < uiCount); uiRun--)
		{
			if (uiControl & TRUE_TYPE_POINTS_ARE_WORDS)
			{
				if ((uiPos + 2 > uiEnd) || !mpcFont->ReadUint16(uiPos, &uiWord))
				{
					return false;
				}
				uiPos += 2;
				iPoint += uiWord;
			}
			else
			{
				if ((uiPos + 1 > uiEnd) || !mpcFont->ReadUint8(uiPos, &uiByte))
				{
					return false;
				}
				uiPos++;
				iPoint += uiByte;
			}
			paiPoints->Add(iPoint);
		}
	}

	*pbAll = false;
	*puiPos = uiPos;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Packed deltas: runs of zeros or byte, word or long deltas.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeVariations::ReadPackedDeltas(lsize* puiPos, lsize uiEnd, size uiCount, CArrayInt32* paiDeltas)
{
	lsize	uiPos;
	uint8	uiControl;
	size	uiRun;
	int32	iDelta;
	uint8	ui8;
	uint16	ui16;
	uint32	ui32;
	lsize	uiSize;

	paiDeltas->ReInit();
	uiPos = *puiPos;
	while (paiDeltas->NumElements() < uiCount)
	{
		if ((uiPos + 1 > uiEnd) || !mpcFont->ReadUint8(uiPos, &uiControl))
		{
			return false;
		}
		uiPos++;

		switch (uiControl & TRUE_TYPE_DELTAS_SIZE_MASK)
		{
		case TRUE_TYPE_DELTAS_ARE_ZERO:
			uiSize = 0;
			break;
		case TRUE_TYPE_DELTAS_ARE_WORDS:
			uiSize = 2;
			break;
		case TRUE_TYPE_DELTAS_ARE_LONGS:
			uiSize = 4;
			break;
		default:
			uiSize = 1;
			break;
		}

		for (uiRun = (uiControl & TRUE_TYPE_DELTA_RUN_COUNT_MASK) + 1; uiRun > 0; uiRun--)
		{
			if (uiPos + uiSize > uiEnd)
			{
				return false;
			}
			if (uiSize == 0)
			{
				iDelta = 0;
			}
			else if (uiSize == 1)
			{
				mpcFont->ReadUint8(uiPos, &ui8);
				iDelta = (int8)ui8;
			}
			else if (uiSize == 2)
			{
				mpcFont->ReadUint16(uiPos, &ui16);
				iDelta = (int16)ui16;
			}
			else
			{
				mpcFont->ReadUint32(uiPos, &ui32);
				iDelta = (int32)ui32;
			}
			uiPos += uiSize;

			//A run that goes past the count is corrupt but its deltas are still read past.
			if (paiDeltas->NumElements() < uiCount)
			{
				paiDeltas->Add(iDelta);
			}
		}
	}

	*puiPos = uiPos;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Interpolates the deltas of untouched points in each contour.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::InferDeltas(STrueTypePoint* pasPoints, size uiNumPoints, int* paiContourEnds, size uiNumContours, bool* pbTouched, int32* paiDeltaX, int32* paiDeltaY)
{
	size	uiContour;
	size	uiStart;
	int		iEnd;

	uiStart = 0;
	for (uiContour = 0; uiContour < uiNumContours; uiContour++)
	{
		iEnd = paiContourEnds[uiContour];
		if ((iEnd < 0) || ((size)iEnd >= uiNumPoints) || ((size)iEnd < uiStart))
		{
			return;
		}
		InferContour(pasPoints, uiStart, (size)iEnd, pbTouched, paiDeltaX, paiDeltaY);
		uiStart = (size)iEnd + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each run of untouched points takes its deltas from the touched points
// either side of it (the contour wraps around).  A contour with no
// touched points doesn't move.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeVariations::InferContour(STrueTypePoint* pasPoints, size uiStart, size uiEnd, bool* pbTouched, int32* paiDeltaX, int32* paiDeltaY)
{
	size	uiFirst;
	size	uiPrevious;
	size	uiNext;
	size	i;

	for (uiFirst = uiStart; uiFirst <= uiEnd; uiFirst++)
	{
		if (pbTouched[uiFirst])
		{
			break;
		}
	}
	if (uiFirst > uiEnd)
	{
		return;
	}

	uiPrevious = uiFirst;
	do
	{
		uiNext = (uiPrevious == uiEnd) ? uiStart : uiPrevious + 1;
		while (!pbTouched[uiNext])
		{
			uiNext = (uiNext == uiEnd) ? uiStart : uiNext + 1;
		}

		i = (uiPrevious == uiEnd) ? uiStart : uiPrevious + 1;
		while (i != uiNext)
		{
			paiDeltaX[i] = InferDelta(pasPoints[i].x, pasPoints[uiPrevious].x, pasPoints[uiNext].x, paiDeltaX[uiPrevious], paiDeltaX[uiNext]);
			paiDeltaY[i] = InferDelta(pasPoints[i].y, pasPoints[uiPrevious].y, pasPoints[uiNext].y, paiDeltaY[uiPrevious], paiDeltaY[uiNext]);
			i = (i == uiEnd) ? uiStart : i + 1;
		}
		uiPrevious = uiNext;
	}
	while (uiPrevious != uiFirst);
}


//////////////////////////////////////////////////////////////////////////
//
// Between the reference points the delta is interpolated, outside them it
// is the nearer one's.  Reference points at the same coordinate give
// their delta if they agree and none if they don't.
//
//////////////////////////////////////////////////////////////////////////
int32 CTrueTypeVariations::InferDelta(int32 iCoordinate, int32 iCoordinate1, int32 iCoordinate2, int32 iDelta1, int32 iDelta2)
{
	int32	iSwap;
	int64	iSlope;

	if (iCoordinate1 == iCoordinate2)
	{
		return (iDelta1 == iDelta2) ? iDelta1 : 0;
	}
	if (iCoordinate1 > iCoordinate2)
	{
		iSwap = iCoordinate1;
		iCoordinate1 = iCoordinate2;
		iCoordinate2 = iSwap;
		iSwap = iDelta1;
		iDelta1 = iDelta2;
		iDelta2 = iSwap;
	}
	if (iCoordinate <= iCoordinate1)
	{
		return iDelta1;
	}
	if (iCoordinate >= iCoordinate2)
	{
		return iDelta2;
	}

	//A rounded slope (16.16 delta per font unit) times the distance, as FreeType does it.
	iSlope = DivideRound((int64)iDelta2 - iDelta1, (int64)iCoordinate2 - iCoordinate1);
	return iDelta1 + (int32)(iSlope * (iCoordinate - iCoordinate1));
}
