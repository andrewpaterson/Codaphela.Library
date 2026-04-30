#ifndef __WINDOW_H__
#define __WINDOW_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Component.h"
#include "Canvas.h"
#include "NativeWindow.h"
#include "CanvasDraw.h"


class CWindowTick;
class CWindow : public CComponent
{
CONSTRUCTABLE(CWindow); 
DESTRUCTABLE(CWindow);
protected:
	CChars					mszWindowTitle;

	CNativeWindow*			mpcNativeWindow;
	Ptr<CCanvas>			mpCanvas;
	Ptr<CWindowTick>		mpWindowTick;
	bool					mbTicking;
	CFocus					mFocus;
	CNativeWindowFactory*	mpcFactory;

public:
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw);
	void					Class(void);
	void 					Free(void);
	
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;
	
	bool					Show(void);
	void					Paint(void);
	void					Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds);
	void					Stop(void);
	void					Layout(void);

	const char*				GetWindowTitle(void);
	Ptr<CCanvas>			GetCanvas(void);
	Ptr<CFocus>				GetFocus(void);

	bool					SetContainer(Ptr<CContainer> pContainer);
	Ptr<CContainer>			GetContainer(void);
	bool					ClearContainer(void);

	CNativeWindowFactory*	GetFactory(void);

	void					SetRequiredSize(void) override;
};


#endif // __WINDOW_H__

