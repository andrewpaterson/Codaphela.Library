/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/DiskFile.h"
#include "BaseLib/FNVHash.h"
#include "BaseLib/Logger.h"
#include "WinRefWindow.h"
#include "WinRefScreen.h"


//////////////////////////////////////////////////////////////////////////
//
// Not watching: windows present nothing to it until it checks or hashes.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Init(void)
{
	mcArea.Init(0, 0, 0, 0);
	mapcWindows.Init();
	mcPixels.Init();
	mbCheck = false;
	muiMismatches = 0;
	mbHash = false;
	meHashArea = WRHA_Presented;
	muiPresents = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Kill(void)
{
	CloseHash();
	mcPixels.Kill();
	mapcWindows.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::SetArea(CInt32Rect* pcArea)
{
	mcArea.Init(pcArea);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the screen's size isn't known.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefScreen::GetArea(CInt32Rect* pcDest)
{
	pcDest->Init(&mcArea);
	return (mcArea.GetWidth() > 0) && (mcArea.GetHeight() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Each present is checked: what the window shows after it must be what
// its canvas holds.  One that isn't is logged ("A present of") and
// counted, and the window shows its canvas again so the next is checked
// on its own.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::SetCheck(bool bCheck)
{
	mbCheck = bCheck;
}


//////////////////////////////////////////////////////////////////////////
//
// Each present writes a line to szFileName: its number, its window's
// title and a hash of what eArea covers.  The file is flushed every line
// so it is whole however the program ends.  NULL stops hashing.  A
// screen hash needs the screen's area.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefScreen::SetHash(const char* szFileName, EWinRefHashArea eArea)
{
	CInt32Rect	cArea;

	CloseHash();
	if (szFileName == NULL)
	{
		return true;
	}
	if ((eArea == WRHA_Screen) && !GetArea(&cArea))
	{
		gcLogger.Error2(__METHOD__, " Cannot hash the screen, its area isn't set.", NULL);
		return false;
	}

	mcHashFile.Init(DiskFile(szFileName));
	if (!mcHashFile.Open(EFM_Write_Replace))
	{
		mcHashFile.Kill();
		gcLogger.Error2(__METHOD__, " Cannot write hashes to [", szFileName, "].", NULL);
		return false;
	}
	mbHash = true;
	meHashArea = eArea;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::CloseHash(void)
{
	if (mbHash)
	{
		mcHashFile.Close();
		mcHashFile.Kill();
		mbHash = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Windows keep what they show only while it is.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefScreen::IsWatching(void)
{
	return mbCheck || mbHash;
}


//////////////////////////////////////////////////////////////////////////
//
// A window made goes on top.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Add(CWinRefWindow* pcWindow)
{
	Remove(pcWindow);
	mapcWindows.Add(pcWindow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Remove(CWinRefWindow* pcWindow)
{
	size	uiIndex;

	uiIndex = mapcWindows.Find(pcWindow);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		mapcWindows.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Only a window on the screen can be raised.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Raise(CWinRefWindow* pcWindow)
{
	if (mapcWindows.Find(pcWindow) != ARRAY_ELEMENT_NOT_FOUND)
	{
		Add(pcWindow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What pcDamage covers of pcCanvas is shown in pcWindow, then checked and
// hashed.  A present with nothing in it isn't hashed.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Present(CWinRefWindow* pcWindow, CWinRefPixels* pcCanvas, CDamageArea* pcDamage)
{
	CWinRefPixels*	pcShown;
	CInt32Rect		cAll;
	CInt32Rect*		pcRectangle;
	size			i;

	if (!IsWatching())
	{
		return;
	}

	pcShown = pcWindow->GetShown();
	pcShown->SetSize(pcCanvas->GetWidth(), pcCanvas->GetHeight(), pcCanvas->GetPixelBytes());
	if (pcDamage->IsAll())
	{
		pcCanvas->GetArea(&cAll);
		pcShown->Copy(pcCanvas, &cAll, 0, 0);
	}
	for (i = 0; i < pcDamage->NumRectangles(); i++)
	{
		pcRectangle = pcDamage->GetRectangle(i);
		pcShown->Copy(pcCanvas, pcRectangle, pcRectangle->GetLeft(), pcRectangle->GetTop());
	}
	muiPresents++;

	if (mbCheck)
	{
		Check(pcWindow, pcCanvas);
	}
	if (mbHash && !pcDamage->IsEmpty())
	{
		WriteHash(pcWindow, pcDamage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Check(CWinRefWindow* pcWindow, CWinRefPixels* pcCanvas)
{
	CWinRefPixels*	pcShown;
	CInt32Rect		cDiffers;
	CInt32Rect		cAll;
	lsize			uiPixels;
	CChars			sz;

	pcShown = pcWindow->GetShown();
	uiPixels = pcShown->FindDifference(pcCanvas, &cDiffers);
	if (uiPixels == 0)
	{
		return;
	}

	muiMismatches++;
	sz.Init(" A present of [");
	sz.Append(pcWindow->GetTitle());
	sz.Append("] shows other than what was drawn in ");
	sz.Append((uint64)uiPixels);
	sz.Append(" pixels from [");
	sz.Append(cDiffers.GetLeft());
	sz.Append(", ");
	sz.Append(cDiffers.GetTop());
	sz.Append("] to [");
	sz.Append(cDiffers.GetRight());
	sz.Append(", ");
	sz.Append(cDiffers.GetBottom());
	sz.Append("].");
	gcLogger.Error2(__METHOD__, sz.Text(), NULL);
	sz.Kill();

	pcCanvas->GetArea(&cAll);
	pcShown->Copy(pcCanvas, &cAll, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// "<present> [<title>] <hash>", the hash in sixteen hex digits.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::WriteHash(CWinRefWindow* pcWindow, CDamageArea* pcDamage)
{
	CWinRefPixels*	pcShown;
	CInt32Rect		cAll;
	uint64			ulliHash;
	size			i;
	CChars			sz;

	ulliHash = FNV_HASH64_START;
	if (meHashArea == WRHA_Screen)
	{
		Compose();
		mcPixels.GetArea(&cAll);
		ulliHash = mcPixels.Hash(&cAll, ulliHash);
	}
	else
	{
		pcShown = pcWindow->GetShown();
		if (pcDamage->IsAll())
		{
			pcShown->GetArea(&cAll);
			ulliHash = pcShown->Hash(&cAll, ulliHash);
		}
		for (i = 0; i < pcDamage->NumRectangles(); i++)
		{
			ulliHash = pcShown->Hash(pcDamage->GetRectangle(i), ulliHash);
		}
	}

	sz.Init();
	sz.Append(muiPresents);
	sz.Append(" [");
	sz.Append(pcWindow->GetTitle());
	sz.Append("] ");
	sz.AppendHexHiLo(&ulliHash, sizeof(ulliHash));
	sz.AppendNewLine();
	mcHashFile.WriteData(sz.Text(), sz.Length());
	mcHashFile.Flush();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The screen's pixels made from what each window shows, the bottom one
// first, at its place on the screen.  Where no window is is black.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefScreen::Compose(void)
{
	CWinRefWindow*	pcWindow;
	CWinRefPixels*	pcShown;
	CInt32Rect		cPlace;
	CInt32Rect		cShown;
	size			uiPixelBytes;
	size			i;

	uiPixelBytes = 0;
	for (i = 0; (i < mapcWindows.NumElements()) && (uiPixelBytes == 0); i++)
	{
		uiPixelBytes = mapcWindows.GetPtr(i)->GetShown()->GetPixelBytes();
	}
	mcPixels.SetSize(mcArea.GetWidth(), mcArea.GetHeight(), uiPixelBytes);
	mcPixels.Clear();
	for (i = 0; i < mapcWindows.NumElements(); i++)
	{
		pcWindow = mapcWindows.GetPtr(i);
		pcShown = pcWindow->GetShown();
		if (pcShown->GetPixelBytes() == uiPixelBytes)
		{
			pcWindow->GetScreenRectangle(&cPlace);
			pcShown->GetArea(&cShown);
			mcPixels.Copy(pcShown, &cShown, cPlace.GetLeft() - mcArea.GetLeft(), cPlace.GetTop() - mcArea.GetTop());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWinRefPixels* CWinRefScreen::GetPixels(void) { return &mcPixels; }
lsize CWinRefScreen::NumMismatches(void) { return muiMismatches; }

