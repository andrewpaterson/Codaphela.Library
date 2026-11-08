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
#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__
#include "BarContainer.h"
#include "WindowTheme.h"
#include "Label.h"


//A status bar along the bottom of a window, as IntelliJ IDEA's: fields of text in the font given, each a gap after
//what is before it, with separator lines where they are added.  The message field (AddMessageField) takes the space
//left over and shows a message for a while (ShowMessage); the window's tick clears it (UpdateMessage).  The sizes are
//the theme's TM_Status metrics; see CBarContainer for the stretch.
class CStatusBar : public CBarContainer
{
CONSTRUCTABLE(CStatusBar);
DESTRUCTABLE(CStatusBar);
protected:
	Ptr<CWindowTheme>	mpTheme;
	Ptr<CFont>			mpFont;
	Ptr<CLabel>			mpMessage;  //Where ShowMessage shows, NULL until there is a message field.
	int64				miMessageEnd;  //When the message is cleared (GetDebugTickCount), 0 if it stays.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pFont);
	void				Class(void) override;

	Ptr<CLabel>			AddField(int32 iWidth);
	Ptr<CLabel>			AddMessageField(void);
	void				AddSeparator(void);

	void				ShowMessage(const char* szText, int32 iMilliseconds, int64 iNow);
	int64				UpdateMessage(int64 iNow);
	char*				GetMessage(void);
};


#endif // __STATUS_BAR_H__

