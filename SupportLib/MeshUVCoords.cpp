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
#include "MeshDefines.h"
#include "MeshConnectivity.h"
#include "MeshUVCoords.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVLayer::Init(void)
{
	mcUVs.Init();
	mcFaces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVLayer::Kill(void)
{
	mcFaces.Kill();
	mcUVs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshUVLayer::AddFace(int iUV1, int iUV2, int iUV3)
{
	SMeshUVFace*	psUVFace;
	int				iFaceNum;

	iFaceNum = mcFaces.NumElements();
	psUVFace = mcFaces.Add();
	psUVFace->aiUVs[0] = iUV1;
	psUVFace->aiUVs[1] = iUV2;
	psUVFace->aiUVs[2] = iUV3;

	return iFaceNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshUVLayer::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcUVs.Read(pcFile));
	ReturnOnFalse(mcFaces.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshUVLayer::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(mcUVs.Write(pcFile));
	ReturnOnFalse(mcFaces.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVCoords::Init(void)
{
	CMeshDetail::Init();
	mcLayers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVCoords::ReInit(void)
{
	mcLayers.Kill();
	mcLayers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVCoords::Kill(void)
{
	CMeshUVLayer*	pcLayer;
	size			i;

	for (i = 0; i < mcLayers.NumElements(); i++)
	{
		pcLayer = mcLayers.Get(i);
		pcLayer->Kill();
	}
	mcLayers.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshUVCoords::Load(CFileReader* pcFile)
{
	CMeshUVLayer*	pcLayer;
	int				i;
	int				iNumLayers;

	Init();

	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(pcFile->ReadInt(&iNumLayers));

	mcLayers.Resize(iNumLayers);

	for (i = 0; i < iNumLayers; i++)
	{
		pcLayer = mcLayers.Get(i);
		ReturnOnFalse(pcLayer->Load(pcFile));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshUVCoords::Save(CFileWriter* pcFile)
{
	CMeshUVLayer*	pcUVLayer;
	size			i;

	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(pcFile->WriteInt(mcLayers.NumElements()));

	for (i = 0; i < mcLayers.NumElements(); i++)
	{
		pcUVLayer = mcLayers.Get(i);
		ReturnOnFalse(pcUVLayer->Save(pcFile));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshUVCoords::AddLayer(CMeshConnectivity* pcConn)
{
	CMeshUVLayer*	pcUVLayer;
	size			i;

	if (mbInUse)
	{
		pcUVLayer = mcLayers.Add();
		pcUVLayer->Init();

		for (i = 0; i < pcConn->mcFaces.NumElements(); i++)
		{
			pcUVLayer->AddFace(-1, -1, -1);
		}
		return mcLayers.NumElements() - 1;
	}
	else
	{
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshUVLayer* CMeshUVCoords::EnsureLayerExists(size iLayer, CMeshConnectivity* pcConn)
{
	size	i;

	if (mbInUse)
	{
		if (iLayer < mcLayers.NumElements())
		{
			return mcLayers.Get(iLayer);
		}
		else
		{
			for (i = mcLayers.NumElements(); i <= iLayer; i++)
			{
				AddLayer(pcConn);
			}
			return mcLayers.Get(iLayer);
		}
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVCoords::AddFace(void)
{	
	CMeshUVLayer*	pcUVLayer;
	size			i;

	if (mbInUse)
	{
		for (i = 0; i < mcLayers.NumElements(); i++)
		{
			pcUVLayer = mcLayers.Get(i);
			pcUVLayer->AddFace(-1, -1, -1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUVCoords::IndexFaceCorners(int* paiDest, ptrdiff iStride, CArrayInt* paiSourceFaces, size iNumLayers)
{
	size				i;
	int					iFaceIndex;
	SMeshUVFace*		psFaceUV;
	size				j;
	CMeshUVLayer*		pcUVLayer;

	for (i = 0; i < paiSourceFaces->NumElements(); i++)
	{
		iFaceIndex = paiSourceFaces->GetValue(i);
		for (j = 0; j < iNumLayers; j++)
		{
			pcUVLayer = mcLayers.Get(j);
			psFaceUV = pcUVLayer->mcFaces.Get(iFaceIndex);

			paiDest[j] = psFaceUV->aiUVs[0];
		}
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		for (j = 0; j < iNumLayers; j++)
		{
			pcUVLayer = mcLayers.Get(j);
			psFaceUV = pcUVLayer->mcFaces.Get(iFaceIndex);

			paiDest[j] = psFaceUV->aiUVs[1];
		}
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		for (j = 0; j < iNumLayers; j++)
		{
			pcUVLayer = mcLayers.Get(j);
			psFaceUV = pcUVLayer->mcFaces.Get(iFaceIndex);

			paiDest[j] = psFaceUV->aiUVs[2];
		}
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshUVCoords::NumUVs(void)
{
	CMeshUVLayer*	pcUVLayer;
	size			i;
	size			iNumUVs;

	if (mbInUse)
	{
		iNumUVs = 0;
		for (i = 0; i < mcLayers.NumElements(); i++)
		{
			pcUVLayer = mcLayers.Get(i);
			iNumUVs += pcUVLayer->mcUVs.NumElements();
		}
		return iNumUVs;
	}
	else
	{
		return 0;
	}
}

