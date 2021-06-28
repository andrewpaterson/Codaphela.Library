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
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "IndexedObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::Init(void)
{
	mcIndexes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedObjects::Kill(void)
{
	mcIndexes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CIndexedObjects::Get(OIndex oi)
{
	CIndexTreeMemoryAccess	cAccess;
	CBaseObject*			pcObject;

	cAccess.Init(&mcIndexes);
	pcObject = (CBaseObject*)cAccess.GetLongPtr(oi);
	cAccess.Kill();
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedObjects::Has(OIndex oi)
{
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bHasObject;

	cAccess.Init(&mcIndexes);
	bHasObject = cAccess.HasLong(oi);
	cAccess.Kill();
	return bHasObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedObjects::Add(OIndex oi, CBaseObject* pvMemory)
{
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bResult;

	cAccess.Init(&mcIndexes);
	bResult = cAccess.PutLongPtr(oi, pvMemory);
	cAccess.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedObjects::Remove(OIndex oi)
{
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bResult;

	cAccess.Init(&mcIndexes);
	bResult =  cAccess.DeleteLong(oi);
	cAccess.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedObjects::NumIndexed(void)
{
	return mcIndexes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedObjects::StartIteration(SIndexesIterator* psIterator)
{
	BOOL	bResult;
	OIndex	oi;
	int		iKeyLength;

	bResult = mcIndexes.StartIteration(psIterator, &oi, &iKeyLength, sizeof(oi), NULL, NULL, 0);
	if ((bResult) && (iKeyLength == sizeof(OIndex)))
	{
			return oi;
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedObjects::Iterate(SIndexesIterator* psIterator)
{
	BOOL	bResult;
	OIndex	oi;
	int		iKeyLength;

	bResult = mcIndexes.Iterate(psIterator, &oi, &iKeyLength, sizeof(oi), NULL, NULL, 0);
	if ((bResult) && (iKeyLength == sizeof(OIndex)))
	{
		return oi;
	}
	return INVALID_O_INDEX;
}

