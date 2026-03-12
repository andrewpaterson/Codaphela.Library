/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/UTF-8.h"
#include "StandardLib/ClassDefines.h"
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
void CFontFactory::Convert(CImageDividerNumbers* pcDest, CFontImportParams* pcSource)
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
Ptr<CFont> CFontFactory::Generate(CFontImportParams* pcParams)
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
	size					uiLength;
	uint8					auiBuffer[64];
	uint16					c16;
	uint32					c32;
	size					ui;

	pImage = ReadImage(pcParams->GetImageFileName());
	
	if (pImage.IsNull())
	{
		return NULL;
	}

	if ((pcParams->msCharSize.x <= 0) ||
		(pcParams->msCharSize.y <= 0))
	{
		return NULL;
	}

	if ((pcParams->msCharsGrid.x == -1) &&
		(pcParams->msCharsGrid.y == -1))
	{
		pcParams->msCharsGrid.x = pImage->GetWidth() / pcParams->msCharSize.x;
		pcParams->msCharsGrid.y = pImage->GetHeight() / pcParams->msCharSize.y;
	}

	cTextFile.Init();
	bResult = cTextFile.Read(pcParams->GetCharacterFileName());
	if (!bResult)
	{
		pImage = NULL;
		cTextFile.Kill();
		return NULL;
	}
	szCharacters.Init(cTextFile.Text());
	cTextFile.Kill();

	Convert(&cNumbers, pcParams);

	cCels.Init();

	cDivider.Init(pImage);
	cDivider.GenerateFromNumbers(&cNumbers);
	cDivider.CopyCellsTo(&cCels);

	pFont = OMalloc<CFont>(pcParams->FontName(), pcParams->miSpaceWidth, pcParams->miAscent, pcParams->miDescent);

	ui = 0;
	cUTF8.Init(&szCharacters);
	uiLength = cUTF8.GetLength();
	while ((uiLength != 0) || (uiLength != cUTF8.GetError()))
	{
		pCel = cCels.Get(ui);
		if (uiLength <= 2)
		{
			c16 = cUTF8.GetUint16();
			pFont->PutGlyph(c16, pCel, pcParams->msCharSize.x);
		}
		else if (uiLength <= 4)
		{
			c32 = cUTF8.GetUint32();
			pFont->PutGlyph(c32, pCel, pcParams->msCharSize.x);
		}
		else
		{
			uiLength = cUTF8.GetMulti(auiBuffer, 64);
			if ((uiLength != 0) || (uiLength != cUTF8.GetError()))
			{
				pFont->PutGlyph(auiBuffer, uiLength, pCel, pcParams->msCharSize.x);
			}
		}
		
		ui++;
		uiLength = cUTF8.GetLength();
	}
	cCels.Kill();
	szCharacters.Kill();
	return pFont;
}

