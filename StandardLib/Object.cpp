/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Object.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObject::CObject(void)
{
	mapTos.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Kill(void)
{
	CBaseObject::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::AddTo(CBaseObject* pcTo)
{
	if (pcTo != NULL)
	{
		mapTos.Add(&pcTo);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveTo(CBaseObject* pcTo)
{
	if (pcTo != NULL)
	{
		mapTos.Remove(&pcTo);
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::NumTos(void)
{
	return mapTos.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::IsCollection(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	int				i;
	CBaseObject**	ppcPointedTo;
	CBaseObject*	pcPointedTo;

	ppcPointedTo = mapTos.GetData();
	for (i = 0; i < mapTos.NumElements(); i++)
	{
		pcPointedTo = ppcPointedTo[i];
		if (pcPointedTo->miDistToRoot != -1)
		{
			pcPointedTo->RemoveFrom(this);
			papcFromsChanged->Add(&pcPointedTo);
		}
	}
	mapTos.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::CollectedThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	int				i;
	CBaseObject**	ppcPointedTo;

	MarkForKilling(papcKilled);

	ppcPointedTo = mapTos.GetData();
	for (i = 0; i < mapTos.NumElements(); i++)
	{
		if (ppcPointedTo[i]->miDistToRoot != -1)
		{
			if (!ppcPointedTo[i]->CanFindRoot())
			{
				ppcPointedTo[i]->CollectedThoseToBeKilled(papcKilled);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetDistToRoot(int iDistToRoot)
{
	int				i;
	CBaseObject**	ppcTos;

	miDistToRoot = iDistToRoot;
	ppcTos = mapTos.GetData();
	for (i = 0; i < mapTos.NumElements(); i++)
	{
		PotentiallySetDistToRoot(ppcTos[i], iDistToRoot+1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetTos(CArrayBaseObjectPtr* papcTos)
{
	int				i;
	CBaseObject**	ppcTos;

	ppcTos = mapTos.GetData();
	for (i = 0; i < mapTos.NumElements(); i++)
	{
		papcTos->Add(&ppcTos[i]);
	}
}

