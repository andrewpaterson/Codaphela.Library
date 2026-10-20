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
void AppendFontInteger(CChars* szText, char* szIdentifier, int32 iValue)
{
	szText->Append("\t");
	szText->Append(szIdentifier);
	szText->Append("\t");
	szText->Append(iValue);
	szText->Append(";\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Each glyph is written as {CodePoint, Length, Cel, Step}.  The code point
// is the glyph's key read as a little endian integer, Length is the number
// of bytes in the key and Cel is the index of the glyph's cel in the cels
// file.
//
//////////////////////////////////////////////////////////////////////////
bool FontWriter(Ptr<CFont> pFont, char* szImagePathName, char* szCelsPathName, char* szPathName)
{
	CTextFile					cTextFile;
	CChars*						szText;
	Ptr<CArrayImageCel>			paCels;
	Ptr<CGlyph>					pGlyph;
	Ptr<CImageCel>				pCel;
	SIndexTreeMemoryIterator	sIter;
	uint8						auiKey[FONT_WRITER_MAX_KEY_LENGTH];
	size						uiKeyLength;
	bool						bExists;
	uint32						uiCodePoint;
	size						i;
	int32						aiValues[4];
	bool						bResult;

	cTextFile.Init();
	szText = &cTextFile.mcText;

	szText->Append("Font\n{\n\tName\t\"");
	szText->Append(pFont->GetName());
	szText->Append("\";\n");
	AppendFontInteger(szText, "SpaceWidth", pFont->GetSpaceWidth());
	AppendFontInteger(szText, "Ascent", pFont->GetAscent());
	AppendFontInteger(szText, "Descent", pFont->GetDescent());
	AppendImageCelsPath(szText, "Cels", szCelsPathName);
	szText->Append("\tGlyphs\n\t{\n");

	paCels = OMalloc<CArrayImageCel>();
	bExists = pFont->StartIteration(&sIter, auiKey, &uiKeyLength, FONT_WRITER_MAX_KEY_LENGTH);
	while (bExists)
	{
		if ((uiKeyLength == 0) || (uiKeyLength > 4))
		{
			cTextFile.Kill();
			return false;
		}

		uiCodePoint = 0;
		for (i = 0; i < uiKeyLength; i++)
		{
			uiCodePoint |= ((uint32)auiKey[i]) << (i * 8);
		}

		pGlyph = pFont->GetGlyph(auiKey, uiKeyLength);
		aiValues[0] = (int32)uiCodePoint;
		aiValues[1] = (int32)uiKeyLength;
		aiValues[2] = (int32)paCels->NumElements();
		aiValues[3] = pGlyph->GetStep();
		AppendImageCelsIntegers(szText, "Glyph", aiValues, 4);

		pCel = pGlyph->GetCel();
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
