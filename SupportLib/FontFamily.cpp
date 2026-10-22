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
#include "BaseLib/Logger.h"
#include "ImageColourConverter.h"
#include "TrueTypeFontFactory.h"
#include "FontFamily.h"


//The width of the image each size's glyphs are rasterised into.
#define FONT_FAMILY_IMAGE_WIDTH		512


//////////////////////////////////////////////////////////////////////////
//
// szCharacters (UTF8) are the characters rasterised for every size.
// fDefaultPixelSize is used for styles with a size of 0.  The faces are
// set with SetFace.
//
//////////////////////////////////////////////////////////////////////////
void CFontFamily::Init(const char* szName, const char* szCharacters, float32 fDefaultPixelSize, EColourOrder eColourOrder)
{
	size	i;

	PreInit();

	mszName.Init(szName);
	mszCharacters.Init(szCharacters);
	mfDefaultPixelSize = fDefaultPixelSize;
	meColourOrder = eColourOrder;
	miImageWidth = FONT_FAMILY_IMAGE_WIDTH;
	for (i = 0; i < NUM_FONT_STYLES; i++)
	{
		masFaces[i].szFilename.Init();
		masFaces[i].bLoaded = false;
	}
	mapColours.Init();
	masSizes.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A family of one font already made: regular in its default size.  Any
// other style or size is an error.
//
//////////////////////////////////////////////////////////////////////////
void CFontFamily::Init(Ptr<CFontColours> pRegular)
{
	Init(pRegular->GetWhite()->GetFontName(), "", 0.0f, CCO_Unknown);
	AddFont(FS_Regular, 0.0f, pRegular);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontFamily::Class(void)
{
	U_String(mszName);
	U_String(mszCharacters);
	U_Float32(mfDefaultPixelSize);
	U_Enum(meColourOrder);
	U_Int32(miImageWidth);
	U_Data(SFontFace[NUM_FONT_STYLES], masFaces);
	M_Embedded(mapColours);
	U_Data(CArrayFontFamilySize, masSizes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontFamily::Free(void)
{
	size	i;

	masSizes.Kill();
	for (i = 0; i < NUM_FONT_STYLES; i++)
	{
		if (masFaces[i].bLoaded)
		{
			masFaces[i].cTrueTypeFont.Kill();
		}
		masFaces[i].szFilename.Kill();
	}
	mszCharacters.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontFamily::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontFamily::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The TrueType font is loaded now so a missing or bad file is found
// straight away.  Returns false (and logs an error) if it can't be loaded.
//
//////////////////////////////////////////////////////////////////////////
bool CFontFamily::SetFace(EFontStyle eStyle, const char* szTrueTypeFilename)
{
	SFontFace*	psFace;

	psFace = &masFaces[eStyle];
	if (psFace->bLoaded)
	{
		psFace->cTrueTypeFont.Kill();
		psFace->bLoaded = false;
	}
	psFace->szFilename.Kill();
	psFace->szFilename.Init(szTrueTypeFilename);

	psFace->cTrueTypeFont.Init();
	if (!psFace->cTrueTypeFont.Load((char*)szTrueTypeFilename))
	{
		psFace->cTrueTypeFont.Kill();
		gcLogger.Error2(__METHOD__, " Font family [", mszName.Text(), "] could not load ", FontStyleToString(eStyle), " face [", szTrueTypeFilename, "].", NULL);
		return false;
	}
	psFace->bLoaded = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontFamily::HasFace(EFontStyle eStyle)
{
	return masFaces[eStyle].bLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
// A font already made (and white, see CFontColours) for a style in one
// size.  A size of 0 is the default size.
//
//////////////////////////////////////////////////////////////////////////
void CFontFamily::AddFont(EFontStyle eStyle, float32 fPixelSize, Ptr<CFontColours> pColours)
{
	SFontFamilySize*	psSize;

	psSize = masSizes.Add();
	psSize->eStyle = eStyle;
	psSize->iSize64 = GetSizeKey(fPixelSize);
	mapColours.Add(pColours);
}


//////////////////////////////////////////////////////////////////////////
//
// The colours of a style in a size, rasterised the first time they are
// asked for.  A size of 0 is the default size.  NULL (and an error logged)
// if the family doesn't have the style.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontColours> CFontFamily::GetColours(EFontStyle eStyle, float32 fPixelSize)
{
	size				i;
	int32				iSize64;
	SFontFamilySize*	psSize;
	Ptr<CFontColours>	pColours;

	iSize64 = GetSizeKey(fPixelSize);
	for (i = 0; i < masSizes.NumElements(); i++)
	{
		psSize = masSizes.Get(i);
		if ((psSize->eStyle == eStyle) && (psSize->iSize64 == iSize64))
		{
			return mapColours.Get(i);
		}
	}

	if (fPixelSize <= 0.0f)
	{
		fPixelSize = mfDefaultPixelSize;
	}
	pColours = Generate(eStyle, fPixelSize);
	if (pColours.IsNotNull())
	{
		psSize = masSizes.Add();
		psSize->eStyle = eStyle;
		psSize->iSize64 = iSize64;
		mapColours.Add(pColours);
	}
	return pColours;
}


//////////////////////////////////////////////////////////////////////////
//
// The font for a style's face, size, position and colour.  NULL (and an
// error logged) if the family doesn't have the style.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFontFamily::GetFont(STextStyle* psStyle)
{
	int16	iBaselineShift;

	return GetRunFont(psStyle, &iBaselineShift);
}


//////////////////////////////////////////////////////////////////////////
//
// The font for a style and how far its baseline moves, in pixels up.
// Superscript and subscript are the style's face at its size times the
// scale the face's font gives, moved by the shift that font gives.  A
// family of fonts already made can't make the smaller size so its
// superscript and subscript are an error.  NULL (and an error logged) if
// the family doesn't have the style.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFontFamily::GetRunFont(STextStyle* psStyle, int16* piBaselineShift)
{
	Ptr<CFontColours>		pColours;
	Ptr<CFontDescription>	pDescription;
	float32					fPixelSize;
	EFontStyle				eStyle;

	*piBaselineShift = 0;
	eStyle = psStyle->GetFontStyle();
	pColours = GetColours(eStyle, psStyle->fPixelSize);
	if (pColours.IsNull())
	{
		return NULL;
	}
	if (psStyle->ePosition == TP_Normal)
	{
		return pColours->GetFont(psStyle->sColour);
	}

	fPixelSize = (psStyle->fPixelSize > 0.0f) ? psStyle->fPixelSize : mfDefaultPixelSize;
	if (fPixelSize <= 0.0f)
	{
		gcLogger.Error2(__METHOD__, " Font family [", mszName.Text(), "] has only fonts already made so can't make superscript or subscript.", NULL);
		return NULL;
	}

	pDescription = pColours->GetWhite()->GetDescription();
	if (psStyle->ePosition == TP_Superscript)
	{
		fPixelSize *= pDescription->GetSuperscriptScale();
		*piBaselineShift = pDescription->GetSuperscriptShift();
	}
	else
	{
		fPixelSize *= pDescription->GetSubscriptScale();
		*piBaselineShift = -pDescription->GetSubscriptShift();
	}

	pColours = GetColours(eStyle, fPixelSize);
	if (pColours.IsNull())
	{
		*piBaselineShift = 0;
		return NULL;
	}
	return pColours->GetFont(psStyle->sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// Sizes are told apart to 1/64 of a pixel.
//
//////////////////////////////////////////////////////////////////////////
int32 CFontFamily::GetSizeKey(float32 fPixelSize)
{
	if (fPixelSize <= 0.0f)
	{
		fPixelSize = mfDefaultPixelSize;
	}
	return (int32)(fPixelSize * 64.0f + 0.5f);
}


//////////////////////////////////////////////////////////////////////////
//
// Rasterises the family's characters from the style's face in white.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontColours> CFontFamily::Generate(EFontStyle eStyle, float32 fPixelSize)
{
	SFontFace*				psFace;
	CTrueTypeFontFactory	cFactory;
	CImageColourRGB			cWhite;
	CChars					szName;
	Ptr<CFontDescription>	pDescription;
	Ptr<CImage>				pImage;

	psFace = &masFaces[eStyle];
	if (!psFace->bLoaded)
	{
		gcLogger.Error2(__METHOD__, " Font family [", mszName.Text(), "] has no ", FontStyleToString(eStyle), " face.", NULL);
		return NULL;
	}

	szName.Init(mszName.Text());
	szName.Append(" ");
	szName.Append(FontStyleToString(eStyle));
	szName.Append(" ");
	szName.Append((int32)(fPixelSize + 0.5f));
	szName.Append("px");

	cWhite.Init(1.0f, 1.0f, 1.0f);
	cFactory.Init(&psFace->cTrueTypeFont, fPixelSize);
	pDescription = cFactory.Describe(szName.Text(), mszCharacters.Text(), miImageWidth);
	pImage = cFactory.Rasterise(pDescription, &cWhite);
	cFactory.Kill();

	if (pImage.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Font family [", mszName.Text(), "] could not rasterise [", szName.Text(), "].", NULL);
		szName.Kill();
		return NULL;
	}
	szName.Kill();

	if ((meColourOrder != CCO_Unknown) && (pImage->GetColourOrder() != meColourOrder))
	{
		pImage = CImageColourConverter::ConvertColour(pImage, meColourOrder);
	}
	return OMalloc<CFontColours>(OMalloc<CFont>(pDescription, pImage));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* FontStyleToString(EFontStyle eStyle)
{
	switch (eStyle)
	{
	case FS_Regular:
		return "Regular";
	case FS_Bold:
		return "Bold";
	case FS_Italic:
		return "Italic";
	case FS_BoldItalic:
		return "Bold Italic";
	}
	return "Unknown";
}


char* CFontFamily::GetFamilyName(void) { return mszName.Text(); }
float32 CFontFamily::GetDefaultPixelSize(void) { return mfDefaultPixelSize; }
size CFontFamily::NumSizes(void) { return masSizes.NumElements(); }

