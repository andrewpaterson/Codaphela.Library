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
#ifndef __COMPONENT_H__
#define __COMPONENT_H__
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"
#include "StandardLib/TimerAlarm.h"
#include "StandardLib/ListenerCall.h"
#include "ComponentPointers.h"
#include "FocusListener.h"
#include "ComponentListener.h"


class CComponentFactory;
class CContainer;
class CWorld;
class CViewport;


struct SContainerBounds
{
	SInt2	msTopLeft;
	SInt2	msSize;
};


class CComponent : public CUnknown, public CFocusListener, public CComponentListener
{
BASE_FUNCTIONS(CComponent);
protected:
	CViewport*			mpcViewport;
	CWorld*				mpcWorld;
	SInt2				msActualSize;
	SInt2	 			msPosition;
	SInt2				msRequiredSize;
	SInt2				msDesiredSize;
	BOOL				mbCanGetFocus;
	CChars				mcName;
	CComponent*			mpcParent;
	CComponentPointers	mcComponents;

public:
	void				Init(CViewport* pcViewport);
	virtual void		Kill(void);
	void				KillAlsoChildren(void);
	void 				SetName(char* szName);
	void 				SetActualSize(int fWidth, int fHeight);
	void				SetActualSize(SInt2 sSize);
	void 				SetDesiredSize(int fWidth, int fHeight);
	SInt2 				GetBestSize(void);
	void 				SetPosition(int x, int y);
	void				SetPosition(SInt2 sPosition);
	SInt2 				GetPosition(void);
	SInt2				GetActualSize(void);
	void 				SetFactory(CComponentFactory* pcFactory);
	virtual void		SetRequiredSize(void);
	virtual BOOL		Draw(void);
	BOOL				DrawChildren(void);
	virtual void		Layout(SInt2 sPosition, SInt2 sAreaSize);
	void				LayoutChildren(SInt2 sPosition, SInt2 sAreaSize);
	void				SetWorld(CWorld* pcWorld);
	void				SetViewport(CViewport* pcViewport);
	CComponentFactory*	GetFactory(void);
	CComponent*			GetParent(void);
	void				SetParent(CComponent* pcComponent);

	void 				StartTimer(int iTime, BOOL bRepeat);
	void				StopTimer(void);

	BOOL 				IsPointIn(int x, int y);
	BOOL 				HasFocus(void);
	virtual	void		AddComponent(CComponent* pcComponent);
	virtual	void		RemoveComponent(CComponent* pcComponent, BOOL bKillComponent);
	virtual	void		RemoveAllComponents(BOOL bKillComponents);
	CComponent*			FindComponentAt(int x, int y);
	void				ToChildSpace(CComponent* pcChildComponent, int x, int y, int* px, int* py);
	void				FromChildSpace(CComponent* pcChildComponent, int x, int y, int* px, int* py);
	virtual BOOL		GetContainerBounds(SContainerBounds* psDest);

	BOOL				IsFocussed(void);

	int					GetDepth(void);
	void				Dump(void);
};


#endif // __COMPONENT_H__

