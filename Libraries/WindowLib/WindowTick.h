/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __WINDOW_TICK_H__
#define __WINDOW_TICK_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


/* The only import method in this class is

Tick()

Everything else just exists as a useful template to copy/paste objects from.

*/


class CWindow;
class CWindowTick : public CObject
{
CONSTRUCTABLE(CWindowTick);
DESTRUCTABLE(CWindowTick);
protected:
public:
			void	Init(void);
			void	Class(void) override;
			void 	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) =0;
};


#endif // __WINDOW_TICK_H__

