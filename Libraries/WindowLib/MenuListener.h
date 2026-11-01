#ifndef __MENU_LISTENER_H__
#define __MENU_LISTENER_H__
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
#include "StandardLib/Listener.h"
#include "StandardLib/Unknown.h"


//pcSource is the CMenuItem that was clicked.  Its menus have already closed.  A menu tells its own listeners and, if
//it hangs from a menu bar, the bar's listeners.  An item that opens another menu is never clicked.
class CMenuListener : public CListener
{
CONSTRUCTABLE(CMenuListener);
public:
	virtual void	MenuItemClicked(CUnknown* pcSource, void* pvContext) {};
};

#endif // __MENU_LISTENER_H__

