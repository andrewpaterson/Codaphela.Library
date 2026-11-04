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
#ifndef __LINE_ZONE_H__
#define __LINE_ZONE_H__
#include "SupportLib/FontFamily.h"
#include "GridContainer.h"
#include "Label.h"
#include "Button.h"
#include "ButtonListener.h"
#include "TextArea.h"
#include "WindowTheme.h"
#include "LineZoneSource.h"


class CLineZone;


//Hears a zone's < and > buttons.
class CLineZoneListener : public CButtonListener
{
CONSTRUCTABLE(CLineZoneListener);
public:
	CLineZone*	mpcZone;

	void	Init(CLineZone* pcZone);
	void	ButtonClicked(CUnknown* pcSource, void* pvContext) override;
};


//A line's pages from a CLineZoneSource, shown under the line in a code editor (see CLineZones): a header, < and > to
//step through the pages then the page's title and "2 of 3", over the page's text in a fixed width font.  It is as high
//as its header and text.  Out of date (the code was edited since the source was made) it keeps what it shows, greyed,
//with "(out of date)" after the title.
class CLineZone : public CGridContainer
{
CONSTRUCTABLE(CLineZone);
DESTRUCTABLE(CLineZone);
protected:
	Ptr<CLabel>			mpTitle;
	Ptr<CButton>		mpPrevious;
	Ptr<CButton>		mpNext;
	Ptr<CTextArea>		mpText;
	CLineZoneListener	mcListener;
	CLineZoneSource*	mpcSource;	//What the pages are from; whatever made the zone keeps it.
	size				muiLine;	//The source's line, as it was when the zone was given it.
	size				muiPages;
	size				muiPage;	//The one shown.
	CChars				mszTitle;	//Its title from the source.
	bool				mbOutOfDate;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pTextFamily, ARGB32 sBackground);
	void			Class(void) override;
	void			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetSource(CLineZoneSource* pcSource, size uiLine);
	void			Step(int iDirection);
	void			SetOutOfDate(bool bOutOfDate);
	void			ButtonClicked(CUnknown* pcButton);

	size			NumPages(void);
	size			GetPage(void);
	char*			GetTitle(void);
	char*			GetText(void);
	bool			IsOutOfDate(void);
	bool			HasButtons(void);
	Ptr<CButton>	GetPreviousButton(void);
	Ptr<CButton>	GetNextButton(void);

protected:
	Ptr<CButton>	CreateButton(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szCaption);
	void			ShowPage(void);
	void			ShowTitle(void);
	void			ShowButtons(void);
};


#endif // __LINE_ZONE_H__

