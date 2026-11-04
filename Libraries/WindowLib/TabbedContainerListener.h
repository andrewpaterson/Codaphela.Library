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
#ifndef __TABBED_CONTAINER_LISTENER_H__
#define __TABBED_CONTAINER_LISTENER_H__
#include "StandardLib/Listener.h"
#include "StandardLib/Unknown.h"


//Hears a CTabbedContainer's tabs.  pcSource is the CTabComponent that was double clicked, selected (only when the
//selection changes) or removed.  A removed tab's pvContext is its page, the CComponent it showed; neither is the
//container's any more, and the listeners hear of it before another tab is selected in its place.
class CTabbedContainerListener : public CListener
{
CONSTRUCTABLE(CTabbedContainerListener);
public:
	virtual void	TabDoubleClicked(CUnknown* pcSource, void* pvContext) {};
	virtual void	TabSelected(CUnknown* pcSource, void* pvContext) {};
	virtual void	TabRemoved(CUnknown* pcSource, void* pvContext) {};
};


#endif // __TABBED_CONTAINER_LISTENER_H__

