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
#include "BaseLib/Version.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "OBJWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddOBJHeading(CChars* psz)
{
	psz->Append("# Codaphela Wavefront OBJ Exporter ");
	psz->Append(ENGINE_VERSION);
	psz->Append(" - copyright 2011 Andrew Paterson");
	psz->AppendNewLine();
	psz->Append("# File Created: ");
	psz->AppendNewLine();
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddOBJName(CMesh* pcMesh, CChars* psz)
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
void AddOBJVertices(CMesh* pcMesh, CChars* psz)
{
	int			i;
	SFloat3*	psPosition;
	SFloat3		sPosition;
	int			iNumPositions;

	iNumPositions = pcMesh->NumCorners();
	for (i = iNumPositions-1; i >= 0 ; i--)
	{
		psPosition = pcMesh->GetPosition(i);
		sPosition = *psPosition;
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
//
//////////////////////////////////////////////////////////////////////////
BOOL AddOBJNormals(CMeshNormals* pcNormals, CChars* psz)
{
	int			i;
	SFloat3*	psNormal;
	SFloat3		sNormal;

	if (pcNormals->mcNormals.NumElements() > 0)
	{
		for (i = 0; i < pcNormals->mcNormals.NumElements(); i++)
		{
			psNormal = pcNormals->mcNormals.Get(i);
			sNormal = *psNormal;
			sNormal.Fix();

			psz->Append("vn ");
			psz->Append(sNormal.x, 6);
			psz->Append(" ");
			psz->Append(sNormal.y, 6);
			psz->Append(" ");
			psz->Append(sNormal.z, 6);
			psz->AppendNewLine();
		}

		psz->Append("# ");
		psz->Append(pcNormals->mcNormals.NumElements());
		psz->Append(" vertex normals\n");
		psz->AppendNewLine();
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL AddOBJTextureCoords(CMeshUVCoords* pcUVs, CChars* psz)
{
	int				i;
	SFloat2*		psUV;
	SFloat2			sUV;
	CMeshUVLayer*	pcUVLayer;

	if (pcUVs->mcLayers.NumElements() > 0)
	{
		pcUVLayer = pcUVs->mcLayers.Get(0);
		for (i = 0; i < pcUVLayer->mcUVs.NumElements(); i++)
		{
			psUV = pcUVLayer->mcUVs.Get(i);
			sUV = *psUV;
			sUV.Fix();

			psz->Append("vn ");
			psz->Append(sUV.x, 6);
			psz->Append(" ");
			psz->Append(sUV.y, 6);
			psz->AppendNewLine();
		}

		psz->Append("# ");
		psz->Append(pcUVLayer->mcUVs.NumElements());
		psz->Append(" texture coords\n");
		psz->AppendNewLine();
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddOBJFaces(CMesh* pcMesh, CChars* psz, BOOL bTextureCoords, BOOL bVertexNormals)
{
	int					i;
	int					iNumFaces;
	CMeshFace*			pcFace;
	SMeshUVFace*		psFaceUV;
	CMeshUVLayer*		pcUVLayer;
	SMeshNormalFace*	psFaceNormal;
	int					j;

	if (bTextureCoords)
	{
		pcUVLayer = pcMesh->mcUVs.mcLayers.Get(0);
	}
	else
	{
		pcUVLayer = NULL;
	}

	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		pcFace = pcMesh->GetFace(i);
		psz->Append("f  ");
		for (j = 0; j < NUM_FACE_CORNERS; j++)
		{
			psz->Append(-(pcFace->asCorner[j]+1));

			if (bTextureCoords || bVertexNormals)
			{
				psz->Append("/");
				if (bTextureCoords)
				{
					psFaceUV = pcUVLayer->mcFaces.Get(i);
					psz->Append(-(psFaceUV->aiUVs[j]+1));
				}
			}

			if (bVertexNormals)
			{
				psz->Append("/");
				psFaceNormal = pcMesh->mcNormals.mcFaces.Get(i);
				psz->Append(-(psFaceNormal->aiCornerNormals[j]+1));
			}
			psz->Append(" ");
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
BOOL SaveOBJ(CMesh *pcMesh, char* szFileName)
{
	CTextFile cTextFile;
	BOOL bTextureCoords;
	BOOL bVertexNormals;

	cTextFile.Init();

	AddOBJHeading(&cTextFile.mcText);
	AddOBJName(pcMesh, &cTextFile.mcText);
	AddOBJVertices(pcMesh, &cTextFile.mcText);
	bVertexNormals = AddOBJNormals(&pcMesh->mcNormals, &cTextFile.mcText);
	bTextureCoords = AddOBJTextureCoords(&pcMesh->mcUVs, &cTextFile.mcText);
	AddOBJFaces(pcMesh, &cTextFile.mcText, bTextureCoords, bVertexNormals);
	
	cTextFile.Write(szFileName);
	cTextFile.Kill();
	return FALSE;
}

