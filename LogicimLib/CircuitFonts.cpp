/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/UTF8.h"
#include "StandardLib/Objects.h"
#include "CircuitTextMarkupParser.h"
#include "CircuitFonts.h"


//Every character circuit text can have: printable ASCII.
#define CIRCUIT_FONT_CHARACTERS	" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCircuitTextSize::Init(float64 fWidth, float64 fHeight, float64 fOverlineTop)
{
	this->fWidth = fWidth;
	this->fHeight = fHeight;
	this->fOverlineTop = fOverlineTop;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitFonts::Init(Ptr<CFontFamily> pSansSerif, Ptr<CFontFamily> pMonospaced)
{
	mpSansSerif = pSansSerif;
	mpMonospaced = pMonospaced;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitFonts::Kill(void)
{
	mpMonospaced = NULL;
	mpSansSerif = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// White, see CFontColours.  NULL if the family hasn't the face.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontColours> CCircuitFonts::GetColours(ECircuitFont eFont, float32 fPixelSize, EFontTurn eTurn)
{
	if (eFont == CF_Monospaced)
	{
		if (mpMonospaced.IsNull())
		{
			return NULL;
		}
		return mpMonospaced->GetColours(FS_Regular, fPixelSize, eTurn);
	}
	if (mpSansSerif.IsNull())
	{
		return NULL;
	}
	return mpSansSerif->GetColours((eFont == CF_SansSerifBold) ? FS_Bold : FS_Regular, fPixelSize, eTurn);
}


//////////////////////////////////////////////////////////////////////////
//
// In grid units, see SCircuitTextSize: the plain text of the markup as
// it's drawn, superscript and subscript at their smaller sizes.  fSize is
// pixels at zoom one and is rounded to whole pixels.  Nothing without the
// font.
//
//////////////////////////////////////////////////////////////////////////
SCircuitTextSize CCircuitFonts::Measure(ECircuitFont eFont, float64 fSize, const char* szMarkup)
{
	CCircuitTextLine	cLine;
	SCircuitTextSize	sSize;

	cLine.Init();
	if (cLine.Layout(this, eFont, (float32)floor(fSize + 0.5), FT_Upright, FONT_COLOURS_WHITE, szMarkup))
	{
		sSize.Init(cLine.GetWidth() / CIRCUIT_TEXT_SCALE, cLine.GetFont()->GetLineHeight() / CIRCUIT_TEXT_SCALE, cLine.GetOverlineTop() / CIRCUIT_TEXT_SCALE);
	}
	else
	{
		sSize.Init(0, 0);
	}
	cLine.Kill();
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontFamily> CCircuitFonts::GetSansSerif(void)
{
	return mpSansSerif;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontFamily> CCircuitFonts::GetMonospaced(void)
{
	return mpMonospaced;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitTextLine::Init(void)
{
	mszText.Init();
	masRuns.Init();
	mpcFont = NULL;
	meTurn = FT_Upright;
	miWidth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitTextLine::Kill(void)
{
	masRuns.Kill();
	mszText.Kill();
	mpcFont = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the line laid out before.  A superscript or subscript size the
// fonts can't make is drawn at the line's size on its baseline.  False
// (and nothing laid out) without the face.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitTextLine::Layout(CCircuitFonts* pcFonts, ECircuitFont eFont, float32 fPixelSize, EFontTurn eTurn, ARGB32 sColour, const char* szMarkup)
{
	CCircuitTextMarkupParser	cParser;
	CArrayTextStyleRun			asStyleRuns;
	STextStyleRun*				psStyleRun;
	SCircuitTextRun*			psRun;
	STextStyle					sStyle;
	Ptr<CFontColours>			pColours;
	Ptr<CFontColours>			pRunColours;
	Ptr<CFontDescription>		pDescription;
	Ptr<CFont>					pFont;
	float32						fRunPixelSize;
	int16						iBaselineShift;
	size						i;

	mszText.ReInit();
	masRuns.ReInit();
	mpcFont = NULL;
	meTurn = eTurn;
	miWidth = 0;

	pColours = pcFonts->GetColours(eFont, fPixelSize, eTurn);
	if (pColours.IsNull())
	{
		return false;
	}
	pFont = pColours->GetFont(sColour);
	mpcFont = &pFont;
	pDescription = pColours->GetWhite()->GetDescription();

	asStyleRuns.Init();
	cParser.Init();
	cParser.Parse(szMarkup, &mszText, &asStyleRuns);
	cParser.Kill();

	for (i = 0; i < asStyleRuns.NumElements(); i++)
	{
		psStyleRun = asStyleRuns.Get(i);
		sStyle.Init(false, false, fPixelSize, sColour);
		psStyleRun->sOverride.ApplyTo(&sStyle);

		pRunColours = pColours;
		iBaselineShift = 0;
		if (sStyle.ePosition != TP_Normal)
		{
			if (sStyle.ePosition == TP_Superscript)
			{
				fRunPixelSize = fPixelSize * pDescription->GetSuperscriptScale();
				iBaselineShift = pDescription->GetSuperscriptShift();
			}
			else
			{
				fRunPixelSize = fPixelSize * pDescription->GetSubscriptScale();
				iBaselineShift = -pDescription->GetSubscriptShift();
			}
			pRunColours = pcFonts->GetColours(eFont, fRunPixelSize, eTurn);
			if (pRunColours.IsNull())
			{
				pRunColours = pColours;
				iBaselineShift = 0;
			}
		}
		pFont = pRunColours->GetFont(sStyle.sColour);

		psRun = masRuns.Add();
		psRun->uiStart = psStyleRun->uiStart;
		psRun->uiLength = psStyleRun->uiLength;
		psRun->pcFont = &pFont;
		psRun->iPen = miWidth;
		psRun->iWidth = (int32)pFont->CalculateWidth(mszText.Text() + psStyleRun->uiStart, psStyleRun->uiLength);
		psRun->iBaselineShift = iBaselineShift;
		psRun->uiDecorations = sStyle.GetDecorations();
		psRun->sColour = sStyle.sColour;
		miWidth += psRun->iWidth;
	}
	asStyleRuns.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCircuitTextLine::GetText(void) { return mszText.Text(); }
CFont* CCircuitTextLine::GetFont(void) { return mpcFont; }
int32 CCircuitTextLine::GetWidth(void) { return miWidth; }
size CCircuitTextLine::NumRuns(void) { return masRuns.NumElements(); }
SCircuitTextRun* CCircuitTextLine::GetRun(size uiIndex) { return masRuns.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// How far above the line's baseline (pixels) the highest ink of the
// glyphs of runs uiFirstRun to uiEndRun (exclusive) is, their baseline
// shifts included.  KiCad's overbars sit just over it.  -1 if they have
// no ink (spaces).
//
//////////////////////////////////////////////////////////////////////////
int32 CCircuitTextLine::GetInkAscent(size uiFirstRun, size uiEndRun)
{
	SCircuitTextRun*		psRun;
	SGlyphDescription*		psGlyph;
	Ptr<CFontDescription>	pDescription;
	CUTF8					cUTF8;
	size					uiUTFLength;
	size					i;
	int32					iInkTop;  //Transparent pixels from the top of the glyph's line to its ink.
	int32					iInkAscent;
	int32					iHighest;

	iHighest = -1;
	for (i = uiFirstRun; i < uiEndRun; i++)
	{
		psRun = masRuns.Get(i);
		pDescription = psRun->pcFont->GetDescription();
		cUTF8.Init((uint8*)(mszText.Text() + psRun->uiStart), psRun->uiLength);
		for (;;)
		{
			uiUTFLength = cUTF8.PeekUTFBytes();
			if ((uiUTFLength == 0) || (uiUTFLength == cUTF8.GetError()))
			{
				break;
			}
			psGlyph = pDescription->GetGlyph(&cUTF8);
			if ((psGlyph == NULL) || psGlyph->cSubImage.IsImageEmpty())
			{
				continue;
			}
			//A turned glyph's top is on its right.
			iInkTop = (meTurn == FT_Clockwise) ? psGlyph->cSubImage.msOffsetBottomRight.x : psGlyph->cSubImage.msOffsetTopLeft.y;
			iInkAscent = pDescription->GetAscent() - iInkTop + psRun->iBaselineShift;
			if (iInkAscent > iHighest)
			{
				iHighest = iInkAscent;
			}
		}
		cUTF8.Kill();
	}
	return iHighest;
}


//////////////////////////////////////////////////////////////////////////
//
// The first run from uiFromRun with the decoration (NumRuns if there's
// none) and, in puiEndRun, the end (exclusive) of the stretch of runs
// with it from there.
//
//////////////////////////////////////////////////////////////////////////
size CCircuitTextLine::FindDecorated(uint8 uiDecoration, size uiFromRun, size* puiEndRun)
{
	size	uiFirst;
	size	uiEnd;

	uiFirst = uiFromRun;
	while ((uiFirst < masRuns.NumElements()) && !(masRuns.Get(uiFirst)->uiDecorations & uiDecoration))
	{
		uiFirst++;
	}
	uiEnd = uiFirst;
	while ((uiEnd < masRuns.NumElements()) && (masRuns.Get(uiEnd)->uiDecorations & uiDecoration))
	{
		uiEnd++;
	}
	*puiEndRun = uiEnd;
	return uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// The line of a decoration along runs uiFirstRun to uiEndRun (exclusive):
// how far below the line's baseline (across the line, away from the tops
// of the glyphs; negative is above) it starts and how thick it is, in
// pixels.  It's placed and as thick as the largest font among the runs
// says and moved with that font's baseline shift, so "~{A_{0}}" has one
// straight overline (as CTextLayout).  Overlines are as KiCad's overbars:
// a gap their thickness over the runs' highest ink.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitTextLine::GetDecoration(uint8 uiDecoration, size uiFirstRun, size uiEndRun, int32* piOffset, int32* piThickness)
{
	SCircuitTextRun*		psLargest;
	SCircuitTextRun*		psRun;
	Ptr<CFontDescription>	pDescription;
	size					i;
	int32					iOffset;
	int32					iThickness;
	int32					iInkAscent;

	psLargest = masRuns.Get(uiFirstRun);
	for (i = uiFirstRun + 1; i < uiEndRun; i++)
	{
		psRun = masRuns.Get(i);
		if (psRun->pcFont->GetLineHeight() > psLargest->pcFont->GetLineHeight())
		{
			psLargest = psRun;
		}
	}

	pDescription = psLargest->pcFont->GetDescription();
	if (uiDecoration == TD_Underline)
	{
		iOffset = pDescription->GetUnderlineTop();
		iThickness = pDescription->GetUnderlineThickness();
	}
	else if (uiDecoration == TD_Overline)
	{
		iOffset = -pDescription->GetAscent();
		iThickness = pDescription->GetUnderlineThickness();
	}
	else
	{
		iOffset = pDescription->GetStrikeoutTop();
		iThickness = pDescription->GetStrikeoutThickness();
	}
	iOffset -= psLargest->iBaselineShift;
	if (iThickness < 1)
	{
		iThickness = 1;
	}
	if (uiDecoration == TD_Overline)
	{
		iInkAscent = GetInkAscent(uiFirstRun, uiEndRun);
		if (iInkAscent >= 0)
		{
			iOffset = -(iInkAscent + iThickness * 2);
		}
	}
	*piOffset = iOffset;
	*piThickness = iThickness;
}


//////////////////////////////////////////////////////////////////////////
//
// How far above the baseline (pixels) the top of the highest overline is.
// 0 if there are none.
//
//////////////////////////////////////////////////////////////////////////
int32 CCircuitTextLine::GetOverlineTop(void)
{
	size	uiFirst;
	size	uiEnd;
	int32	iOffset;
	int32	iThickness;
	int32	iTop;

	iTop = 0;
	uiFirst = FindDecorated(TD_Overline, 0, &uiEnd);
	while (uiFirst < masRuns.NumElements())
	{
		GetDecoration(TD_Overline, uiFirst, uiEnd, &iOffset, &iThickness);
		if (-iOffset > iTop)
		{
			iTop = -iOffset;
		}
		uiFirst = FindDecorated(TD_Overline, uiEnd, &uiEnd);
	}
	return iTop;
}


//////////////////////////////////////////////////////////////////////////
//
// A family of the regular face and, if szBoldFilename isn't NULL, the
// bold face.  NULL (and an error logged) if a face can't be loaded.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontFamily> CreateCircuitFontFamily(const char* szName, const char* szRegularFilename, const char* szBoldFilename, EColourOrder eColourOrder)
{
	Ptr<CFontFamily>	pFamily;

	pFamily = OMalloc<CFontFamily>(szName, CIRCUIT_FONT_CHARACTERS, 10.0f, eColourOrder);
	if (!pFamily->SetFace(FS_Regular, szRegularFilename))
	{
		return NULL;
	}
	if ((szBoldFilename != NULL) && !pFamily->SetFace(FS_Bold, szBoldFilename))
	{
		return NULL;
	}
	return pFamily;
}


//////////////////////////////////////////////////////////////////////////
//
// Liberation Sans (LiberationSans-Regular.ttf and LiberationSans-Bold.ttf)
// and Liberation Mono (LiberationMono-Regular.ttf) from the directory,
// coloured in eColourOrder (that of the image they're drawn on).
//
//////////////////////////////////////////////////////////////////////////
bool LoadCircuitFonts(CCircuitFonts* pcFonts, const char* szDirectory, EColourOrder eColourOrder)
{
	CFileUtil			cFileUtil;
	CChars				szRegular;
	CChars				szBold;
	CChars				szMono;
	Ptr<CFontFamily>	pSansSerif;
	Ptr<CFontFamily>	pMonospaced;

	szRegular.Init(szDirectory);
	cFileUtil.AppendToPath(&szRegular, "LiberationSans-Regular.ttf");
	szBold.Init(szDirectory);
	cFileUtil.AppendToPath(&szBold, "LiberationSans-Bold.ttf");
	szMono.Init(szDirectory);
	cFileUtil.AppendToPath(&szMono, "LiberationMono-Regular.ttf");

	pSansSerif = CreateCircuitFontFamily("Liberation Sans", szRegular.Text(), szBold.Text(), eColourOrder);
	pMonospaced = CreateCircuitFontFamily("Liberation Mono", szMono.Text(), NULL, eColourOrder);

	szMono.Kill();
	szBold.Kill();
	szRegular.Kill();

	pcFonts->Init(pSansSerif, pMonospaced);
	return pSansSerif.IsNotNull() && pMonospaced.IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's HorizontalAlignment names, in lower case.
//
//////////////////////////////////////////////////////////////////////////
const char* TextAlignmentToString(ETextAlignment eAlignment)
{
	switch (eAlignment)
	{
	case TA_Centre:
		return "center";
	case TA_Right:
		return "right";
	default:
		return "left";
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if it isn't one TextAlignmentToString gives.
//
//////////////////////////////////////////////////////////////////////////
bool StringToTextAlignment(const char* szAlignment, ETextAlignment* peAlignment)
{
	if (szAlignment == NULL)
	{
		return false;
	}
	if (StringCompare(szAlignment, "left") == 0)
	{
		*peAlignment = TA_Left;
	}
	else if (StringCompare(szAlignment, "center") == 0)
	{
		*peAlignment = TA_Centre;
	}
	else if (StringCompare(szAlignment, "right") == 0)
	{
		*peAlignment = TA_Right;
	}
	else
	{
		return false;
	}
	return true;
}
