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
#include "Set.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSet::Init(int iChunkSize)
{
	CArrayCommonObject::Init(FALSE, TRUE, FALSE, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSet::Kill(void)
{
	CArrayCommonObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSet::RemoveDuringIteration(SSetIterator* psIter)
{
	mcArray.RemoveDuringIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CSet::StartIteration(SSetIterator* psIter)
{
	CBaseObject*	pcObject;
	CPointerObject	pObject;

	pcObject = (CBaseObject*)mcArray.StartIteration(psIter);
	pObject.mpcObject = pcObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CSet::Iterate(SSetIterator* psIter)
{
	CBaseObject*	pcObject;
	CPointerObject	pObject;

	pcObject = (CBaseObject*)mcArray.Iterate(psIter);
	pObject.mpcObject = pcObject;
	return pObject;
}


