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
#include "Focus.h"
#include "Viewport.h"
#include "Window.h"
#include "FocusListener.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Init(CViewport* pcViewport)
{
	mpcViewport = pcViewport;
	mpcMouseOverComponent = NULL;
	mpcFocussedComponent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Update(int x, int y)
{
	CComponent*		pcComponent;

	x = x - mpcViewport->mpcClientRect->miLeft;
	y = y - mpcViewport->mpcClientRect->miTop;

	pcComponent = mpcViewport->FindComponentAt(x, y);

	if (mpcMouseOverComponent != pcComponent)
	{
		if (mpcMouseOverComponent)
		{
			mpcMouseOverComponent->MovedOutof();
		}
		mpcMouseOverComponent = pcComponent;
		if (pcComponent)
		{
			mpcMouseOverComponent->MovedInto();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Request(void)
{
	if (mpcMouseOverComponent != mpcFocussedComponent)
	{
		if (mpcFocussedComponent)
		{
			mpcFocussedComponent->FocusLost();
		}
		mpcFocussedComponent = mpcMouseOverComponent;
		if (mpcFocussedComponent)
		{
			mpcFocussedComponent->FocusGained();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Activate(int x, int y)
{
	Update(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Deactivate(void)
{
	if (mpcMouseOverComponent)
	{
		mpcMouseOverComponent->MovedOutof();
	}
	if (mpcFocussedComponent)
	{
		mpcFocussedComponent->FocusLost();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CFocus::GetFocussedComponent(void)
{
	return mpcFocussedComponent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CFocus::GetMouseOverComponent(void)
{
	return mpcFocussedComponent;
}

