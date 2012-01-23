/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Polygon.h"
#include "Triangle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::Init(void)
{
	mapsPositions.Init(sizeof(SFloat3*), 1);
	mbConvex = TRUE;
	mpsNormal = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::Init(SFloat3* psNormal)
{
	Init();
	mpsNormal = psNormal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::Kill(void)
{
	mapsPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::Set(void)
{
	SFloat3*	ps1;
	SFloat3*	ps2;
	SFloat3*	ps3;

	if (mapsPositions.NumElements() >= 3)
	{
		ps1 = *((SFloat3**)mapsPositions.Get(0));
		ps2 = *((SFloat3**)mapsPositions.Get(1));
		ps3 = *((SFloat3**)mapsPositions.Get(2));

		CPlane::Set(ps1, ps2, ps3);

		if (mapsPositions.NumElements() == 3)
		{
			mbConvex = TRUE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygon::AddPosition(SFloat3* psPosition)
{
	if (mapsPositions.NumElements() < 3)
	{
		//If we have less than three points just add the point.
		mapsPositions.Add(&psPosition);
		if (mapsPositions.NumElements() == 1)
		{
			mpsPosition = psPosition;
		}
		if (mapsPositions.NumElements() == 3)
		{
			Set();
		}
		return TRUE;
	}
	else
	{
		//We need to check if this point lies in the plane before we can add it.
		if (On(psPosition))
		{
			mapsPositions.Add(&psPosition);
			PrivateCalculateConvexity();
			return TRUE;	
		}
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygon::AddPositions(SFloat3* psPos1, SFloat3* psPos2, SFloat3* psPos3)
{
	ReturnOnFalse(AddPosition(psPos1));
	ReturnOnFalse(AddPosition(psPos2));
	return AddPosition(psPos3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::PrivateCalculateConvexity(void)
{
	//Test the convexity of only the last point...
	
	mbConvex = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::CalculateConvexity(void)
{
	//Test the convexity of all points...

	mbConvex = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygon::AddTriangle(CTriangle* pcTriangle)
{
	SFloat3**	ppsPositions;
	int			i;
	SFloat3*	psPosition;
	BOOL		bResult;

	if (mapsPositions.NumElements() == 0)
	{
		//This preserves the orientation of the plane
		mpsNormal = pcTriangle->mpsNormal;
		mapsPositions.GrowByNumElements(3);
		ppsPositions = (SFloat3**)mapsPositions.GetData();
		ppsPositions[0] = pcTriangle->mpsPosition;
		ppsPositions[1] = pcTriangle->mpsPosition1;
		ppsPositions[2] = pcTriangle->mpsPosition2;
		mpsPosition = pcTriangle->mpsPosition;
		d = pcTriangle->d;
		return TRUE;
	}
	else
	{
		bResult = TRUE;
		for (i = 0; i < 3; i++)
		{
			psPosition = pcTriangle->GetPoint(i);
			if (!UnionPosition(psPosition))
			{
				bResult = FALSE;
			}
		}
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygon::HasPositionPtr(SFloat3* psPosition)
{
	int			i;
	SFloat3*	psOther;
	
	for (i = 0; i < mapsPositions.NumElements(); i++)
	{
		psOther = *((SFloat3**)mapsPositions.Get(i));
		if (psOther == psPosition)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygon::UnionPosition(SFloat3* psPosition)
{
	CArrayInt	cIndices;
	int			iInsertionIndex;
	int			i;
	int			iIndex;
	
	if (mbConvex)
	{
		if (On(psPosition))
		{
			if (!Contains(psPosition))
			{
				if (HasPositionPtr(psPosition))
				{
					return TRUE;
				}

				cIndices.Init(4);
				FindIndicesOfVisibleHalfSpaces(psPosition, &cIndices);
				if (cIndices.NumElements() == 0)
				{
					//Something went wrong.
					return FALSE;
				}

				iInsertionIndex = cIndices.GetValue(1);  //End index of the first visible line.

				for (i = 2; i < cIndices.NumElements(); i+= 2)
				{
					iIndex = cIndices.GetValue(i);
					mapsPositions.RemoveAt(iIndex);
					if (iIndex < iInsertionIndex)
					{
						iInsertionIndex--;
					}
				}

				mapsPositions.InsertAt(&psPosition, iInsertionIndex);
				cIndices.Kill();
			}
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygon::Contains(SFloat3* psPosition)
{
	int			i;
	SFloat3*	psStart;
	SFloat3*	psEnd;
	CHalfSpace	cHalfSpace;
	SFloat3		sVector;
	SFloat3		sTempNormal;

	if (CHalfSpace::Contains(psPosition))
	{
		for (i = 0; i < mapsPositions.NumElements(); i++)
		{
			psStart = *((SFloat3**)mapsPositions.Get(i));
			if (i != mapsPositions.NumElements()-1)
				psEnd = *((SFloat3**)mapsPositions.Get(i+1));
			else
				psEnd = *((SFloat3**)mapsPositions.Get(0));

			//Now we have the start and end of the line.
			sVector = *psEnd - *psStart;
			cHalfSpace.Init(&sTempNormal);
			cHalfSpace.Set2(psStart, (SFloat3*)mpsNormal, (SFloat3*)&sVector);
			if (!cHalfSpace.Contains(psPosition))
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygon::FindIndicesOfVisibleHalfSpaces(SFloat3* psPosition, CArrayInt* pcVisibleIndices)
{
	int			i;
	CHalfSpace	cHalfSpace;
	int			iStart;
	int			iEnd;
	SFloat3		sVector;
	SFloat3		sNormal;
	SFloat3*	psStart;
	SFloat3*	psEnd;
	int*		piLastIndex;
	int			iElementPos;

	iStart = -1;
	iEnd = -1;
	iElementPos = 0;
	for (i = 0; i < mapsPositions.NumElements(); i++)
	{
		iStart = i;
		psStart = *((SFloat3**)mapsPositions.Get(iStart));

		if (iStart != mapsPositions.NumElements()-1)
			iEnd = iStart+1;
		else
			iEnd = 0;
		psEnd = *((SFloat3**)mapsPositions.Get(iEnd));

		//Now we have the start and end of the line.
		sVector = *psStart - *psEnd;
		Float3Normalize(&sVector, &sVector);
		Float3Cross(&sNormal, mpsNormal, &sVector);

		cHalfSpace.Init(&sNormal);
		cHalfSpace.Set(psStart, &sNormal);

		if (cHalfSpace.NotContains(psPosition))
		{
			piLastIndex = pcVisibleIndices->Tail();
			if (piLastIndex)
			{
				if (*piLastIndex == iStart)
				{
					pcVisibleIndices->Add(iStart);
					pcVisibleIndices->Add(iEnd);
				}
				else
				{
					pcVisibleIndices->InsertAt(iStart, iElementPos);
					pcVisibleIndices->InsertAt(iEnd, iElementPos+1);
					iElementPos+=2;
				}
			}
			else
			{
				pcVisibleIndices->Add(iStart);
				pcVisibleIndices->Add(iEnd);
			}
		}
	}
}

