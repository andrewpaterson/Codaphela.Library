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
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FileUtil.h"
#include "WorldLib/World.h"
#include "SupportLib/Font.h"
#include "Viewport.h"
#include "ComponentFactory.h"
#include "FocusListener.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::Init(CWorld* pcWorld, CInput* pcInput, CComponentFactory* pcFactory, CRectangle* pcClientRect)
{
	mpcWorld = pcWorld;
	mpcFactory = pcFactory;
	mpcClientRect = pcClientRect;
	mpcGraphicsObject = CreateGraphicsObject();
	mcMap2D.Init(mpcGraphicsObject);
	mcFocus.Init(this);
	mcInput.Init(pcInput, this);
	CFixedContainer::Init(this);

	mcInput.GetPointer()->Update();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::Kill(void)
{
	mcInput.Kill();
	mcFocus.Kill();
	mcMap2D.Kill();

	mpcWorld->ReleaseGraphicsObject(mpcGraphicsObject);
	mpcGraphicsObject = NULL;
	CFixedContainer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsObject* CViewport::CreateGraphicsObject(void)
{
	CGraphicsObject*	pcGraphicsObject;

	pcGraphicsObject = mpcWorld->CreateGraphicsObject(TRUE);
	pcGraphicsObject->AddMatrix(mpcWorld->GetIdentityMatrix());
	return pcGraphicsObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::Layout(void)
{
	CFixedContainer::Layout(msPosition, msActualSize);
	mcFocus.Update(mcInput.GetPointer()->GetX(), mcInput.GetPointer()->GetY());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::FinalisePrimitives(void)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < mpcGraphicsObject->GetPrimitives()->NumElements(); i++)
	{
		pcPrimitive = mpcGraphicsObject->GetPrimitive(i);
		pcPrimitive->miFlags = GRAPH_PRIM_FLAGS_DRAW;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CViewport::Draw(void)
{
	if (!mpcGraphicsObject->BeginDynamic())
	{
		return FALSE;
	}
	mcMap2D.Clear();
	if (!CFixedContainer::Draw())
	{
		return FALSE;
	}
	mcMap2D.Draw();
	FinalisePrimitives();
	if (!mpcGraphicsObject->EndDynamic())
	{
		return FALSE;
	}
	return mpcGraphicsObject->Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::SetRequiredSize(void)
{
	msRequiredSize.Init(msActualSize.x, msActualSize.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::SetActualSize(void)
{
	CComponent::SetActualSize(mpcClientRect->GetWidth(), mpcClientRect->GetHeight());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CViewport::FindComponentAt(int x, int y)
{
	int sx, sy;

	if ((x < 0) || (y < 0))
	{
		return NULL;
	}

	sx = mpcClientRect->GetWidth();
	sy = mpcClientRect->GetHeight();
	if ((x > sx) || (y > sy))
	{
		return NULL;
	}

	return CComponent::FindComponentAt(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::Activate(void)
{
	CSystemPointer*	pcPointer;

	pcPointer = mcInput.GetPointer();

	pcPointer->Update();
	mcFocus.Activate(pcPointer->GetX(), pcPointer->GetY());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewport::Deactivate(void)
{
	mcFocus.Deactivate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CViewport::IsPointInAbsolute(int x, int y)
{
	x -= mpcClientRect->GetLeft();
	y -= mpcClientRect->GetTop();
	return IsPointIn(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CViewport::GetFocussedComponent(void)
{
	return mcFocus.GetFocussedComponent();
}

