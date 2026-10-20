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
TRISTATE GetFontInt16(CTextParser* pcTextParser, char* szIdentifier, int16* piValue)
{
	int64	iValue;

	ReturnOnErrorAndFalse(pcTextParser->GetHFExactIdentifierAndInteger(szIdentifier, &iValue));
	pcTextParser->GetExactCharacter(';');
	if (iValue != (int16)iValue)
	{
		return TRIERROR;
	}
	*piValue = (int16)iValue;
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// Glyph is {CodePoint, Length, Cel, Step}, see FontWriter.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetFontGlyph(CTextParser* pcTextParser, Ptr<CFont> pFont, Ptr<CArrayImageCel> paCels)
{
	int32			aiValues[4];
	uint32			uiCodePoint;
	size			uiLength;
	Ptr<CImageCel>	pCel;

	ReturnOnErrorAndFalse(GetImageCelsIdentifierAndIntegers(pcTextParser, "Glyph", aiValues, 4));

	uiCodePoint = (uint32)aiValues[0];
	if ((aiValues[1] < 1) || (aiValues[1] > 4))
	{
		return TRIERROR;
	}
	uiLength = (size)aiValues[1];
	if ((uiLength < 4) && ((uiCodePoint >> (uiLength * 8)) != 0))
	{
		return TRIERROR;
	}
	if ((aiValues[2] < 0) || ((size)aiValues[2] >= paCels->NumElements()))
	{
		return TRIERROR;
	}
	if (aiValues[3] != (int16)aiValues[3])
	{
		return TRIERROR;
	}

	pCel = paCels->Get(aiValues[2]);
	pFont->PutGlyph(uiCodePoint, uiLength, pCel, (int16)aiValues[3]);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetFontGlyphs(CTextParser* pcTextParser, Ptr<CFont> pFont, Ptr<CArrayImageCel> paCels)
{
	TRISTATE	tResult;

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("Glyphs"));
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));
	for (;;)
	{
		tResult = GetFontGlyph(pcTextParser, pFont, paCels);
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
	Ptr<CFont>			pFont;

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

	pFont = OMalloc<CFont>(szName.Text(), iSpaceWidth, iAscent, iDescent);
	szName.Kill();

	tResult = GetFontGlyphs(&cTextParser, pFont, paCels);
	if (tResult == TRITRUE)
	{
		tResult = cTextParser.GetExactCharacter('}');
	}
	cTextParser.Kill();
	cTextFile.Kill();

	if (tResult != TRITRUE)
	{
		pFont->Kill();
		paCels->Kill();
		return NULL;
	}

	//CelsReader read every cel from one image.
	if (paCels->NumElements() > 0)
	{
		pFont->SetImage(paCels->Get(0)->GetSourceImage());
	}
	return pFont;
}
