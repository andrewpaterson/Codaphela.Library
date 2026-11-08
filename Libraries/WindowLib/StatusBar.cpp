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
#include "StandardLib/ClassDefines.h"
#include "Window.h"
#include "StatusBar.h"


//////////////////////////////////////////////////////////////////////////
//
// An empty status bar whose fields are in pFont.
//
//////////////////////////////////////////////////////////////////////////
void CStatusBar::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pFont)
{
	PreInit();

	CBarContainer::Init(pWindow, false, pTheme->GetMetric(TM_StatusBarHeight), pTheme->GetMetric(TM_StatusFieldGap));
	mpTheme = pTheme;
	mpFont = pFont;
	mpMessage = NULL;
	miMessageEnd = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStatusBar::Class(void)
{
	CBarContainer::Class();
	M_Pointer(mpTheme);
	M_Pointer(mpFont);
	M_Pointer(mpMessage);
	U_Int64(miMessageEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// An empty field iWidth pixels wide, -1 for as wide as its text, after a
// gap.  Its text is set by the application.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLabel> CStatusBar::AddField(int32 iWidth)
{
	Ptr<CLabel>	pLabel;

	AddGap(mpTheme->GetMetric(TM_StatusFieldGap));
	pLabel = OMalloc<CLabel>(mpWindow, mpFont, "");
	pLabel->SetDesiredSize(iWidth, -1);
	pLabel->SetAlignment(CSH_Left, CSV_Center);
	Append(pLabel);
	return pLabel;
}


//////////////////////////////////////////////////////////////////////////
//
// The field ShowMessage shows in, after a gap, taking the space left
// over (unless a stretch already does): the fields added after it are at
// the bar's right end.  A bar has one at most; another returns the first.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLabel> CStatusBar::AddMessageField(void)
{
	if (mpMessage.IsNotNull())
	{
		return mpMessage;
	}
	AddGap(mpTheme->GetMetric(TM_StatusFieldGap));
	mpMessage = OMalloc<CLabel>(mpWindow, mpFont, "");
	mpMessage->SetAlignment(CSH_Stretch, CSV_Center);
	if (!AppendStretch(mpMessage))
	{
		Append(mpMessage);
	}
	return mpMessage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStatusBar::AddSeparator(void)
{
	AppendLine(mpTheme->GetMetric(TM_StatusFieldGap), mpTheme->GetMetric(TM_StatusSeparatorLength), mpTheme->GetColour(TC_Separator));
}


//////////////////////////////////////////////////////////////////////////
//
// The message field says szText for iMilliseconds from iNow, 0 for until
// something else is said.  The window is asked to tick when it is to be
// cleared, which UpdateMessage then does.
//
//////////////////////////////////////////////////////////////////////////
void CStatusBar::ShowMessage(const char* szText, int32 iMilliseconds, int64 iNow)
{
	if (mpMessage.IsNull())
	{
		return;
	}
	mpMessage->SetText(szText);
	if (iMilliseconds > 0)
	{
		miMessageEnd = iNow + iMilliseconds;
		mpWindow->RequestTick((uint32)iMilliseconds);
	}
	else
	{
		miMessageEnd = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Clears the message if its time is up.  How many milliseconds until it
// is, -1 if there is no message to clear.
//
//////////////////////////////////////////////////////////////////////////
int64 CStatusBar::UpdateMessage(int64 iNow)
{
	if (miMessageEnd == 0)
	{
		return -1;
	}
	if (iNow >= miMessageEnd)
	{
		mpMessage->SetText("");
		miMessageEnd = 0;
		return -1;
	}
	return miMessageEnd - iNow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CStatusBar::GetMessage(void)
{
	return mpMessage.IsNotNull() ? mpMessage->GetText() : NULL;
}

