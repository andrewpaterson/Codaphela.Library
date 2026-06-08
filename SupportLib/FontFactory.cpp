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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/TextFile.h"
#include "BaseLib/UTF8.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageDivider.h"
#include "FontFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontFactory::Init(void)
{
	PreInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontFactory::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontFactory::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontFactory::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontFactory::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontFactory::Convert(CImageDividerNumbers* pcDest, CFontImportParameters* pcSource)
{
	pcDest->InitGeneral(pcSource->msCharSize.x,
						pcSource->msCharSize.y,
						pcSource->msCharsGrid.x,
						pcSource->msCharsGrid.y,
						pcSource->msTopLeftOffset.x,
						pcSource->msTopLeftOffset.y,
						pcSource->msInterspace.x,
						pcSource->msInterspace.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFontFactory::Generate(CFontImportParameters* pcImportParams, CFontCreationParameters* pcCreationParams)
{
	CTextFile				cTextFile;
	Ptr<CImage>				pImage;
	CImageDivider			cDivider;
	CImageDividerNumbers	cNumbers;
	CArrayImageCel			cCels;
	CChars					szCharacters;
	Ptr<CImageCel>			pCel;
	Ptr<CFont>				pFont;
	bool					bResult;
	CUTF8					cUTF8;
	size					uiElementLength;
	size					uiCodePointLength;
	uint8					auiBuffer[20];
	uint16					cCodePoint16;
	uint32					cCodePoint32;
	size					ui;

	pImage = ReadImage(pcImportParams->GetImageFileName());
	
	if (pImage.IsNull())
	{
		return NULL;
	}

	if ((pcImportParams->msCharSize.x <= 0) ||
		(pcImportParams->msCharSize.y <= 0))
	{
		return NULL;
	}

	if ((pcImportParams->msCharsGrid.x == -1) &&
		(pcImportParams->msCharsGrid.y == -1))
	{
		pcImportParams->msCharsGrid.x = pImage->GetWidth() / pcImportParams->msCharSize.x;
		pcImportParams->msCharsGrid.y = pImage->GetHeight() / pcImportParams->msCharSize.y;
	}

	cTextFile.Init();
	bResult = cTextFile.Read(pcImportParams->GetCharacterFileName());
	if (!bResult)
	{
		pImage = NULL;
		cTextFile.Kill();
		return NULL;
	}
	szCharacters.Init(cTextFile.Text());
	cTextFile.Kill();

	Convert(&cNumbers, pcImportParams);

	cCels.Init();

	cDivider.Init(pImage);
	cDivider.GenerateFromNumbers(&cNumbers);
	cDivider.CopyCellsTo(&cCels);

	pFont = OMalloc<CFont>(pcImportParams->FontName(), pcCreationParams->miSpaceWidth, pcCreationParams->miAscent, pcCreationParams->miDescent);

	ui = 0;
	cUTF8.Init(&szCharacters);
	uiElementLength = cUTF8.PeekUTFBytes();
	while ((uiElementLength != 0) && (uiElementLength != cUTF8.GetError()))
	{
		pCel = cCels.Get(ui);
		if (uiElementLength <= 2)
		{
			cCodePoint16 = cUTF8.GetCodePointUint16();
			uiCodePointLength = cUTF8.GetUnicodeCodePointLength(cCodePoint16);
			if (uiCodePointLength == 1)
			{
				pFont->PutGlyph((uint8)cCodePoint16, pCel, pcImportParams->msCharSize.x);
			}
			else
			{
				pFont->PutGlyph(cCodePoint16, pCel, pcImportParams->msCharSize.x);
			}
		}
		else if (uiElementLength <= 4)
		{
			cCodePoint32 = cUTF8.GetCodePointUint32();
			uiCodePointLength = cUTF8.GetUnicodeCodePointLength(cCodePoint32);
			pFont->PutGlyph(cCodePoint32, uiCodePointLength, pCel, pcImportParams->msCharSize.x);
		}
		else
		{
			uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 20);
			if ((uiCodePointLength != 0) || (uiCodePointLength != cUTF8.GetError()))
			{
				pFont->PutGlyph(auiBuffer, uiCodePointLength, pCel, pcImportParams->msCharSize.x);
			}
		}
		
		ui++;
		uiElementLength = cUTF8.PeekUTFBytes();
	}
	cCels.Kill();
	szCharacters.Kill();
	return pFont;
}

