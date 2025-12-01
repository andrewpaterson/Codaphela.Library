#ifndef __FOCUS_H__
#define __FOCUS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "InputLib/Input.h"
#include "SupportLib/Rectangle.h"
#include "BasicComponent.h"


class CWindow;
class CBasicComponent;
class CFocus : public CObject
{
CONSTRUCTABLE(CFocus);
protected:
	Ptr<CBasicComponent>	mpMouseOverComponent;
	Ptr<CBasicComponent>	mpFocussedComponent;
	Ptr<CWindow>			mpWindow;

public:
	void 					Init(Ptr<CWindow> pcViewport);
	void					Class(void);
	void 					Free(void);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void 					Update(int x, int y);
	void 					Request(void);
	void 					Activate(int x, int y);
	void 					Deactivate(void);
	Ptr<CBasicComponent>	GetFocussedComponent(void);
	Ptr<CBasicComponent>	GetMouseOverComponent(void);
};


#endif // __FOCUS_H__

