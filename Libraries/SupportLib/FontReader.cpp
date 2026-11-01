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
#include "BaseLib/TextFile.h"
#include "BaseLib/TextParser.h"
#include "ImageCelsReader.h"
#include "FontReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetFontInteger(CTextParser* pcTextParser, char* szIdentifier, int64* piValue)
{
	ReturnOnErrorAndFalse(pcTextParser->GetHFExactIdentifierAndInteger(szIdentifier, piValue));
	pcTextParser->GetExactCharacter(';');
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetFontInt16(CTextParser* pcTextParser, char* szIdentifier, int16* piValue)
{
	int64	iValue;

	ReturnOnErrorAndFalse(GetFontInteger(pcTextParser, szIdentifier, &iValue));
	if (iValue != (int16)iValue)
	{
		return TRIERROR;
	}
	*piValue = (int16)iValue;
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// See FontWriter for what a glyph's values are.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetFontGlyph(CTextParser* pcTextParser, Ptr<CFontDescription> pDescription, Ptr<CArrayImageCel> paCels)
{
	int64			iCodePoint;
	int64			iLength;
	int64			iCelIndex;
	int16			iStep;
	uint32			uiCodePoint;
	size			uiLength;
	Ptr<CImageCel>	pCel;
	TRISTATE		tResult;

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("Glyph"));

	//Once a glyph has started every value must be there.
	tResult = pcTextParser->GetExactCharacter('{');
	if (tResult == TRITRUE)
	{
		tResult = GetFontInteger(pcTextParser, "CodePoint", &iCodePoint);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetFontInteger(pcTextParser, "Length", &iLength);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetFontInteger(pcTextParser, "CelIndex", &iCelIndex);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetFontInt16(pcTextParser, "Step", &iStep);
	}
	if (tResult == TRITRUE)
	{
		tResult = pcTextParser->GetExactCharacter('}');
	}
	if (tResult != TRITRUE)
	{
		return TRIERROR;
	}

	if ((iLength < 1) || (iLength > 4))
	{
		return TRIERROR;
	}
	uiLength = (size)iLength;
	if ((iCodePoint < 0) || ((iCodePoint >> (uiLength * 8)) != 0))
	{
		return TRIERROR;
	}
	uiCodePoint = (uint32)iCodePoint;
	if ((iCelIndex < 0) || ((size)iCelIndex >= paCels->NumElements()))
	{
		return TRIERROR;
	}

	pCel = paCels->Get((size)iCelIndex);
	pDescription->PutGlyph(uiCodePoint, uiLength, pCel->GetSubImage(), iStep);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetFontGlyphs(CTextParser* pcTextParser, Ptr<CFontDescription> pDescription, Ptr<CArrayImageCel> paCels)
{
	TRISTATE	tResult;

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("Glyphs"));
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));
	for (;;)
	{
		tResult = GetFontGlyph(pcTextParser, pDescription, paCels);
		if (tResult != TRITRUE)
		{
			break;
		}
	}
	if (tResult == TRIFALSE)
	{
		tResult = pcTextParser->GetExactCharacter('}');
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> FontReader(char* szPathName)
{
	CTextParser 		cTextParser;
	CTextFile			cTextFile;
	CChars				szName;
	CChars				szCelsPathName;
	int16				iSpaceWidth;
	int16				iAscent;
	int16				iDescent;
	TRISTATE			tResult;
	Ptr<CArrayImageCel>	paCels;
	Ptr<CImage>			pImage;
	Ptr<CFontDescription>	pDescription;

	cTextFile.Init();
	if (!cTextFile.Read(szPathName))
	{
		cTextFile.Kill();
		return NULL;
	}
	cTextFile.PassifyNewlines();
	cTextParser.Init(cTextFile.Text(), cTextFile.Length());

	szName.Init();
	szCelsPathName.Init();
	tResult = cTextParser.GetExactIdentifier("Font");
	if (tResult == TRITRUE)
	{
		tResult = cTextParser.GetExactCharacter('{');
	}
	if (tResult == TRITRUE)
	{
		tResult = GetImageCelsIdentifierAndString(&cTextParser, "Name", &szName);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetFontInt16(&cTextParser, "SpaceWidth", &iSpaceWidth);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetFontInt16(&cTextParser, "Ascent", &iAscent);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetFontInt16(&cTextParser, "Descent", &iDescent);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetImageCelsPath(&cTextParser, "Cels", &szCelsPathName);
	}
	if (tResult == TRITRUE)
	{
		paCels = CelsReader(szCelsPathName.Text());
		if (paCels.IsNull())
		{
			tResult = TRIERROR;
		}
	}
	szCelsPathName.Kill();
	if (tResult != TRITRUE)
	{
		szName.Kill();
		cTextParser.Kill();
		cTextFile.Kill();
		return NULL;
	}

	//CelsReader read every cel from one image.
	if (paCels->NumElements() > 0)
	{
		pImage = paCels->Get(0)->GetSourceImage();
	}
	if (pImage.IsNotNull())
	{
		pDescription = OMalloc<CFontDescription>(szName.Text(), iSpaceWidth, iAscent, iDescent, pImage->GetWidth(), pImage->GetHeight());
	}
	else
	{
		pDescription = OMalloc<CFontDescription>(szName.Text(), iSpaceWidth, iAscent, iDescent, 0, 0);
	}
	szName.Kill();

	tResult = GetFontGlyphs(&cTextParser, pDescription, paCels);
	if (tResult == TRITRUE)
	{
		tResult = cTextParser.GetExactCharacter('}');
	}
	cTextParser.Kill();
	cTextFile.Kill();

	if (tResult != TRITRUE)
	{
		pDescription->Kill();
		paCels->Kill();
		return NULL;
	}

	return OMalloc<CFont>(pDescription, pImage);
}

