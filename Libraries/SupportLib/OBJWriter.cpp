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
#include "BaseLib/Version.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "OBJWriter.h"


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
// The normal a face's corner uses: its own, or the face's flat one.
//
//////////////////////////////////////////////////////////////////////////
static int MeshCornerNormal(SMeshNormalFace* psFaceNormal, int iFaceCorner)
{
	if (psFaceNormal->aiCornerNormals[iFaceCorner] >= 0)
	{
		return psFaceNormal->aiCornerNormals[iFaceCorner];
	}
	return psFaceNormal->iFaceNormal;
}


//////////////////////////////////////////////////////////////////////////
//
// Only the normals a face uses are written, as a reader makes flat normals
// of its own.  paiOBJNormals is given each normal's index in the file from
// its index in the mesh, -1 if it isn't written.  False if there are none.
//
//////////////////////////////////////////////////////////////////////////
static bool AddOBJNormals(CMesh* pcMesh, CLargeChars* psz, CArrayInt* paiOBJNormals)
{
	CMeshNormals*		pcNormals;
	SMeshNormalFace*	psFaceNormal;
	SFloat32Vec3		sNormal;
	int					iNumNormals;
	int					iNumWritten;
	int					iNumFaces;
	int					i;
	int					j;

	pcNormals = &pcMesh->mcNormals;
	if (!pcNormals->InUse())
	{
		return false;
	}

	iNumNormals = (int)pcNormals->mcNormals.NumElements();
	paiOBJNormals->Resize(iNumNormals);
	paiOBJNormals->SetArrayValues(-1);

	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		psFaceNormal = pcNormals->mcFaces.Get(i);
		for (j = 0; j < NUM_FACE_CORNERS; j++)
		{
			paiOBJNormals->SetValue(MeshCornerNormal(psFaceNormal, j), 0);
		}
	}

	iNumWritten = 0;
	for (i = 0; i < iNumNormals; i++)
	{
		if (paiOBJNormals->GetValue(i) == 0)
		{
			paiOBJNormals->SetValue(i, iNumWritten);
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

	psz->Append("# ");
	psz->Append(iNumWritten);
	psz->Append(" vertex normals\n");
	psz->AppendNewLine();
	return iNumWritten > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A file only holds one layer of UVs.  False if there are none.
//
//////////////////////////////////////////////////////////////////////////
static bool AddOBJTextureCoords(CMeshUVCoords* pcUVs, CLargeChars* psz)
{
	size			i;
	SFloat32Vec2	sUV;
	CMeshUVLayer*	pcUVLayer;
	size			uiNumUVs;

	if (!pcUVs->InUse() || (pcUVs->mcLayers.NumElements() == 0))
	{
		return false;
	}

	pcUVLayer = pcUVs->mcLayers.Get(0);
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
	return uiNumUVs > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A file's indices count from 1.  iUV and iNormal are -1 if the corner has
// none.  A corner with a normal and no UV is written v//n.
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJFaceCorner(CLargeChars* psz, int iPosition, int iUV, int iNormal)
{
	psz->Append(iPosition + 1);

	if ((iUV >= 0) || (iNormal >= 0))
	{
		psz->Append("/");
		if (iUV >= 0)
		{
			psz->Append(iUV + 1);
		}
	}

	if (iNormal >= 0)
	{
		psz->Append("/");
		psz->Append(iNormal + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcUVLayer and paiOBJNormals are NULL if the file doesn't have any.  A
// face that isn't textured has a UV of -1 in the layer.
//
//////////////////////////////////////////////////////////////////////////
static void AddOBJFaces(CMesh* pcMesh, CLargeChars* psz, CMeshUVLayer* pcUVLayer, CArrayInt* paiOBJNormals)
{
	int					i;
	int					iNumFaces;
	int*				piCorners;
	SMeshUVFace*		psFaceUV;
	SMeshNormalFace*	psFaceNormal;
	int					j;
	int					iUV;
	int					iNormal;

	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		psFaceUV = pcUVLayer ? pcUVLayer->mcFaces.Get(i) : NULL;
		psFaceNormal = paiOBJNormals ? pcMesh->mcNormals.mcFaces.Get(i) : NULL;

		psz->Append("f  ");
		for (j = 0; j < NUM_FACE_CORNERS; j++)
		{
			iUV = psFaceUV ? psFaceUV->aiUVs[j] : -1;
			iNormal = psFaceNormal ? paiOBJNormals->GetValue(MeshCornerNormal(psFaceNormal, j)) : -1;

			if (j > 0)
			{
				psz->Append(" ");
			}
			AddOBJFaceCorner(psz, piCorners[j], iUV, iNormal);
		}
		psz->AppendNewLine();
	}

	psz->Append("# ");
	psz->Append(iNumFaces);
	psz->Append(" triangles\n");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveOBJ(CMesh* pcMesh, char* szFilename)
{
	CTextFile		cTextFile;
	CArrayInt		aiOBJNormals;
	bool			bTextureCoords;
	bool			bVertexNormals;
	bool			bResult;

	cTextFile.Init();
	aiOBJNormals.Init();

	AddOBJHeading(&cTextFile.mcText);
	AddOBJName(pcMesh, &cTextFile.mcText);
	AddOBJVertices(pcMesh, &cTextFile.mcText);
	bVertexNormals = AddOBJNormals(pcMesh, &cTextFile.mcText, &aiOBJNormals);
	bTextureCoords = AddOBJTextureCoords(&pcMesh->mcUVs, &cTextFile.mcText);
	AddOBJFaces(pcMesh, &cTextFile.mcText, bTextureCoords ? pcMesh->mcUVs.mcLayers.Get(0) : NULL, bVertexNormals ? &aiOBJNormals : NULL);

	bResult = cTextFile.Write(szFilename);

	aiOBJNormals.Kill();
	cTextFile.Kill();
	return bResult;
}

