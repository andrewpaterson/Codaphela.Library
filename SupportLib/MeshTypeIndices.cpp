/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/SystemAllocator.h"
#include "MeshTypeIndices.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMeshFaceIndex::Init(int iFaceIndex)
{
	this->iFaceIndex = iFaceIndex;
	iVertIndex[0] = -1;
	iVertIndex[1] = -1;
	iVertIndex[2] = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTypeIndices::Init(CMeshVertexArray* pcVertexArray, int iStride, int iNormalOffset, int iColourOffset, int iUVOffset, int iSkinOffset)
{
	mpcVertexArray = pcVertexArray;
	miStride = iStride;
	maVerts.Init(iStride);

	//miPositionOffset = 0;  //Always.
	//All of the offsets are indexed as integer arrays but passed in as bytes.

	miNormalOffset = iNormalOffset;
	if (miNormalOffset != -1)
	{
		miNormalOffset /= sizeof(int);
	}

	miColourOffset = iColourOffset;
	if (miColourOffset != -1)
	{
		miColourOffset /= sizeof(int);
	}

	miUVOffset = iUVOffset;
	if (miUVOffset != -1)
	{
		miUVOffset /= sizeof(int);
	}

	miSkinOffset = iSkinOffset;
	if (miSkinOffset != -1)
	{
		miSkinOffset /= sizeof(int);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTypeIndices::Kill(void)
{
	mpcVertexArray = NULL;
	maVerts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshTypeIndices::Generate(CArrayInt* paiFaces, void* pavFaceIndices)
{
	int						i;
	int						iNumFaces;
	SMeshFaceIndex*			psFaceIndex;
	void*					pvFaceVertIndex1;
	void*					pvFaceVertIndex2;
	void*					pvFaceVertIndex3;
	CArrayMeshFaceIndex*	pcFaceIndicies;

	iNumFaces = paiFaces->NumElements();

	pcFaceIndicies = mpcVertexArray->GetFaceIndicies();
	pcFaceIndicies->Kill();
	pcFaceIndicies->Init(); 
	pcFaceIndicies->Resize(iNumFaces);

	maVerts.Kill();
	maVerts.Init(miStride);

	for (i = 0; i < iNumFaces; i++)
	{
		psFaceIndex = pcFaceIndicies->Get(i);
		psFaceIndex->Init(paiFaces->GetValue(i));
	}

	for (i = 0; i < iNumFaces; i++)
	{
		psFaceIndex = pcFaceIndicies->Get(i);

		pvFaceVertIndex1 = RemapSinglePointer(pavFaceIndices, miStride * 3 * i);
		pvFaceVertIndex2 = RemapSinglePointer(pvFaceVertIndex1, miStride);
		pvFaceVertIndex3 = RemapSinglePointer(pvFaceVertIndex2, miStride);

		SetVertIndex(&psFaceIndex->iVertIndex[0], pvFaceVertIndex1);
		SetVertIndex(&psFaceIndex->iVertIndex[1], pvFaceVertIndex2);
		SetVertIndex(&psFaceIndex->iVertIndex[2], pvFaceVertIndex3);
	}

	maVerts.Finalise();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTypeIndices::SetVertIndex(int* piVertIndex, void* pvFaceVertIndex)
{
	int					iVertIndex;

	if (*piVertIndex == -1)
	{
		iVertIndex = FindExistingVert((int*)pvFaceVertIndex);
		if (iVertIndex == -1)
		{
			iVertIndex = maVerts.NumElements();
			maVerts.Add(pvFaceVertIndex);
		}
		*piVertIndex = iVertIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::GetNormalIndex(int iVert)
{
	int*	piVert;

	piVert = (int*)maVerts.Get(iVert);
	return piVert[miNormalOffset];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::GetPositionIndex(int iVert)
{
	int*	piVert;

	piVert = (int*)maVerts.Get(iVert);
	return *piVert;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::GetColourIndex(int iVert)
{
	int*	piVert;

	piVert = (int*)maVerts.Get(iVert);
	return piVert[miColourOffset];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::GetUVIndex(int iVert, int iLayer)
{
	int*	piVert;

	piVert = (int*)maVerts.Get(iVert);
	return piVert[miUVOffset + iLayer];
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::GetSkinIndex(int iVert)
{
	int*	piVert;

	piVert = (int*)maVerts.Get(iVert);
	return *piVert;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::FindExistingVert(int* piFaceVertIndex)
{
	//Optimise this by creating an array of size mcPosition.NumElements of index into an array of ArrayInt (per position) indexing into maVerts.
	int		i;
	int*	piVert;

	for (i = 0; i < maVerts.NumElements(); i++)
	{
		piVert = (int*)maVerts.Get(i);

		//Test if position indices are the same.  No need to check further if they're not.
		if (*piVert == *piFaceVertIndex)
		{
			if (memcmp(piVert, piFaceVertIndex, miStride) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTypeIndices::NumVerts(void)
{
	return maVerts.NumElements();
}

