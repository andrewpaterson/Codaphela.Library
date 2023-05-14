/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/WinInit.h"
#include "SupportLib/WinText.h"
#include "WorldLib/Map2D.h"
#include "FixedContainer.h"
#include "TextParameters.h"
#include "Focus.h"
#include "ComponentInput.h"


class CViewport : public CFixedContainer
{
public:
	CONSTRUCTABLE(CViewport);

	CGraphicsObject*	mpcGraphicsObject;
	CFocus				mcFocus;
	CComponentFactory*	mpcFactory;
	CComponentInput		mcInput;
	CRectangle*			mpcClientRect;
	CMap2D				mcMap2D;

	void 				Init(CWorld* pcWorld, CInput* pcInput, CComponentFactory* pcFactory, CRectangle* pcClientRect);
	void 				Kill(void);
	BOOL 				Draw(void);
	void				SetRequiredSize(void);
	void				SetActualSize(void);
	CGraphicsObject*	CreateGraphicsObject(void);

	void 				FinalisePrimitives(void);
	CComponent*			FindComponentAt(int x, int y);
	void				Layout(void);
	void				Activate(void);
	void				Deactivate(void);

	BOOL				IsPointInAbsolute(int x, int y);
	CComponent*			GetFocussedComponent(void);
};


#endif // __VIEWPORT_H__

