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
#include "BaseLib/EnumeratorVoid.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/TextParser.h"
#include "Mesh.h"
#include "MeshEditor.h"
#include "MeshCornerBuilder.h"
#include "OBJReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadOBJ(Ptr<CMesh> pcMesh, char *szFileName)
{
	CObjReader	cReader;
	bool		bResult;

	cReader.Init(pcMesh, szFileName);
	bResult = cReader.Read();
	cReader.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Init(Ptr<CMesh> pcMesh, char *szFileName)
{
	mcWrapper.Init(pcMesh);
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjReader::Init(Ptr<CMeshEditor> pcEditor, char *szFileName)
{
	mcWrapper.Init(pcEditor);
	mszFileName.Init(szFileName);

	miMaterial = -1;
	miSmoothingGroup = 1;
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
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::Read(void)
{
	CTextParser 		cTextParser;
	CTextFile			cTextFile;
	CEnumeratorVoid		cTypes;
	size				iType;
	CArrayChars			cLines;
	CChars*				pszLine;
	size				i;
	TRISTATE			tResult;

	cTypes.Init();
	cTypes.Add("v",			WOBJP_v);
	cTypes.Add("vt",		WOBJP_vt);
	cTypes.Add("vn",		WOBJP_vn);
	cTypes.Add("g",			WOBJP_g);
	cTypes.Add("f",			WOBJP_f);
	cTypes.Add("s",			WOBJP_s);
	cTypes.Add("usemtl",	WOBJP_usemtl);
	cTypes.Add("mtllib",	WOBJP_mtllib);

	cTextFile.Init();
	if (!cTextFile.Read(mszFileName.Text()))
	{
		cTypes.Kill();
		cTextFile.Kill();
		return false;
	}

	cTextFile.PassifyNewlines();
	cLines.Fake();
	cTextFile.mcText.SplitLines(&cLines);
	

	//The Mesh/MeshEditor must already be initialised.
	mcWrapper.ClearConnectivity();

	for (i = 0; i < cLines.NumElements(); i++)
	{
		pszLine = cLines.Get(i);
		cTextParser.Init(pszLine->Text(), pszLine->Length());

		tResult = cTextParser.GetEnumeratorIdentifier(&cTypes, &iType);

		if (tResult == TRITRUE)
		{
			switch (iType)
			{
			case WOBJP_v:
				ReturnOnFalse(ReadPosition(&cTextParser));
				break;
			case WOBJP_vt:
				ReturnOnFalse(ReadUVCoord(&cTextParser));
				break;
			case WOBJP_vn:
				ReturnOnFalse(ReadNormal(&cTextParser));
				break;
			case WOBJP_g:
				ReturnOnFalse(ReadObjectName(&cTextParser));
				break;
			case WOBJP_f:
				ReturnOnFalse(ReadFace(&cTextParser, mcWrapper.NumCorners(), mcWrapper.NumNormals(), mcWrapper.NumUVs()));
				break;
			case WOBJP_s:
				ReturnOnFalse(ReadSmoothingGroup(&cTextParser));
				break;
			case WOBJP_usemtl:
				ReturnOnFalse(ReadMaterial(&cTextParser));
				break;
			case WOBJP_mtllib:
				ReturnOnFalse(ReadMaterialLibrary(&cTextParser));
				break;
			default:
				break;
			};
		}

		cTextParser.Kill();
	}
	
	cLines.Kill();
	cTypes.Kill();
	cTextFile.Kill();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadPosition(CTextParser* pcTextParser)
{
	float				x, y, z;

	pcTextParser->GetFloat(&x);
	pcTextParser->GetFloat(&y);
	pcTextParser->GetFloat(&z);
	mcWrapper.AddCorner(x, y, z);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadNormal(CTextParser* pcTextParser)
{
	float				x, y, z;

	pcTextParser->GetFloat(&x);
	pcTextParser->GetFloat(&y);
	pcTextParser->GetFloat(&z);
	mcWrapper.SetNormalsInUse(true);
	mcWrapper.AddNormal(x, y, z);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadUVCoord(CTextParser* pcTextParser)
{
	float				u, v;

	pcTextParser->GetFloat(&u);
	pcTextParser->GetFloat(&v);
	mcWrapper.SetUVsInUse(true);
	mcWrapper.AddUV(u, v);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadFace(CTextParser* pcTextParser, int iNumCorners, int iNumNormals, int iNumUVs)
{
	TRISTATE				tResult;
	CArrayMeshCornerBuilder	asCorners;
	int64					iCorner;
	int64					iUV;
	CMeshCornerBuilder*		psCorner;
	int64					iNormal;
	CArrayInt				aiCorners;
	size					i;
	int						iNumFaces;
	bool					bNormals;
	bool					bUVS;
	SMeshNormalFace*		psFaceNormal;
	CMFRet					r;
	int						aiCornerIndices[6];

	asCorners.Init();
	while (true)
	{
		tResult = pcTextParser->GetInteger(&iCorner);
		if (tResult != TRITRUE)
		{
			break;
		}
		psCorner = asCorners.Add();
		psCorner->Init((int)iCorner);

		//No skipping whitespace because // is whitespace.
		tResult = pcTextParser->GetExactCharacter('/', false);
		if (tResult == TRITRUE)
		{
			tResult = pcTextParser->GetInteger(&iUV);
			if (tResult == TRITRUE)
			{
				psCorner->bUVCoord = true;
				psCorner->iUVCoord = (int)iUV;
			}

			tResult = pcTextParser->GetExactCharacter('/', false);
			if (tResult == TRITRUE)
			{
				tResult = pcTextParser->GetInteger(&iNormal);
				if (tResult == TRITRUE)
				{
					psCorner->bNormal = true;
					psCorner->iNormal = (int)iNormal;
				}
			}
		}
	}

	bNormals = true;
	bUVS = true;

	aiCorners.Init();
	aiCorners.Resize(asCorners.NumElements());
	for (i = 0; i < asCorners.NumElements(); i++)
	{
		psCorner = asCorners.Get(i);
		iCorner = psCorner->iVertex;
		if (iCorner < 0)
		{
			iCorner = iNumCorners + iCorner;
		}
		aiCorners.SetValue(i, (int)iCorner);

		if (!psCorner->bNormal)
		{
			bNormals = false;
		}
		if (!psCorner->bUVCoord)
		{
			bUVS = false;
		}
	}

	r = mcWrapper.AddPolygon(&aiCorners);
	iNumFaces = mcWrapper.NumFaces() - r.miFirstFace;

	if (bNormals)
	{
		if (iNumFaces <= 2)
		{
			if (iNumFaces == 1)
			{
				r.Unpack1(&aiCornerIndices[0], &aiCornerIndices[1], &aiCornerIndices[2]);
				psFaceNormal = mcWrapper.GetFaceNormal(r.miFirstFace);
				psFaceNormal->aiCornerNormals[aiCornerIndices[0]] = iNumNormals + asCorners.Get(0)->iNormal;
				psFaceNormal->aiCornerNormals[aiCornerIndices[1]] = iNumNormals + asCorners.Get(1)->iNormal;
				psFaceNormal->aiCornerNormals[aiCornerIndices[2]] = iNumNormals + asCorners.Get(2)->iNormal;
			}
			else if (iNumFaces == 2)
			{
				psCorner = asCorners.Get(0);
				r.Unpack1(&aiCornerIndices[0], &aiCornerIndices[1], &aiCornerIndices[2]);
				psFaceNormal = mcWrapper.GetFaceNormal(r.miFirstFace);
				psFaceNormal->aiCornerNormals[aiCornerIndices[0]] = iNumNormals + asCorners.Get(0)->iNormal;
				psFaceNormal->aiCornerNormals[aiCornerIndices[1]] = iNumNormals + asCorners.Get(1)->iNormal;
				psFaceNormal->aiCornerNormals[aiCornerIndices[2]] = iNumNormals + asCorners.Get(2)->iNormal;

				psCorner = asCorners.Get(1);
				r.Unpack2(&aiCornerIndices[3], &aiCornerIndices[4], &aiCornerIndices[5]);
				psFaceNormal = mcWrapper.GetFaceNormal(r.miFirstFace);
				psFaceNormal->aiCornerNormals[aiCornerIndices[3]] = iNumNormals + asCorners.Get(3)->iNormal;
				psFaceNormal->aiCornerNormals[aiCornerIndices[4]] = iNumNormals + asCorners.Get(4)->iNormal;
				psFaceNormal->aiCornerNormals[aiCornerIndices[5]] = iNumNormals + asCorners.Get(5)->iNormal;
			}
		}
	}

	aiCorners.Kill();
	asCorners.Kill();
	return true;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadObjectName(CTextParser* pcTextParser)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadMaterialLibrary(CTextParser* pcTextParser)
{
	pcTextParser->SkipWhiteSpace();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadMaterial(CTextParser* pcTextParser)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjReader::ReadSmoothingGroup(CTextParser* pcTextParser)
{
	return true;
}

