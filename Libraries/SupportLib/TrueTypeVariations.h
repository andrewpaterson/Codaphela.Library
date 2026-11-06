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
#ifndef __TRUE_TYPE_VARIATIONS_H__
#define __TRUE_TYPE_VARIATIONS_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayInt32.h"
#include "TrueTypeOutline.h"


//Axis values are 16.16 fixed point in the axis' own units (wght 400 is 400 << 16).
#define TRUE_TYPE_FIXED_BITS			16
#define TRUE_TYPE_FIXED_ONE				65536

//Normalised coordinates are 16.16 fixed point from -1 to 1.  The font stores region peaks etc. as 2.14.


//A design axis from the fvar table.
struct STrueTypeAxis
{
	char	szTag[5];
	int32	iMinimum;	//16.16.
	int32	iDefault;	//16.16.
	int32	iMaximum;	//16.16.
	uint16	uiNameID;
	bool	bHidden;
};


//A named instance (Bold, Light ...) from the fvar table: a name and a value for every axis.
struct STrueTypeNamedInstance
{
	uint16	uiSubfamilyNameID;
	lsize	uiCoordinates;  //File offset of the axis values (16.16 each).
};


//The font wide metrics the MVAR table can vary.
enum ETrueTypeMetric
{
	TTM_HorizontalAscender,
	TTM_HorizontalDescender,
	TTM_HorizontalLineGap,
	TTM_UnderlineOffset,
	TTM_UnderlineSize,
	TTM_StrikeoutOffset,
	TTM_StrikeoutSize,
	TTM_SubscriptYSize,
	TTM_SubscriptYOffset,
	TTM_SuperscriptYSize,
	TTM_SuperscriptYOffset,

	TTM_NumMetrics
};


//An item variation store (used by HVAR and MVAR) and the scalar of each of its regions at the current coordinates.
struct STrueTypeItemVariationStore
{
	lsize		uiOffset;			//0 if the font doesn't have it.
	lsize		uiRegionList;
	uint16		uiRegionAxisCount;
	uint16		uiRegionCount;
	uint16		uiDataCount;
	CArrayInt32	aiRegionScalars;	//16.16.
};


typedef CArrayTemplate<STrueTypeAxis>			CArrayTrueTypeAxis;
typedef CArrayTemplate<STrueTypeNamedInstance>	CArrayTrueTypeNamedInstance;


//A 16.16 delta to whole font units, halves rounded up as FreeType does.
int32 TrueTypeRoundDelta(int64 iDelta);


class CTrueTypeFont;
//The OpenType font variations of a TrueType font: the axes and named instances (fvar), the axis value mapping
//(avar), outline deltas (gvar), advance width deltas (HVAR) and font wide metric deltas (MVAR).  The axes start at
//their defaults, which is the font's default instance, and nothing is varied.  All the arithmetic is integer (axis
//values, normalised coordinates, scalars and accumulated deltas are 16.16) so every build gives the same outlines.
//FreeType's roundings are followed: Inter's outlines match FreeType 2.13's on all but a handful of points per instance
//(by one unit).  Advance widths always come from HVAR, so composite glyphs flagged USE_MY_METRICS can differ from
//FreeType (fontTools agrees with HVAR).
class CTrueTypeVariations
{
protected:
	CTrueTypeFont*					mpcFont;

	CArrayTrueTypeAxis				masAxes;
	CArrayTrueTypeNamedInstance		masInstances;
	CArrayInt32						maiAxisValues;		//16.16, by axis.
	CArrayInt32						maiCoordinates;		//Normalised 16.16, by axis.
	bool							mbVaried;			//Some coordinate isn't 0.

	lsize							muiAvarOffset;		//0 if the font doesn't have it.
	lsize							muiAvarLength;

	lsize							muiGvarOffset;		//0 if the font doesn't have it.
	lsize							muiGvarLength;
	uint16							muiGvarAxisCount;
	uint16							muiSharedTupleCount;
	lsize							muiSharedTuples;
	uint16							muiGvarGlyphCount;
	bool							mbGvarLongOffsets;
	lsize							muiGvarOffsets;
	lsize							muiGlyphVariationData;

	STrueTypeItemVariationStore		msHvarStore;
	lsize							muiHvarAdvanceMap;	//0 if glyph IDs are the inner indices.

	STrueTypeItemVariationStore		msMvarStore;
	int32							maiMetricDeltas[TTM_NumMetrics];	//Font units, rounded.

public:
	void					Init(CTrueTypeFont* pcFont);
	void					Kill(void);

	void					Parse(void);

	bool					IsVariable(void);
	bool					IsVaried(void);
	size					NumAxes(void);
	STrueTypeAxis*			GetAxis(size uiAxis);
	size					FindAxis(const char* szTag);
	bool					SetAxisValue(const char* szTag, int32 iValue);
	int32					GetAxisValue(const char* szTag);
	void					ResetAxes(void);
	int32					GetNormalisedCoordinate(size uiAxis);

	size					NumNamedInstances(void);
	bool					GetNamedInstanceName(size uiInstance, CChars* psz);
	bool					SetNamedInstance(size uiInstance);
	bool					SetNamedInstance(const char* szName);

	bool					HasAdvanceDeltas(void);
	int32					GetAdvanceDelta(int iGlyph);
	int32					GetMetricDelta(ETrueTypeMetric eMetric);
	bool					GetGlyphDeltas(int iGlyph, STrueTypePoint* pasPoints, size uiNumPoints, int* paiContourEnds, size uiNumContours, int32* paiDeltaX, int32* paiDeltaY);

protected:
	bool					ParseFvar(void);
	void					ParseGvar(void);
	void					ParseHvar(void);
	void					ParseMvar(void);
	void					ParseItemVariationStore(lsize uiOffset, lsize uiTableEnd, STrueTypeItemVariationStore* psStore);
	void					InitItemVariationStore(STrueTypeItemVariationStore* psStore);
	void					KillItemVariationStore(STrueTypeItemVariationStore* psStore);

	void					UpdateCoordinates(void);
	int32					Normalise(size uiAxis, int32 iValue);
	int32					MapAvar(size uiAxis, int32 iNormalised);
	int32					ApplyAxis(int32 iScalar, int32 iCoordinate, int32 iStart, int32 iPeak, int32 iEnd);
	int32					TupleScalar(lsize uiPeak, lsize uiStart, lsize uiEnd);
	void					UpdateRegionScalars(STrueTypeItemVariationStore* psStore);
	int32					GetItemDelta(STrueTypeItemVariationStore* psStore, uint16 uiOuter, uint16 uiInner);

	bool					ReadPackedPoints(lsize* puiPos, lsize uiEnd, CArrayInt32* paiPoints, bool* pbAll);
	bool					ReadPackedDeltas(lsize* puiPos, lsize uiEnd, size uiCount, CArrayInt32* paiDeltas);
	void					InferDeltas(STrueTypePoint* pasPoints, size uiNumPoints, int* paiContourEnds, size uiNumContours, bool* pbTouched, int32* paiDeltaX, int32* paiDeltaY);
	void					InferContour(STrueTypePoint* pasPoints, size uiStart, size uiEnd, bool* pbTouched, int32* paiDeltaX, int32* paiDeltaY);
	int32					InferDelta(int32 iCoordinate, int32 iCoordinate1, int32 iCoordinate2, int32 iDelta1, int32 iDelta2);
};


#endif // __TRUE_TYPE_VARIATIONS_H__
