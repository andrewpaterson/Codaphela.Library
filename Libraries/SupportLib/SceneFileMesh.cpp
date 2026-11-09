/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Logger.h"
#include "MeshDefines.h"
#include "MeshPolygonShape.h"
#include "MeshSmoothGroups.h"
#include "SceneFileMesh.h"


typedef CArrayTemplatePrimitive<float32>	CArraySceneFileFloat;
typedef CArrayTemplatePrimitive<uint32>		CArraySceneFileUint;


//A corner of a polygon being written, as a corner of one of the mesh's faces.
struct SSceneFileCorner
{
	int		iFace;
	int		iFaceCorner;
};


typedef CArrayTemplate<SSceneFileCorner>	CArraySceneFileCorner;


//How a mesh's faces came from a file's polygons: which of the file's corners each corner of each face is, and which polygon each face is of.
struct SSceneFileFaces
{
	CArraySceneFileIndex	aiCorners;
	CArraySceneFileIndex	aiPolygons;
};


//What's going in a file for a mesh: its polygons, with what's written for each.
struct SSceneFilePolygons
{
	CMesh*					pcMesh;
	CMeshSmoothGroups*		pcGroups;
	CArraySceneFileIndex	aiSizes;
	CArraySceneFileIndex	aiFirstFaces;
	CArraySceneFileCorner	asCorners;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileFaces(CSceneFileReader* pcReader, CMeshEditor* pcEditor, SSceneFileFaces* psFaces)
{
	CArraySceneFileIndex		aiSizes;
	CArraySceneFileIndex		aiPositions;
	CArrayMeshPolygonCorners	aiFaceCorners;
	size						uiNumPolygons;
	size						uiCorner;
	size						i;
	size						j;
	int32						iSize;
	int32						aiFile[NUM_FACE_CORNERS];
	const int32*				paiPolygon;

	uiNumPolygons = pcReader->NumPolygons();
	if (uiNumPolygons == 0)
	{
		return true;
	}

	aiSizes.Init();
	aiPositions.Init();
	aiSizes.Resize(uiNumPolygons);
	aiPositions.Resize(pcReader->NumCorners());
	ReturnOnFalse(pcReader->ReadPolygonSizes(aiSizes.GetData()));
	ReturnOnFalse(pcReader->ReadCornerPositions(aiPositions.GetData()));

	uiCorner = 0;
	for (i = 0; i < uiNumPolygons; i++)
	{
		iSize = aiSizes.GetValue(i);
		paiPolygon = aiPositions.GetData() + uiCorner;

		aiFaceCorners.Init();
		if (iSize == NUM_FACE_CORNERS)
		{
			aiFaceCorners.Add(0);
			aiFaceCorners.Add(1);
			aiFaceCorners.Add(2);
		}
		else
		{
			CutMeshPolygon(&pcEditor->mpMesh, paiPolygon, iSize, &aiFaceCorners);
		}

		for (j = 0; j < aiFaceCorners.NumElements(); j += NUM_FACE_CORNERS)
		{
			aiFile[0] = aiFaceCorners.GetValue(j);
			aiFile[1] = aiFaceCorners.GetValue(j + 1);
			aiFile[2] = aiFaceCorners.GetValue(j + 2);
			pcEditor->AddFace(paiPolygon[aiFile[0]], paiPolygon[aiFile[1]], paiPolygon[aiFile[2]], 0,
							  IsMeshPolygonEdge(aiFile[0], aiFile[1], iSize), IsMeshPolygonEdge(aiFile[1], aiFile[2], iSize), IsMeshPolygonEdge(aiFile[2], aiFile[0], iSize));
			psFaces->aiCorners.Add((int32)uiCorner + aiFile[0]);
			psFaces->aiCorners.Add((int32)uiCorner + aiFile[1]);
			psFaces->aiCorners.Add((int32)uiCorner + aiFile[2]);
			psFaces->aiPolygons.Add((int32)i);
		}
		aiFaceCorners.Kill();

		uiCorner += (size)iSize;
	}

	aiPositions.Kill();
	aiSizes.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh's flat normals, one for each face, come before the normals from
// the file.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileNormals(CSceneFileReader* pcReader, CMesh* pcMesh, SSceneFileFaces* psFaces)
{
	CArraySceneFileFloat	afNormals;
	CArraySceneFileIndex	aiCorners;
	SFloat32Vec3*			psNormal;
	SMeshNormalFace*		psFace;
	size					i;
	int						k;
	int						iNumFlat;
	int32					iNormal;

	if (pcReader->NumCornerNormals() == 0)
	{
		return true;
	}

	afNormals.Init();
	aiCorners.Init();
	afNormals.Resize(pcReader->NumNormals() * 3);
	aiCorners.Resize(pcReader->NumCornerNormals());
	if (afNormals.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadNormals(afNormals.GetData()));
	}
	ReturnOnFalse(pcReader->ReadCornerNormals(aiCorners.GetData()));

	for (i = 0; i < afNormals.NumElements(); i += 3)
	{
		psNormal = pcMesh->mcNormals.mcNormals.Add();
		psNormal->Init(afNormals.GetValue(i), afNormals.GetValue(i + 1), afNormals.GetValue(i + 2));
	}

	iNumFlat = pcMesh->NumFaces();
	for (i = 0; i < (size)pcMesh->NumFaces(); i++)
	{
		psFace = pcMesh->mcNormals.mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			iNormal = aiCorners.GetValue((size)psFaces->aiCorners.GetValue(i * NUM_FACE_CORNERS + (size)k));
			if (iNormal != SCENE_FILE_NONE)
			{
				psFace->aiCornerNormals[k] = iNumFlat + iNormal;
			}
		}
	}

