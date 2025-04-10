/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "PlainTextEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Init(void)
{
	Init("");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Init(char* szText)
{
	mszText.Init(szText);
	mszText.PassifyNewlines();
	mbMultiLine = true;
	miEditPos = 0;
	miPageLength = 25;

	miSelectionAnchor = SIZE_MAX;
	miSelectionFloating = SIZE_MAX;
	miTabWidth = 4;
	miUpDownCharCount = SIZE_MAX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Kill(void)
{
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Left(void)
{
	ClearSelection();
	miEditPos = FindLeft(miEditPos);
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Right(void)
{
	ClearSelection();
	miEditPos = FindRight(miEditPos);
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Up(void)
{
	ClearSelection();
	UpdateUpDown(miEditPos);
	miEditPos = FindUp(miEditPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Down(void)
{
	ClearSelection();
	UpdateUpDown(miEditPos);
	miEditPos = FindDown(miEditPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Home(void)
{
	ClearSelection();
	miEditPos = FindHome(miEditPos);
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::End(void)
{
	ClearSelection();
	miEditPos = FindEnd(miEditPos);
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::DocumentHome(void)
{
	ClearSelection();
	miEditPos = FindDocumentHome();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::DocumentEnd(void)
{
	ClearSelection();
	miEditPos = FindDocumentEnd();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::PageUp(void)
{
	ClearSelection();
	UpdateUpDown(miEditPos);
	miEditPos = FindPageUp(miEditPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::PageDown(void)
{
	ClearSelection();
	UpdateUpDown(miEditPos);
	miEditPos = FindPageDown(miEditPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::WordLeft(void)
{
	ClearSelection();
	miEditPos = FindWordLeft(miEditPos);
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::WordRight(void)
{
	ClearSelection();
	miEditPos = FindWordRight(miEditPos);
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectLeft(void)
{
	BeginSelection();
	miEditPos = FindLeft(miEditPos);
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectRight(void)
{
	BeginSelection();
	miEditPos = FindRight(miEditPos);
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectWordLeft(void)
{
	BeginSelection();
	miEditPos = FindWordLeft(miEditPos);
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectWordRight(void)
{
	BeginSelection();
	miEditPos = FindWordRight(miEditPos);
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectUp(void)
{
	UpdateUpDown(miEditPos);
	BeginSelection();
	miEditPos = FindUp(miEditPos);
	EndSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectDown(void)
{
	UpdateUpDown(miEditPos);
	BeginSelection();
	miEditPos = FindDown(miEditPos);
	EndSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectHome(void)
{
	BeginSelection();
	miEditPos = FindHome(miEditPos);
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectEnd(void)
{
	BeginSelection();
	miEditPos = FindEnd(miEditPos);
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectDocumentHome(void)
{
	BeginSelection();
	miEditPos = FindDocumentHome();
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectDocumentEnd(void)
{
	BeginSelection();
	miEditPos = FindDocumentEnd();
	EndSelection();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectPageUp(void)
{
	UpdateUpDown(miEditPos);
	BeginSelection();
	miEditPos = FindPageUp(miEditPos);
	EndSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectPageDown(void)
{
	UpdateUpDown(miEditPos);
	BeginSelection();
	miEditPos = FindPageDown(miEditPos);
	EndSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectWholeWord(void)
{
	SelectWordLeft();
	miSelectionAnchor = miSelectionFloating;
	SelectWordRight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Delete(void)
{
	if (!IsSelected())
	{
		if (CanGoRight(miEditPos))
		{
			mszText.RemoveCharacter(miEditPos);
			ResetUpDown();
		}
	}
	else
	{
		miEditPos = DeleteSelection();
		ResetUpDown();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Backspace(void)
{
	if (!IsSelected())
	{
		if (CanGoLeft(miEditPos))
		{
			mszText.RemoveCharacter(miEditPos - 1);
			Left();
		}
	}
	else
	{
		miEditPos = DeleteSelection();
		ResetUpDown();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Enter(void)
{
	char	szTemp[2];

	if (!IsSelected())
	{
		mszText.Insert(miEditPos, '\n');
		Right();
	}
	else
	{
		szTemp[0] = '\n';
		szTemp[1] = 0;
		OverwriteSelection(szTemp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Printable(char c)
{
	Printable(c, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Printable(char c, bool bInsert)
{
	char	cCurrent;
	size		iCharCount;
	char	szTemp[2];

	if (!IsSelected())
	{
		if (bInsert)
		{
			mszText.Insert(miEditPos, c);
		}
		else
		{
			cCurrent = mszText.GetChar(miEditPos);
			if ((miEditPos == mszText.Length()) || (cCurrent == '\n'))
			{
				mszText.Insert(miEditPos, c);
			}
			else if (cCurrent == '\t')
			{
				iCharCount = GetCharCountAlongLine(miEditPos);
				if (iCharCount % miTabWidth == miTabWidth - 1)
				{
					mszText.SetChar(miEditPos, c);
				}
				else
				{
					mszText.Insert(miEditPos, c);
				}
			}
			else
			{
				mszText.SetChar(miEditPos, c);
			}
		}
		Right();
	}
	else
	{
		szTemp[0] = c;
		szTemp[1] = 0;
		OverwriteSelection(szTemp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Cut(CChars* pszDest)
{
	if (IsSelected())
	{
		Copy(pszDest);
		miEditPos = DeleteSelection();
		ResetUpDown();
	}
	else
	{
		CutLine(pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Copy(CChars* pszDest)
{
	size	iStart;
	size	iEnd;

	if (IsSelected())
	{
		if (miSelectionAnchor < miSelectionFloating)
		{
			iStart = miSelectionAnchor;
			iEnd = miSelectionFloating;
		}
		else
		{
			iStart = miSelectionFloating;
			iEnd = miSelectionAnchor;
		}
		pszDest->AppendSubString(mszText, iStart, iEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Paste(char* szSource)
{
	CChars	szTemp;

	if (!IsSelected())
	{
		szTemp.Init(szSource);
		szTemp.PassifyNewlines();

		mszText.Insert(miEditPos, &szTemp);
		miEditPos += szTemp.Length();
		szTemp.Kill();

		ResetUpDown();
	}
	else
	{
		OverwriteSelection(szSource);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::CutLine(CChars* pszDest)
{
	size		iCount;
	size		iStart;
	size		iEnd;

	if (!IsSelected())
	{
		iCount = GetCharCountAlongLine(miEditPos);
		iStart = FindStartOfLine(miEditPos);
		iEnd = FindEndOfLine(miEditPos);

		if (CanGoRight(iEnd))
		{
			iEnd++;
		}

		pszDest->AppendSubString(mszText, iStart, iEnd);
		mszText.Remove(iStart, iEnd);
		ResetUpDown();

		miEditPos = FindPositionAlongLine(iStart, iCount);
	}
	else
	{
		Cut(pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Duplicate(void)
{
	size	iStart;
	size	iEnd;
	CChars	szTemp;
	bool	bNewLine;

	if (!IsSelected())
	{
		iStart = FindStartOfLine(miEditPos);
		iEnd = FindEndOfLine(miEditPos);
		bNewLine = true;
	}
	else
	{
		if (miSelectionAnchor < miSelectionFloating)
		{
			iStart = miSelectionAnchor;
			iEnd = miSelectionFloating;
		}
		else
		{
			iStart = miSelectionFloating;
			iEnd = miSelectionAnchor;
		}
		bNewLine = false;
	}

	szTemp.Init();
	szTemp.AppendSubString(mszText, iStart, iEnd);
	if (bNewLine)
	{
		szTemp.AppendNewLine();
	}
	mszText.Insert(iStart, &szTemp);
	szTemp.Kill();
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::DeleteWordRemainingRight(void)
{
	size	iEnd;

	iEnd = FindWordRight(miEditPos);
	if (miEditPos != iEnd)
	{
		ClearSelection();
		mszText.Remove(miEditPos, iEnd);
	}
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::DeleteWordRemainingLeft(void)
{
	size	iEnd;

	iEnd = FindWordLeft(miEditPos);
	if (miEditPos != iEnd)
	{
		ClearSelection();
		mszText.Remove(miEditPos, iEnd);
	}
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::DeleteLine(void)
{
	size	iCount;
	size	iStart;
	size	iEnd;

	iCount = GetCharCountAlongLine(miEditPos);
	if (!IsSelected())
	{
		iStart = FindStartOfLine(miEditPos);
		iEnd = FindEndOfLine(miEditPos);
	}
	else
	{
		if (miSelectionAnchor < miSelectionFloating)
		{
			iStart = miSelectionAnchor;
			iEnd = miSelectionFloating;
		}
		else
		{
			iStart = miSelectionFloating;
			iEnd = miSelectionAnchor;
		}

		iStart = FindStartOfLine(iStart);
		iEnd = FindEndOfLine(iEnd);
	}

	if (CanGoRight(iEnd))
	{
		iEnd++;
	}

	mszText.Remove(iStart, iEnd);
	ResetUpDown();

	miEditPos = FindPositionAlongLine(iStart, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::HomeEnter(void)
{
	Home();
	Enter();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::EndEnter(void)
{
	End();
	Enter();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Space(void)
{
	Space(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Space(bool bInsert)
{
	Printable(' ', bInsert);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Tab(void)
{
	Tab(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::Tab(bool bInsert)
{
	char	szTemp[2];

	if (!IsSelected())
	{
		mszText.Insert(miEditPos, '\t');
		Right();
	}
	else
	{
		szTemp[0] = '\t';
		szTemp[1] = 0;
		OverwriteSelection(szTemp);
	}
}	


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::BackTab(void)
{
	//If whitespace exists at start of line then remove a tabs worth from it.
}	


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindLeft(size iCurrentPos)
{
	if (CanGoLeft(iCurrentPos))
	{
		iCurrentPos--;
	}
	return iCurrentPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindRight(size iCurrentPos)
{
	if (CanGoRight(iCurrentPos))
	{
		iCurrentPos++;
	}
	return iCurrentPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindUp(size iCurrentPos)
{
	size	iStartOfThisLine;
	size	iStartOfPrevLine;

	iStartOfThisLine = FindStartOfLine(iCurrentPos);
	if (iStartOfThisLine == 0)
	{
		return 0;
	}
	else
	{
		iStartOfPrevLine = FindStartOfLine(iStartOfThisLine - 1);
		return FindPositionAlongLine(iStartOfPrevLine, miUpDownCharCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindDown(size iCurrentPos)
{
	size	iEndOfThisLine;
	size	iStartOfNextLine;

	iEndOfThisLine = FindEndOfLine(iCurrentPos);
	if (iEndOfThisLine == mszText.Length())
	{
		return iEndOfThisLine;
	}
	else
	{
		iStartOfNextLine = iEndOfThisLine+1;
		return FindPositionAlongLine(iStartOfNextLine, miUpDownCharCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindHome(size iCurrentPos)
{
	return FindStartOfLine(iCurrentPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindEnd(size iCurrentPos)
{
	return FindEndOfLine(iCurrentPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindDocumentHome(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindDocumentEnd(void)
{
	return mszText.Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindPageUp(size iCurrentPos)
{
	size	i;
	size	iLineStart;

	iLineStart = FindStartOfLine(iCurrentPos);
	if (iLineStart == 0)
	{
		return iCurrentPos;
	}

	for (i = 0; i < miPageLength; i++)
	{
		iLineStart = FindStartOfLine(iLineStart - 1);
		if (iLineStart == 0)
		{
			break;
		}
	}
	return FindPositionAlongLine(iLineStart, miUpDownCharCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindPageDown(size iCurrentPos)
{
	size	iLineEnd;
	size	i;
	size	iLineStart;

	iLineEnd = FindEndOfLine(iCurrentPos);
	if (iLineEnd == mszText.Length())
	{
		return iCurrentPos;
	}

	for (i = 0; i < miPageLength; i++)
	{
		iLineEnd = FindEndOfLine(iLineEnd+1);
		if (iLineEnd == mszText.Length())
		{
			break;
		}
	}
	iLineStart = FindStartOfLine(iLineEnd);
	return FindPositionAlongLine(iLineStart, miUpDownCharCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindWordLeft(size iPos)
{
	char c;

	if (!CanGoLeft(iPos))
	{
		return iPos;
	}

	iPos--;
	c = mszText.GetChar(iPos);
	if (IsWhitespace(c))
	{
		for (;;)
		{
			if (CanGoLeft(iPos))
			{
				iPos--;
				c = mszText.GetChar(iPos);
				if (!IsWhitespace(c))
				{
					return iPos+1;
				}
			}
			else
			{
				return iPos;
			}
		}
	}
	else if (IsNormalChar(c))
	{
		for (;;)
		{
			if (CanGoLeft(iPos))
			{
				iPos--;
				c = mszText.GetChar(iPos);
				if (!IsNormalChar(c))
				{
					return iPos+1;
				}
			}
			else
			{
				return iPos;
			}
		}
	}
	else
	{
		return iPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPlainTextEditor::CanGoLeft(size iPos)
{
	return (iPos > 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPlainTextEditor::CanGoRight(size iPos)
{
	return (iPos < mszText.Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPlainTextEditor::IsWhitespace(char cCurrent)
{
	return ((cCurrent == ' ') || (cCurrent == '\n') || (cCurrent == '\t'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPlainTextEditor::IsNormalChar(char cCurrent)
{
	return (((cCurrent >= 'a') && (cCurrent <= 'z')) || 
			((cCurrent >= 'A') && (cCurrent <= 'Z')) ||
			((cCurrent >= '0') && (cCurrent <= '9')) ||
			 (cCurrent == '_'));
}	


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindWordRight(size iPos)
{
	char c;

	c = mszText.GetChar(iPos);
	if (IsWhitespace(c))
	{
		for (;;)
		{
			if (CanGoRight(iPos))
			{
				iPos++;
				c = mszText.GetChar(iPos);
				if (!IsWhitespace(c))
				{
					return iPos;
				}
			}
			else
			{
				return iPos;
			}
		}
	}
	else if (IsNormalChar(c))
	{
		for (;;)
		{
			if (CanGoRight(iPos))
			{
				iPos++;
				c = mszText.GetChar(iPos);
				if (!IsNormalChar(c))
				{
					return iPos;
				}
			}
			else
			{
				return iPos;
			}
		}
	}
	else
	{
		if (CanGoRight(iPos))
		{
			iPos++;
		}
		return iPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPlainTextEditor::IsSelected(void)
{
	if ((miSelectionAnchor == SIZE_MAX) && (miSelectionFloating == SIZE_MAX))
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPlainTextEditor::Text(void)
{
	return mszText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::Length(void)
{
	return mszText.Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SetPageLength(size iPageLength)
{
	miPageLength = iPageLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SetPosition(size iPosition)
{
	ClearSelection();
	ResetUpDown();
	miEditPos = iPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindStartOfLine(size iPos)
{
	char	c;

	if (iPos == 0)
	{
		return 0;
	}

	c = mszText.GetChar(iPos);
	iPos--;
	for (;;)
	{
		c = mszText.GetChar(iPos);
		if (iPos == 0)
		{
			return iPos;
		}
		else if (c == '\n')
		{
			return iPos+1;
		}
		iPos--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindEndOfLine(size iPos)
{
	char	c;

	if (iPos == mszText.Length())
	{
		return iPos;
	}

	c = mszText.GetChar(iPos);
	for (;;)
	{
		c = mszText.GetChar(iPos);
		if (iPos == mszText.Length())
		{
			return iPos;
		}
		else if (c == '\n')
		{
			return iPos;
		}
		iPos++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::GetCharCountAlongLine(size iPos)
{
	size		iStartOfLine;
	size		i;
	char	c;
	size		iCount;

	iStartOfLine = FindStartOfLine(iPos);
	if (iStartOfLine == iPos)
	{
		return 0;
	}

	iCount = 0;
	for (i = iStartOfLine; i < iPos; i++)
	{
		c = mszText.GetChar(i);
		iCount = CountChar(iCount, c);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CPlainTextEditor::GetEditPosition(void)
{
	return GetPosition(miEditPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CPlainTextEditor::GetPosition(size iPosition)
{
	char	c;
	SInt2	sPos;
	size	i;

	sPos.x = 0;
	sPos.y = 0;

	for (i = 0; i < iPosition; i++)
	{
		c = mszText.GetChar(i);
		if (c == '\0')
		{
			break;
		}
		else if (c == '\n')
		{
			sPos.y++;
			sPos.x = 0;
		}
		else
		{
			sPos.x++;
		}
	}
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::FindPositionAlongLine(size iStartOfLine, size iCharCount)
{
	size	iPos;
	char	c;
	size	iCountedSoFar;

	if (iCharCount == 0)
	{
		return iStartOfLine;
	}

	iPos = iStartOfLine;
	iCountedSoFar = 0;
	for (;;)
	{
		c = mszText.GetChar(iPos);
		if ((c == '\n') || (c == '\0'))
		{
			return iPos;
		}
		else if (iCountedSoFar == iCharCount)
		{
			return iPos;
		}
		else if (iCountedSoFar > iCharCount)
		{
			return iPos - 1;
		}

		iCountedSoFar = CountChar(iCountedSoFar, c);
		iPos++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::CountChar(size iCharCount, char c)
{
	size	iTabMod;

	if (c == '\t')
	{
		iTabMod = iCharCount % miTabWidth;
		if (iTabMod == 0)
		{
			iTabMod = 4;
		}
		else
		{
			iTabMod = 4 - iTabMod;
		}
		iCharCount += iTabMod;
	}
	else
	{
		iCharCount++;
	}
	return iCharCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::ResetUpDown(void)
{
	miUpDownCharCount = SIZE_MAX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::UpdateUpDown(size iPos)
{
	if (miUpDownCharCount == SIZE_MAX)
	{
		miUpDownCharCount = GetCharCountAlongLine(iPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::SelectAll(void)
{
	miSelectionAnchor = FindDocumentHome();
	miEditPos = FindDocumentEnd();
	miSelectionFloating = miEditPos;
	ResetUpDown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::ClearSelection(void)
{
	miSelectionAnchor = SIZE_MAX;
	miSelectionFloating = SIZE_MAX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::FixSelection(void)
{
	if (IsSelected())
	{
		if (miSelectionFloating == miSelectionAnchor)
		{
			ClearSelection();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::BeginSelection(void)
{
	if (!IsSelected())
	{
		miSelectionAnchor = miEditPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::EndSelection(void)
{
	miSelectionFloating = miEditPos;
	FixSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPlainTextEditor::DeleteSelection(void)
{
	size	iStart;
	size	iEnd;

	if (IsSelected())
	{
		if (miSelectionAnchor < miSelectionFloating)
		{
			iStart = miSelectionAnchor;
			iEnd = miSelectionFloating;
		}
		else
		{
			iStart = miSelectionFloating;
			iEnd = miSelectionAnchor;
		}
		mszText.Remove(iStart, iEnd);
		ClearSelection();
		return iStart;
	}
	else
	{
		return miEditPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlainTextEditor::OverwriteSelection(char* szText)
{
	miEditPos = DeleteSelection();
	Paste(szText);
}

