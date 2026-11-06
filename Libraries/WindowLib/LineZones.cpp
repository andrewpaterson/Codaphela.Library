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
// pCodeEditor's lines get zones, their headers in pTitleFont over text in
// pTextFamily's fixed width font in sText, on sBackground.
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::Init(Ptr<CCodeEditor> pCodeEditor, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pTextFamily, ARGB32 sBackground, ARGB32 sText)
{
	PreInit();

	mpCodeEditor = pCodeEditor;
	mpTheme = pTheme;
	mpTitleFont = pTitleFont;
	mpTextFamily = pTextFamily;
	msBackground = sBackground;
	msText = sText;
	mpcSource = NULL;
	mbOutOfDate = false;
	mpCodeEditor->GetGutterClickEvent()->Add(this, &CLineZones::GutterClicked);
	mpCodeEditor->GetChangeEvent()->Add(this, &CLineZones::CodeChanged);

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
	U_UInt32(msText);
	U_Pointer(mpcSource);
	U_Bool(mbOutOfDate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::Free(void)
{
	if (mpCodeEditor.IsNotNull())
	{
		mpCodeEditor->GetGutterClickEvent()->Remove(this);
		mpCodeEditor->GetChangeEvent()->Remove(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A click on the line's mark in the gutter shows or hides its pages.
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::GutterClicked(CCodeEditor* pcEditor, size uiLine)
{
	Toggle(uiLine);
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

	i = mpCodeEditor->NumLineComponents();
	while (i > 0)
	{
		i--;
		pZone = GetZoneComponent(i);
		if (pZone.IsNull())
		{
			continue;
		}
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

	pZone = OMalloc<CLineZone>(mpCodeEditor->GetWindow(), mpTheme, mpTitleFont, mpTextFamily, msBackground, msText);
	pZone->SetSource(mpcSource, uiLine);
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
	for (i = 0; i < NumZones(); i++)
	{
		GetZone(i)->SetOutOfDate(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The zone under line uiLine, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLineZone> CLineZones::FindZone(size uiLine)
{
	size			i;
	Ptr<CLineZone>	pZone;

	for (i = 0; i < mpCodeEditor->NumLineComponents(); i++)
	{
		pZone = GetZoneComponent(i);
		if (pZone.IsNotNull() && (mpCodeEditor->GetLineOf(pZone) == uiLine))
		{
			return pZone;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// How many zones are shown, in the order they were opened.
//
//////////////////////////////////////////////////////////////////////////
size CLineZones::NumZones(void)
{
	size	i;
	size	uiZones;

	uiZones = 0;
	for (i = 0; i < mpCodeEditor->NumLineComponents(); i++)
	{
		if (GetZoneComponent(i).IsNotNull())
		{
			uiZones++;
		}
	}
	return uiZones;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL past the last zone.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLineZone> CLineZones::GetZone(size uiIndex)
{
	size			i;
	Ptr<CLineZone>	pZone;

	for (i = 0; i < mpCodeEditor->NumLineComponents(); i++)
	{
		pZone = GetZoneComponent(i);
		if (pZone.IsNotNull())
		{
			if (uiIndex == 0)
			{
				return pZone;
			}
			uiIndex--;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The code editor's line component uiComponent if it is a zone, else
// NULL.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLineZone> CLineZones::GetZoneComponent(size uiComponent)
{
	Ptr<CComponent>	pComponent;

	pComponent = mpCodeEditor->GetLineComponent(uiComponent);
	return Ptr<CLineZone>(dynamic_cast<CLineZone*>(&pComponent));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZones::RemoveZone(Ptr<CLineZone> pZone)
{
	mpCodeEditor->RemoveLineComponent(pZone);
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


bool CLineZones::IsOutOfDate(void) { return mbOutOfDate; }

