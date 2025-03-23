/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/ArrayInt.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshDefines.h"
#include "MeshConnectivity.h"
#include "MeshSkin.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareSkinWeight(const void* arg1, const void* arg2)
{
	SSkinWeight*	ps1;
	SSkinWeight*	ps2;

	ps1 = (SSkinWeight*)arg1;
	ps2 = (SSkinWeight*)arg2;

	if (ps1->fWeight < ps2->fWeight)
	{
		return 1;
	}
	if (ps1->fWeight > ps2->fWeight)
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkinVert::Init(void)
{
	mcWeights.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkinVert::Kill(void)
{
	mcWeights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSkinVert::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcWeights.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSkinVert::Save(CFileWriter* pcFile)
{
	return mcWeights.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkinVert::SortSkinWeights(void)
{
	int				j;
	SSkinWeight*	psWeight;

	for (j = mcWeights.NumElements(); j >= 0; j--)
	{
		psWeight = mcWeights.Get(j);
		if (psWeight->fWeight == 0.0f)
		{
			mcWeights.RemoveAt(j);
		}
	}

	mcWeights.QuickSort(&CompareSkinWeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mcSkinVerts.Init();
	mcInverseSkinMatricies.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::Free(void)
{
	KillSkinVerts();
	mcInverseSkinMatricies.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::KillSkinVerts(void)
{
	CMeshSkinVert*	psSkinVert;
	size			i;

	for (i = 0; i < mcSkinVerts.NumElements(); i++)
	{
		psSkinVert = mcSkinVerts.Get(i);
		psSkinVert->Kill();
	}
	mcSkinVerts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::Class(void)
{
	CMeshDetail::Class();

	U_Unknown(CArrayMeshSkinVert, mcSkinVerts);
	U_Unknown(CArrayFloat4x4, mcInverseSkinMatricies);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::Clear(void)
{
	KillSkinVerts();
	mcSkinVerts.Init();

	mcInverseSkinMatricies.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSkin::Load(CObjectReader* pcFile)
{
	CMeshSkinVert*	psSkinVert;
	size			i;

	Init();

	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(mcSkinVerts.ReadAllocatorAndHeader(pcFile));

	for (i = 0; i < mcSkinVerts.NumElements(); i++)
	{
		psSkinVert = mcSkinVerts.Get(i);
		ReturnOnFalse(psSkinVert->Load(pcFile));
	}

	ReturnOnFalse(mcInverseSkinMatricies.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSkin::Save(CObjectWriter* pcFile)
{
	CMeshSkinVert*	psSkinVert;
	size			i;

	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(mcSkinVerts.WriteAllocatorAndHeader(pcFile));

	for (i = 0; i < mcSkinVerts.NumElements(); i++)
	{
		psSkinVert = mcSkinVerts.Get(i);
		ReturnOnFalse(psSkinVert->Save(pcFile));
	}

	ReturnOnFalse(mcInverseSkinMatricies.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::Touch(void)
{
	CMeshSkinVert*	pcSkinVert;
	size			i;

	if (mbInUse)
	{
		for (i = 0; i < mcSkinVerts.NumElements(); i++)
		{
			pcSkinVert = mcSkinVerts.Get(i);
			pcSkinVert->SortSkinWeights();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::AddCorner(void)
{
	CMeshSkinVert*	pcSkinVert;

	if (mbInUse)
	{
		pcSkinVert = mcSkinVerts.Add();
		pcSkinVert->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSkin::GetFaceMatrixCount(int iVert1, int iVert2, int iVert3)
{
	CMeshSkinVert*	pcVert;
	CArrayInt		aiMatricies;
	int				iNumMatricies;

	aiMatricies.Init();

	pcVert = mcSkinVerts.Get(iVert1);
	PrivateAddMatricies(pcVert, &aiMatricies);

	pcVert = mcSkinVerts.Get(iVert2);
	PrivateAddMatricies(pcVert, &aiMatricies);

	pcVert = mcSkinVerts.Get(iVert3);
	PrivateAddMatricies(pcVert, &aiMatricies);

	aiMatricies.QuickSort();
	iNumMatricies = aiMatricies.NumElements();
	aiMatricies.Kill();

	return iNumMatricies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::PrivateAddMatricies(CMeshSkinVert* pcVert, CArrayInt* paiMatricies)
{
	size			i;
	SSkinWeight*	psWeight;

	for (i = 0; i < pcVert->mcWeights.NumElements(); i++)
	{
		psWeight = pcVert->mcWeights.Get(i);
		paiMatricies->AddIfUnique(psWeight->iMatrix);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSkin::GetFaceVertexMaxMatrixCount(int iVert1, int iVert2, int iVert3)
{
	CMeshSkinVert*	pcVert;
	size			iNumMatricies;

	pcVert = mcSkinVerts.Get(iVert1);
	iNumMatricies = pcVert->mcWeights.NumElements();

	pcVert = mcSkinVerts.Get(iVert2);
	if (pcVert->mcWeights.NumElements() > iNumMatricies)
	{
		iNumMatricies = pcVert->mcWeights.NumElements();
	}

	pcVert = mcSkinVerts.Get(iVert3);
	if (pcVert->mcWeights.NumElements() > iNumMatricies)
	{
		iNumMatricies = pcVert->mcWeights.NumElements();
	}

	return iNumMatricies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSkin::IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces, CMeshConnectivity* pcConn)
{
	size		i;
	size		iFaceIndex;
	CMeshFace*	pcFace;

	if (mbInUse)
	{
		for (i = 0; i < paiSourceFaces->NumElements(); i++)
		{
			iFaceIndex = paiSourceFaces->GetValue(i);
			pcFace = pcConn->mcFaces.Get(iFaceIndex);

			*paiDest = pcFace->asCorner[0];
			paiDest = (int*)RemapSinglePointer(paiDest, iStride);

			*paiDest = pcFace->asCorner[1];
			paiDest = (int*)RemapSinglePointer(paiDest, iStride);

			*paiDest = pcFace->asCorner[2];
			paiDest = (int*)RemapSinglePointer(paiDest, iStride);
		}
	}
}

