/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include <string.h>
#include "BaseLib/SystemAllocator.h"
#include "IndexedObjects.h"


//Where oi is: its directory, its page in the directory, its place in the page, and the first index of its page.
static inline OIndex IndexedObjectsDirectory(OIndex oi) { return oi >> INDEXED_OBJECTS_DIRECTORY_SHIFT; }
static inline size IndexedObjectsPageInDirectory(OIndex oi) { return (size)((oi >> INDEXED_OBJECTS_PAGE_SHIFT) & (INDEXED_OBJECTS_DIRECTORY_SIZE - 1)); }
static inline size IndexedObjectsPlace(OIndex oi) { return (size)(oi & (INDEXED_OBJECTS_PAGE_SIZE - 1)); }
static inline OIndex IndexedObjectsPageFirst(OIndex oi) { return oi & ~(OIndex)(INDEXED_OBJECTS_PAGE_SIZE - 1); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::Init(void)
{
	mapsDirectories.Init();
	mpsLastPage = NULL;
	mpsEmptiedPage = NULL;
	mpsSparePage = NULL;
	muiNumIndexed = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::Kill(void)
{
	SIndexedObjectsDirectory*	psDirectory;
	size						i;
	size						j;

	for (i = 0; i < mapsDirectories.NumElements(); i++)
	{
		psDirectory = mapsDirectories.GetPtr(i);
		if (psDirectory != NULL)
		{
			for (j = 0; j < INDEXED_OBJECTS_DIRECTORY_SIZE; j++)
			{
				if (psDirectory->apsPages[j] != NULL)
				{
					gcSystemAllocator.Free(psDirectory->apsPages[j]);
				}
			}
			gcSystemAllocator.Free(psDirectory);
		}
	}
	mapsDirectories.Kill();
	if (mpsSparePage != NULL)
	{
		gcSystemAllocator.Free(mpsSparePage);
	}
	mpsLastPage = NULL;
	mpsEmptiedPage = NULL;
	mpsSparePage = NULL;
	muiNumIndexed = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The directory with oi's page in it, NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
SIndexedObjectsDirectory* CIndexedObjects::GetDirectory(OIndex oi)
{
	OIndex	oiDirectory;

	oiDirectory = IndexedObjectsDirectory(oi);
	if ((oiDirectory < 0) || (oiDirectory >= (OIndex)mapsDirectories.NumElements()))
	{
		return NULL;
	}
	return mapsDirectories.GetPtr((size)oiDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// The page with oi's place in it, NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
SIndexedObjectsPage* CIndexedObjects::GetPage(OIndex oi)
{
	SIndexedObjectsPage*		psPage;
	SIndexedObjectsDirectory*	psDirectory;

	psPage = mpsLastPage;
	if ((psPage != NULL) && (psPage->oiFirst == IndexedObjectsPageFirst(oi)))
	{
		return psPage;
	}

	psDirectory = GetDirectory(oi);
	if (psDirectory == NULL)
	{
		return NULL;
	}
	psPage = psDirectory->apsPages[IndexedObjectsPageInDirectory(oi)];
	if (psPage != NULL)
	{
		mpsLastPage = psPage;
	}
	return psPage;
}


//////////////////////////////////////////////////////////////////////////
//
// An empty page for oi's place, in its directory (made if it has none).
// NULL if oi can't be kept.
//
//////////////////////////////////////////////////////////////////////////
SIndexedObjectsPage* CIndexedObjects::AddPage(OIndex oi)
{
	SIndexedObjectsDirectory*	psDirectory;
	SIndexedObjectsPage*		psPage;
	OIndex						oiDirectory;
	size						uiDirectory;

	oiDirectory = IndexedObjectsDirectory(oi);
	uiDirectory = (size)oiDirectory;
	if ((oiDirectory < 0) || ((OIndex)uiDirectory != oiDirectory) || (uiDirectory == MAX_SIZE))
	{
		return NULL;
	}
	if (uiDirectory >= mapsDirectories.NumElements())
	{
		mapsDirectories.GrowToAtLeastNumElements(uiDirectory + 1, true, 0);
	}

	psDirectory = mapsDirectories.GetPtr(uiDirectory);
	if (psDirectory == NULL)
	{
		psDirectory = (SIndexedObjectsDirectory*)gcSystemAllocator.Malloc(sizeof(SIndexedObjectsDirectory));
		memset(psDirectory, 0, sizeof(SIndexedObjectsDirectory));
		mapsDirectories.SetPtr(uiDirectory, psDirectory);
	}

	if (mpsSparePage != NULL)
	{
		psPage = mpsSparePage;
		mpsSparePage = NULL;
	}
	else
	{
		psPage = (SIndexedObjectsPage*)gcSystemAllocator.Malloc(sizeof(SIndexedObjectsPage));
		memset(psPage->apcObjects, 0, sizeof(psPage->apcObjects));
	}
	psPage->oiFirst = IndexedObjectsPageFirst(oi);
	psPage->uiNumObjects = 0;

	psDirectory->apsPages[IndexedObjectsPageInDirectory(oi)] = psPage;
	psDirectory->uiNumPages++;
	mpsLastPage = psPage;
	return psPage;
}


//////////////////////////////////////////////////////////////////////////
//
// psPage, empty, is taken out of its directory (and the directory goes if
// it was its last page).  It is kept as the spare if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::RemovePage(SIndexedObjectsPage* psPage)
{
	SIndexedObjectsDirectory*	psDirectory;
	size						uiDirectory;

	uiDirectory = (size)IndexedObjectsDirectory(psPage->oiFirst);
	psDirectory = mapsDirectories.GetPtr(uiDirectory);
	psDirectory->apsPages[IndexedObjectsPageInDirectory(psPage->oiFirst)] = NULL;
	psDirectory->uiNumPages--;
	if (psDirectory->uiNumPages == 0)
	{
		mapsDirectories.SetPtr(uiDirectory, NULL);
		gcSystemAllocator.Free(psDirectory);
	}

	if (mpsLastPage == psPage)
	{
		mpsLastPage = NULL;
	}
	if (mpsSparePage == NULL)
	{
		mpsSparePage = psPage;
	}
	else
	{
		gcSystemAllocator.Free(psPage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// psPage has no objects left.  It stays until another page empties: the
// page that emptied before it goes then, if it is still empty.
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::PageEmptied(SIndexedObjectsPage* psPage)
{
	if ((mpsEmptiedPage != NULL) && (mpsEmptiedPage != psPage) && (mpsEmptiedPage->uiNumObjects == 0))
	{
		RemovePage(mpsEmptiedPage);
	}
	mpsEmptiedPage = psPage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CIndexedObjects::Get(OIndex oi)
{
	SIndexedObjectsPage*	psPage;

	psPage = GetPage(oi);
	if (psPage == NULL)
	{
		return NULL;
	}
	return psPage->apcObjects[IndexedObjectsPlace(oi)];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedObjects::Has(OIndex oi)
{
	return Get(oi) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject at oi, in place of what was there.  False if oi can't be kept
// (it is negative, or too big for a size on this target).
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedObjects::Add(OIndex oi, CBaseObject* pcObject)
{
	SIndexedObjectsPage*	psPage;
	CBaseObject**			ppcPlace;

	if (pcObject == NULL)
	{
		return false;
	}

	psPage = GetPage(oi);
	if (psPage == NULL)
	{
		psPage = AddPage(oi);
		if (psPage == NULL)
		{
			return false;
		}
	}

	ppcPlace = &psPage->apcObjects[IndexedObjectsPlace(oi)];
	if (*ppcPlace == NULL)
	{
		psPage->uiNumObjects++;
		muiNumIndexed++;
	}
	*ppcPlace = pcObject;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// False if nothing was at oi.
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedObjects::Remove(OIndex oi)
{
	SIndexedObjectsPage*	psPage;
	CBaseObject**			ppcPlace;

	psPage = GetPage(oi);
	if (psPage == NULL)
	{
		return false;
	}

	ppcPlace = &psPage->apcObjects[IndexedObjectsPlace(oi)];
	if (*ppcPlace == NULL)
	{
		return false;
	}

	*ppcPlace = NULL;
	muiNumIndexed--;
	psPage->uiNumObjects--;
	if (psPage->uiNumObjects == 0)
	{
		PageEmptied(psPage);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedObjects::NumIndexed(void) const
{
	return muiNumIndexed;
}


//////////////////////////////////////////////////////////////////////////
//
// The first object at or after the iteration's index, and its index.  The
// iteration moves past it.  False after the last.
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedObjects::Step(SIndexesIterator* psIterator, OIndex* poi, CBaseObject** ppcObject)
{
	SIndexedObjectsDirectory*	psDirectory;
	SIndexedObjectsPage*		psPage;
	CBaseObject*				pcObject;
	OIndex						oi;
	size						uiPlace;

	oi = psIterator->oiNext;
	if (oi < 0)
	{
		oi = 0;
	}
	while (IndexedObjectsDirectory(oi) < (OIndex)mapsDirectories.NumElements())
	{
		psDirectory = GetDirectory(oi);
		if (psDirectory == NULL)
		{
			oi = (IndexedObjectsDirectory(oi) + 1) << INDEXED_OBJECTS_DIRECTORY_SHIFT;
			continue;
		}

		psPage = psDirectory->apsPages[IndexedObjectsPageInDirectory(oi)];
		if (psPage != NULL)
		{
			for (uiPlace = IndexedObjectsPlace(oi); uiPlace < INDEXED_OBJECTS_PAGE_SIZE; uiPlace++)
			{
				pcObject = psPage->apcObjects[uiPlace];
				if (pcObject != NULL)
				{
					*poi = psPage->oiFirst + uiPlace;
					*ppcObject = pcObject;
					psIterator->oiNext = *poi + 1;
					return true;
				}
			}
		}
		oi = IndexedObjectsPageFirst(oi) + INDEXED_OBJECTS_PAGE_SIZE;
	}
	psIterator->oiNext = oi;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The indexes, in order.  INVALID_O_INDEX after the last.
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedObjects::StartIteration(SIndexesIterator* psIterator)
{
	psIterator->oiNext = NULL_O_INDEX;
	return Iterate(psIterator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedObjects::Iterate(SIndexesIterator* psIterator)
{
	OIndex			oi;
	CBaseObject*	pcObject;

	return Step(psIterator, &oi, &pcObject) ? oi : INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
// The objects themselves, in their indexes' order.  NULL after the last.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CIndexedObjects::StartIterationObject(SIndexesIterator* psIterator)
{
	psIterator->oiNext = NULL_O_INDEX;
	return IterateObject(psIterator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CIndexedObjects::IterateObject(SIndexesIterator* psIterator)
{
	OIndex			oi;
	CBaseObject*	pcObject;

	return Step(psIterator, &oi, &pcObject) ? pcObject : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The pages: the indexes each covers and how many objects it has.
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::Print(CChars* pszDest)
{
	SIndexedObjectsDirectory*	psDirectory;
	SIndexedObjectsPage*		psPage;
	size						i;
	size						j;

	pszDest->Append("Objects [");
	pszDest->Append(muiNumIndexed);
	pszDest->Append("] in pages of [");
	pszDest->Append((int32)INDEXED_OBJECTS_PAGE_SIZE);
	pszDest->Append("]\n");
	for (i = 0; i < mapsDirectories.NumElements(); i++)
	{
		psDirectory = mapsDirectories.GetPtr(i);
		if (psDirectory == NULL)
		{
			continue;
		}
		for (j = 0; j < INDEXED_OBJECTS_DIRECTORY_SIZE; j++)
		{
			psPage = psDirectory->apsPages[j];
			if (psPage != NULL)
			{
				pszDest->Append("  [");
				pszDest->Append(psPage->oiFirst);
				pszDest->Append("..");
				pszDest->Append(psPage->oiFirst + INDEXED_OBJECTS_PAGE_SIZE - 1);
				pszDest->Append("]: ");
				pszDest->Append(psPage->uiNumObjects);
				pszDest->AppendNewLine();
			}
		}
	}
}
