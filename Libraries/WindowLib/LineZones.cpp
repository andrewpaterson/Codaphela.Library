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
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "LineZones.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZonesListener::Init(CLineZones* pcZones)
{
	mpcZones = pcZones;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZonesListener::GutterClicked(CUnknown* pcSource, void* pvContext)
{
	mpcZones->Toggle(*(size*)pvContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZonesListener::CodeChanged(CUnknown* pcSource, void* pvContext)
{
	mpcZones->CodeChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// pCodeEditor's lines get zones, their headers in pTitleFont over text in
// pTextFamily's fixed width font, on sBackground.
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::Init(Ptr<CCodeEditor> pCodeEditor, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pTextFamily, ARGB32 sBackground)
{
	PreInit();

	mpCodeEditor = pCodeEditor;
	mpTheme = pTheme;
	mpTitleFont = pTitleFont;
	mpTextFamily = pTextFamily;
	msBackground = sBackground;
	mcListener.Init(this);
	mpcSource = NULL;
	mbOutOfDate = false;
	maZones.Init();
	mpCodeEditor->AddListener(&mcListener);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::Class(void)
{
	M_Pointer(mpCodeEditor);
	M_Pointer(mpTheme);
	M_Pointer(mpTitleFont);
	M_Pointer(mpTextFamily);
	U_UInt32(msBackground);
	U_Unknown(CLineZonesListener, mcListener);
	U_Pointer(mpcSource);
	U_Bool(mbOutOfDate);
	M_Embedded(maZones);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::Free(void)
{
	mcListener.RemoveFromListenerCalls();
}


//////////////////////////////////////////////////////////////////////////
//
// Line zones aren't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CLineZones::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLineZones::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The lines' pages are pcSource's (NULL for none), up to date with the
// code as it is: each line with pages is marked, and each zone shows its
// line's pages, or goes if its line hasn't any now (or was deleted).
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::SetSource(CLineZoneSource* pcSource)
{
	CArrayLineZoneLines	auiLines;
	Ptr<CLineZone>		pZone;
	size				i;
	size				uiLine;

	mpcSource = pcSource;
	mbOutOfDate = false;
	mpCodeEditor->ClearLineMarks();

	i = maZones.Size();
	while (i > 0)
	{
		i--;
		pZone = maZones.Get(i);
		uiLine = mpCodeEditor->GetLineOf(pZone);
		if (HasPages(uiLine))
		{
			pZone->SetSource(mpcSource, uiLine);
		}
		else
		{
			RemoveZone(pZone);
		}
	}

	if (mpcSource)
	{
		auiLines.Init();
		mpcSource->GetLines(&auiLines);
		for (i = 0; i < auiLines.NumElements(); i++)
		{
			MarkLine(*auiLines.Get(i));
		}
		auiLines.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shows line uiLine's pages under it, or hides them if they are shown.
// False if there are none to show: no source, an out of date one, or no
// pages for the line.
//
//////////////////////////////////////////////////////////////////////////
bool CLineZones::Toggle(size uiLine)
{
	Ptr<CLineZone>	pZone;

	pZone = FindZone(uiLine);
	if (pZone.IsNotNull())
	{
		RemoveZone(pZone);
		MarkLine(uiLine);
		return true;
	}
	if (mbOutOfDate || !HasPages(uiLine))
	{
		return false;
	}

	pZone = OMalloc<CLineZone>(mpCodeEditor->GetWindow(), mpTheme, mpTitleFont, mpTextFamily, msBackground);
	pZone->SetSource(mpcSource, uiLine);
	maZones.Add(pZone);
	mpCodeEditor->AddLineComponent(uiLine, pZone);
	MarkLine(uiLine);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The code was edited: the source's lines may not be the code's.  The
// marks go and the zones say they are out of date.
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::CodeChanged(void)
{
	size	i;

	if (!mpcSource || mbOutOfDate)
	{
		return;
	}
	mbOutOfDate = true;
	mpCodeEditor->ClearLineMarks();
	for (i = 0; i < maZones.Size(); i++)
	{
		maZones.Get(i)->SetOutOfDate(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The zone under line uiLine, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLineZone> CLineZones::FindZone(size uiLine)
{
	size	i;

	for (i = 0; i < maZones.Size(); i++)
	{
		if (mpCodeEditor->GetLineOf(maZones.Get(i)) == uiLine)
		{
			return maZones.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::RemoveZone(Ptr<CLineZone> pZone)
{
	mpCodeEditor->RemoveLineComponent(pZone);
	maZones.Remove(pZone);
}


//////////////////////////////////////////////////////////////////////////
//
// Line uiLine's mark: open if its pages are shown, otherwise whether it
// has pages (an out of date source's lines aren't marked).
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::MarkLine(size uiLine)
{
	char	cMark;

	cMark = '\0';
	if (FindZone(uiLine).IsNotNull())
	{
		cMark = LINE_ZONES_OPEN_MARK;
	}
	else if (!mbOutOfDate && HasPages(uiLine))
	{
		cMark = LINE_ZONES_MARK;
	}
	mpCodeEditor->SetLineMark(uiLine, cMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLineZones::HasPages(size uiLine)
{
	return mpcSource && (uiLine != MAX_SIZE) && (mpcSource->NumPages(uiLine) > 0);
}


size CLineZones::NumZones(void) { return maZones.Size(); }
Ptr<CLineZone> CLineZones::GetZone(size uiIndex) { return maZones.Get(uiIndex); }
bool CLineZones::IsOutOfDate(void) { return mbOutOfDate; }

