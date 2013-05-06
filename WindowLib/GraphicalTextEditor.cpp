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
#include "BaseLib/IntegerHelper.h"
#include "GraphicalTextEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Init(CTextData* pcText)
{
	mpcText = pcText;
	mbMultiLine = FALSE;
	mbInsert = TRUE;
	DocumentHome();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Kill(void)
{
	mpcText = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Left(void)
{
	msEditPos = FindLeft(msEditPos);
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Right(void)
{
	msEditPos = FindRight(msEditPos);
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::WordLeft(void)
{
	msEditPos = FindStartOfWord();
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::WordRight(void)
{
	msEditPos = FindEndOfWord();
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectLeft(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionEnd = msEditPos;
	}
	msEditPos = FindLeft(msEditPos);
	msSelectionStart = msEditPos;

	TouchSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectRight(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionStart = msEditPos;
	}
	msEditPos = FindRight(msEditPos);
	msSelectionEnd = msEditPos;

	TouchSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectWordLeft(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionEnd = msEditPos;
	}
	msEditPos = FindStartOfWord();
	msSelectionStart = msEditPos;

	TouchSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectWordRight(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionStart = msEditPos;
	}
	msEditPos = FindEndOfWord();
	msSelectionEnd = msEditPos;

	TouchSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Up(void)
{
	msEditPos = FindUp(msEditPos);
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Down(void)
{
	msEditPos = FindDown(msEditPos);
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectUp(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionEnd = msEditPos;
	}
	msEditPos = FindUp(msEditPos);
	msSelectionStart = msEditPos;

	TouchSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectDown(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionStart = msEditPos;
	}
	msEditPos = FindDown(msEditPos);
	msSelectionEnd = msEditPos;

	TouchSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Delete(void)
{
	if (msSelectionStart.x == -1)
	{
		mpcText->Remove(msEditPos);
	}
	else
	{
		mpcText->Remove(msSelectionStart, msSelectionEnd);
		msEditPos = msSelectionStart;
		ClearSelection();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Backspace(void)
{
	if (msSelectionStart.x == -1)
	{
		if (msEditPos.x > 0)
		{
			msEditPos = FindLeft(msEditPos);
			mpcText->Remove(msEditPos);
		}
		else
		{
			if (msEditPos.y > 0)
			{
				msEditPos = mpcText->Join(msEditPos);
			}
		}
	}
	else
	{
		mpcText->Remove(msSelectionStart, msSelectionEnd);
		msEditPos = msSelectionStart;
		ClearSelection();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Enter(void)
{
	if (mbMultiLine)
	{
		msEditPos = mpcText->Split(msEditPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Printable(char c, BOOL bInsert)
{
	msEditPos = mpcText->Insert(msEditPos, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Space(BOOL bInsert)
{
	msEditPos = mpcText->Insert(msEditPos, ' ');
	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Tab(BOOL bInsert)
{
	msEditPos = mpcText->Insert(msEditPos, '\t');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::BackTab(void)
{
	//If whitespace exists at start of line then remove a tabs worth from it.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectHome(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectEnd(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectDocumentHome(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectDocumentEnd(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectPageUp(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectPageDown(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectAll(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::HomeEnter(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Home(void)
{
	int				iLine;
	CTextCharLine*	pcLine;

	iLine = mpcText->GetLineIndex(msEditPos.y);
	pcLine = mpcText->GetLine(iLine);
	msEditPos = FindStartOfLine(msEditPos, !pcLine->IsLeftAllWhiteSpace(msEditPos.x));
	ClearSelection();
}
  

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::End(void)
{
	int				iLine;
	CTextCharLine*	pcLine;

	iLine = mpcText->GetLineIndex(msEditPos.y);
	pcLine = mpcText->GetLine(iLine);
	msEditPos = FindEndOfLine(msEditPos, pcLine->IsRightAllWhiteSpace(msEditPos.x));
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::DocumentHome(void)
{
	msEditPos.Init(0, 0);
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::DocumentEnd(void)
{
	CTextCharLine*	pcLine;

	pcLine = mpcText->GetLastLine();
	msEditPos.y = pcLine->miBaseline;
	msEditPos = FindEndOfLine(msEditPos, FALSE);
	ClearSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Cut(CChars* pszDest)
{
	pszDest->Init();
	if (msSelectionStart.x == -1)
	{
		CutLine(pszDest);
	}
	else
	{
		mpcText->GetText(pszDest, msSelectionStart, msSelectionEnd);
		DeleteSubstring(msSelectionStart, msSelectionEnd);
		msEditPos = msSelectionStart;
		ClearSelection();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Copy(CChars* pszDest)
{
	SInt2	sStart;
	SInt2	sEnd;

	pszDest->Init();
	if (msSelectionStart.x == -1)
	{
		sStart = FindStartOfLine(msEditPos, FALSE);
		sEnd = FindEndOfLine(msEditPos, FALSE);
		mpcText->GetText(pszDest, sStart, sEnd);
	}
	else
	{
		mpcText->GetText(pszDest, msSelectionStart, msSelectionEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Paste(char* szSource)
{
	if (msSelectionStart.x == -1)
	{
		msEditPos = mpcText->Insert(msEditPos, szSource);
	}
	else
	{
		DeleteSubstring(msSelectionStart, msSelectionEnd);
		msEditPos = msSelectionStart;
		msEditPos = mpcText->Insert(msEditPos, szSource);
		ClearSelection();
	}

	////Move this out of this class please.
	//HGLOBAL   hglb; 
	//LPTSTR    lptstr; 

	//if (!IsClipboardFormatAvailable(CF_TEXT)) 
	//	return; 

	//if (!OpenClipboard(hWnd)) 
	//	return; 

	//hglb = GetClipboardData(CF_TEXT); 
	//if (hglb != NULL) 
	//{ 
	//	lptstr = GlobalLock(hglb); 
	//	if (lptstr != NULL) 
	//	{ 
	//		ReplaceSelection(hwndSelected, pbox, lptstr); 
	//		GlobalUnlock(hglb); 
	//	} 
	//} 
	//CloseClipboard(); 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::CutLine(CChars* pszDest)
{
	SInt2	sStart;
	SInt2	sEnd;

	sStart = FindStartOfLine(msEditPos, FALSE);
	sEnd = FindEndOfLine(msEditPos, FALSE);

	mpcText->GetText(pszDest, sStart, sEnd);
	msEditPos = DeleteSubstring(sStart, sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::Duplicate(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::DeleteLine(void)
{
	SInt2	sStart;
	SInt2	sEnd;

	sStart = FindStartOfLine(msEditPos, FALSE);
	sEnd = FindEndOfLine(msEditPos, FALSE);
	msEditPos = DeleteSubstring(sStart, sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::DeleteWordRemainingRight(void)
{
	SInt2	sEnd;

	sEnd = FindEndOfWord();
	DeleteSubstring(msEditPos, sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::DeleteWordRemainingLeft(void)
{
	SInt2	sEnd;

	sEnd = FindStartOfWord();
	DeleteSubstring(msEditPos, sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::EndEnter(void)
{
	End();
	Enter();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::SelectWholeWord(void)
{
	if (msSelectionStart.x == -1)
	{
		msSelectionStart = FindStartOfWord();
		msSelectionEnd = FindEndOfWord();
		msEditPos = msSelectionEnd;
	}
	else
	{
		
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::PageUp(void)
{
	if (mbMultiLine)
	{
		
	}
	else
	{
		Home();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::PageDown(void)
{
	if (mbMultiLine)
	{

	}
	else
	{
		End();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::ClearSelection(void)
{
	msSelectionStart.Init(-1, -1);
	msSelectionEnd.Init(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindLeft(SInt2 sPos)
{
	int				iLine;
	CTextCharLine*	pcLine;
	int				iChar;
	CTextChar*		pcChar;

	iLine = mpcText->GetLineIndex(sPos.y);
	pcLine = mpcText->GetLine(iLine);
	iChar = pcLine->GetCharIndex(sPos.x);
	if (iChar > 0)
	{
		pcChar = pcLine->GetChar(iChar-1);
		sPos.x = pcChar->msTopLeft.x;
		return sPos;
	}
	else if (iLine > 0)
	{
		pcLine = mpcText->GetLine(iLine-1);
		pcChar = pcLine->GetLastChar();
		sPos.x = pcChar->msTopLeft.x + pcChar->GetWidth();
		sPos.y = pcLine->miBaseline;
		return sPos;
	}
	else
	{
		return sPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindRight(SInt2 sPos)
{
	int				iLine;
	CTextCharLine*	pcLine;
	int				iChar;
	CTextChar*		pcChar;

	iLine = mpcText->GetLineIndex(sPos.y);
	pcLine = mpcText->GetLine(iLine);
	iChar = pcLine->GetCharIndex(sPos.x);
	if (iChar < pcLine->GetLastCharIndex())
	{
		pcChar = pcLine->GetChar(iChar+1);
		sPos.x = pcChar->msTopLeft.x;
		return sPos;
	}
	else if (iChar == pcLine->GetLastCharIndex())
	{
		pcChar = pcLine->GetChar(iChar);
		sPos.x = pcChar->msTopLeft.x + pcChar->GetWidth() + 1;
		return sPos;
	}
	else if (iLine < mpcText->GetLastLineIndex())
	{
		pcLine = mpcText->GetLine(iLine+1);
		pcChar = pcLine->GetChar(0);
		sPos.x = pcChar->msTopLeft.x;
		sPos.y = pcLine->miBaseline;
		return sPos;
	}
	else
	{
		return sPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicalTextEditor::TouchSelection(void)
{
	if (msSelectionStart == msSelectionEnd)
	{
		ClearSelection();
	}
	else if (msSelectionStart.y > msSelectionEnd.y)
	{
		Swap(&msSelectionStart.y, &msSelectionEnd.y);
		Swap(&msSelectionStart.x, &msSelectionEnd.x);
	}
	if (msSelectionStart.y == msSelectionEnd.y)
	{
		if (msSelectionStart.x > msSelectionEnd.x)
		{
			Swap(&msSelectionStart.x, &msSelectionEnd.x);
		}	
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindUp(SInt2 sPos)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;
	int				iChar;

	iLine = mpcText->GetLineIndex(sPos.y);
	if (iLine > 0)
	{
		pcLine = mpcText->GetLine(iLine-1);
		sPos.y = pcLine->miBaseline;
		iChar = pcLine->GetCharIndex(miCenterLineX, TRUE);
		pcChar = pcLine->GetChar(iChar);
		if (pcChar)
		{
			sPos.x = pcChar->msTopLeft.x;
		}
	}
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindDown(SInt2 sPos)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;
	int				iChar;

	iLine = mpcText->GetLineIndex(sPos.y);
	if (iLine < mpcText->GetLastLineIndex())
	{
		pcLine = mpcText->GetLine(iLine+1);
		sPos.y = pcLine->miBaseline;
		iChar = pcLine->GetCharIndex(miCenterLineX, TRUE);
		pcChar = pcLine->GetChar(iChar);
		if (pcChar)
		{
			sPos.x = pcChar->msTopLeft.x;
		}
	}
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindStartOfLine(SInt2 sPos, BOOL bSkipWhiteSpace)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;
	int				iChar;

    iLine = mpcText->GetLineIndex(sPos.y);
	pcLine = mpcText->GetLine(iLine);
	if (bSkipWhiteSpace)
	{
		//This skips over initial whitespace.
		iChar = pcLine->GetFirstNonWhiteSpaceCharIndex();
		pcChar = pcLine->GetChar(iChar);
	}
	else
	{
		pcChar = pcLine->GetChar(0);
	}
	sPos.x = pcChar->msTopLeft.x;
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindEndOfLine(SInt2 sPos, BOOL bSkipWhiteSpace)
{
	int				iLine;
	CTextCharLine*	pcLine;
	CTextChar*		pcChar;
	int				iChar;

	iLine = mpcText->GetLineIndex(sPos.y);
	pcLine = mpcText->GetLine(iLine);
	if (bSkipWhiteSpace)
	{
		//This skips *backwards* over remaining whitespace starting at the end of the line.
		iChar = pcLine->GetLastNonWhiteSpaceCharIndex();
		pcChar = pcLine->GetChar(iChar);
	}
	else
	{
		pcChar = pcLine->GetLastChar();
	}
	pcChar = pcLine->GetLastChar();
	sPos.x = pcChar->msTopLeft.x + pcChar->GetWidth();
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindEndOfWord(void)
{
	SInt2	sPos;

	sPos.Init(-1, -1);
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::FindStartOfWord(void)
{
	SInt2	sPos;

	sPos.Init(-1, -1);
	return sPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CGraphicalTextEditor::DeleteSubstring(SInt2 sStartInclusive, SInt2 sEndExclusive)
{
	return mpcText->Remove(sStartInclusive, sEndExclusive);
}

