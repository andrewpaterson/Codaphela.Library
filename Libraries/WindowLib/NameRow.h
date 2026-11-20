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
#ifndef __NAME_ROW_H__
#define __NAME_ROW_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/NameMatcher.h"
#include "ListContainer.h"
#include "Label.h"
#include "IconComponent.h"
#include "Dialog.h"


//Something listed by its name: the name a pattern matched, where it is (a class's file, a file's folder), its icon
//(an index its list gives, -1 for none) and what it is to whoever listed it.
struct SGoToItem
{
	CChars		szName;
	CChars		szLocation;
	int32		iIcon;
	size		uiData;
	SNameMatch	sMatch;
};


typedef CArrayTemplate<SGoToItem>	CArrayGoToItems;


//The items a search found, best first once sorted.
class CGoToItems
{
public:
	CArrayGoToItems		masItems;

	void		Init(void);
	void		Kill(void);
	void		Clear(void);

	SGoToItem*	Add(const char* szName, const char* szLocation, int32 iIcon, size uiData, SNameMatch* psMatch);
	size		NumItems(void) const;
	SGoToItem*	Get(size uiIndex);
	void		Sort(void);
};


//A list's row that names something, as a Go to dialog's results and a chooser's (CChooserPopup) are: its icon, its
//name with the characters a pattern matched in the match font, and where it is, in the look's dim font, after it.
//The row is added to the list; its index is returned.  A row that isn't enabled can't be selected (a heading).
int32 AddNameRow(Ptr<CListContainer> pList, Ptr<CWindow> pWindow, SDialogLook* psLook, Ptr<CFont> pMatchFont, Ptr<CImageCel> pIcon, SGoToItem* psItem, bool bEnabled = true);
Ptr<CComponent> CreateNameRowName(Ptr<CWindow> pWindow, SDialogLook* psLook, Ptr<CFont> pFont, Ptr<CFont> pMatchFont, const char* szName, SNameMatch* psMatch);


#endif // __NAME_ROW_H__

