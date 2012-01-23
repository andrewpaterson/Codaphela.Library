/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ImageRecolourInvertBlackAndWhite.h"
#include "ImageAccessorCreator.h"
#include "ImageWriter.h"
#include "ImageCombiner.h"
#include "ImageCelsSource.h"
#include "ImageCelSourceRectangles.h"
#include "WinText.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CALLBACK EnumFontsProc(CONST LOGFONT *lplf, CONST TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData)
{
	CWinText*	pcWinText;
	SWinFont*	psWinFont;

	if ((dwType & RASTER_FONTTYPE) || (dwType & TRUETYPE_FONTTYPE))
	{
		pcWinText = (CWinText*)lpData;
		psWinFont = pcWinText->mcWinFonts.Add();
		psWinFont->sInstances.Init();
		strcpy_s(psWinFont->szName, lplf->lfFaceName);
	}

	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::Init(HWND hWnd)
{
	HDC		hDC;

	mcWinFonts.Init(10);

	hDC = GetDC(hWnd);
	EnumFonts(hDC, NULL, EnumFontsProc, (LPARAM)this);

	miNumFonts = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::Kill()
{
	int		i;
	int		j;
	HFONT	hFont;

	for (i = 0; i < mcWinFonts.NumElements(); i++)
	{
		for (j = 0; j < mcWinFonts[i].sInstances.NumElements(); j++)
		{
			hFont = mcWinFonts[i].sInstances[j].hFont;
			DeleteObject(hFont);
		}
		mcWinFonts[i].sInstances.Kill();
	}
	mcWinFonts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SWinFontInstance* CWinText::Create(char* szName, int iHeight, int iWidth, int iWeight)
{
	HFONT				hFont;
	int					i;
	SWinFont*			psWinFont;
	SWinFontInstance*	psInstance;

	psInstance = NULL;
	psWinFont = NULL;

	for (i = 0; i < mcWinFonts.NumElements(); i++)
	{
		psWinFont = &mcWinFonts[i];
		if (strcmp(psWinFont->szName, szName) == 0)
		{
			break;
		}
		psWinFont = NULL;
	}

	hFont = NULL;

	if (psWinFont)
	{
		for (i = 0; i < psWinFont->sInstances.NumElements(); i++)
		{
			psInstance = &psWinFont->sInstances[i];
			if ((psInstance->iHeight == iHeight) &&
				(psInstance->iWeight == iWeight) &&
				(psInstance->iWidth == iWidth))
			{
				hFont = psInstance->hFont;
				break;
			}
		}
		if (hFont == NULL)
		{
			hFont = CreateFont(iHeight, iWidth, 0, 0, iWeight, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, szName);
			psInstance = psWinFont->sInstances.Add();
			
			psInstance->hFont = hFont;
			psInstance->iHeight = iHeight;
			psInstance->iWeight = iWeight;
			psInstance->iWidth = iWidth;
			psInstance->iUniqueID = miNumFonts;
			miNumFonts++;
		}
	}
	return psInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::Draw(char* szString, SWinFontInstance* psInstance, HDC hDC, int iFlags, BOOL bSingleLine, RECT* psRect)
{
	int	iLen;
	int	iFormat;

	iLen = (int)strlen(szString);
	iFormat = 0;
	
	if (iFlags & TEXT_ALIGN_HCENTER)	{ iFormat |= DT_CENTER; }
	if (iFlags & TEXT_ALIGN_VCENTER)	{ iFormat |= DT_VCENTER; }
	if (iFlags & TEXT_ALIGN_LEFT)		{ iFormat |= DT_LEFT; }
	if (iFlags & TEXT_ALIGN_RIGHT)		{ iFormat |= DT_RIGHT; }
	if (iFlags & TEXT_ALIGN_TOP)		{ iFormat |= DT_TOP; }
	if (iFlags & TEXT_ALIGN_BOTTOM)		{ iFormat |= DT_BOTTOM; }
	if (bSingleLine)					{ iFormat |= DT_SINGLELINE; }

	SelectObject(hDC, psInstance->hFont);
	DrawText(hDC, szString, iLen, psRect, iFormat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::Draw(char* szString, SWinFontInstance* psInstance, HDC hDC, int x, int y)
{
	int		iLen;
	BOOL	bReturn;

	iLen = (int)strlen(szString);

	SelectObject(hDC, psInstance->hFont);
	bReturn = TextOut(hDC, x, y, szString, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFont* CWinText::GenerateFont(SWinFontInstance* pcWinFont, char* szFontName)
{
	HFONT								hFont;
	HDC									hDC;
	CImage								cImageTemp;
	TEXTMETRIC							sTextMetric;
	CFont*								pcFont;
	CArrayRectangle						acRectangles;
	CChars								szLetters;

	if (!pcWinFont)
	{
		return NULL;
	}

	GetSourceGlyphs(&szLetters);

	hDC = CreateCompatibleDC(NULL);
	hFont = pcWinFont->hFont;
	SelectObject(hDC, hFont);

	GetSourceRectangles(&acRectangles, &szLetters, hDC);

	GetTextMetrics(hDC, &sTextMetric);

	pcFont = UMalloc(CFont);
	pcFont->Init(szFontName, acRectangles.Get(0)->GetWidth(), sTextMetric.tmAscent, sTextMetric.tmDescent);

	DrawTextToImage(&cImageTemp, &szLetters, hDC);

	pcFont->GetImage()->Kill();
	PackImage(pcFont->GetImage(), pcFont, &cImageTemp, &acRectangles);

	pcFont->Done();

	szLetters.Kill();
	acRectangles.Kill();
	cImageTemp.Kill();

	return pcFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::PackImage(CImage* pcDestImage, CFont* pcFont, CImage* pcSource, CArrayRectangle* pacRectangles)
{
	CImageCelsSource					cSource;
	CImageCombiner						cCombiner;
	SImageColour						sColour;
	CArrayUnknown*						pacCels;
	CImageCel*							pcCel;
	CImageRecolourInvertBlackAndWhite	cRecolour;
	int									i;
	CImageCelSourceRectangles			cRectangles;

	sColour.Zero();
	cRectangles.Init(pacRectangles);

	cRecolour.Init();

	cSource.Init();
	cSource.AddMemorySource(pcSource, &cRectangles);
	cSource.AddModifier(&cRecolour);
	cSource.Load();

	cCombiner.Init(pcDestImage, ICL_Best, ICS_PowerOf2, ICC_FromCels);
	cCombiner.AddCels(cSource.GetCels());
	cCombiner.Combine();
	pacCels = cCombiner.GetCels();

	for (i = 0; i < pacCels->NumElements(); i++)
	{
		pcCel = (CImageCel*)pacCels->Get(i);
		pcFont->AddGlyph(pcCel, pcCel->GetSubImage()->GetFullWidth());
	}

	cCombiner.Kill();
	cRecolour.Kill();
	cRectangles.Kill();
	cSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::GetSourceRectangles(CArrayRectangle* pacRectangles, CChars* pszLetters, HDC hDC)
{
	char			szSingle[2];
	int				i;
	int				iWidth;
	SIZE			sSize;
	CRectangle*		pcRect;

	szSingle[1] = 0;
	iWidth = 0;
	sSize.cx = 0;
	pacRectangles->Init(10);
	for (i = 0; i < pszLetters->Length(); i++)
	{
		szSingle[0] = pszLetters->GetChar(i);
		GetTextExtentPoint32(hDC, szSingle, 1, &sSize);

		pcRect = pacRectangles->Add();
		pcRect->Init(iWidth, 0, iWidth + sSize.cx, sSize.cy);
		iWidth += sSize.cx;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::GetSourceGlyphs(CChars* pszDest)
{
	int		i;

	pszDest->Init();
	for (i = 0; i < NUM_LETTERS; i++)
	{
		pszDest->Append((char)(i + FIRST_LETTER));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinText::DrawTextToImage(CImage* pcDestImage, CChars* pszLetters, HDC hDC)
{
	HBITMAP			hBM;
	BITMAPINFO		bmi;
	int*			pBits;
	SIZE			sSize;

	GetTextExtentPoint32(hDC, pszLetters->Text(), pszLetters->Length(), &sSize);

	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biHeight = -sSize.cy;
	bmi.bmiHeader.biWidth = sSize.cx;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;

	hBM = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	SelectObject(hDC, hBM);
	TextOut(hDC, 0, 0, pszLetters->Text(), pszLetters->Length());

	pcDestImage->Init(sSize.cx, sSize.cy, PT_uchar, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_ZERO);
	memcpy(pcDestImage->GetData(), pBits, pcDestImage->GetByteSize());

	DeleteObject(hBM);

}
