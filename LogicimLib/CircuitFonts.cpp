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
#include "StandardLib/Objects.h"
#include "CircuitFonts.h"


//Every character circuit text can have: printable ASCII.
#define CIRCUIT_FONT_CHARACTERS	" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCircuitTextSize::Init(float64 fWidth, float64 fHeight)
{
	this->fWidth = fWidth;
	this->fHeight = fHeight;
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
// In grid units, see SCircuitTextSize.  fSize is pixels at zoom one and is
// rounded to whole pixels.  Nothing without the font.
//
//////////////////////////////////////////////////////////////////////////
SCircuitTextSize CCircuitFonts::Measure(ECircuitFont eFont, float64 fSize, const char* szText)
{
	Ptr<CFontColours>	pColours;
	Ptr<CFont>			pFont;
	SCircuitTextSize	sSize;

	pColours = GetColours(eFont, (float32)floor(fSize + 0.5), FT_Upright);
	if (pColours.IsNull())
	{
		sSize.Init(0, 0);
		return sSize;
	}
	pFont = pColours->GetWhite();
	sSize.Init(pFont->CalculateWidth((char*)szText) / CIRCUIT_TEXT_SCALE, pFont->GetLineHeight() / CIRCUIT_TEXT_SCALE);
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
