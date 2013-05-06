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
#include "BaseLib/DebugLog.h"
#include "ComponentPointers.h"
#include "Component.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentPointers::Init(void)
{
	mcComponents.Init(4);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentPointers::Kill(void)
{
	mcComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentPointers::KillComponents(void)
{
	int				i;
	CComponent*		pcComponent;

	for (i = mcComponents.NumElements()-1; i >= 0 ; i--)
	{
		pcComponent = Get(i);
		pcComponent->Kill();
	}
	mcComponents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentPointers::Add(CComponent* pcComponent)
{
	mcComponents.Add(pcComponent, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentPointers::Remove(CComponent* pcComponent, BOOL bKillComponent)
{
	DEBUG_LOG_P(pcComponent->ClassName())

	mcComponents.Remove(pcComponent);
	if (bKillComponent)
	{
		pcComponent->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentPointers::RemoveAll(void)
{
	KillComponents();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CComponentPointers::Get(int iIndex)
{
	return (CComponent*)mcComponents.SafeGetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CComponentPointers::Size(void)
{
	return mcComponents.NumElements();
}
