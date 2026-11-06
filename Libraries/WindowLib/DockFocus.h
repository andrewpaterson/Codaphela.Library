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
#ifndef __DOCK_FOCUS_H__
#define __DOCK_FOCUS_H__
#include "SupportLib/Int32Rect.h"
#include "StandardLib/Object.h"
#include "DockNode.h"


class CDockManager;
class CAutoHideFlyout;
class CDockPane;
class CComponent;
class CWindow;
class CFocus;


//The active pane and document, following the focus in the manager's windows, and the auto hide flyout opening and
//closing.  Part of its manager, which holds it.
class CDockFocus : public CObject
{
CONSTRUCTABLE(CDockFocus);
DESTRUCTABLE(CDockFocus);
protected:
	Ptr<CDockManager>			mpManager;
	Ptr<CDockPane>				mpActivePane;
	Ptr<CDockPane>				mpActiveDocument;
	Ptr<CAutoHideFlyout>		mpFlyout;

public:
	void						Init(Ptr<CDockManager> pManager);
	void						Class(void) override;
	void						Free(void) override;
	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	Ptr<CDockPane>				GetActivePane(void);
	Ptr<CDockPane>				GetActiveDocument(void);
	Ptr<CAutoHideFlyout>		GetFlyout(void);
	bool						IsInFlyout(Ptr<CDockPane> pPane);
	void						Forget(Ptr<CDockPane> pPane);
	void						Clear(void);

	void						Watch(Ptr<CWindow> pWindow);
	void						Unwatch(Ptr<CWindow> pWindow);
	void						SetActivePane(Ptr<CDockPane> pPane);
	void						SetActiveDocument(Ptr<CDockPane> pPane);
	void						FocusPane(Ptr<CDockPane> pPane);
	void						RestoreFocus(Ptr<CWindow> pWindow);
	void						ShowFlyout(Ptr<CDockPane> pPane);
	void						CloseFlyout(void);
	void						FlyoutClosed(void);

protected:
	void						FocusChanged(CFocus* pcFocus);
	bool						IsOutsideFlyout(Ptr<CComponent> pFocussed);
	Ptr<CComponent>				FindFocusable(Ptr<CComponent> pComponent);
};


#endif // __DOCK_FOCUS_H__

