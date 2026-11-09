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
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/TextParser.h"
#include "Mesh.h"
#include "MeshEditor.h"
#include "OBJReader.h"


//Indexed by EWavefrontOBJPrefix.
static const char* const gaszOBJTypes[] = { "v", "vt", "vn", "f" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadOBJ(Ptr<CMesh> pMesh, const char* szFilename)
{
	CObjReader	cReader;
	bool		bResult;

	cReader.Init(pMesh, szFilename);
	bResult = cReader.Read();
	cReader.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadOBJ(Ptr<CMeshEditor> pEditor, const char* szFilename)
{
	CObjReader	cReader;
	bool		bResult;

	cReader.Init(pEditor, szFilename);
	bResult = cReader.Read();
	cReader.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An index in a file counts from 1, or back from the last one read if it's
// negative.  False if there is no such one.
//
//////////////////////////////////////////////////////////////////////////
static bool ResolveOBJIndex(int* piIndex, int iNumRead)
{
	int		iIndex;

	iIndex = *piIndex;
	if (iIndex > 0)
	{
		iIndex = iIndex - 1;
	}
	else if (iIndex < 0)
	{
		iIndex = iNumRead + iIndex;
	}
	else
	{
		return false;
	}

	if ((iIndex < 0) || (iIndex >= iNumRead))
	{
		return false;
	}
	*piIndex = iIndex;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadOBJFloats(CTextParser* pcTextParser, float32* pafValues, int iNumValues)
{
	int		i;

	for (i = 0; i < iNumValues; i++)
	{
		if (pcTextParser->GetFloatWithExponent(&pafValues[i]) != TRITRUE)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two of a polygon's corners are next to each other around it, so
// that the edge between them is on its outside.
//
//////////////////////////////////////////////////////////////////////////
static bool IsOBJPolygonEdge(int iCorner1, int iCorner2, int iNumCorners)
{
	return (((iCorner1 + 1) % iNumCorners) == iCorner2) || (((iCorner2 + 1) % iNumCorners) == iCorner1);
}


//////////////////////////////////////////////////////////////////////////
//
// Flattens a polygon by dropping the axis its normal is most along, so that
// its corners go anticlockwise.  A polygon need not be exactly flat.
//
//////////////////////////////////////////////////////////////////////////
static void ProjectOBJPolygon(CMesh* pcMesh, CArrayMeshCornerBuilder* pasCorners, CArrayFloat64Vec2* pasPoints)
{
	SFloat32Vec3*	psPosition;
	SFloat32Vec3*	psNext;
	SFloat64Vec2*	psPoint;
	float64			fNormalX;
	float64			fNormalY;
	float64			fNormalZ;
	float64			fSizeX;
	float64			fSizeY;
	float64			fSizeZ;
	float64			fU;
	float64			fV;
	bool			bSwap;
	size			i;
	size			uiNumCorners;

	fNormalX = 0.0;
	fNormalY = 0.0;
	fNormalZ = 0.0;
	uiNumCorners = pasCorners->NumElements();
	for (i = 0; i < uiNumCorners; i++)
	{
		psPosition = pcMesh->GetPosition(pasCorners->Get(i)->miVertex);
		psNext = pcMesh->GetPosition(pasCorners->Get((i + 1) % uiNumCorners)->miVertex);
		fNormalX += ((float64)psPosition->y - (float64)psNext->y) * ((float64)psPosition->z + (float64)psNext->z);
		fNormalY += ((float64)psPosition->z - (float64)psNext->z) * ((float64)psPosition->x + (float64)psNext->x);
		fNormalZ += ((float64)psPosition->x - (float64)psNext->x) * ((float64)psPosition->y + (float64)psNext->y);
	}

	fSizeX = fabs(fNormalX);
	fSizeY = fabs(fNormalY);
	fSizeZ = fabs(fNormalZ);
	for (i = 0; i < uiNumCorners; i++)
	{
		psPosition = pcMesh->GetPosition(pasCorners->Get(i)->miVertex);
		if ((fSizeZ >= fSizeX) && (fSizeZ >= fSizeY))
		{
			fU = psPosition->x;
			fV = psPosition->y;
			bSwap = fNormalZ < 0.0;
		}
		else if (fSizeX >= fSizeY)
		{
			fU = psPosition->y;
			fV = psPosition->z;
			bSwap = fNormalX < 0.0;
		}
		else
		{
			fU = psPosition->z;
			fV = psPosition->x;
			bSwap = fNormalY < 0.0;
		}

		psPoint = pasPoints->Add();
		psPoint->x = bSwap ? fV : fU;
		psPoint->y = bSwap ? fU : fV;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Above zero if the three points go anticlockwise.
//
//////////////////////////////////////////////////////////////////////////
static float64 OBJPointsTurn(SFloat64Vec2* psA, SFloat64Vec2* psB, SFloat64Vec2* psC)
{
	return (psB->x - psA->x) * (psC->y - psA->y) - (psB->y - psA->y) * (psC->x - psA->x);
}


//////////////////////////////////////////////////////////////////////////
//
// A corner can be cut off if it turns the way the polygon does and no other
// corner is in the triangle it would cut off.  A corner in the same place as
// one of the triangle's doesn't count, or a polygon that touches itself
// would have no ears.
//
//////////////////////////////////////////////////////////////////////////
static bool IsOBJEar(CArrayFloat64Vec2* pasPoints, CArrayInt* paiRemaining, int iNumRemaining, int iCorner)
{
	SFloat64Vec2*	psA;
	SFloat64Vec2*	psB;
	SFloat64Vec2*	psC;
	SFloat64Vec2*	psOther;
	int				iPrevious;
	int				iNext;
	int				i;

	iPrevious = (iCorner + iNumRemaining - 1) % iNumRemaining;
	iNext = (iCorner + 1) % iNumRemaining;
	psA = pasPoints->Get(paiRemaining->GetValue(iPrevious));
	psB = pasPoints->Get(paiRemaining->GetValue(iCorner));
	psC = pasPoints->Get(paiRemaining->GetValue(iNext));

	if (OBJPointsTurn(psA, psB, psC) <= 0.0)
	{
		return false;
	}

	for (i = 0; i < iNumRemaining; i++)
	{
		if ((i != iPrevious) && (i != iCorner) && (i != iNext))
		{
			psOther = pasPoints->Get(paiRemaining->GetValue(i));
			if (((psOther->x != psA->x) || (psOther->y != psA->y)) &&
				((psOther->x != psB->x) || (psOther->y != psB->y)) &&
				((psOther->x != psC->x) || (psOther->y != psC->y)) &&
				(OBJPointsTurn(psA, psB, psOther) >= 0.0) &&
				(OBJPointsTurn(psB, psC, psOther) >= 0.0) &&
				(OBJPointsTurn(psC, psA, psOther) >= 0.0))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A polygon with no ears, as it has no area or crosses itself, is still cut
// up, from its first corner.
//
//////////////////////////////////////////////////////////////////////////
static int FindOBJEar(CArrayFloat64Vec2* pasPoints, CArrayInt* paiRemaining, int iNumRemaining)
{
	int		i;

	for (i = 0; i < iNumRemaining; i++)
	{
		if (IsOBJEar(pasPoints, paiRemaining, iNumRemaining, i))
		{
			return i;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Init(Ptr<CMesh> pMesh, const char* szFilename)
{
	mcWrapper.Init(pMesh);
	mszFileName.Init(szFilename);
	miLine = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Init(Ptr<CMeshEditor> pEditor, const char* szFilename)
{
	mcWrapper.Init(pEditor);
	mszFileName.Init(szFilename);
	miLine = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Kill(void)
{
	mszFileName.Kill();
	mcWrapper.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The Mesh/MeshEditor must already be initialised.  Lines that aren't
// positions, UVs, normals or faces are ignored, including groups, smoothing
// groups and materials: a file's normals say how it is smoothed.
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::Read(void)
{
	CTextFile	cTextFile;
	CArrayChars	cLines;
	CChars*		pszLine;
	CTextParser	cTextParser;
	size		i;
	size		uiNumLines;
	bool		bResult;

	cTextFile.Init();
	if (!cTextFile.Read(mszFileName.Text()))
	{
		cTextFile.Kill();
		return Error("Could not read the file");
	}

	cTextFile.PassifyNewlines();
	cLines.Fake();
	cTextFile.mcText.SplitLines(&cLines);

	mcWrapper.ClearConnectivity();

	bResult = true;
	uiNumLines = cLines.NumElements();
	for (i = 0; (i < uiNumLines) && bResult; i++)
	{
		miLine = (int)i + 1;
		pszLine = cLines.Get(i);
		cTextParser.Init(pszLine->Text(), pszLine->Length());
		bResult = ReadLine(&cTextParser);
		cTextParser.Kill();
	}

	cLines.Kill();
	cTextFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadLine(CTextParser* pcTextParser)
{
	size	iType;

	if (pcTextParser->GetLongestIdentifier(gaszOBJTypes, sizeof(gaszOBJTypes) / sizeof(gaszOBJTypes[0]), &iType) != TRITRUE)
	{
		return true;
	}

	switch (iType)
	{
	case WOBJP_v:
		return ReadPosition(pcTextParser);
	case WOBJP_vt:
		return ReadUVCoord(pcTextParser);
	case WOBJP_vn:
		return ReadNormal(pcTextParser);
	case WOBJP_f:
		return ReadFace(pcTextParser);
	default:
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadPosition(CTextParser* pcTextParser)
{
	float32		afPosition[3];

	if (!ReadOBJFloats(pcTextParser, afPosition, 3))
	{
		return Error("Could not read a position");
	}
	mcWrapper.AddCorner(afPosition[0], afPosition[1], afPosition[2]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh only keeps normals for its faces from when it has normals, so a
// file's normals must come before its faces.
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadNormal(CTextParser* pcTextParser)
{
	float32		afNormal[3];

	if (!ReadOBJFloats(pcTextParser, afNormal, 3))
	{
		return Error("Could not read a normal");
	}
	if ((mcWrapper.NumNormals() == 0) && (mcWrapper.NumFaces() > 0))
	{
		return Error("Normals must come before faces");
	}
	mcWrapper.SetNormalsInUse(true);
	mcWrapper.AddNormal(afNormal[0], afNormal[1], afNormal[2]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadUVCoord(CTextParser* pcTextParser)
{
	float32		afUV[2];

	if (!ReadOBJFloats(pcTextParser, afUV, 2))
	{
		return Error("Could not read a texture coord");
	}
	mcWrapper.SetUVsInUse(true);
	mcWrapper.AddUV(afUV[0], afUV[1]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// v, v/vt, v//vn or v/vt/vn for each corner.
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::ReadFaceCorners(CTextParser* pcTextParser, CArrayMeshCornerBuilder* pasCorners)
{
	int32				iValue;
	CMeshCornerBuilder*	psCorner;

	while (pcTextParser->GetInteger(&iValue) == TRITRUE)
	{
		psCorner = pasCorners->Add();
		psCorner->Init(iValue);

		//No skipping whitespace because // is whitespace.
		if (pcTextParser->GetExactCharacter('/', false) == TRITRUE)
		{
			if (pcTextParser->GetInteger(&iValue, NULL, false) == TRITRUE)
			{
				psCorner->mbUVCoord = true;
				psCorner->miUVCoord = iValue;
			}

			if (pcTextParser->GetExactCharacter('/', false) == TRITRUE)
			{
				if (pcTextParser->GetInteger(&iValue, NULL, false) == TRITRUE)
				{
					psCorner->mbNormal = true;
					psCorner->miNormal = iValue;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Turns the corners' indices in the file to ones from 0, among what the
// file has read so far.
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ResolveFaceCorners(CArrayMeshCornerBuilder* pasCorners)
{
	CMeshCornerBuilder*	psCorner;
	size				i;
	size				uiNumCorners;
	int					iNumFileNormals;

	//A mesh's flat normals, one for each face, come before the normals from the file.
	iNumFileNormals = mcWrapper.NumNormals() - mcWrapper.NumFaces();

	uiNumCorners = pasCorners->NumElements();
	for (i = 0; i < uiNumCorners; i++)
	{
		psCorner = pasCorners->Get(i);
		if (!ResolveOBJIndex(&psCorner->miVertex, mcWrapper.NumCorners()))
		{
			return Error("A face refers to a position that isn't there");
		}
		if (psCorner->mbUVCoord && !ResolveOBJIndex(&psCorner->miUVCoord, mcWrapper.NumUVs()))
		{
			return Error("A face refers to a texture coord that isn't there");
		}
		if (psCorner->mbNormal && !ResolveOBJIndex(&psCorner->miNormal, iNumFileNormals))
		{
			return Error("A face refers to a normal that isn't there");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// aiPolygonCorners is which of the polygon's corners each of the face's
// corners is.  A corner keeps the face's flat normal unless it has its own.
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::SetFaceDetails(int iFace, int* aiPolygonCorners, CArrayMeshCornerBuilder* pasCorners)
{
	CMeshCornerBuilder*	psCorner;
	SMeshNormalFace*	psFaceNormal;
	SMeshUVFace*		psFaceUV;
	int					iNumFlatNormals;
	int					i;

	iNumFlatNormals = mcWrapper.NumFaces();
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		psCorner = pasCorners->Get(aiPolygonCorners[i]);
		if (psCorner->mbNormal)
		{
			psFaceNormal = mcWrapper.GetFaceNormal(iFace);
			psFaceNormal->aiCornerNormals[i] = iNumFlatNormals + psCorner->miNormal;
		}
		if (psCorner->mbUVCoord)
		{
			psFaceUV = mcWrapper.GetUVFace(iFace, 0);
			psFaceUV->aiUVs[i] = psCorner->miUVCoord;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Makes one face of a polygon.  The edges inside the polygon are hidden, so
// that an editor still sees one polygon.
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::AddPolygonFace(int* aiPolygonCorners, CArrayMeshCornerBuilder* pasCorners)
{
	int		iNumCorners;
	bool	bEdge12;
	bool	bEdge23;
	bool	bEdge31;
	CMFRet	r;

	iNumCorners = (int)pasCorners->NumElements();
	bEdge12 = IsOBJPolygonEdge(aiPolygonCorners[0], aiPolygonCorners[1], iNumCorners);
	bEdge23 = IsOBJPolygonEdge(aiPolygonCorners[1], aiPolygonCorners[2], iNumCorners);
	bEdge31 = IsOBJPolygonEdge(aiPolygonCorners[2], aiPolygonCorners[0], iNumCorners);

	r = mcWrapper.AddFace(pasCorners->Get(aiPolygonCorners[0])->miVertex, pasCorners->Get(aiPolygonCorners[1])->miVertex, pasCorners->Get(aiPolygonCorners[2])->miVertex, 0, bEdge12, bEdge23, bEdge31);
	SetFaceDetails(r.miFirstFace, aiPolygonCorners, pasCorners);
}


//////////////////////////////////////////////////////////////////////////
//
// Cuts a polygon into faces by taking the first ear each time.  A convex quad
// comes out as CMesh::AddQuad cuts it: corners 0, 1, 3 and then 1, 2, 3.
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::AddPolygonFaces(CArrayMeshCornerBuilder* pasCorners)
{
	CArrayFloat64Vec2	asPoints;
	CArrayInt			aiRemaining;
	int					iNumRemaining;
	int					iEar;
	int					aiPolygonCorners[NUM_FACE_CORNERS];
	int					i;

	asPoints.Init();
	ProjectOBJPolygon(mcWrapper.mpcMesh, pasCorners, &asPoints);

	iNumRemaining = (int)pasCorners->NumElements();
	aiRemaining.Init();
	aiRemaining.Resize(iNumRemaining);
	for (i = 0; i < iNumRemaining; i++)
	{
		aiRemaining.SetValue(i, i);
	}

	while (iNumRemaining >= NUM_FACE_CORNERS)
	{
		iEar = FindOBJEar(&asPoints, &aiRemaining, iNumRemaining);
		aiPolygonCorners[0] = aiRemaining.GetValue(iEar);
		aiPolygonCorners[1] = aiRemaining.GetValue((iEar + 1) % iNumRemaining);
		aiPolygonCorners[2] = aiRemaining.GetValue((iEar + iNumRemaining - 1) % iNumRemaining);
		AddPolygonFace(aiPolygonCorners, pasCorners);

		iNumRemaining--;
		for (i = iEar; i < iNumRemaining; i++)
		{
			aiRemaining.SetValue(i, aiRemaining.GetValue(i + 1));
		}
	}

	aiRemaining.Kill();
	asPoints.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadFace(CTextParser* pcTextParser)
{
	CArrayMeshCornerBuilder	asCorners;
	bool					bResult;

	asCorners.Init();
	ReadFaceCorners(pcTextParser, &asCorners);

	if (asCorners.NumElements() < NUM_FACE_CORNERS)
	{
		asCorners.Kill();
		return Error("A face must have at least three corners");
	}

	bResult = ResolveFaceCorners(&asCorners);
	if (bResult)
	{
		AddPolygonFaces(&asCorners);
	}

	asCorners.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::Error(const char* szMessage)
{
	return gcLogger.Error2(__METHOD__, " ", szMessage, " in [", mszFileName.Text(), "] line [", IntToString(miLine), "].", NULL);
}

