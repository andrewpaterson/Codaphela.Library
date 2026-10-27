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
#ifndef __TABBED_CONTAINER_H__
#define __TABBED_CONTAINER_H__
#include "GridContainer.h"
#include "FillContainer.h"
#include "IconComponent.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "WindowTheme.h"
#include "TabComponent.h"


//A bar of tabs over a page, like IntelliJ IDEA's editor tabs.  Each tab has its own page; the selected tab's page
//fills the area under the bar and the others are kept, not shown.  The first tab added is selected.  A closable tab's
//cross removes it and its page; the tab after it (or before, if it was the last) is selected if it was.
class CTabbedContainer : public CGridContainer
{
CONSTRUCTABLE(CTabbedContainer);
DESTRUCTABLE(CTabbedContainer);
protected:
	Ptr<CWindowTheme>		mpTheme;
	Ptr<CGridContainer>		mpBar;
	Ptr<CIconComponent>		mpFiller;  //The space after the last tab.
	Ptr<CFillContainer>		mpPage;
	CArray<CTabComponent>	maTabs;
	CArray<CComponent>		maPages;  //By tab.
	Ptr<CTabComponent>		mpSelected;
	int						miNextColumn;  //The bar's column for the next tab; a closed tab's stays, empty.

public:
	void					Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void					Class(void) override;
	void 					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	Ptr<CTabComponent>		AddTab(const char* szName, Ptr<CComponent> pPage, Ptr<CImageCel> pIcon = NULL, bool bClosable = false);
	bool					RemoveTab(Ptr<CTabComponent> pTab);
	void					Select(Ptr<CTabComponent> pTab);
	void					Select(size uiIndex);
	Ptr<CTabComponent>		GetSelected(void);
	size					NumTabs(void);
	Ptr<CTabComponent>		GetTab(size uiIndex);
	Ptr<CComponent>			GetPage(size uiIndex);

protected:
	size					IndexOf(Ptr<CTabComponent> pTab);
};


#endif // __TABBED_CONTAINER_H__
