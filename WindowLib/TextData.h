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
#ifndef __TEXT_DATA_H__
#define __TEXT_DATA_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "TextParameters.h"


class CTextChar
{
public:
	SInt2				msTopLeft;  
	CTextParameters*	mpcFont;
	char				mcChar;	 //Could be a tab or a space.


	void 				Init(char c, CTextParameters* pcFont);
	void				Layout(int x, int y);
	void				CreateQuad(CGraphicsObject* pcGraphicsObject, CGraphicsViewport* pcViewport, SInt2* psPosition, CMap2D* pcMap2D);
	void				CreateQuad(char c, CGraphicsObject* pcGraphicsObject, CGraphicsViewport* pcViewport, SInt2* psPosition, CMap2D* pcMap2D);
	int					GetWidth(void);
	int					GetRight(void);
	bool				IsWhiteSpace(void);
	bool				GetBounds(CRectangle* pcDest);
};


typedef CArrayTemplate<CTextChar>	CArrayRenderChars;


class CTextCharLine
{
public:
	CArrayRenderChars	mcChars;
	SInt2				msTopLeft;
	int					miBaseline;
	SInt2				msBottomRight;  //Inclusive (ie: this line msBottomRight.y+1 == next line msBottomRight.y)

	void 				Init(void);
	void 				Init(CTextParameters* pcFont, char* szText, int iLen);
	void 				Kill(void);
	CTextParameters*	Layout(int x, int y, CTextParameters* pcLastFont);
	void				CreateQuads(CGraphicsObject* pcGraphicsObject, CGraphicsViewport* pcViewport, SInt2* psPosition, CMap2D* pcMap2D);
	CTextChar* 			Insert(int x, CTextParameters* pcFont, char* szText, int iLen);
	CTextChar*			Insert(int x, CTextParameters* pcFont, char c);
	void 				Append(CChars* pcText);

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
};


typedef CArrayTemplate<CTextCharLine>	CArrayRenderCharLines;


class CTextData
{
public:
	CArrayRenderCharLines	mcLines;
	CTextParameters*		mpcDefaultFont;

	void 			Init(CTextParameters* pcDefaultFont);
	void 			Init(CTextParameters* pcDefaultFont, char* szText);
	void 			Init(CTextParameters* pcDefaultFont, CChars* pcText);
	void			Reinit(void);
	void			Reinit(char* szText);
	void			Reinit(CChars* pcText);
	void 			Kill(void);
	void 			Layout(void);
	void			Clear(void);
	void 			CreateQuads(CGraphicsObject* pcGraphicsObject, CGraphicsViewport* pcViewport, SInt2* psPosition, CMap2D* pcMap2D);
	void 			AddLines(int iNumLines);
	SInt2			GetRequiredSize(void);

	SInt2			Remove(SInt2 sPos);
	SInt2			Remove(SInt2 sPosStart, SInt2 sPosEnd);
	SInt2			Insert(SInt2 sPos, char c);
	SInt2			Insert(SInt2 sPos, char* szString);
	void 			Append(CChars* pcText);
	CTextChar*  	Insert(int iColumn, size iLine, CTextParameters* pcFont, char* szText, size iLen);
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

