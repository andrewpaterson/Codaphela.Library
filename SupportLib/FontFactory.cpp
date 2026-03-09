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
	Ptr<CImage>	pImage = ReadImage(pcParams->mszFileName.Text());
	
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

	Ptr<CFont> pFont = OMalloc<CFont>(pcParams->FontName(), pcParams->miSpaceWidth, pcParams->miAscent, pcParams->miDescent);
	
	CImageDivider			cDivider;
	CImageDividerNumbers	cNumbers;
	CArrayImageCel			cCels;
	size					ui;
	char*					szCharacters;
	char					c;
	Ptr<CImageCel>			pCel;

	Convert(&cNumbers, pcParams);

	cCels.Init();

	cDivider.Init(pImage);
	cDivider.GenerateFromNumbers(&cNumbers);
	cDivider.CopyCellsTo(&cCels);

	szCharacters = pcParams->Characters();
	ui = 0;
	c = szCharacters[ui];
	while (c != '\0')
	{
		pCel = cCels.Get(ui);
		pFont->PutGlyph((uint16)c, pCel, pcParams->msCharSize.x);
		c = szCharacters[ui];
	}
	cCels.Kill();
	return pFont;
}

