/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Iterables.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIterableList::Init(char* szName)
{
	mcFreeListPtrs.Init(128);
	mszName.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIterableList::Kill(void)
{
	mszName.Kill();
	mcFreeListPtrs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIterableList::Add(CUnknown* pcUnknown)
{
	mcFreeListPtrs.Add(&pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIterableList::Remove(CUnknown* pcUnknown)
{
	SFreeListIterator	sIter;
	CUnknown**			ppcUnknown;

	if (pcUnknown)
	{
		ppcUnknown = mcFreeListPtrs.StartIteration(&sIter);
		while (ppcUnknown)
		{
			if (pcUnknown == *ppcUnknown)
			{
				mcFreeListPtrs.RemoveDuringIteration(&sIter);
				return TRUE;
			}
			ppcUnknown = mcFreeListPtrs.Iterate(&sIter);
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIterableList::Is(char* szName)
{
	return mszName.Equals(szName);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CIterableList::StartIteration(SIteratorUnknown* psIter)
{
	CUnknown**	ppv;

	psIter->pcList = this;
	ppv = mcFreeListPtrs.StartIteration(&psIter->sIter);
	if (ppv)
	{
		return *ppv;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CIterableList::Iterate(SIteratorUnknown* psIter)
{
	CUnknown**	ppv;

	ppv = mcFreeListPtrs.Iterate(&psIter->sIter);
	if (ppv)
	{
		return *ppv;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIterables::Init(void)
{
	mcIterableLists.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIterables::Kill(void)
{
	CIterableList*	pcList;

	pcList = mcIterableLists.GetHead();
	while (pcList)
	{
		pcList->Kill();
		pcList = mcIterableLists.GetNext(pcList);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIterables::Add(CUnknown* pcUnknown)
{
	CIterableList*	pcList;
	char*			szName;

	szName = pcUnknown->ClassName();

	pcList = mcIterableLists.GetHead();
	while (pcList)
	{
		if (pcList->Is(szName))
		{
			pcList->Add(pcUnknown);
			return;
		}
		pcList = mcIterableLists.GetNext(pcList);
	}
	pcList = mcIterableLists.Add();
	pcList->Init(szName);
	pcList->Add(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIterables::Remove(CUnknown* pcUnknown)
{
	CIterableList*	pcList;
	char*			szName;

	szName = pcUnknown->ClassName();

	pcList = mcIterableLists.GetHead();
	while (pcList)
	{
		if (pcList->Is(szName))
		{
			pcList->Remove(pcUnknown);
			return TRUE;
		}
		pcList = mcIterableLists.GetNext(pcList);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CIterables::StartIteration(char* szName, SIteratorUnknown* psIter)
{
	CIterableList*	pcList;

	pcList = mcIterableLists.GetHead();
	while (pcList)
	{
		if (pcList->Is(szName))
		{
			return pcList->StartIteration(psIter);
		}
		pcList = mcIterableLists.GetNext(pcList);
	}
	psIter->pcList = NULL;
	psIter->sIter.psCurrent = NULL;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CIterables::Iterate(SIteratorUnknown* psIter)
{
	CUnknown**	ppv;

	if (psIter->pcList)
	{
		ppv = psIter->pcList->mcFreeListPtrs.Iterate(&psIter->sIter);
		if (ppv)
		{
			return *ppv;
		}
	}
	return NULL;
}

