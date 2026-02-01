/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "SupportLib/Font.h"
#include "TextData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextChar::Init(uint16 c, CFont* pcFont)
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
size CTextChar::GetWidth(void)
{
	if (mcChar == ' ')
	{
		return mpcFont->GetSpaceWidth();
	}
	else if (mcChar == '\t')
	{
		return mpcFont->GetTabSpaceCount() * mpcFont->GetSpaceWidth();
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
size CTextChar::GetHeight(void)
{
	CRectangle	cRect;

	GetBounds(&cRect);
	
	return cRect.GetHeight();
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
bool CTextChar::IsWhiteSpace(void)
{
	return ((mcChar == ' ') || (mcChar == '\t'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextChar::GetBounds(CRectangle* pcDest)
{
	CGlyph*		pcGlyph;

	pcGlyph = mpcFont->GetGlyph(mcChar);
	pcGlyph->GetFullDestBounds(msTopLeft.x, msTopLeft.y, pcDest);
	pcDest->miRight--;
	pcDest->miBottom--;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CTextChar::GetASCIIChar(void) { return (char)mcChar; }
int CTextChar::GetX(void) { return msTopLeft.x; }
int CTextChar::GetY(void) { return msTopLeft.y; }
int CTextChar::GetAscent(void) { return mpcFont->GetAscent(); }
int CTextChar::GetDescent(void) { return mpcFont->GetDescent(); }


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
void CTextCharLine::Init(CFont* pcFont, char* szText, int iLen)
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
void CTextCharLine::Layout(int x, int y)
{
	size				i;
	CTextChar*			pcChar;
	int					iMaxAscent;
	int					iMaxDescent;
	int					iCharX;
	size				uiNumChars;

	iMaxAscent = 0;
	iMaxDescent = 0;
	iCharX = x;

	uiNumChars = mcChars.NumElements();
	for (i = 0; i < uiNumChars; i++)
	{
		pcChar = mcChars.Get(i);
		if (pcChar->GetAscent() > iMaxAscent)
		{
			iMaxAscent = pcChar->GetAscent();
		}
		if (pcChar->GetDescent() > iMaxDescent)
		{
			iMaxDescent = pcChar->GetDescent();
		}
	}

	for (i = 0; i < uiNumChars; i++)
	{
		pcChar = mcChars.Get(i);
		pcChar->Layout(iCharX, y + iMaxAscent);
		iCharX += pcChar->GetWidth();

	}

	msTopLeft.Init(x, y);
	miBaseline = y + iMaxAscent;
	msBottomRight.x = iCharX - 1;
	if (msBottomRight.x < x)
	{
		msBottomRight.x = x;
	}
	msBottomRight.y = y + iMaxAscent + iMaxDescent;  //Er, possible off by one error.  Dunno.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextCharLine::Insert(int x, CFont* pcFont, char* szText, int iLen)
{
	CTextChar* pcNewArray;
	CTextChar* pcChar;
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
	return &pcNewArray[iLen - 1];
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextChar* CTextCharLine::Insert(int x, CFont* pcFont, char c)
{
	return Insert(x, pcFont, &c, 1);
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
int CTextCharLine::GetCharIndex(int x, bool bClamp)
{
	size		i;
	CTextChar* pcChar;

	if (IsEmpty())
	{
		return bClamp ? 0 : -1;
	}

	pcChar = mcChars.Get(0);
	if (x < pcChar->GetX())
	{
		return bClamp ? 0 : -1;
	}

	for (i = 0; i < mcChars.NumElements(); i++)
	{
		pcChar = mcChars.Get(i);
		if ((x >= pcChar->GetX()) && (x <= pcChar->GetRight()))
		{
			return i;
		}
	}
	return bClamp ? mcChars.NumElements() - 1 : mcChars.NumElements();
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
bool CTextCharLine::IsLeftAllWhiteSpace(int x)
{
	int			iChar;
	int			i;
	CTextChar* pcChar;

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
				return false;
			}
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextCharLine::IsRightAllWhiteSpace(int x)
{
	int			iChar;
	int			i;
	CTextChar* pcChar;
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
				return false;
			}
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextCharLine::GetFirstNonWhiteSpaceCharIndex(void)
{
	size		i;
	CTextChar* pcChar;

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
	CTextChar* pcChar;

	for (i = mcChars.NumElements() - 1; i >= 0; i--)
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
bool CTextCharLine::IsEmpty(void)
{
	return mcChars.NumElements() == 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextCharLine::IsOutOfBoundsRight(int iIndex)
{
	return iIndex >= (int)mcChars.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextCharLine::IsOutOfBoundsLeft(int iIndex)
{
	return iIndex < 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextCharLine::IsOutOfBounds(int iIndex)
{
	return IsOutOfBoundsLeft(iIndex) || IsOutOfBoundsRight(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextCharLine::GetBounds(int iX, CRectangle* pcDest)
{
	CTextChar* pcChar;

	pcChar = GetChar(iX);
	if (pcChar)
	{
		return pcChar->GetBounds(pcDest);
	}
	else
	{
		pcDest->miLeft = this->msBottomRight.x;
		pcDest->miTop = this->msTopLeft.y;
		pcDest->miRight = this->msBottomRight.x + 1;
		pcDest->miBottom = this->msBottomRight.y;
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextCharLine::GetTopLeftY(void) { return msTopLeft.y; }
int CTextCharLine::GetBottomRightY(void) { return msBottomRight.y; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Init(void)
{
	CTextCharLine* pcLine;

	mcLines.Init();
	pcLine = mcLines.Add();
	pcLine->Init();
	Layout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Init(CFont* pcFont, char* szText)
{
	int		iLen;

	Init();
	iLen = (int)strlen(szText);
	Insert(0, 0, pcFont, szText, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Init(CFont* pcFont, CChars* pcText)
{
	Init();
	Insert(0, 0, pcFont, pcText->Text(), pcText->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Kill(void)
{
	size			i;
	CTextCharLine* pcLine;

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
void CTextData::Reinit(CFont* pcFont, char* szText)
{
	int		iLen;

	Clear();
	iLen = (int)strlen(szText);
	Insert(0, 0, pcFont, szText, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Reinit(CFont* pcFont, CChars* pcText)
{
	Clear();
	Insert(0, 0, pcFont, pcText->Text(), pcText->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Layout(void)
{
	size				i;
	CTextCharLine*		pcLine;
	int					x;
	int					y;

	x = 0;
	y = 0;
	for (i = 0; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		pcLine->Layout(x, y);
		y += (pcLine->GetBottomRightY() - pcLine->GetTopLeftY());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::Clear(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextData::AddLines(int iNumLines)
{
	int					i;
	CTextCharLine* pcLine;

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
CTextChar* CTextData::Insert(int iColumn, size iLine, CFont* pcFont, char* szText, size iLen)
{
	size				i;
	char				c;
	CTextCharLine*		pcLine;
	size				iLineStart;
	size				iLineEnd;
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
SInt2 CTextData::Insert(SInt2 sPos, CFont* pcFont, char c)
{
	int				iLine;
	CTextCharLine* pcLine;
	CTextChar* pcChar;

	iLine = GetLineIndex(sPos.y);
	pcLine = GetLine(iLine);
	pcChar = pcLine->Insert(sPos.x, pcFont, c);
	Layout();

	sPos.x = pcChar->GetRight() + 1;
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CTextData::Insert(SInt2 sPos, CFont* pcFont, char* szString)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;
	int				iLen;

	iLine = GetLineIndex(sPos.y);
	pcLine = GetLine(iLine);
	iLen = (int)strlen(szString);
	pcChar = pcLine->Insert(sPos.x, pcFont, szString, iLen);
	Layout();

	sPos.x = pcChar->GetRight() + 1;
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
	size			i;
	CTextCharLine*	pcLine;

	//Does not take X into account.
	for (i = 0; i < mcLines.NumElements(); i++)
	{
		pcLine = mcLines.Get(i);
		if ((y >= pcLine->GetTopLeftY()) && (y <= pcLine->GetBottomRightY()))
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
	CTextCharLine* pcLine;
	CTextChar* pcChar;

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
bool CTextData::GetBounds(int iX, int iLine, CRectangle* pcDest)
{
	CTextCharLine* pcLine;

	pcLine = mcLines.SafeGet(iLine);
	if (pcLine)
	{
		return pcLine->GetBounds(iX, pcDest);
	}
	return false;
}

