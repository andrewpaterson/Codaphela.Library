#ifndef __TEXT_DATA_H__
#define __TEXT_DATA_H__
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
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Int2.h"
#include "SupportLib/Font.h"


class CTextChar
{
protected:
	SInt2				msTopLeft;
	uint16				mcChar;	 //Could be a tab or a space.
	CFont*				mpcFont;

public:
	void 				Init(uint16 c, CFont* pcFont);
	void				Layout(int x, int y);
	size				GetWidth(void);
	size				GetHeight(void);
	int					GetRight(void);
	bool				IsWhiteSpace(void);
	bool				GetBounds(CRectangle* pcDest);
	char				GetASCIIChar(void);
	int					GetX(void);
	int					GetY(void);
	int					GetAscent(void);
	int					GetDescent(void);
};


typedef CArrayTemplate<CTextChar>	CArrayTextChars;


class CTextCharLine
{
protected:
	CArrayTextChars		mcChars;
	SInt2				msTopLeft;
	int					miBaseline;
	SInt2				msBottomRight;  //Inclusive (ie: this line msBottomRight.y+1 == next line msBottomRight.y)

public:
	void 				Init(void);
	void 				Init(CFont* pcFont, char* szText, int iLen);
	void 				Kill(void);
	void				Layout(int x, int y);
	CTextChar*			Insert(int x, CFont* pcFont, char* szText, int iLen);
	CTextChar*			Insert(int x, CFont* pcFont, char c);

	CTextChar*			GetLastChar(void);
	int					GetCharIndex(int x, bool bClamp = false);
	CTextChar*			GetChar(int iIndex);
	int					GetLastCharIndex(void);
	bool				IsLeftAllWhiteSpace(int x);
	bool				IsRightAllWhiteSpace(int x);
	int					GetFirstNonWhiteSpaceCharIndex(void);
	int					GetLastNonWhiteSpaceCharIndex(void);
	bool				IsEmpty(void);
	bool 				IsOutOfBoundsRight(int iIndex);
	bool 				IsOutOfBoundsLeft(int iIndex);
	bool 				IsOutOfBounds(int iIndex);
	bool				GetBounds(int iX, CRectangle* pcDest);
	int					GetTopLeftY(void);
	int					GetBottomRightY(void);
};


typedef CArrayTemplate<CTextCharLine>	CArrayRenderCharLines;


class CTextData
{
protected:
	CArrayRenderCharLines	mcLines;

public:
	void 			Init(void);
	void 			Init(CFont* pcFont, char* szText);
	void 			Init(CFont* pcFont, CChars* pcText);
	void			Reinit(void);
	void			Reinit(CFont* pcFont, char* szText);
	void			Reinit(CFont* pcFont, CChars* pcText);
	void 			Kill(void);
	void 			Layout(void);
	void			Clear(void);
	void 			AddLines(int iNumLines);
	SInt2			GetRequiredSize(void);

	SInt2			Remove(SInt2 sPos);
	SInt2			Remove(SInt2 sPosStart, SInt2 sPosEnd);
	SInt2			Insert(SInt2 sPos, CFont* pcFont, char c);
	SInt2			Insert(SInt2 sPos, CFont* pcFont, char* szString);
	CTextChar*		Insert(int iColumn, size iLine, CFont* pcFont, char* szText, size iLen);
	void			GetText(CChars* pcText);
	void			GetText(CChars* pcText, SInt2 sStart, SInt2 iEnd);
	SInt2			Split(SInt2 sPos);
	SInt2			Join(SInt2 sPos);

	int				GetLineIndex(int y);
	CTextCharLine*	GetLine(int iIndex);
	CTextCharLine*	GetLastLine(void);
	int				GetLastLineIndex(void);
	CTextChar*		GetChar(int iX, int iLine);
	bool			GetBounds(int iX, int iLine, CRectangle* pcDest);
};


#endif // __TEXT_DATA_H__

