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
#include "Mesh.h"
#include "MeshFaceTypes.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareMeshFaceTypes(const void* arg1, const void* arg2)
{
	SMeshFaceType*	sType1;
	SMeshFaceType*	sType2;

	sType1 = (SMeshFaceType*)arg1;
	sType2 = (SMeshFaceType*)arg2;

	if (sType1->iFlags1 < sType2->iFlags1)
	{
		return -1;
	}
	else if (sType1->iFlags1 > sType2->iFlags1)
	{
		return 1;
	}

	if (sType1->iFlags2 < sType2->iFlags2)
	{
		return -1;
	}
	else if (sType1->iFlags2 > sType2->iFlags2)
	{
		return 1;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMeshFaceType::Clear(void)
{
	iFlags1 = 0;
	iFlags2 = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SMeshFaceType::Equals(SMeshFaceType* psOther)
{
	return (iFlags1 == psOther->iFlags1) && (iFlags2 == psOther->iFlags2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SMeshFaceType::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadBits(&iFlags1, sizeof(unsigned int)));
	ReturnOnFalse(pcFile->ReadBits(&iFlags2, sizeof(unsigned int)));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SMeshFaceType::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteBits(&iFlags1, sizeof(unsigned int)));
	ReturnOnFalse(pcFile->WriteBits(&iFlags2, sizeof(unsigned int)));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SMeshFaceType::IsNormal(void) { return FixBool(iFlags1 & MESH_FACE_NORMAL_FLAG1); }
bool SMeshFaceType::IsColour(void) { return FixBool(iFlags1 & MESH_FACE_COLOUR_FLAG1); }
int SMeshFaceType::GetUVs(void) { return (iFlags1 & MESH_FACE_UV_MASK1) >> MESH_FACE_UV_SHIFT; }
int SMeshFaceType::GetMatricies(void) { return (iFlags1 & MESH_FACE_MATRIX_MASK1) >> MESH_FACE_MATRIX_SHIFT; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SMeshFaceType::GetMaterial(void) 
{ 
	if ((iFlags2 & MESH_FACE_MATERIAL_MASK2) == MESH_FACE_MATERIAL_MASK2)
	{
		return -1;
	}
	else
	{
		return (iFlags2 & MESH_FACE_MATERIAL_MASK2) >> MESH_FACE_MATERIAL_SHIFT; 
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::Init(void)
{
	CMeshDetail::Init();
	mcFaces.Init();
	mcUniqueTypes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::Kill(void)
{
	mcUniqueTypes.Kill();
	mcFaces.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshFaceTypes::Load(CFileReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(mcFaces.Read(pcFile));
	ReturnOnFalse(mcUniqueTypes.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshFaceTypes::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(mcFaces.Write(pcFile));
	ReturnOnFalse(mcUniqueTypes.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::ClearFaceTypes(int iNumFaces)
{
	int				i;
	SMeshFaceType*	psType;

	mcFaces.ReInit();
	mcFaces.Resize(iNumFaces);

	for (i = 0; i < iNumFaces; i++)
	{
		psType = mcFaces.Get(i);
		psType->Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GenerateFromNormals(CMeshNormals* pcNormals)
{
	int					i;
	SMeshFaceType*		psType;
	SMeshNormalFace*	psFaceNormals;
	int					iNumFaces;

	if (pcNormals->InUse())
	{
		iNumFaces = mcFaces.NumElements();
		for (i = 0; i < iNumFaces; i++)
		{
			psFaceNormals = pcNormals->mcFaces.Get(i);
			psType = mcFaces.Get(i);
			
			if ((psFaceNormals->aiCornerNormals[0] != -1) && 
				(psFaceNormals->aiCornerNormals[1] != -1) &&
				(psFaceNormals->aiCornerNormals[2] != -1))
			{
				psType->iFlags1 |= MESH_FACE_NORMAL_FLAG1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GenerateFromSkin(CMesh* pcMesh)
{
	int					i;
	SMeshFaceType*		psType;
	CMeshFace*			pcFace;
	int					iNumFaces;
	int					iMatricies;
	CMeshSkin*			pcSkin;

	pcSkin = &pcMesh->mcSkin;

	iNumFaces = mcFaces.NumElements();
	if (pcSkin->InUse())
	{
		for (i = 0; i < iNumFaces; i++)
		{
			pcFace = pcMesh->GetFace(i);
			psType = mcFaces.Get(i);

			iMatricies = pcFace->asCorner[0] = pcSkin->GetFaceVertexMaxMatrixCount(pcFace->asCorner[0], pcFace->asCorner[1], pcFace->asCorner[0]);
			psType->iFlags1 |= MESH_FACE_MATRIX_MASK1 & (iMatricies << MESH_FACE_MATRIX_SHIFT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GenerateFromUVCoords(CMeshUVCoords* pcUVs)
{
	int					i;
	SMeshFaceType*		psType;
	CMeshUVLayer*		pcUVLayer;
	int					j;
	SMeshUVFace*		psFaceUVs;
	int					iNumFaces;
	int					iNumUVs;

	if (pcUVs->InUse())
	{
		iNumFaces = mcFaces.NumElements();
		for (i = 0; i < iNumFaces; i++)
		{
			psType = mcFaces.Get(i);

			iNumUVs = 0;
			for (j = 0; j < pcUVs->mcLayers.NumElements(); j++)
			{
				pcUVLayer = pcUVs->mcLayers.Get(j);
				psFaceUVs = pcUVLayer->mcFaces.Get(i);

				if ((psFaceUVs->aiUVs[0] != -1) &&
					(psFaceUVs->aiUVs[1] != -1) &&
					(psFaceUVs->aiUVs[2] != -1))
				{
					iNumUVs++;
				}
			}
			psType->iFlags1 |= MESH_FACE_UV_MASK1 & (iNumUVs << MESH_FACE_UV_SHIFT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GenerateFromColours(CMeshColours* pcColours)
{
	int					i;
	SMeshFaceType*		psType;
	SMeshFaceColour*	psFaceColour;
	int					iNumFaces;

	if (pcColours->InUse())
	{
		iNumFaces = mcFaces.NumElements();
		for (i = 0; i < iNumFaces; i++)
		{
			psFaceColour = pcColours->mcFaces.Get(i);
			psType = mcFaces.Get(i);

			if ((psFaceColour->aiCornerColours[0] != -1) && 
				(psFaceColour->aiCornerColours[1] != -1) &&
				(psFaceColour->aiCornerColours[2] != -1))
			{
				psType->iFlags1 |= MESH_FACE_COLOUR_FLAG1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GenerateFromMaterals(CMeshMaterials* pcMaterials)
{
	int					i;
	SMeshFaceType*		psType;
	int					iMaterial;
	int					iNumFaces;

	if (pcMaterials->InUse())
	{
		iNumFaces = mcFaces.NumElements();
		for (i = 0; i < iNumFaces; i++)
		{
			iMaterial = pcMaterials->mcFaces.GetValue(i);
			psType = mcFaces.Get(i);

			//iMaterial is the local material, not the index into the Material Tracker.
			//iMaterial may be -1.
			psType->iFlags2 |= MESH_FACE_MATERIAL_MASK2 & (iMaterial << MESH_FACE_MATERIAL_SHIFT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::UpdateUniquesSkin(void)
{
	int				i;
	SMeshFaceType*	psUniqueType;
	SMeshFaceType*	psFaceType;
	int				iMaxMatricies;
	int				j;
	int				iNumFaces;

	iNumFaces = mcFaces.NumElements();
	for (i = 0; i < mcUniqueTypes.NumElements(); i++)
	{
		psUniqueType = mcUniqueTypes.Get(i);
		iMaxMatricies = 0;
		for (j = 0; j < iNumFaces; j++)
		{
			psFaceType = mcFaces.Get(j);
			if (((psFaceType->iFlags1 & (~MESH_FACE_MATRIX_MASK1)) == psUniqueType->iFlags1) &&
				(psFaceType->iFlags2 == psUniqueType->iFlags2))
			{
				if (iMaxMatricies < psFaceType->GetMatricies())
				{
					iMaxMatricies = psFaceType->GetMatricies();
				}
			}
		}

		psUniqueType->iFlags1 |= MESH_FACE_MATRIX_MASK1 & (iMaxMatricies << MESH_FACE_MATRIX_SHIFT);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GenerateUniques(void)
{
	int				i;
	int				iNumFaces;
	SMeshFaceType*	psType;

	mcUniqueTypes.ReInit();

	iNumFaces = mcFaces.NumElements();
	for (i = 0; i < iNumFaces; i++)
	{
		psType = mcFaces.Get(i);
		mcUniqueTypes.InsertIntoSorted(&CompareMeshFaceTypes, psType, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::Touch(CMesh* pcMesh)
{
	ClearFaceTypes(pcMesh->NumFaces());

	GenerateFromNormals(&pcMesh->mcNormals);
	GenerateFromColours(&pcMesh->mcColours);
	GenerateFromUVCoords(&pcMesh->mcUVs);
	GenerateFromMaterals(&pcMesh->mcMaterials);

	GenerateUniques();

	GenerateFromSkin(pcMesh);
	UpdateUniquesSkin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFaceTypes::GetFacesForType(SMeshFaceType* psFaceType, CArrayInt* paiFaceIndices)
{
	int				iFaceNum;
	SMeshFaceType*	psThisFaceType;

	for (iFaceNum = 0; iFaceNum < mcFaces.NumElements(); iFaceNum++)
	{
		psThisFaceType = mcFaces.Get(iFaceNum);

		if (psThisFaceType->Equals(psFaceType))
		{
			paiFaceIndices->Add(iFaceNum);
		}
	}

	//This list is sorted.
}


