/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TextData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextChar::Init(char c, CTextParameters* pcFont)
{
	msTopLeft.Init(0, 0);
	mcChar = c;
	mpcFont = pcFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextChar::Layout(int x, int y)
{
	msTopLeft.Init(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextChar::CreateQuad(CGraphicsObject* pcGraphicsObject, CViewportInstance* pcViewport, SInt2* psPosition, CMap2D* pcMap2D)
{
	if (!IsWhiteSpace())
	{
		CreateQuad(mcChar, pcGraphicsObject, pcViewport, psPosition, pcMap2D);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextChar::CreateQuad(char c, CGraphicsObject* pcGraphicsObject, CViewportInstance* pcViewport, SInt2* psPosition, CMap2D* pcMap2D)
{
	CQuad2D*	pcQuad;
	CGlyph*		pcGlyph;
	int			x;
	int			y;
	int			iWidth;
	int			iHeight;
	CSubImage*	psSubImage;
	CRectangle	cDestRect;

	x = (int)psPosition->x + msTopLeft.x;
	y = (int)psPosition->y + msTopLeft.y;

	pcGlyph = mpcFont->GetGlyph(c);
	psSubImage = pcGlyph->GetSubImage();
	pcGlyph->GetImageDestBounds(x, y, &cDestRect);

	if (pcViewport->Contains(&cDestRect))
	{
		iWidth = psSubImage->GetImageWidth();
		iHeight = psSubImage->GetImageHeight();
		pcQuad = pcMap2D->CreateQuad(mpcFont->GetGraphicsMaterial(), mpcFont->GetGraphicsState(), pcViewport, iWidth, iHeight, cDestRect.miLeft, cDestRect.miTop);

		pcQuad->SetUVCoordsFromRectangle(0, &psSubImage->mcImageRect);
		
		//You need to set the texture coordinates for multi-texturing here.
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextChar::GetWidth(void)
{
	CFont*	pcFont;

	pcFont = mpcFont->GetFont();
	if (mcChar == ' ')
	{
		return pcFont->GetSpace();
	}
	else if (mcChar == '\t')
	{
		return mpcFont->miTabSpaceCount * pcFont->GetSpace();
	}
	else
	{
		return mpcFont->GetGlyph(mcChar)->GetFullWidth();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextChar::GetRight(void)
{
	CGlyph*		pcGlyph;

	pcGlyph = mpcFont->GetGlyph(mcChar);
	return pcGlyph->GetFullDestRight(msTopLeft.x) - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextChar::IsWhiteSpace(void)
{
	return ((mcChar == ' ') || (mcChar == '\t'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextChar::GetBounds(CRectangle* pcDest)
{
	CGlyph*		pcGlyph;

	pcGlyph = mpcFont->GetGlyph(mcChar);
	pcGlyph->GetFullDestBounds(msTopLeft.x, msTopLeft.y, pcDest);
	pcDest->miRight--;
	pcDest->miBottom--;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextCharLine::Init(void)
{
	mcChars.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextCharLine::Init(CTextParameters* pcFont, char* szText, int iLen)
{
	Init();
	Insert(0, pcFont, szText, iLen);

}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextCharLine::Kill(void)
{
	mcChars.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextParameters* CTextCharLine::Layout(int x, int y, CTextParameters* pcLastFont)
{
	int					i;
	CTextChar*			pcChar;
	int					iMaxAscent;
	int					iMaxDescent;
	CTextParameters*	pcFont;
	int					iCharX;

	iMaxAscent = 0;
	iMaxDescent = 0;
	iCharX = x;

	if (mcChars.NumElements() > 0)
	{
		for (i = 0; i < mcChars.NumElements(); i++)
		{
			pcChar = mcChars.Get(i);
			pcFont = pcChar->mpcFont;
			pcLastFont = pcFont;
			if (pcFont->GetAscent() > iMaxAscent)
			{
				iMaxAscent = pcFont->GetAscent();
			}
			if (pcFont->GetDescent() > iMaxDescent)
			{
				iMaxDescent = pcFont->GetDescent();
			}
		}

		for (i = 0; i < mcChars.NumElements(); i++)
		{
			pcChar = mcChars.Get(i);
			pcChar->Layout(iCharX, y + iMaxAscent);
			iCharX += pcChar->GetWidth();
		}
	}
	else
	{
		iMaxAscent = pcLastFont->GetAscent();
		iMaxDescent = pcLastFont->GetDescent();
	}

	msTopLeft.Init(x, y);
	miBaseline = y + iMaxAscent;
	msBottomRight.x = iCharX-1;
	if (msBottomRight.x < x)
	{
		msBottomRight.x = x;
	}
	msBottomRight.y = y + iMaxAscent + iMaxDescent;  //Er, possible off by one error.  Dunno.

	return pcLastFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextCharLine::CreateQuads(CGraphicsObject* pcGraphicsObject, CViewportInstance* pcViewport, SInt2* psPosition, CMap2D* pcMap2D)
{
	int					i;
	CTextChar*			pcChar;

	for (i = 0; i < mcChars.NumElements(); i++)
	{
		pcChar = mcChars.Get(i);
		pcChar->CreateQuad(pcGraphicsObject, pcViewport, psPosition, pcMap2D);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextCharLine::Insert(int x, CTextParameters* pcFont, char* szText, int iLen)
{
	CTextChar*	pcNewArray;
	CTextChar*	pcChar;
	int			i;
	char		c;
	int			iPos;

	if ((iLen <= 0) || (szText == NULL))
	{
		return NULL;
	}

	if (IsEmpty())
	{
		pcNewArray = mcChars.InsertNumAt(iLen, 0);
	}
	else
	{
		iPos = GetCharIndex(x);
		if (iPos < 0)
		{
			iPos = 0;
		}
		pcNewArray = mcChars.InsertNumAt(iLen, iPos);
	}

	for (i = 0; i < iLen; i++)
	{
		pcChar = &pcNewArray[i];
		c = szText[i];
		if ((c != '\t') && (c < ' '))
		{
			c = ' ';
		}
		pcChar->Init(c, pcFont);
	}
	return &pcNewArray[iLen-1];
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextCharLine::Insert(int x, CTextParameters* pcFont, char c)
{
	return Insert(x, pcFont, &c, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextCharLine::Append(CChars* pcText)
{
	CTextChar*	pcChar;
	int				i;

	for (i = 0; i < mcChars.NumElements(); i++)
	{
		pcChar = mcChars.Get(i);
		pcText->Append(pcChar->mcChar);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextCharLine::GetLastChar(void)
{
	return mcChars.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextCharLine::GetCharIndex(int x, BOOL bClamp)
{
	int			i;
	CTextChar*	pcChar;	

	if (IsEmpty())
	{
		return bClamp ? 0 : -1;
	}

	pcChar = mcChars.Get(0);
	if (x < pcChar->msTopLeft.x)
	{
		return bClamp ? 0 : -1;
	}

	for (i = 0; i < mcChars.NumElements(); i++)
	{
		pcChar = mcChars.Get(i);
		if ((x >= pcChar->msTopLeft.x) && (x <= pcChar->GetRight()))
		{
			return i;
		}
	}
	return bClamp ? mcChars.NumElements()-1 : mcChars.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextCharLine::GetChar(int iIndex)
{
	return mcChars.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextCharLine::GetLastCharIndex(void)
{
	return mcChars.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::IsLeftAllWhiteSpace(int x)
{
	int			iChar;
	int			i;
	CTextChar*	pcChar;

	iChar = GetCharIndex(x);

	if (IsOutOfBoundsRight(iChar))
	{
		iChar = GetLastCharIndex();
	}

	if (iChar > 0)
	{
		for (i = 0; i < iChar; i++)
		{
			pcChar = mcChars.Get(i);
			if (!pcChar->IsWhiteSpace())
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::IsRightAllWhiteSpace(int x)  
{
	int			iChar;
	int			i;
	CTextChar*	pcChar;
	int			iLastIndex;

	iChar = GetCharIndex(x);

	if (IsOutOfBoundsLeft(x))
	{
		iChar = 0;
	}

	iLastIndex = GetLastCharIndex();
	if (iChar <= iLastIndex)  //This will only work if GetCharIndex can return miUsedElemets when passed the end of the line.
	{
		for (i = iChar; i < iLastIndex; i++)
		{
			pcChar = mcChars.Get(i);
			if (!pcChar->IsWhiteSpace())
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextCharLine::GetFirstNonWhiteSpaceCharIndex(void)
{
	int			i;
	CTextChar*	pcChar;

	for (i = 0; i < mcChars.NumElements(); i++)
	{
		pcChar = GetChar(i);
		if (!pcChar->IsWhiteSpace())
		{
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextCharLine::GetLastNonWhiteSpaceCharIndex(void)
{
	int			i;
	CTextChar*	pcChar;

	for (i = mcChars.NumElements()-1; i >= 0; i--)
	{
		pcChar = GetChar(i);
		if (!pcChar->IsWhiteSpace())
		{
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::IsEmpty(void)
{
	return mcChars.NumElements() == 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::IsOutOfBoundsRight(int iIndex)
{
	return iIndex >= mcChars.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::IsOutOfBoundsLeft(int iIndex)
{
	return iIndex < 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::IsOutOfBounds(int iIndex)
{
	return IsOutOfBoundsLeft(iIndex) || IsOutOfBoundsRight(iIndex);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextCharLine::GetBounds(int iX, CRectangle* pcDest)
{
	CTextChar*	pcChar;

	pcChar = GetChar(iX);
	if (pcChar)
	{
		return pcChar->GetBounds(pcDest);
	}
	else
	{
		pcDest->miLeft = this->msBottomRight.x;
		pcDest->miTop = this->msTopLeft.y;
		pcDest->miRight = this->msBottomRight.x+1;
		pcDest->miBottom = this->msBottomRight.y;
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Init(CTextParameters* pcDefaultFont)
{
	CTextCharLine*	pcLine;

	mcLines.Init();
	mpcDefaultFont = pcDefaultFont;
	pcLine = mcLines.Add();
	pcLine->Init();
	Layout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Init(CTextParameters* pcDefaultFont, char* szText)
{
	int		iLen;

	Init(pcDefaultFont);
	iLen = (int)strlen(szText);
	Insert(0, 0, pcDefaultFont, szText, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Init(CTextParameters* pcDefaultFont, CChars* pcText)
{
	Init(pcDefaultFont);
	Insert(0, 0, pcDefaultFont, pcText->Text(), pcText->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Kill(void)
{
	int					i;
	CTextCharLine*	pcLine;

	for (i = 0; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		pcLine->Kill();
	}
	mcLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Reinit(void)
{
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Reinit(char* szText)
{
	int		iLen;

	Clear();
	iLen = (int)strlen(szText);
	Insert(0, 0, mpcDefaultFont, szText, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Reinit(CChars* pcText)
{
	Clear();
	Insert(0, 0, mpcDefaultFont, pcText->Text(), pcText->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Append(CChars* pcText)
{
	int					i;
	CTextCharLine*	pcLine;

	for (i = 0; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		pcLine->Append(pcText);
		pcText->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Layout(void)
{
	int					i;
	CTextCharLine*		pcLine;
	int					x;
	int					y;
	CTextParameters*	pcLastFont;

	pcLastFont = mpcDefaultFont;
	x = 0;
	y = 0;
	for (i = 0; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		pcLastFont = pcLine->Layout(x, y, pcLastFont);
		y += (pcLine->msBottomRight.y - pcLine->msTopLeft.y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Clear(void)
{
	Kill();
	Init(mpcDefaultFont);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::CreateQuads(CGraphicsObject* pcGraphicsObject, CViewportInstance* pcViewport, SInt2* psPosition, CMap2D* pcMap2D)
{
	int				i;
	CTextCharLine*	pcLine;

	for (i = 0; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		pcLine->CreateQuads(pcGraphicsObject, pcViewport, psPosition, pcMap2D);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::AddLines(int iNumLines)
{
	int					i;
	CTextCharLine*	pcLine;

	for (i = 0; i < iNumLines; i++)
	{
		pcLine = mcLines.Add();
		pcLine->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::GetRequiredSize(void)
{
	SInt2	sSize;

	sSize.Init(-1, -1);
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Remove(SInt2 sPos)
{
	SInt2	sNewPos;

	sNewPos.Init(-1, -1);
	return sNewPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Remove(SInt2 sPosStart, SInt2 sPosEnd)
{
	SInt2	sNewPos;

	sNewPos.Init(-1, -1);
	return sNewPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextData::Insert(int iColumn, int iLine, CTextParameters* pcFont, char* szText, int iLen)
{
	int					i;
	char				c;
	CTextCharLine*		pcLine;
	int					iLineStart;
	int					iLineEnd;
	CTextChar*			pcChar;

	if (iLine >= mcLines.NumElements())
	{
		AddLines((iLine - mcLines.NumElements()) + 1);
	}
	pcLine = mcLines.Get(iLine);

	iLineStart = 0;
	iLineEnd = 0;
	for (i = 0; i < iLen; i++)
	{
		c = szText[i];
		if (c == '\n')
		{
			iLineEnd = i;
			pcLine->Insert(iColumn, pcFont, &szText[iLineStart], iLineEnd - iLineStart);
			iLineStart = i + 1;
			iColumn = 0;
			iLine++;
			if (iLine >= mcLines.NumElements())
			{
				AddLines(1);
			}

			pcLine = mcLines.Get(iLine);
		}
	}
	iLineEnd = i;
	pcChar = pcLine->Insert(iColumn, pcFont, &szText[iLineStart], iLineEnd - iLineStart);
	return pcChar;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Insert(SInt2 sPos, char c)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;

	iLine = GetLineIndex(sPos.y);
	pcLine = GetLine(iLine);
	pcChar = pcLine->Insert(sPos.x, mpcDefaultFont, c);
	Layout();
	
	sPos.x = pcChar->GetRight()+1;
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Insert(SInt2 sPos, char* szString)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;
	int				iLen;

	iLine = GetLineIndex(sPos.y);
	pcLine = GetLine(iLine);
	iLen = (int)strlen(szString);
	pcChar = pcLine->Insert(sPos.x, mpcDefaultFont, szString, iLen);
	Layout();

	sPos.x = pcChar->GetRight()+1;
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::GetText(CChars* pcText)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::GetText(CChars* pcText, SInt2 sStart, SInt2 iEnd)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Split(SInt2 sPos)
{
	SInt2	sNewPos;

	sNewPos.Init(-1, -1);
	return sNewPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Join(SInt2 sPos)
{
	SInt2	sNewPos;

	sNewPos.Init(-1, -1);
	return sNewPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextData::GetLineIndex(int y)
{
	int				i;
	CTextCharLine*	pcLine;
	
	//Does not take X into account.
	for (i = 0 ; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		if ((y >= pcLine->msTopLeft.y) && (y <= pcLine->msBottomRight.y))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextCharLine* CTextData::GetLine(int iIndex)
{
	return mcLines.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextCharLine* CTextData::GetLastLine(void)
{
	return mcLines.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextData::GetLastLineIndex(void)
{
	return mcLines.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextData::GetChar(int iX, int iLine)
{
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;

	pcLine = mcLines.SafeGet(iLine);
	if (pcLine)
	{
		pcChar = pcLine->GetChar(iX);
		return pcChar;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextData::GetBounds(int iX, int iLine, CRectangle* pcDest)
{
	CTextCharLine*	pcLine;

	pcLine = mcLines.SafeGet(iLine);
	if (pcLine)
	{
		return pcLine->GetBounds(iX, pcDest);
	}
	return FALSE;
}

