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
#include "ImageCelsWriter.h"
#include "FontWriter.h"


#define FONT_WRITER_MAX_KEY_LENGTH	64


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AppendFontInteger(CChars* szText, size uiDepth, char* szIdentifier, int32 iValue)
{
	szText->Append('\t', uiDepth);
	szText->Append(szIdentifier);
	szText->Append("\t");
	szText->Append(iValue);
	szText->Append(";\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Each glyph is written as a CodePoint, Length, CelIndex and Step.  The
// code point is the glyph's key read as a little endian integer, Length is
// the number of bytes in the key and CelIndex is the index of the glyph's
// cel in the cels file.
//
//////////////////////////////////////////////////////////////////////////
bool FontWriter(Ptr<CFont> pFont, char* szImagePathName, char* szCelsPathName, char* szPathName)
{
	CTextFile					cTextFile;
	CChars*						szText;
	Ptr<CArrayImageCel>			paCels;
	SGlyphDescription*			psDescription;
	Ptr<CImageCel>				pCel;
	SIndexTreeMemoryIterator	sIter;
	uint8						auiKey[FONT_WRITER_MAX_KEY_LENGTH];
	size						uiKeyLength;
	bool						bExists;
	uint32						uiCodePoint;
	size						i;
	bool						bResult;

	cTextFile.Init();
	szText = &cTextFile.mcText;

	szText->Append("Font\n{\n\tName\t\"");
	szText->Append(pFont->GetFontName());
	szText->Append("\";\n");
	AppendFontInteger(szText, 1, "SpaceWidth", pFont->GetSpaceWidth());
	AppendFontInteger(szText, 1, "Ascent", pFont->GetAscent());
	AppendFontInteger(szText, 1, "Descent", pFont->GetDescent());
	AppendImageCelsPath(szText, "Cels", szCelsPathName);
	szText->Append("\tGlyphs\n\t{\n");

	paCels = OMalloc<CArrayImageCel>();
	bExists = pFont->StartIteration(&sIter, auiKey, &uiKeyLength, FONT_WRITER_MAX_KEY_LENGTH);
	while (bExists)
	{
		if ((uiKeyLength == 0) || (uiKeyLength > 4))
		{
			cTextFile.Kill();
			sIter.Kill();
			return false;
		}

		uiCodePoint = 0;
		for (i = 0; i < uiKeyLength; i++)
		{
			uiCodePoint |= ((uint32)auiKey[i]) << (i * 8);
		}

		psDescription = pFont->GetGlyphDescription(auiKey, uiKeyLength);
		szText->Append("\t\tGlyph\n\t\t{\n");
		AppendFontInteger(szText, 3, "CodePoint", (int32)uiCodePoint);
		AppendFontInteger(szText, 3, "Length", (int32)uiKeyLength);
		AppendFontInteger(szText, 3, "CelIndex", (int32)paCels->NumElements());
		AppendFontInteger(szText, 3, "Step", psDescription->iStep);
		szText->Append("\t\t}\n");

		//A cel just for writing so the font's glyphs aren't all generated.
		pCel = pFont->CreateCel(psDescription);
		paCels->Add(pCel);

		bExists = pFont->Iterate(&sIter, auiKey, &uiKeyLength, FONT_WRITER_MAX_KEY_LENGTH);
	}
	szText->Append("\t}\n}\n");

	bResult = CelsWriter(paCels, szImagePathName, szCelsPathName);
	if (bResult)
	{
		bResult = cTextFile.Write(szPathName);
	}
	cTextFile.Kill();
	return bResult;
}
