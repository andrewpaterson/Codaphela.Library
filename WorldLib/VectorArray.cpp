/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "VectorArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSkinnedVector::Init(void)
{
	memset(this, 0, sizeof(SSkinnedVector));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSkinnedVector::Kill(void)
{
	//Does nothing.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSkinnedVector::Cache(void)
{
	SFloat3*	psVec;
	int				i;
	SFloat3		sTemp;

	psCachedVector->x = apsSource[0]->x * afWeights[0];
	psCachedVector->y = apsSource[0]->y * afWeights[0];
	psCachedVector->z = apsSource[0]->z * afWeights[0];
	for (i = 1; i < 4; i++)
	{
		psVec = apsSource[i];
		if (!psVec)
		{
			break;
		}
		psCachedVector->x += apsSource[i]->x * afWeights[i];
		psCachedVector->y += apsSource[i]->y * afWeights[i];
		psCachedVector->z += apsSource[i]->z * afWeights[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSkinnedVector::SetInput(int i, SFloat3* psSource, float fWeight)
{
	apsSource[i] = psSource;
	afWeights[i] = fWeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSkinnedVector::SetOutput(SFloat3* psCached)
{
	psCachedVector = psCached;
}


