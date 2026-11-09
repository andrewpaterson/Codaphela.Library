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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Version.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "MeshEditor.h"
#include "OBJWriter.h"


//A corner of a line in the file.  iUV and iNormal are -1 if it has none.  iNormal is the mesh's until AddOBJNormals.
struct SOBJCorner
{
	int		iPosition;
	int		iUV;
	int		iNormal;
};


//An edge of a face, going the way the face goes around.
struct SOBJEdge
{
	int		iFrom;
	int		iTo;
	int		iFace;
	int		iFaceCorner;
};


typedef CArrayTemplate<SOBJCorner>	CArrayOBJCorner;
typedef CArrayTemplate<SOBJEdge>	CArrayOBJEdge;


//What's in a mesh that goes in a file, and the lines the file's faces will be.
struct SOBJFaces
{
	CMesh*			pcMesh;
	CMeshUVLayer*	pcUVLayer;  //NULL if the file has no UVs.  A file only holds one layer of them.
	bool			bNormals;
	CArrayOBJCorner	asCorners;
	CArrayInt		aiLineSizes;
};


//Normals this close are one normal: a written normal is rounded to more than this.
#define OBJ_SAME_NORMAL_TOLERANCE	(1.0f / 65536.0f)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJHeading(CLargeChars* psz)
{
	psz->Append("# Codaphela Wavefront OBJ Exporter ");
	psz->Append(ENGINE_VERSION);
	psz->Append(" - copyright 2025 Andrew Paterson");
	psz->AppendNewLine();
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJName(CMesh* pcMesh, CLargeChars* psz)
{
	psz->Append("#\n");
	psz->Append("# ");
	psz->Append(pcMesh->GetName());
	psz->AppendNewLine();
	psz->Append("#\n");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJVertices(CMesh* pcMesh, CLargeChars* psz)
{
	int				i;
	SFloat32Vec3	sPosition;
	int				iNumPositions;

	iNumPositions = pcMesh->NumCorners();
	for (i = 0; i < iNumPositions; i++)
	{
		sPosition = *pcMesh->GetPosition(i);
		sPosition.Fix();

		psz->Append("v  ");
		psz->Append(sPosition.x, 6);
		psz->Append(" ");
		psz->Append(sPosition.y, 6);
		psz->Append(" ");
		psz->Append(sPosition.z, 6);
		psz->AppendNewLine();
	}

	psz->Append("# ");
	psz->Append(iNumPositions);
	psz->Append(" vertices\n");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A corner of a face: its own normal if it has one, or the face's flat one.
//
//////////////////////////////////////////////////////////////////////////
static void GetOBJCorner(SOBJFaces* psFaces, int iFace, int iFaceCorner, SOBJCorner* psCorner)
{
	SMeshNormalFace*	psFaceNormal;

	psCorner->iPosition = psFaces->pcMesh->GetFaceCorners(iFace)[iFaceCorner];
	psCorner->iUV = psFaces->pcUVLayer ? psFaces->pcUVLayer->mcFaces.Get(iFace)->aiUVs[iFaceCorner] : -1;
	psCorner->iNormal = -1;
	if (psFaces->bNormals)
	{
		psFaceNormal = psFaces->pcMesh->mcNormals.mcFaces.Get(iFace);
		psCorner->iNormal = psFaceNormal->aiCornerNormals[iFaceCorner];
		if (psCorner->iNormal < 0)
		{
			psCorner->iNormal = psFaceNormal->iFaceNormal;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gets the edges of a polygon's faces that are on its outside, those with no
// face going back along them.  False if an edge is used twice the same way.
//
//////////////////////////////////////////////////////////////////////////
static bool GetOBJPolygonOutsideEdges(SOBJFaces* psFaces, CMeshPolygon* pcPolygon, CArrayOBJEdge* pasEdges, CArrayOBJEdge* pasOutside)
{
	SOBJEdge*	psEdge;
	SOBJEdge*	psOther;
	int*		piCorners;
	int			iFace;
	bool		bReversed;
	size		i;
	size		j;
	size		uiNumEdges;
	int			k;

	for (i = 0; i < pcPolygon->maiFaces.NumElements(); i++)
	{
		iFace = pcPolygon->maiFaces.GetValue(i);
		piCorners = psFaces->pcMesh->GetFaceCorners(iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psEdge = pasEdges->Add();
			psEdge->iFrom = piCorners[k];
			psEdge->iTo = piCorners[(k + 1) % NUM_FACE_CORNERS];
			psEdge->iFace = iFace;
			psEdge->iFaceCorner = k;
		}
	}

	uiNumEdges = pasEdges->NumElements();
	for (i = 0; i < uiNumEdges; i++)
	{
		psEdge = pasEdges->Get(i);
		bReversed = false;
		for (j = 0; j < uiNumEdges; j++)
		{
			psOther = pasEdges->Get(j);
			if ((j != i) && (psOther->iFrom == psEdge->iFrom) && (psOther->iTo == psEdge->iTo))
			{
				return false;
			}
			bReversed |= (psOther->iFrom == psEdge->iTo) && (psOther->iTo == psEdge->iFrom);
		}
		if (!bReversed)
		{
			*pasOutside->Add() = *psEdge;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts a polygon's outside edges in order around it.  False unless they are
// one loop with every corner of the polygon on it: n faces with no corners
// inside them have n + 2 edges around them.
//
//////////////////////////////////////////////////////////////////////////
static bool OrderOBJPolygonLoop(CMeshPolygon* pcPolygon, CArrayOBJEdge* pasOutside)
{
	SOBJEdge*	psEdge;
	SOBJEdge*	psNext;
	SOBJEdge	sSwap;
	size		uiNumEdges;
	size		i;
	size		j;
	size		uiFound;

	uiNumEdges = pasOutside->NumElements();
	if (uiNumEdges != (pcPolygon->maiFaces.NumElements() + 2))
	{
		return false;
	}

	for (i = 0; i + 1 < uiNumEdges; i++)
	{
		psEdge = pasOutside->Get(i);
		uiFound = uiNumEdges;
		for (j = 0; j < uiNumEdges; j++)
		{
			psNext = pasOutside->Get(j);
			if (psNext->iFrom == psEdge->iTo)
			{
				if ((j <= i) || (uiFound != uiNumEdges))
				{
					//It came back too soon, or it can go two ways from here.
					return false;
				}
				uiFound = j;
			}
		}
		if (uiFound == uiNumEdges)
		{
			return false;
		}

		sSwap = *pasOutside->Get(i + 1);
		*pasOutside->Get(i + 1) = *pasOutside->Get(uiFound);
		*pasOutside->Get(uiFound) = sSwap;
	}
	return pasOutside->Get(uiNumEdges - 1)->iTo == pasOutside->Get(0)->iFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool AreOBJNormalsSame(CMesh* pcMesh, int iNormal1, int iNormal2)
{
	SFloat32Vec3*	psNormal1;
	SFloat32Vec3*	psNormal2;

	if (iNormal1 == iNormal2)
	{
		return true;
	}
	psNormal1 = pcMesh->mcNormals.mcNormals.Get(iNormal1);
	psNormal2 = pcMesh->mcNormals.mcNormals.Get(iNormal2);
	return (fabsf(psNormal1->x - psNormal2->x) <= OBJ_SAME_NORMAL_TOLERANCE) &&
		   (fabsf(psNormal1->y - psNormal2->y) <= OBJ_SAME_NORMAL_TOLERANCE) &&
		   (fabsf(psNormal1->z - psNormal2->z) <= OBJ_SAME_NORMAL_TOLERANCE);
}


//////////////////////////////////////////////////////////////////////////
//
// A line in a file only has one UV and one normal for each corner.  False if
// a polygon's faces don't agree about one, as with a seam across it.
//
//////////////////////////////////////////////////////////////////////////
static bool DoOBJPolygonCornersAgree(SOBJFaces* psFaces, CArrayOBJEdge* pasEdges, CArrayOBJEdge* pasLoop)
{
	SOBJEdge*	psEdge;
	SOBJEdge*	psLoop;
	SOBJCorner	sCorner;
	SOBJCorner	sLoopCorner;
	size		i;
	size		j;
	size		uiNumEdges;
	size		uiNumLoop;

	uiNumEdges = pasEdges->NumElements();
	uiNumLoop = pasLoop->NumElements();
	for (i = 0; i < uiNumEdges; i++)
	{
		psEdge = pasEdges->Get(i);
		GetOBJCorner(psFaces, psEdge->iFace, psEdge->iFaceCorner, &sCorner);
		for (j = 0; j < uiNumLoop; j++)
		{
			psLoop = pasLoop->Get(j);
			if (psLoop->iFrom == psEdge->iFrom)
			{
				GetOBJCorner(psFaces, psLoop->iFace, psLoop->iFaceCorner, &sLoopCorner);
				if (sCorner.iUV != sLoopCorner.iUV)
				{
					return false;
				}
				if (psFaces->bNormals && !AreOBJNormalsSame(psFaces->pcMesh, sCorner.iNormal, sLoopCorner.iNormal))
				{
					return false;
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a polygon as one line, starting from its lowest numbered corner, so
// that the line doesn't depend on how the polygon was cut into faces.  False
// if one line can't say what its faces do, and then it's left to its faces.
//
//////////////////////////////////////////////////////////////////////////
static bool AddOBJPolygonLine(SOBJFaces* psFaces, CMeshPolygon* pcPolygon)
{
	CArrayOBJEdge	asEdges;
	CArrayOBJEdge	asLoop;
	SOBJEdge*		psLoop;
	SOBJCorner*		psCorner;
	int				iFirstNormal;
	bool			bResult;
	size			i;
	size			uiNumLoop;
	size			uiStart;

	asEdges.Init();
	asLoop.Init();

	bResult = GetOBJPolygonOutsideEdges(psFaces, pcPolygon, &asEdges, &asLoop);
	bResult = bResult && OrderOBJPolygonLoop(pcPolygon, &asLoop);
	bResult = bResult && DoOBJPolygonCornersAgree(psFaces, &asEdges, &asLoop);
	if (bResult)
	{
		uiNumLoop = asLoop.NumElements();
		uiStart = 0;
		for (i = 1; i < uiNumLoop; i++)
		{
			if (asLoop.Get(i)->iFrom < asLoop.Get(uiStart)->iFrom)
			{
				uiStart = i;
			}
		}

		for (i = 0; i < uiNumLoop; i++)
		{
			psLoop = asLoop.Get((uiStart + i) % uiNumLoop);
			psCorner = psFaces->asCorners.Add();
			GetOBJCorner(psFaces, psLoop->iFace, psLoop->iFaceCorner, psCorner);

			//A flat polygon's faces each have a normal, and it only needs one.
			if (i == 0)
			{
				iFirstNormal = psCorner->iNormal;
			}
			else if (psFaces->bNormals && AreOBJNormalsSame(psFaces->pcMesh, iFirstNormal, psCorner->iNormal))
			{
				psCorner->iNormal = iFirstNormal;
			}
		}
		psFaces->aiLineSizes.Add((int)uiNumLoop);
	}

	asLoop.Kill();
	asEdges.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJFaceLine(SOBJFaces* psFaces, int iFace)
{
	int		i;

	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		GetOBJCorner(psFaces, iFace, i, psFaces->asCorners.Add());
	}
	psFaces->aiLineSizes.Add(NUM_FACE_CORNERS);
}


//////////////////////////////////////////////////////////////////////////
//
// A line for each face, in order, but for a polygon that can be one line,
// which goes where its first face would.  pcPolygons is NULL for a mesh on
// its own.  A face added directly to the mesh has no polygon.
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJLines(SOBJFaces* psFaces, CMeshPolygons* pcPolygons)
{
	CArrayInt	aiPolygonLines;  //-1 for not looked at yet, 1 if it's a line, 0 if it's left to its faces.
	int			iNumFaces;
	int			iPolygon;
	int			i;

	aiPolygonLines.Init();
	if (pcPolygons)
	{
		aiPolygonLines.Resize(pcPolygons->mcPolygons.NumElements());
		aiPolygonLines.SetArrayValues(-1);
	}

	iNumFaces = psFaces->pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		iPolygon = -1;
		if (pcPolygons && (i < (int)pcPolygons->maiFacesToPolygons.NumElements()))
		{
			iPolygon = pcPolygons->maiFacesToPolygons.GetValue(i);
		}

		if (iPolygon == -1)
		{
			AddOBJFaceLine(psFaces, i);
		}
		else
		{
			if (aiPolygonLines.GetValue(iPolygon) == -1)
			{
				aiPolygonLines.SetValue(iPolygon, AddOBJPolygonLine(psFaces, pcPolygons->mcPolygons.Get(iPolygon)) ? 1 : 0);
			}
			if (aiPolygonLines.GetValue(iPolygon) == 0)
			{
				AddOBJFaceLine(psFaces, i);
			}
		}
	}

	aiPolygonLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Only the normals the lines use are written, as a reader makes flat normals
// of its own.  The lines' corners are changed to index what's written.
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJNormals(SOBJFaces* psFaces, CLargeChars* psz)
{
	CMeshNormals*	pcNormals;
	CArrayInt		aiOBJNormals;
	SOBJCorner*		psCorner;
	SFloat32Vec3	sNormal;
	int				iNumNormals;
	int				iNumWritten;
	size			i;
	size			uiNumCorners;

	if (!psFaces->bNormals)
	{
		return;
	}

	pcNormals = &psFaces->pcMesh->mcNormals;
	iNumNormals = (int)pcNormals->mcNormals.NumElements();
	aiOBJNormals.Init();
	aiOBJNormals.Resize(iNumNormals);
	aiOBJNormals.SetArrayValues(-1);

	uiNumCorners = psFaces->asCorners.NumElements();
	for (i = 0; i < uiNumCorners; i++)
	{
		aiOBJNormals.SetValue(psFaces->asCorners.Get(i)->iNormal, 0);
	}

	iNumWritten = 0;
	for (i = 0; i < (size)iNumNormals; i++)
	{
		if (aiOBJNormals.GetValue(i) == 0)
		{
			aiOBJNormals.SetValue(i, iNumWritten);
			iNumWritten++;

			sNormal = *pcNormals->mcNormals.Get(i);
			sNormal.Fix();

			psz->Append("vn ");
			psz->Append(sNormal.x, 6);
			psz->Append(" ");
			psz->Append(sNormal.y, 6);
			psz->Append(" ");
			psz->Append(sNormal.z, 6);
			psz->AppendNewLine();
		}
	}

	for (i = 0; i < uiNumCorners; i++)
	{
		psCorner = psFaces->asCorners.Get(i);
		psCorner->iNormal = aiOBJNormals.GetValue(psCorner->iNormal);
	}
	aiOBJNormals.Kill();

	psz->Append("# ");
	psz->Append(iNumWritten);
	psz->Append(" vertex normals\n");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJTextureCoords(CMeshUVLayer* pcUVLayer, CLargeChars* psz)
{
	size			i;
	SFloat32Vec2	sUV;
	size			uiNumUVs;

	if (!pcUVLayer)
	{
		return;
	}

	uiNumUVs = pcUVLayer->mcUVs.NumElements();
	for (i = 0; i < uiNumUVs; i++)
	{
		sUV = *pcUVLayer->mcUVs.Get(i);
		sUV.Fix();

		psz->Append("vt ");
		psz->Append(sUV.x, 6);
		psz->Append(" ");
		psz->Append(sUV.y, 6);
		psz->AppendNewLine();
	}

	psz->Append("# ");
	psz->Append(uiNumUVs);
	psz->Append(" texture coords\n");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A file's indices count from 1.  A corner with a normal and no UV is written
// v//n.
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJFaceCorner(CLargeChars* psz, SOBJCorner* psCorner)
{
	psz->Append(psCorner->iPosition + 1);

	if ((psCorner->iUV >= 0) || (psCorner->iNormal >= 0))
	{
		psz->Append("/");
		if (psCorner->iUV >= 0)
		{
			psz->Append(psCorner->iUV + 1);
		}
	}

	if (psCorner->iNormal >= 0)
	{
		psz->Append("/");
		psz->Append(psCorner->iNormal + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJFaces(SOBJFaces* psFaces, CLargeChars* psz)
{
	size	i;
	size	uiNumLines;
	size	uiCorner;
	int		iLineSize;
	int		j;
	bool	bTriangles;

	bTriangles = true;
	uiCorner = 0;
	uiNumLines = psFaces->aiLineSizes.NumElements();
	for (i = 0; i < uiNumLines; i++)
	{
		iLineSize = psFaces->aiLineSizes.GetValue(i);
		bTriangles &= iLineSize == NUM_FACE_CORNERS;

		psz->Append("f  ");
		for (j = 0; j < iLineSize; j++)
		{
			if (j > 0)
			{
				psz->Append(" ");
			}
			AddOBJFaceCorner(psz, psFaces->asCorners.Get(uiCorner));
			uiCorner++;
		}
		psz->AppendNewLine();
	}

	psz->Append("# ");
	psz->Append(uiNumLines);
	psz->Append(bTriangles ? " triangles\n" : " polygons\n");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool SaveOBJFaces(CMesh* pcMesh, CMeshPolygons* pcPolygons, const char* szFilename)
{
	CTextFile	cTextFile;
	SOBJFaces	sFaces;
	bool		bResult;

	sFaces.pcMesh = pcMesh;
	sFaces.pcUVLayer = NULL;
	if (pcMesh->mcUVs.InUse() && (pcMesh->mcUVs.mcLayers.NumElements() > 0) && (pcMesh->mcUVs.mcLayers.Get(0)->mcUVs.NumElements() > 0))
	{
		sFaces.pcUVLayer = pcMesh->mcUVs.mcLayers.Get(0);
	}
	sFaces.bNormals = pcMesh->mcNormals.InUse();
	sFaces.asCorners.Init();
	sFaces.aiLineSizes.Init();
	AddOBJLines(&sFaces, pcPolygons);

	cTextFile.Init();
	AddOBJHeading(&cTextFile.mcText);
	AddOBJName(pcMesh, &cTextFile.mcText);
	AddOBJVertices(pcMesh, &cTextFile.mcText);
	AddOBJNormals(&sFaces, &cTextFile.mcText);
	AddOBJTextureCoords(sFaces.pcUVLayer, &cTextFile.mcText);
	AddOBJFaces(&sFaces, &cTextFile.mcText);

	bResult = cTextFile.Write(szFilename);

	cTextFile.Kill();
	sFaces.aiLineSizes.Kill();
	sFaces.asCorners.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveOBJ(CMesh* pcMesh, const char* szFilename)
{
	return SaveOBJFaces(pcMesh, NULL, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveOBJ(CMeshEditor* pcEditor, const char* szFilename)
{
	return SaveOBJFaces(&pcEditor->mpMesh, &pcEditor->mcPolygons, szFilename);
}

