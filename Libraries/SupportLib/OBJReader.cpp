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
bool LoadOBJ(Ptr<CMesh> pMesh, char* szFilename)
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
bool LoadOBJ(Ptr<CMeshEditor> pEditor, char* szFilename)
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
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Init(Ptr<CMesh> pMesh, char* szFilename)
{
	mcWrapper.Init(pMesh);
	mszFileName.Init(szFilename);
	miLine = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Init(Ptr<CMeshEditor> pEditor, char* szFilename)
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
// A mesh can only make faces from three and four sided polygons.
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadFace(CTextParser* pcTextParser)
{
	CArrayMeshCornerBuilder	asCorners;
	CArrayInt				aiCorners;
	CMFRet					r;
	size					i;
	size					uiNumCorners;
	int						aiPolygonCorners[NUM_FACE_CORNERS];
	bool					bResult;

	asCorners.Init();
	ReadFaceCorners(pcTextParser, &asCorners);

	uiNumCorners = asCorners.NumElements();
	if ((uiNumCorners != 3) && (uiNumCorners != 4))
	{
		asCorners.Kill();
		return Error("A face must have three or four corners");
	}

	bResult = ResolveFaceCorners(&asCorners);
	if (bResult)
	{
		aiCorners.Init();
		aiCorners.Resize(uiNumCorners);
		for (i = 0; i < uiNumCorners; i++)
		{
			aiCorners.SetValue(i, asCorners.Get(i)->miVertex);
		}
		r = mcWrapper.AddPolygon(&aiCorners);
		aiCorners.Kill();

		r.Unpack1(&aiPolygonCorners[0], &aiPolygonCorners[1], &aiPolygonCorners[2]);
		SetFaceDetails(r.miFirstFace, aiPolygonCorners, &asCorners);
		if (r.NumFaces() == 2)
		{
			r.Unpack2(&aiPolygonCorners[0], &aiPolygonCorners[1], &aiPolygonCorners[2]);
			SetFaceDetails(r.miFirstFace + 1, aiPolygonCorners, &asCorners);
		}
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

