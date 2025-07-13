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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Numbers.h"
#include "StandardLib/TimerListener.h"
#include "Component.h"
#include "Viewport.h"
#include "ComponentFactory.h"
#include "FocusListener.h"
#include "ComponentListener.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Init(CViewport* pcViewport)
{
	mcComponents.Init();
	msActualSize.Init(0, 0);
	msPosition.Init(0, 0);
	msRequiredSize.Init(-1, -1);
	msDesiredSize.Init(-1, -1);
	mbCanGetFocus = false;
	mcName.Init();
	mpcParent = NULL;
	mpcViewport = pcViewport;
	mpcWorld = pcViewport->mpcWorld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Kill(void)
{
	mcComponents.Kill();

	mcName.Kill();

	if (mpcParent)
	{
		mpcParent->RemoveComponent(this, false);
	}
	mpcParent = NULL;

	GetFactory()->RemoveComponent(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::KillAlsoChildren(void)
{
	mcComponents.KillComponents();
	Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CComponent::GetBestSize(void)
{
	if (msDesiredSize.x != -1.0f)
	{
		return msDesiredSize;
	}
	SetRequiredSize();
	return msRequiredSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsPointIn(int x, int y)
{
	//x and y are assumed relative to 'this' component.
	return (((x >= msPosition.x) && (x <= msPosition.x + msActualSize.x)) && ((y >= msPosition.y) && (y <= msPosition.y + msActualSize.y)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::StartTimer(int iTime, bool bRepeat)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::StopTimer(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::HasFocus()
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Draw(void)
{
	return DrawChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::DrawChildren(void)
{
	int				i;
	CComponent*		pcComponent;
	bool			bResult;

	for (i = 0; i < mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		bResult = pcComponent->Draw();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::AddComponent(CComponent* pcComponent)
{
	mcComponents.Add(pcComponent);
	pcComponent->mpcParent = this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::RemoveComponent(CComponent* pcComponent, bool bKillComponent)
{
	mcComponents.Remove(pcComponent, bKillComponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::RemoveAllComponents(bool bKillComponents)
{
	if (bKillComponents)
	{
		mcComponents.KillComponents();
	}
	else
	{
		mcComponents.Kill();
		mcComponents.Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CComponent::FindComponentAt(int x, int y)
{
	int				i;
	CComponent*		pcComponent;

	//x and y are assumed relative to 'this' component.
	if (!IsPointIn(x, y))
	{
		return NULL;
	}

	for (i = 0; i < mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		pcComponent = pcComponent->FindComponentAt(x, y);
		if (pcComponent)
		{
			return pcComponent;
		}
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::ToChildSpace(CComponent* pcChildComponent, int x, int y, int* px, int* py)
{
	*px = x - (int)pcChildComponent->msPosition.x;
	*py = y - (int)pcChildComponent->msPosition.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::FromChildSpace(CComponent* pcChildComponent, int x, int y, int* px, int* py)
{
	*px = x + (int)pcChildComponent->msPosition.x;
	*py = y + (int)pcChildComponent->msPosition.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::GetContainerBounds(SContainerBounds* psDest)
{
	//The only way the function can be called is if the component is not a container.
	if (mpcParent)
	{
		return mpcParent->GetContainerBounds(psDest);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Layout(SInt2 sPosition, SInt2 sAreaSize)
{

	SetPosition(sPosition);
	SetActualSize(sAreaSize);
	
	LayoutChildren(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::LayoutChildren(SInt2 sPosition, SInt2 sAreaSize)
{
	int				i;
	CComponent*		pcComponent;

	for (i = 0; i < mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		pcComponent->Layout(sPosition, sAreaSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CComponent::GetDepth(void)
{
	int				iDepth;
	CComponent*		pcComponent;

	iDepth = 0;
	pcComponent = mpcParent;
	while (pcComponent != NULL)
	{
		iDepth++;
		pcComponent = pcComponent->mpcParent;
	}
	return iDepth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Dump(void)
{
	CChars			sz;
	size			i;
	CComponent*		pcComponent;

	sz.Init();
	sz.RightAlign(' ', GetDepth());
	sz.Append(ClassName());
	sz.Append(": ");
	sz.Append(mcName);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();

	for (i = 0; i < mcComponents.mcComponents.NumElements(); i++)
	{
		pcComponent = mcComponents.Get(i);
		pcComponent->Dump();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsFocussed(void)
{
	if (mpcViewport->mcFocus.GetFocussedComponent() == this)
	{
		return true;
	}
	return false;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SetWorld(CWorld* pcWorld) { mpcWorld = pcWorld; }
void CComponent::SetViewport(CViewport* pcViewport) { mpcViewport = pcViewport; }
CComponentFactory* CComponent::GetFactory(void) { return mpcViewport->mpcFactory; }
CComponent* CComponent::GetParent(void) { return mpcParent; }
void CComponent::SetParent(CComponent* pcComponent) { mpcParent = pcComponent; }
void CComponent::SetName(char* szName) { mcName.Set(szName); }
void CComponent::SetActualSize(int fWidth, int fHeight) { msActualSize.Init(fWidth, fHeight); }
void CComponent::SetActualSize(SInt2 sSize) { msActualSize = sSize; }
void CComponent::SetPosition(int x, int y) { msPosition.Init(x, y); }
void CComponent::SetPosition(SInt2 sPosition) { msPosition = sPosition; }
SInt2 CComponent::GetPosition(void) { return msPosition; }
void CComponent::SetDesiredSize(int fWidth, int fHeight) { msDesiredSize.Init(fWidth, fHeight); }
void CComponent::SetRequiredSize(void) { msRequiredSize = msDesiredSize; }
SInt2 CComponent::GetActualSize(void) { return msActualSize; }

