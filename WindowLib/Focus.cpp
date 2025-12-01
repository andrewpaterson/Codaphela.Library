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
#include "StandardLib/ClassDefines.h"
#include "Focus.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Init(Ptr<CWindow> pcViewport)
{
	mpWindow = pcViewport;
	mpMouseOverComponent = NULL;
	mpFocussedComponent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Class(void)
{
	M_Pointer(mpMouseOverComponent);
	M_Pointer(mpFocussedComponent);
	M_Pointer(mpWindow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFocus::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Update(int x, int y)
{
	Ptr<CBasicComponent> pcComponent;

	x = x - mpWindow->GetClientRect()->miLeft;
	y = y - mpWindow->GetClientRect()->miTop;

	pcComponent = mpWindow->FindComponentAt(x, y);

	if (mpMouseOverComponent != pcComponent)
	{
		if (mpMouseOverComponent)
		{
			mpMouseOverComponent->MovedOutof();
		}
		mpMouseOverComponent = pcComponent;
		if (pcComponent)
		{
			mpMouseOverComponent->MovedInto();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFocus::Request(void)
{
	if (mpMouseOverComponent != mpFocussedComponent)
	{
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusLost();
		}
		mpFocussedComponent = mpMouseOverComponent;
		if (mpFocussedComponent)
		{
			mpFocussedComponent->FocusGained();
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
	if (mpMouseOverComponent)
	{
		mpMouseOverComponent->MovedOutof();
	}
	if (mpFocussedComponent)
	{
		mpFocussedComponent->FocusLost();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBasicComponent> CFocus::GetFocussedComponent(void)
{
	return mpFocussedComponent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBasicComponent> CFocus::GetMouseOverComponent(void)
{
	return mpFocussedComponent;
}

