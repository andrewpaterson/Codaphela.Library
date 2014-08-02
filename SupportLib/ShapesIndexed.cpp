/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "ShapesIndexed.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/PointerRemapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereIndexed::Init(void)
{
	miPosition = -1;
	mfRadius = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSphereIndexed::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(miPosition));
	ReturnOnFalse(pcFile->WriteFloat(mfRadius));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSphereIndexed::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt(&miPosition));
	ReturnOnFalse(pcFile->ReadFloat(&mfRadius));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSphereIndexed::Shape(CSphere* pcShape, SFloat3* psVertexArray)
{
	if (miPosition != -1)
	{
		pcShape->Init(&psVertexArray[miPosition]);
		pcShape->mfRadius = mfRadius;
		return TRUE;
	}
	else
	{
		pcShape->mpsPosition = NULL;
		pcShape->mfRadius = 0.0f;
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangleIndexed::Init(void)
{
	miNormalIndex = -1;
	maiPositions[0] = -1;
	maiPositions[1] = -1;
	maiPositions[2] = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangleIndexed::Init(int p1, int p2, int p3, int n)
{
	miNormalIndex = n;
	maiPositions[0] = p1;
	maiPositions[1] = p2;
	maiPositions[2] = p3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTriangleIndexed::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteIntArray(maiPositions, 3));
	ReturnOnFalse(pcFile->WriteInt(miNormalIndex));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTriangleIndexed::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadIntArray(maiPositions, 3));
	ReturnOnFalse(pcFile->ReadInt(&miNormalIndex));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTriangleIndexed::Shape(CTriangle* pcShape, SFloat3* psVertexArray, SFloat3* psNormalArray)
{
	SFloat3*	psNormal;

	if ((maiPositions[0] != -1) && (maiPositions[1] != -1) && (maiPositions[2] != -1) && (miNormalIndex != -1))
	{
		if (psNormalArray)
		{
			psNormal = &psNormalArray[miNormalIndex];
		}
		else
		{
			psNormal = NULL;
		}
		pcShape->Init(&psVertexArray[maiPositions[0]], &psVertexArray[maiPositions[1]], &psVertexArray[maiPositions[2]], psNormal);
		pcShape->Set();
		return TRUE;
	}
	else
	{
		pcShape->mpsPosition = NULL;
		pcShape->mpsPosition1 = NULL;
		pcShape->mpsPosition2 = NULL;
		pcShape->mpsNormal = NULL;
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxIndexed::Init(void)
{
	Init(-1, -1, 0.0f, -1, 0.0f, -1, 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxIndexed::Init(int iCenterIndex, int iLongAxisIndex, float fLongAxisLength, int iMiddleAxisIndex, float fMiddleAxisLength, int iShortAxisIndex, float fShortAxisLength)
{
	miCenterIndex = iCenterIndex;
	miLongAxisIndex = iLongAxisIndex;
	miMiddleAxisIndex = iMiddleAxisIndex;
	miShortAxisIndex = iShortAxisIndex;

	mfLongAxisLength = fLongAxisLength;
	mfMiddleAxisLength = fMiddleAxisLength;
	mfShortAxisLength = fShortAxisLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBoxIndexed::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(miCenterIndex));
	ReturnOnFalse(pcFile->WriteInt(miLongAxisIndex));
	ReturnOnFalse(pcFile->WriteInt(miMiddleAxisIndex));
	ReturnOnFalse(pcFile->WriteInt(miShortAxisIndex));
	ReturnOnFalse(pcFile->WriteFloat(mfLongAxisLength));
	ReturnOnFalse(pcFile->WriteFloat(mfMiddleAxisLength));
	ReturnOnFalse(pcFile->WriteFloat(mfShortAxisLength));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBoxIndexed::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt(&miCenterIndex));
	ReturnOnFalse(pcFile->ReadInt(&miLongAxisIndex));
	ReturnOnFalse(pcFile->ReadInt(&miMiddleAxisIndex));
	ReturnOnFalse(pcFile->ReadInt(&miShortAxisIndex));
	ReturnOnFalse(pcFile->ReadFloat(&mfLongAxisLength));
	ReturnOnFalse(pcFile->ReadFloat(&mfMiddleAxisLength));
	ReturnOnFalse(pcFile->ReadFloat(&mfShortAxisLength));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygonIndexed::Init(void)
{
	maiPositions.Init();
	mbConvex = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPolygonIndexed::Kill(void)
{
	maiPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygonIndexed::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(maiPositions.Write(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPolygonIndexed::Load(CFileReader* pcFile)
{
	ReturnErrorOnFalse(maiPositions.Read(pcFile));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullIndexed::Init(void)
{
	mcPolygons.Init(4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullIndexed::Init(int iNumPolygons)
{
	int					i;
	CPolygonIndexed*	pcPolygon;

	mcPolygons.Init(1);
	mcPolygons.SetUsedElements(iNumPolygons);

	for (i = 0; i < iNumPolygons; i++)
	{
		pcPolygon = mcPolygons.Get(i);
		pcPolygon->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullIndexed::Kill(void)
{
	CPolygonIndexed*	pcPolygon;
	int					i;

	for (i = 0; i < mcPolygons.NumElements(); i++)
	{
		pcPolygon = mcPolygons.Get(i);
		pcPolygon->Kill();
	}

	mcPolygons.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullIndexed::Save(CFileWriter* pcFile)
{
	CPolygonIndexed*	pcPolygon;
	int					i;


	ReturnOnFalse(mcPolygons.WriteAllocatorAndHeader(pcFile));
	for (i = 0; i < mcPolygons.NumElements(); i++)
	{
		pcPolygon = mcPolygons.Get(i);
		ReturnOnFalse(pcPolygon->Save(pcFile));
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullIndexed::Load(CFileReader* pcFile)
{
	CPolygonIndexed*	pcPolygon;
	int					i;


	ReturnOnFalse(mcPolygons.ReadAllocatorAndHeader(pcFile));
	for (i = 0; i < mcPolygons.NumElements(); i++)
	{
		pcPolygon = mcPolygons.Get(i);
		ReturnOnFalse(pcPolygon->Load(pcFile));
	}

	return TRUE;
}