	afNormals.Kill();
	aiCorners.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileUVLayer(CSceneFileReader* pcReader, CMeshUVLayer* pcLayer, CMesh* pcMesh, SSceneFileFaces* psFaces)
{
	CArraySceneFileFloat	afUVs;
	CArraySceneFileIndex	aiCorners;
	SFloat32Vec2*			psUV;
	SMeshUVFace*			psFace;
	size					i;
	int						k;

	afUVs.Init();
	aiCorners.Init();
	afUVs.Resize(pcReader->NumUVs() * 2);
	aiCorners.Resize(pcReader->NumCornerUVs());
	if (afUVs.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadUVs(afUVs.GetData()));
	}
	if (aiCorners.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadCornerUVs(aiCorners.GetData()));
	}

	for (i = 0; i < afUVs.NumElements(); i += 2)
	{
		psUV = pcLayer->mcUVs.Add();
		psUV->Init(afUVs.GetValue(i), afUVs.GetValue(i + 1));
	}

	for (i = 0; (i < (size)pcMesh->NumFaces()) && (aiCorners.NumElements() > 0); i++)
	{
		psFace = pcLayer->mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psFace->aiUVs[k] = aiCorners.GetValue((size)psFaces->aiCorners.GetValue(i * NUM_FACE_CORNERS + (size)k));
		}
	}

	aiCorners.Kill();
	afUVs.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileColours(CSceneFileReader* pcReader, CMesh* pcMesh, SSceneFileFaces* psFaces)
{
	CArraySceneFileUint		auiColours;
	CArraySceneFileIndex	aiCorners;
	SMeshFaceColour*		psFace;
	size					i;
	int						k;

	auiColours.Init();
	aiCorners.Init();
	auiColours.Resize(pcReader->NumColours());
	aiCorners.Resize(pcReader->NumCornerColours());
	if (auiColours.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadColours(auiColours.GetData()));
	}
	if (aiCorners.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadCornerColours(aiCorners.GetData()));
	}

	for (i = 0; i < auiColours.NumElements(); i++)
	{
		pcMesh->mcColours.mcColours.Add(auiColours.GetValue(i));
	}
	for (i = 0; (i < (size)pcMesh->NumFaces()) && (aiCorners.NumElements() > 0); i++)
	{
		psFace = pcMesh->mcColours.mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psFace->aiCornerColours[k] = aiCorners.GetValue((size)psFaces->aiCorners.GetValue(i * NUM_FACE_CORNERS + (size)k));
		}
	}

	aiCorners.Kill();
	auiColours.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileMaterials(CSceneFileReader* pcReader, CMesh* pcMesh, SSceneFileFaces* psFaces, CArrayMaterial* papMaterials)
{
	CArraySceneFileIndex	aiMaterials;
	CArraySceneFileIndex	aiSlots;
	size					i;

	aiMaterials.Init();
	aiSlots.Init();
	aiMaterials.Resize(pcReader->NumMaterialSlots());
	aiSlots.Resize(pcReader->NumPolygonSlots());
	if (aiMaterials.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadMaterialSlots(aiMaterials.GetData()));
	}
	if (aiSlots.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadPolygonSlots(aiSlots.GetData()));
	}

	for (i = 0; i < aiMaterials.NumElements(); i++)
	{
		pcMesh->mcMaterials.mapMaterials.Add(papMaterials->Get((size)aiMaterials.GetValue(i)));
	}
	for (i = 0; (i < (size)pcMesh->NumFaces()) && (aiSlots.NumElements() > 0); i++)
	{
		pcMesh->mcMaterials.mcFaces.SetValue(i, aiSlots.GetValue((size)psFaces->aiPolygons.GetValue(i)));
	}

	aiSlots.Kill();
	aiMaterials.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileSkin(CSceneFileReader* pcReader, CMesh* pcMesh)
{
	CArraySceneFileIndex	aiCounts;
	CArraySceneFileIndex	aiMatrices;
	CArraySceneFileFloat	afWeights;
	CArraySceneFileFloat	afInverses;
	CMeshSkinVert*			pcVert;
	SSkinWeight*			psWeight;
	SFloat4x4*				psInverse;
	size					i;
	size					uiWeight;
	int32					j;

	aiCounts.Init();
	aiMatrices.Init();
	afWeights.Init();
	afInverses.Init();
	aiCounts.Resize(pcReader->NumSkinCounts());
	aiMatrices.Resize(pcReader->NumSkinWeights());
	afWeights.Resize(pcReader->NumSkinWeights());
	afInverses.Resize(pcReader->NumInverseMatrices() * SCENE_FILE_FLOATS_IN_MATRIX);
	if (aiCounts.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadSkinCounts(aiCounts.GetData()));
		ReturnOnFalse(pcReader->ReadSkinMatrices(aiMatrices.GetData()));
		ReturnOnFalse(pcReader->ReadSkinWeights(afWeights.GetData()));
	}
	if (afInverses.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadInverseMatrices(afInverses.GetData()));
	}

	uiWeight = 0;
	for (i = 0; i < aiCounts.NumElements(); i++)
	{
		pcVert = pcMesh->mcSkin.mcSkinVerts.Get(i);
		for (j = 0; j < aiCounts.GetValue(i); j++)
		{
			psWeight = pcVert->mcWeights.Add();
			psWeight->fWeight = afWeights.GetValue(uiWeight);
			psWeight->iMatrix = aiMatrices.GetValue(uiWeight);
			uiWeight++;
		}
		pcVert->SortSkinWeights();
	}
	for (i = 0; i < pcReader->NumInverseMatrices(); i++)
	{
		psInverse = pcMesh->mcSkin.mcInverseSkinMatricies.Add();
		memcpy(psInverse, afInverses.GetData() + i * SCENE_FILE_FLOATS_IN_MATRIX, sizeof(SFloat4x4));
	}

	afInverses.Kill();
	afWeights.Kill();
	aiMatrices.Kill();
	aiCounts.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSceneFileSmoothGroups(CSceneFileReader* pcReader, CMesh* pcMesh, CMeshSmoothGroups* pcGroups, SSceneFileFaces* psFaces)
{
	CArraySceneFileUint	auiGroups;
	size				i;

	auiGroups.Init();
	auiGroups.Resize(pcReader->NumSmoothGroups());
	ReturnOnFalse(pcReader->ReadSmoothGroups(auiGroups.GetData()));
	pcGroups->mcSmoothingGroups.Resize(pcMesh->NumFaces());
	for (i = 0; i < (size)pcMesh->NumFaces(); i++)
	{
		pcGroups->mcSmoothingGroups.SetValue(i, (int32)auiGroups.GetValue((size)psFaces->aiPolygons.GetValue(i)));
	}
	auiGroups.Kill();

	if (pcReader->NumCornerNormals() == 0)
	{
		pcGroups->GenerateNormals(pcMesh);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadSceneFileMesh(CSceneFileReader* pcReader, size uiMesh, CMeshEditor* pcEditor, CArrayMaterial* papMaterials)
{
	SSceneFileFaces			sFaces;
	CArraySceneFileFloat	afPositions;
	CMesh*					pcMesh;
	CMeshUVLayer*			pcLayer;
	CMeshSmoothGroups*		pcGroups;
	size					i;
	size					uiNumLayers;

	ReturnOnFalse(pcReader->BeginMesh(uiMesh));
	ReturnOnFalse(pcReader->ReadName(&pcEditor->mszName));

	pcMesh = &pcEditor->mpMesh;
	pcMesh->mcNormals.mbInUse = (pcReader->NumCornerNormals() > 0) || (pcReader->NumSmoothGroups() > 0);
	pcMesh->mcColours.mbInUse = pcReader->NumColours() > 0;
	pcMesh->mcMaterials.mbInUse = pcReader->NumMaterialSlots() > 0;
	pcMesh->mcSkin.mbInUse = pcReader->NumSkinCounts() > 0;
	uiNumLayers = pcReader->NumUVLayers();
	pcMesh->mcUVs.mbInUse = uiNumLayers > 0;
	for (i = 0; i < uiNumLayers; i++)
	{
		pcMesh->mcUVs.AddLayer(pcMesh->GetConnectivity());
	}

	pcGroups = NULL;
	if (pcReader->NumSmoothGroups() > 0)
	{
		pcGroups = pcEditor->AddModifier<CMeshSmoothGroups>();
		pcGroups->Init(0);
	}

	afPositions.Init();
	afPositions.Resize(pcReader->NumPositions() * 3);
	if (afPositions.NumElements() > 0)
	{
		ReturnOnFalse(pcReader->ReadPositions(afPositions.GetData()));
	}
	for (i = 0; i < afPositions.NumElements(); i += 3)
	{
		pcEditor->AddCorner(afPositions.GetValue(i), afPositions.GetValue(i + 1), afPositions.GetValue(i + 2));
	}
	afPositions.Kill();

	sFaces.aiCorners.Init();
	sFaces.aiPolygons.Init();
	ReturnOnFalse(ReadSceneFileFaces(pcReader, pcEditor, &sFaces));
	ReturnOnFalse(ReadSceneFileNormals(pcReader, pcMesh, &sFaces));
	for (i = 0; i < uiNumLayers; i++)
	{
		pcLayer = pcMesh->mcUVs.mcLayers.Get(i);
		ReturnOnFalse(pcReader->BeginUVLayer(i));
		ReturnOnFalse(ReadSceneFileUVLayer(pcReader, pcLayer, pcMesh, &sFaces));
		ReturnOnFalse(pcReader->End());
	}
	ReturnOnFalse(ReadSceneFileColours(pcReader, pcMesh, &sFaces));
	ReturnOnFalse(ReadSceneFileMaterials(pcReader, pcMesh, &sFaces, papMaterials));
	ReturnOnFalse(ReadSceneFileSkin(pcReader, pcMesh));
	if (pcGroups != NULL)
	{
		ReturnOnFalse(ReadSceneFileSmoothGroups(pcReader, pcMesh, pcGroups, &sFaces));
	}
	sFaces.aiPolygons.Kill();
	sFaces.aiCorners.Kill();

	return pcReader->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two corners of a mesh's faces can be one corner of a polygon.
//
//////////////////////////////////////////////////////////////////////////
static bool DoSceneFileCornersAgree(CMesh* pcMesh, SMeshPolygonEdge* psA, SMeshPolygonEdge* psB)
{
	CMeshUVLayer*	pcLayer;
	size			i;

	if (pcMesh->mcNormals.InUse() && (pcMesh->mcNormals.mcFaces.Get((size)psA->iFace)->aiCornerNormals[psA->iFaceCorner] != pcMesh->mcNormals.mcFaces.Get((size)psB->iFace)->aiCornerNormals[psB->iFaceCorner]))
	{
		return false;
	}
	if (pcMesh->mcColours.InUse() && (pcMesh->mcColours.mcFaces.Get((size)psA->iFace)->aiCornerColours[psA->iFaceCorner] != pcMesh->mcColours.mcFaces.Get((size)psB->iFace)->aiCornerColours[psB->iFaceCorner]))
	{
		return false;
	}
	for (i = 0; i < pcMesh->mcUVs.mcLayers.NumElements(); i++)
	{
		pcLayer = pcMesh->mcUVs.mcLayers.Get(i);
		if (pcLayer->mcFaces.Get((size)psA->iFace)->aiUVs[psA->iFaceCorner] != pcLayer->mcFaces.Get((size)psB->iFace)->aiUVs[psB->iFaceCorner])
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// False if a polygon's faces don't agree on what one polygon can only say
// once.
//
//////////////////////////////////////////////////////////////////////////
static bool DoSceneFileFacesAgree(SSceneFilePolygons* psPolygons, CArrayMeshPolygonEdge* pasEdges, CArrayMeshPolygonEdge* pasLoop)
{
	CMesh*				pcMesh;
	SMeshPolygonEdge*	psEdge;
	SMeshPolygonEdge*	psLoop;
	size				i;
	size				j;

	pcMesh = psPolygons->pcMesh;
	for (i = 0; i < pasEdges->NumElements(); i++)
	{
		psEdge = pasEdges->Get(i);
		if (pcMesh->mcMaterials.InUse() && (pcMesh->mcMaterials.mcFaces.GetValue((size)psEdge->iFace) != pcMesh->mcMaterials.mcFaces.GetValue((size)pasEdges->Get(0)->iFace)))
		{
			return false;
		}
		if ((psPolygons->pcGroups != NULL) && (psPolygons->pcGroups->mcSmoothingGroups.GetValue((size)psEdge->iFace) != psPolygons->pcGroups->mcSmoothingGroups.GetValue((size)pasEdges->Get(0)->iFace)))
		{
			return false;
		}
		for (j = 0; j < pasLoop->NumElements(); j++)
		{
			psLoop = pasLoop->Get(j);
			if ((psLoop->iFrom == psEdge->iFrom) && !DoSceneFileCornersAgree(pcMesh, psEdge, psLoop))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddSceneFileFace(SSceneFilePolygons* psPolygons, int iFace)
{
	SSceneFileCorner*	psCorner;
	int					k;

	psPolygons->aiSizes.Add(NUM_FACE_CORNERS);
	psPolygons->aiFirstFaces.Add(iFace);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		psCorner = psPolygons->asCorners.Add();
		psCorner->iFace = iFace;
		psCorner->iFaceCorner = k;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if one polygon can't say what its faces do, and then it's left to
// them.
//
//////////////////////////////////////////////////////////////////////////
static bool AddSceneFilePolygon(SSceneFilePolygons* psPolygons, CMeshPolygon* pcPolygon)
{
	CArrayMeshPolygonEdge	asEdges;
	CArrayMeshPolygonEdge	asLoop;
	SMeshPolygonEdge*		psLoop;
	SSceneFileCorner*		psCorner;
	size					i;
	bool					bResult;

	asEdges.Init();
	asLoop.Init();
	bResult = GetMeshPolygonLoop(psPolygons->pcMesh, pcPolygon, &asEdges, &asLoop);
	bResult = bResult && DoSceneFileFacesAgree(psPolygons, &asEdges, &asLoop);
	if (bResult)
	{
		psPolygons->aiSizes.Add((int32)asLoop.NumElements());
		psPolygons->aiFirstFaces.Add(asLoop.Get(0)->iFace);
		for (i = 0; i < asLoop.NumElements(); i++)
		{
			psLoop = asLoop.Get(i);
			psCorner = psPolygons->asCorners.Add();
			psCorner->iFace = psLoop->iFace;
			psCorner->iFaceCorner = psLoop->iFaceCorner;
		}
	}
	asLoop.Kill();
	asEdges.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A polygon goes where its first face would.
//
//////////////////////////////////////////////////////////////////////////
static void GatherSceneFilePolygons(SSceneFilePolygons* psPolygons, CMeshEditor* pcEditor)
{
	CMeshPolygons*			pcPolygons;
	CMeshPolygon*			pcPolygon;
	CArraySceneFileIndex	aiDone;  //For each polygon: -1 for not looked at yet, 1 if it was written whole, 0 if it's left to its faces.
	size					i;
	int						iPolygon;
	int						iNumFaces;

	pcPolygons = &pcEditor->mpMesh->mcPolygons;
	iNumFaces = psPolygons->pcMesh->NumFaces();
	aiDone.Init();
	aiDone.Resize(pcPolygons->mcPolygons.NumElements());
	for (i = 0; i < aiDone.NumElements(); i++)
	{
		aiDone.SetValue(i, -1);
	}

	for (i = 0; i < (size)iNumFaces; i++)
	{
		iPolygon = (i < pcPolygons->maiFacesToPolygons.NumElements()) ? pcPolygons->maiFacesToPolygons.GetValue(i) : -1;
		if (iPolygon == -1)
		{
			AddSceneFileFace(psPolygons, (int)i);
			continue;
		}
		pcPolygon = pcPolygons->mcPolygons.Get((size)iPolygon);
		if (pcPolygon->maiFaces.NumElements() == 1)
		{
			//A triangle keeps the corner it started at.
			AddSceneFileFace(psPolygons, (int)i);
			continue;
		}
		if (aiDone.GetValue((size)iPolygon) == -1)
		{
			aiDone.SetValue((size)iPolygon, AddSceneFilePolygon(psPolygons, pcPolygon) ? 1 : 0);
		}
		if (aiDone.GetValue((size)iPolygon) == 0)
		{
			AddSceneFileFace(psPolygons, (int)i);
		}
	}
	aiDone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteSceneFileNormals(CSceneFileWriter* pcWriter, SSceneFilePolygons* psPolygons)
{
	CArraySceneFileIndex	aiCorners;
	CMesh*					pcMesh;
	SSceneFileCorner*		psCorner;
	size					uiNumFlat;
	size					i;
	int						iNormal;

	pcMesh = psPolygons->pcMesh;
	uiNumFlat = (size)pcMesh->NumFaces();
	if (!pcMesh->mcNormals.InUse() || (pcMesh->mcNormals.mcNormals.NumElements() <= uiNumFlat))
	{
		return;
	}

	aiCorners.Init();
	for (i = 0; i < psPolygons->asCorners.NumElements(); i++)
	{
		psCorner = psPolygons->asCorners.Get(i);
		iNormal = pcMesh->mcNormals.mcFaces.Get((size)psCorner->iFace)->aiCornerNormals[psCorner->iFaceCorner];
		//Smoothing leaves a face in no group pointing at its flat normal.
		aiCorners.Add((iNormal < (int)uiNumFlat) ? SCENE_FILE_NONE : (iNormal - (int)uiNumFlat));
	}
	pcWriter->WriteNormals((float32*)pcMesh->mcNormals.mcNormals.Get(uiNumFlat), pcMesh->mcNormals.mcNormals.NumElements() - uiNumFlat);
	pcWriter->WriteCornerNormals(aiCorners.GetData(), aiCorners.NumElements());
	aiCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteSceneFileColours(CSceneFileWriter* pcWriter, SSceneFilePolygons* psPolygons)
{
	CArraySceneFileIndex	aiCorners;
	CMesh*					pcMesh;
	SSceneFileCorner*		psCorner;
	size					i;

	pcMesh = psPolygons->pcMesh;
	if (!pcMesh->mcColours.InUse())
	{
		return;
	}

	aiCorners.Init();
	for (i = 0; i < psPolygons->asCorners.NumElements(); i++)
	{
		psCorner = psPolygons->asCorners.Get(i);
		aiCorners.Add(pcMesh->mcColours.mcFaces.Get((size)psCorner->iFace)->aiCornerColours[psCorner->iFaceCorner]);
	}
	pcWriter->WriteColours(pcMesh->mcColours.mcColours.GetData(), pcMesh->mcColours.mcColours.NumElements());
	pcWriter->WriteCornerColours(aiCorners.GetData(), aiCorners.NumElements());
	aiCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteSceneFileUVLayers(CSceneFileWriter* pcWriter, SSceneFilePolygons* psPolygons)
{
	CArraySceneFileIndex	aiCorners;
	CMeshUVLayer*			pcLayer;
	SSceneFileCorner*		psCorner;
	CMesh*					pcMesh;
	size					i;
	size					j;

	pcMesh = psPolygons->pcMesh;
	for (i = 0; i < pcMesh->mcUVs.mcLayers.NumElements(); i++)
	{
		pcLayer = pcMesh->mcUVs.mcLayers.Get(i);
		aiCorners.Init();
		for (j = 0; j < psPolygons->asCorners.NumElements(); j++)
		{
			psCorner = psPolygons->asCorners.Get(j);
			aiCorners.Add(pcLayer->mcFaces.Get((size)psCorner->iFace)->aiUVs[psCorner->iFaceCorner]);
		}
		pcWriter->WriteUVLayer((float32*)pcLayer->mcUVs.GetData(), pcLayer->mcUVs.NumElements(), aiCorners.GetData(), aiCorners.NumElements());
		aiCorners.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteSceneFileSkin(CSceneFileWriter* pcWriter, CMesh* pcMesh)
{
	CArraySceneFileIndex	aiCounts;
	CArraySceneFileIndex	aiMatrices;
	CArraySceneFileFloat	afWeights;
	CMeshSkinVert*			pcVert;
	SSkinWeight*			psWeight;
	size					i;
	size					j;

	if (!pcMesh->mcSkin.InUse())
	{
		return;
	}

	aiCounts.Init();
	aiMatrices.Init();
	afWeights.Init();
	for (i = 0; i < pcMesh->mcSkin.mcSkinVerts.NumElements(); i++)
	{
		pcVert = pcMesh->mcSkin.mcSkinVerts.Get(i);
		aiCounts.Add((int32)pcVert->mcWeights.NumElements());
		for (j = 0; j < pcVert->mcWeights.NumElements(); j++)
		{
			psWeight = pcVert->mcWeights.Get(j);
			aiMatrices.Add(psWeight->iMatrix);
			afWeights.Add(psWeight->fWeight);
		}
	}
	pcWriter->WriteSkin(aiCounts.GetData(), aiCounts.NumElements(), aiMatrices.GetData(), afWeights.GetData(), afWeights.NumElements());
	if (pcMesh->mcSkin.mcInverseSkinMatricies.NumElements() > 0)
	{
		pcWriter->WriteInverseMatrices((float32*)pcMesh->mcSkin.mcInverseSkinMatricies.GetData(), pcMesh->mcSkin.mcInverseSkinMatricies.NumElements());
	}
	afWeights.Kill();
	aiMatrices.Kill();
	aiCounts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WriteSceneFileMesh(CSceneFileWriter* pcWriter, CMeshEditor* pcEditor, CArraySceneFileIndex* paiMaterials)
{
	SSceneFilePolygons		sPolygons;
	CArraySceneFileFloat	afPositions;
	CArraySceneFileIndex	aiPositions;
	CArraySceneFileIndex	aiSlots;
	CArraySceneFileUint		auiGroups;
	CMesh*					pcMesh;
	SFloat32Vec3*			psPosition;
	SSceneFileCorner*		psCorner;
	size					i;

	pcMesh = &pcEditor->mpMesh;
	sPolygons.pcMesh = pcMesh;
	sPolygons.pcGroups = pcEditor->mcModifiers.GetModifier<CMeshSmoothGroups>();
	sPolygons.aiSizes.Init();
	sPolygons.aiFirstFaces.Init();
	sPolygons.asCorners.Init();
	GatherSceneFilePolygons(&sPolygons, pcEditor);

	pcWriter->BeginMesh(pcEditor->mszName.Text());

	afPositions.Init();
	for (i = 0; i < (size)pcMesh->NumCorners(); i++)
	{
		psPosition = pcMesh->GetPosition((int)i);
		afPositions.Add(&psPosition->x, 3);
	}
	if (afPositions.NumElements() > 0)
	{
		pcWriter->WritePositions(afPositions.GetData(), afPositions.NumElements() / 3);
	}
	afPositions.Kill();

	if (sPolygons.aiSizes.NumElements() > 0)
	{
		aiPositions.Init();
		for (i = 0; i < sPolygons.asCorners.NumElements(); i++)
		{
			psCorner = sPolygons.asCorners.Get(i);
			aiPositions.Add(pcMesh->GetFaceCorners(psCorner->iFace)[psCorner->iFaceCorner]);
		}
		pcWriter->WritePolygons(sPolygons.aiSizes.GetData(), sPolygons.aiSizes.NumElements(), aiPositions.GetData(), aiPositions.NumElements());
		aiPositions.Kill();
	}

	WriteSceneFileNormals(pcWriter, &sPolygons);
	WriteSceneFileColours(pcWriter, &sPolygons);
	WriteSceneFileUVLayers(pcWriter, &sPolygons);

	if (pcMesh->mcMaterials.InUse())
	{
		aiSlots.Init();
		for (i = 0; i < sPolygons.aiFirstFaces.NumElements(); i++)
		{
			aiSlots.Add(pcMesh->mcMaterials.mcFaces.GetValue((size)sPolygons.aiFirstFaces.GetValue(i)));
		}
		pcWriter->WriteMaterialSlots(paiMaterials->GetData(), paiMaterials->NumElements());
		pcWriter->WritePolygonSlots(aiSlots.GetData(), aiSlots.NumElements());
		aiSlots.Kill();
	}

	if (sPolygons.pcGroups != NULL)
	{
		auiGroups.Init();
		for (i = 0; i < sPolygons.aiFirstFaces.NumElements(); i++)
		{
			auiGroups.Add((uint32)sPolygons.pcGroups->mcSmoothingGroups.GetValue((size)sPolygons.aiFirstFaces.GetValue(i)));
		}
		pcWriter->WriteSmoothGroups(auiGroups.GetData(), auiGroups.NumElements());
		auiGroups.Kill();
	}

	WriteSceneFileSkin(pcWriter, pcMesh);

	pcWriter->EndMesh();

	sPolygons.asCorners.Kill();
	sPolygons.aiFirstFaces.Kill();
	sPolygons.aiSizes.Kill();
	return true;
}

