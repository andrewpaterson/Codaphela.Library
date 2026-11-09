/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Logger.h"
#include "OldDragonCursor.h"
#include "OldDragonOle.h"
#include "OldDragonReader.h"


//A .max file's Scene stream is a list of objects, each a tree of chunks.  An object's chunk ID is the index of its class in
//the ClassDirectory stream.  None of this is documented: it was worked out from the files, and checked by the vertices
//matching what the exporter wrote to Melee.DRG, one for one.
#define MAX_CHUNK_CLASS_INFO		0x2060
#define MAX_CHUNK_REFERENCES		0x2034
#define MAX_CHUNK_SLOT_REFERENCES	0x2035
#define MAX_CHUNK_NODE_NAME			0x0962
#define MAX_CHUNK_MATERIAL			0x4000
#define MAX_CHUNK_MATERIAL_NAME		0x4001
#define MAX_CHUNK_POLY				0x08fe
#define MAX_CHUNK_POLY_VERTICES		0x0100
#define MAX_CHUNK_POLY_FACES		0x011a
#define MAX_CHUNK_POLY_CHANNEL		0x0124
#define MAX_CHUNK_POLY_MAP_VERTICES	0x0128
#define MAX_CHUNK_POLY_MAP_FACES	0x012b

#define MAX_CLASS_NODE_A			0x00000001
#define MAX_CLASS_NODE_B			0x00000000
#define MAX_CLASS_NODE_SUPER		0x00000001
#define MAX_CLASS_POLY_A			0x1bf8338d
#define MAX_CLASS_POLY_B			0x192f6098
#define MAX_CLASS_STANDARD_A		0x00000002
#define MAX_CLASS_MULTI_A			0x00000200
#define MAX_CLASS_MATERIAL_B		0x00000000
#define MAX_CLASS_MATERIAL_SUPER	0x00000c00
#define MAX_SUPER_GEOMETRY			0x00000010

#define MAX_SLOT_OBJECT				1
#define MAX_SLOT_MATERIAL			3

#define MAX_FACE_HAS_SMOOTHING		0x01
#define MAX_FACE_HAS_MATERIAL		0x08
#define MAX_FACE_HAS_EXTRA			0x10
#define MAX_FACE_HAS_DIAGONALS		0x20
#define MAX_FACE_KNOWN_FLAGS		(MAX_FACE_HAS_SMOOTHING | MAX_FACE_HAS_MATERIAL | MAX_FACE_HAS_EXTRA | MAX_FACE_HAS_DIAGONALS)

#define MAX_VERTEX_SIZE				16		//Flags, then x, y, z.
#define MAX_MAP_VERTEX_SIZE			12
#define MAX_MAX_POLYGON_CORNERS		1024
#define MAX_MATERIAL_DEPTH			8
#define MAX_UV_CHANNEL				1

#define MAX_SAME_POSITION			0.001f


struct SMaxChunk
{
	uint32			uiID;
	bool			bContainer;
	const uint8*	puiData;
	size			uiSize;
};


struct SMaxClass
{
	uint32	uiA;
	uint32	uiB;
	uint32	uiSuper;
};


struct SMaxObject
{
	uint32			uiClass;
	const uint8*	puiData;
	size			uiSize;
	bool			bContainer;
};


struct SMaxReader
{
	CArrayTemplate<SMaxClass>	asClasses;
	CArrayTemplate<SMaxObject>	asObjects;
	CArrayTemplate<int32>		aiObjectNodes;		//For each object, the node that uses it or -1.
	CArrayTemplate<int32>		aiObjectMaterials;	//For each object, the scene material made from it or -1.
	COldDragonScene*			pcScene;
};


//////////////////////////////////////////////////////////////////////////
//
// id(2) size(4), or if that size is zero then size(8).  A size includes
// its header and its top bit says the chunk holds chunks.  Returns false
// at the end, and sets *pbBad if that was because the chunk made no sense.
//
//////////////////////////////////////////////////////////////////////////
static bool NextMaxChunk(const uint8** ppuiPos, const uint8* puiEnd, SMaxChunk* psChunk, bool* pbBad)
{
	const uint8*	puiPos;
	size			uiLeft;
	uint16			uiID;
	uint32			uiSize;
	uint64			ulliSize;
	uint64			ulliTotal;
	size			uiHeader;

	puiPos = *ppuiPos;
	uiLeft = (size)(puiEnd - puiPos);
	if (uiLeft == 0)
	{
		return false;
	}
	if (uiLeft < 6)
	{
		*pbBad = true;
		return false;
	}

	memcpy(&uiID, puiPos, sizeof(uint16));
	memcpy(&uiSize, puiPos + sizeof(uint16), sizeof(uint32));
	uiHeader = 6;
	if (uiSize == 0)
	{
		if (uiLeft < 14)
		{
			*pbBad = true;
			return false;
		}
		memcpy(&ulliSize, puiPos + 6, sizeof(uint64));
		uiHeader = 14;
		psChunk->bContainer = (ulliSize >> 63) != 0;
		ulliTotal = ulliSize & 0x7fffffffffffffffULL;
	}
	else
	{
		psChunk->bContainer = (uiSize & 0x80000000) != 0;
		ulliTotal = uiSize & 0x7fffffff;
	}

	if ((ulliTotal < uiHeader) || (ulliTotal > (uint64)uiLeft))
	{
		*pbBad = true;
		return false;
	}

	psChunk->uiID = uiID;
	psChunk->puiData = puiPos + uiHeader;
	psChunk->uiSize = (size)ulliTotal - uiHeader;
	*ppuiPos = puiPos + (size)ulliTotal;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool FindMaxChild(const uint8* puiData, size uiSize, uint32 uiID, SMaxChunk* psChild)
{
	const uint8*	puiPos;
	bool			bBad;

	puiPos = puiData;
	bBad = false;
	while (NextMaxChunk(&puiPos, puiData + uiSize, psChild, &bBad))
	{
		if (psChild->uiID == uiID)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void CopyMaxName(char* szDest, size uiDestSize, const uint8* puiName, size uiNameSize)
{
	size	i;
	size	uiLength;

	uiLength = uiNameSize / 2;
	for (i = 0; (i < uiLength) && (i + 1 < uiDestSize) && (puiName[i * 2] != 0); i++)
	{
		szDest[i] = (puiName[i * 2 + 1] == 0) ? (char)puiName[i * 2] : '?';
	}
	szDest[i] = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxClasses(COldDragonOle* pcOle, SMaxReader* psReader)
{
	uint8*			puiStream;
	size			uiSize;
	const uint8*	puiPos;
	SMaxChunk		sEntry;
	SMaxChunk		sInfo;
	SMaxClass		sClass;
	uint32			auiInfo[4];
	bool			bBad;

	if (!pcOle->ReadStream("ClassDirectory", &puiStream, &uiSize))
	{
		return gcLogger.Error2(__METHOD__, " No ClassDirectory stream.", NULL);
	}

	puiPos = puiStream;
	bBad = false;
	while (NextMaxChunk(&puiPos, puiStream + uiSize, &sEntry, &bBad))
	{
		memset(&sClass, 0, sizeof(sClass));
		if (FindMaxChild(sEntry.puiData, sEntry.uiSize, MAX_CHUNK_CLASS_INFO, &sInfo) && (sInfo.uiSize == sizeof(auiInfo)))
		{
			memcpy(auiInfo, sInfo.puiData, sizeof(auiInfo));
			sClass.uiA = auiInfo[1];
			sClass.uiB = auiInfo[2];
			sClass.uiSuper = auiInfo[3];
		}
		psReader->asClasses.Add(&sClass);
	}
	free(puiStream);
	return !bBad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxObjects(const uint8* puiStream, size uiSize, SMaxReader* psReader)
{
	const uint8*	puiPos;
	SMaxChunk		sRoot;
	SMaxChunk		sObject;
	SMaxObject		sEntry;
	bool			bBad;
	int32			iNone;

	if ((uiSize >= 2) && (puiStream[0] == 0x1f) && (puiStream[1] == 0x8b))
	{
		return gcLogger.Error2(__METHOD__, " Compressed scenes aren't supported.", NULL);
	}

	puiPos = puiStream;
	bBad = false;
	if (!NextMaxChunk(&puiPos, puiStream + uiSize, &sRoot, &bBad) || !sRoot.bContainer)
	{
		return gcLogger.Error2(__METHOD__, " The Scene stream has no root chunk.", NULL);
	}

	iNone = -1;
	puiPos = sRoot.puiData;
	while (NextMaxChunk(&puiPos, sRoot.puiData + sRoot.uiSize, &sObject, &bBad))
	{
		sEntry.uiClass = sObject.uiID;
		sEntry.puiData = sObject.puiData;
		sEntry.uiSize = sObject.uiSize;
		sEntry.bContainer = sObject.bContainer;
		psReader->asObjects.Add(&sEntry);
		psReader->aiObjectNodes.Add(&iNone);
		psReader->aiObjectMaterials.Add(&iNone);
	}
	return !bBad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SMaxClass* GetMaxClass(SMaxReader* psReader, size uiObject)
{
	SMaxObject*	psObject;

	psObject = psReader->asObjects.Get(uiObject);
	if (psObject->uiClass >= psReader->asClasses.NumElements())
	{
		return NULL;
	}
	return psReader->asClasses.Get(psObject->uiClass);
}


//////////////////////////////////////////////////////////////////////////
//
// A node's references are {slot, object} after a first int that isn't.
// Returns -1 if the slot's empty.
//
//////////////////////////////////////////////////////////////////////////
static int32 MaxNodeSlot(SMaxObject* psNode, int32 iSlot)
{
	SMaxChunk	sReferences;
	size		i;
	size		uiNum;
	int32		aiPair[2];

	if (!psNode->bContainer || !FindMaxChild(psNode->puiData, psNode->uiSize, MAX_CHUNK_SLOT_REFERENCES, &sReferences))
	{
		return -1;
	}
	uiNum = sReferences.uiSize / sizeof(int32);
	for (i = 1; i + 1 < uiNum; i += 2)
	{
		memcpy(aiPair, &sReferences.puiData[i * sizeof(int32)], sizeof(aiPair));
		if (aiPair[0] == iSlot)
		{
			return aiPair[1];
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void LinkMaxNodes(SMaxReader* psReader)
{
	size		i;
	SMaxClass*	psClass;
	int32		iObject;

	for (i = 0; i < psReader->asObjects.NumElements(); i++)
	{
		psClass = GetMaxClass(psReader, i);
		if ((psClass == NULL) || (psClass->uiA != MAX_CLASS_NODE_A) || (psClass->uiB != MAX_CLASS_NODE_B) || (psClass->uiSuper != MAX_CLASS_NODE_SUPER))
		{
			continue;
		}
		iObject = MaxNodeSlot(psReader->asObjects.Get(i), MAX_SLOT_OBJECT);
		if ((iObject >= 0) && ((size)iObject < psReader->asObjects.NumElements()))
		{
			if (*psReader->aiObjectNodes.Get((size)iObject) == -1)
			{
				*psReader->aiObjectNodes.Get((size)iObject) = (int32)i;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The scene material for a face.  A Standard material is one.  A
// Multi/Sub-Object picks one by the face's material number, wrapping as
// Max does.  Anything else, or an empty slot, is no material.
//
//////////////////////////////////////////////////////////////////////////
static int32 ResolveMaxMaterial(SMaxReader* psReader, int32 iObject, uint32 uiFaceMaterial, int32 iDepth)
{
	SMaxObject*			psObject;
	SMaxClass*			psClass;
	SMaxChunk			sReferences;
	SMaxChunk			sMaterial;
	SMaxChunk			sName;
	SOldDragonMaterial*	psMaterial;
	int32*				piMade;
	int32				iSub;
	size				uiNum;
	size				uiFirst;
	size				uiSlot;

	if ((iObject < 0) || ((size)iObject >= psReader->asObjects.NumElements()) || (iDepth > MAX_MATERIAL_DEPTH))
	{
		return OLD_DRAGON_NO_INDEX;
	}
	psObject = psReader->asObjects.Get((size)iObject);
	psClass = GetMaxClass(psReader, (size)iObject);
	if ((psClass == NULL) || !psObject->bContainer || (psClass->uiB != MAX_CLASS_MATERIAL_B) || (psClass->uiSuper != MAX_CLASS_MATERIAL_SUPER))
	{
		return OLD_DRAGON_NO_INDEX;
	}

	if (psClass->uiA == MAX_CLASS_STANDARD_A)
	{
		piMade = psReader->aiObjectMaterials.Get((size)iObject);
		if (*piMade == -1)
		{
			*piMade = (int32)psReader->pcScene->macMaterials.NumElements();
			psMaterial = psReader->pcScene->AddMaterial();
			psMaterial->iID = iObject;
			if (FindMaxChild(psObject->puiData, psObject->uiSize, MAX_CHUNK_MATERIAL, &sMaterial) && FindMaxChild(sMaterial.puiData, sMaterial.uiSize, MAX_CHUNK_MATERIAL_NAME, &sName))
			{
				CopyMaxName(psMaterial->szName, sizeof(psMaterial->szName), sName.puiData, sName.uiSize);
			}
		}
		return *piMade;
	}

	if (psClass->uiA != MAX_CLASS_MULTI_A)
	{
		return OLD_DRAGON_NO_INDEX;
	}

	//The first reference is the material's parameter block and the rest are its slots.
	if (!FindMaxChild(psObject->puiData, psObject->uiSize, MAX_CHUNK_REFERENCES, &sReferences))
	{
		return OLD_DRAGON_NO_INDEX;
	}
	uiNum = sReferences.uiSize / sizeof(int32);
	uiFirst = 1;
	if (uiNum <= uiFirst)
	{
		return OLD_DRAGON_NO_INDEX;
	}
	uiSlot = uiFirst + (size)uiFaceMaterial % (uiNum - uiFirst);
	memcpy(&iSub, &sReferences.puiData[uiSlot * sizeof(int32)], sizeof(int32));
	return ResolveMaxMaterial(psReader, iSub, uiFaceMaterial, iDepth + 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Cuts a polygon into triangles along Max's own diagonals.  Any ear whose
// chord is one of the diagonals can be cut off, and a triangulated polygon
// always has one.  aiTriangles gets corners of the polygon, three for each.
//
//////////////////////////////////////////////////////////////////////////
static bool TriangulateMaxPolygon(size uiCorners, const uint8* puiDiagonals, size uiNumDiagonals, int32* aiTriangles)
{
	int32	aiRing[MAX_MAX_POLYGON_CORNERS];
	size	uiRing;
	size	i;
	size	j;
	size	k;
	size	uiNext;
	size	uiAfter;
	int32	aiDiagonal[2];
	bool	bCut;
	size	uiMade;

	if (puiDiagonals == NULL)
	{
		for (i = 1; i + 1 < uiCorners; i++)
		{
			aiTriangles[(i - 1) * 3 + 0] = 0;
			aiTriangles[(i - 1) * 3 + 1] = (int32)i;
			aiTriangles[(i - 1) * 3 + 2] = (int32)(i + 1);
		}
		return true;
	}

	for (i = 0; i < uiCorners; i++)
	{
		aiRing[i] = (int32)i;
	}
	uiRing = uiCorners;
	uiMade = 0;
	while (uiRing > 3)
	{
		bCut = false;
		for (i = 0; (i < uiRing) && !bCut; i++)
		{
			uiNext = (i + 1) % uiRing;
			uiAfter = (i + 2) % uiRing;
			for (j = 0; (j < uiNumDiagonals) && !bCut; j++)
			{
				memcpy(aiDiagonal, &puiDiagonals[j * sizeof(aiDiagonal)], sizeof(aiDiagonal));
				if (((aiDiagonal[0] == aiRing[i]) && (aiDiagonal[1] == aiRing[uiAfter])) || ((aiDiagonal[1] == aiRing[i]) && (aiDiagonal[0] == aiRing[uiAfter])))
				{
					aiTriangles[uiMade * 3 + 0] = aiRing[i];
					aiTriangles[uiMade * 3 + 1] = aiRing[uiNext];
					aiTriangles[uiMade * 3 + 2] = aiRing[uiAfter];
					uiMade++;
					for (k = uiNext; k + 1 < uiRing; k++)
					{
						aiRing[k] = aiRing[k + 1];
					}
					uiRing--;
					bCut = true;
				}
			}
		}
		if (!bCut)
		{
			return false;
		}
	}
	aiTriangles[uiMade * 3 + 0] = aiRing[0];
	aiTriangles[uiMade * 3 + 1] = aiRing[1];
	aiTriangles[uiMade * 3 + 2] = aiRing[2];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Max is z up, so these turn positions and UVs the way the exporter did:
// a position's y is its z and its z is minus its y, and v is flipped.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxPositions(SMaxChunk* psChunk, COldDragonMesh* pcMesh)
{
	uint32			uiNum;
	uint32			i;
	float32			afVertex[3];
	SOldDragonVec3	sPosition;

	if (psChunk->uiSize < sizeof(uint32))
	{
		return false;
	}
	memcpy(&uiNum, psChunk->puiData, sizeof(uint32));
	if ((uint64)psChunk->uiSize != (uint64)sizeof(uint32) + (uint64)uiNum * MAX_VERTEX_SIZE)
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		memcpy(afVertex, &psChunk->puiData[sizeof(uint32) + (size)i * MAX_VERTEX_SIZE + sizeof(uint32)], sizeof(afVertex));
		sPosition.x = afVertex[0];
		sPosition.y = afVertex[2];
		sPosition.z = -afVertex[1];
		pcMesh->macPositions.Add(&sPosition);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxUVs(SMaxChunk* psChunk, COldDragonMesh* pcMesh)
{
	uint32			uiNum;
	uint32			i;
	float32			afVertex[3];
	SOldDragonVec2	sUV;

	if (psChunk->uiSize < sizeof(uint32))
	{
		return false;
	}
	memcpy(&uiNum, psChunk->puiData, sizeof(uint32));
	if ((uint64)psChunk->uiSize != (uint64)sizeof(uint32) + (uint64)uiNum * MAX_MAP_VERTEX_SIZE)
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		memcpy(afVertex, &psChunk->puiData[sizeof(uint32) + (size)i * MAX_MAP_VERTEX_SIZE], sizeof(afVertex));
		sUV.x = afVertex[0];
		sUV.y = 1.0f - afVertex[1];
		pcMesh->macUVs.Add(&sUV);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Each polygon: number of corners, its positions, flags (a short), and
// whichever of smoothing, material (a short), an unknown int and the
// diagonals the flags say follow.  The map polygons run alongside them:
// number of corners, then that many UVs.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxFaces(SMaxChunk* psFaces, SMaxChunk* psMapFaces, COldDragonMesh* pcMesh, SMaxReader* psReader, int32 iMaterialObject)
{
	COldDragonCursor	cCursor;
	COldDragonCursor	cMap;
	uint32				uiNumPolys;
	uint32				i;
	uint32				uiCorners;
	const uint8*		puiPositions;
	const uint8*		puiDiagonals;
	const uint8*		puiMapCorners;
	const uint8*		puiBytes;
	uint16				uiFlags;
	uint16				uiMaterial;
	uint32				uiSmoothing;
	size				uiNumDiagonals;
	size				t;
	size				c;
	int32				aiTriangles[(MAX_MAX_POLYGON_CORNERS - 2) * 3];
	int32				iCorner;
	int32				iValue;
	uint32				uiMapCorners;
	SOldDragonFace		sFace;
	int32				iFaceMaterial;

	cCursor.Init(psFaces->puiData, psFaces->uiSize);
	uiNumPolys = cCursor.ReadUint32();
	if (psMapFaces != NULL)
	{
		cMap.Init(psMapFaces->puiData, psMapFaces->uiSize);
	}

	for (i = 0; i < uiNumPolys; i++)
	{
		uiCorners = cCursor.ReadUint32();
		if ((uiCorners < 3) || (uiCorners > MAX_MAX_POLYGON_CORNERS))
		{
			return false;
		}
		puiPositions = cCursor.ReadBytes((size)uiCorners * sizeof(int32));

		puiBytes = cCursor.ReadBytes(sizeof(uint16));
		if (puiBytes == NULL)
		{
			return false;
		}
		memcpy(&uiFlags, puiBytes, sizeof(uint16));
		if ((uiFlags & ~MAX_FACE_KNOWN_FLAGS) != 0)
		{
			return false;
		}

		uiSmoothing = 0;
		if (uiFlags & MAX_FACE_HAS_SMOOTHING)
		{
			uiSmoothing = cCursor.ReadUint32();
		}
		uiMaterial = 0;
		if (uiFlags & MAX_FACE_HAS_MATERIAL)
		{
			puiBytes = cCursor.ReadBytes(sizeof(uint16));
			if (puiBytes == NULL)
			{
				return false;
			}
			memcpy(&uiMaterial, puiBytes, sizeof(uint16));
		}
		if (uiFlags & MAX_FACE_HAS_EXTRA)
		{
			cCursor.ReadUint32();
		}
		puiDiagonals = NULL;
		uiNumDiagonals = 0;
		if (uiFlags & MAX_FACE_HAS_DIAGONALS)
		{
			if (uiCorners < 3)
			{
				return false;
			}
			uiNumDiagonals = (size)uiCorners - 3;
			puiDiagonals = cCursor.ReadBytes(uiNumDiagonals * 2 * sizeof(int32));
		}

		puiMapCorners = NULL;
		if (psMapFaces != NULL)
		{
			uiMapCorners = cMap.ReadUint32();
			if (uiMapCorners != uiCorners)
			{
				return false;
			}
			puiMapCorners = cMap.ReadBytes((size)uiMapCorners * sizeof(int32));
		}
		if (cCursor.Failed() || ((psMapFaces != NULL) && cMap.Failed()))
		{
			return false;
		}

		if ((uiCorners > 3) && !TriangulateMaxPolygon(uiCorners, puiDiagonals, uiNumDiagonals, aiTriangles))
		{
			return false;
		}
		if (uiCorners == 3)
		{
			aiTriangles[0] = 0;
			aiTriangles[1] = 1;
			aiTriangles[2] = 2;
		}

		iFaceMaterial = ResolveMaxMaterial(psReader, iMaterialObject, uiMaterial, 0);
		for (t = 0; t < (size)uiCorners - 2; t++)
		{
			for (c = 0; c < 3; c++)
			{
				iCorner = aiTriangles[t * 3 + c];
				if ((iCorner < 0) || ((uint32)iCorner >= uiCorners))
				{
					return false;
				}
				memcpy(&iValue, &puiPositions[(size)iCorner * sizeof(int32)], sizeof(int32));
				sFace.aiPositions[c] = iValue;
				sFace.aiNormals[c] = OLD_DRAGON_NO_INDEX;
				sFace.aiUVs[c] = OLD_DRAGON_NO_INDEX;
				if (puiMapCorners != NULL)
				{
					memcpy(&iValue, &puiMapCorners[(size)iCorner * sizeof(int32)], sizeof(int32));
					sFace.aiUVs[c] = iValue;
				}
			}
			sFace.uiSmoothing = uiSmoothing;
			sFace.iMaterial = iFaceMaterial;
			pcMesh->macFaces.Add(&sFace);
		}
	}
	return !cCursor.Failed() && (cCursor.Remaining() == 0) && ((psMapFaces == NULL) || (!cMap.Failed() && (cMap.Remaining() == 0)));
}


//////////////////////////////////////////////////////////////////////////
//
// The children of an Editable Poly's 0x08FE chunk, which has to have
// positions and polygons.  UVs come from channel 1 if there is one: a
// channel's number is followed by its map vertices and polygons.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxPoly(SMaxReader* psReader, size uiObject, SMaxChunk* psPoly)
{
	COldDragonMesh*	pcMesh;
	const uint8*	puiPos;
	SMaxChunk		sChunk;
	SMaxChunk		sPositions;
	SMaxChunk		sFaces;
	SMaxChunk		sMapVertices;
	SMaxChunk		sMapFaces;
	bool			bBad;
	bool			bPositions;
	bool			bFaces;
	bool			bMapVertices;
	bool			bMapFaces;
	uint32			uiChannel;
	int32			iNode;
	int32			iMaterial;
	SMaxObject*		psNode;
	SMaxChunk		sName;

	bPositions = false;
	bFaces = false;
	bMapVertices = false;
	bMapFaces = false;
	bBad = false;
	uiChannel = 0xffffffff;
	puiPos = psPoly->puiData;
	while (NextMaxChunk(&puiPos, psPoly->puiData + psPoly->uiSize, &sChunk, &bBad))
	{
		switch (sChunk.uiID)
		{
		case MAX_CHUNK_POLY_VERTICES:
			sPositions = sChunk;
			bPositions = true;
			break;
		case MAX_CHUNK_POLY_FACES:
			sFaces = sChunk;
			bFaces = true;
			break;
		case MAX_CHUNK_POLY_CHANNEL:
			uiChannel = 0xffffffff;
			if (sChunk.uiSize == sizeof(uint32))
			{
				memcpy(&uiChannel, sChunk.puiData, sizeof(uint32));
			}
			break;
		case MAX_CHUNK_POLY_MAP_VERTICES:
			if (uiChannel == MAX_UV_CHANNEL)
			{
				sMapVertices = sChunk;
				bMapVertices = true;
			}
			break;
		case MAX_CHUNK_POLY_MAP_FACES:
			if (uiChannel == MAX_UV_CHANNEL)
			{
				sMapFaces = sChunk;
				bMapFaces = true;
			}
			break;
		default:
			break;
		}
	}
	if (bBad)
	{
		return false;
	}

	//A poly with nothing in it, like one that's been deleted down to nothing, isn't a mesh.
	if (!bPositions || !bFaces)
	{
		psReader->pcScene->muiSkippedChunks++;
		return true;
	}

	pcMesh = psReader->pcScene->AddMesh();
	pcMesh->miID = (int32)uiObject;

	iMaterial = -1;
	iNode = *psReader->aiObjectNodes.Get(uiObject);
	if (iNode != -1)
	{
		psNode = psReader->asObjects.Get((size)iNode);
		iMaterial = MaxNodeSlot(psNode, MAX_SLOT_MATERIAL);
		if (FindMaxChild(psNode->puiData, psNode->uiSize, MAX_CHUNK_NODE_NAME, &sName))
		{
			CopyMaxName(pcMesh->mszName, sizeof(pcMesh->mszName), sName.puiData, sName.uiSize);
		}
	}

	ReturnOnFalse(ReadMaxPositions(&sPositions, pcMesh));
	if (bMapVertices && bMapFaces)
	{
		ReturnOnFalse(ReadMaxUVs(&sMapVertices, pcMesh));
		ReturnOnFalse(ReadMaxFaces(&sFaces, &sMapFaces, pcMesh, psReader, iMaterial));
	}
	else
	{
		ReturnOnFalse(ReadMaxFaces(&sFaces, NULL, pcMesh, psReader, iMaterial));
	}
	return ValidateOldDragonMesh(pcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxGeometry(SMaxReader* psReader)
{
	size		i;
	SMaxClass*	psClass;
	SMaxObject*	psObject;
	SMaxChunk	sPoly;

	for (i = 0; i < psReader->asObjects.NumElements(); i++)
	{
		psClass = GetMaxClass(psReader, i);
		psObject = psReader->asObjects.Get(i);
		if ((psClass == NULL) || !psObject->bContainer || (psClass->uiSuper != MAX_SUPER_GEOMETRY))
		{
			continue;
		}
		if ((psClass->uiA != MAX_CLASS_POLY_A) || (psClass->uiB != MAX_CLASS_POLY_B))
		{
			psReader->pcScene->muiSkippedChunks++;
			continue;
		}
		if (!FindMaxChild(psObject->puiData, psObject->uiSize, MAX_CHUNK_POLY, &sPoly) || !sPoly.bContainer)
		{
			psReader->pcScene->muiSkippedChunks++;
			continue;
		}
		ReturnOnFalse(ReadMaxPoly(psReader, i, &sPoly));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the Editable Polys in a 3ds Max file.  There's no placing them:
// where things are is in controllers that aren't decoded.
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragonMaxScene(const char* szFilename, COldDragonScene* pcScene)
{
	COldDragonOle	cOle;
	SMaxReader		sReader;
	uint8*			puiScene;
	size			uiScene;
	bool			bResult;

	puiScene = NULL;
	sReader.asClasses.Init();
	sReader.asObjects.Init();
	sReader.aiObjectNodes.Init();
	sReader.aiObjectMaterials.Init();
	sReader.pcScene = pcScene;

	bResult = cOle.Init(szFilename);
	if (bResult)
	{
		bResult = ReadMaxClasses(&cOle, &sReader) && cOle.ReadStream("Scene", &puiScene, &uiScene);
	}
	if (bResult)
	{
		bResult = ReadMaxObjects(puiScene, uiScene, &sReader);
	}
	if (bResult)
	{
		LinkMaxNodes(&sReader);
		bResult = ReadMaxGeometry(&sReader);
	}
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read Max file [", szFilename, "].", NULL);
	}

	free(puiScene);
	sReader.aiObjectMaterials.Kill();
	sReader.aiObjectNodes.Kill();
	sReader.asObjects.Kill();
	sReader.asClasses.Kill();
	cOle.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool SameOldDragonPositions(COldDragonMesh* pcA, COldDragonMesh* pcB)
{
	size			i;
	SOldDragonVec3*	psA;
	SOldDragonVec3*	psB;

	if (pcA->macPositions.NumElements() != pcB->macPositions.NumElements())
	{
		return false;
	}
	for (i = 0; i < pcA->macPositions.NumElements(); i++)
	{
		psA = pcA->macPositions.Get(i);
		psB = pcB->macPositions.Get(i);
		if ((fabsf(psA->x - psB->x) > MAX_SAME_POSITION) || (fabsf(psA->y - psB->y) > MAX_SAME_POSITION) || (fabsf(psA->z - psB->z) > MAX_SAME_POSITION))
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
static int32 FindOldDragonMaterial(COldDragonScene* pcScene, const char* szName)
{
	size	i;

	for (i = 0; i < pcScene->macMaterials.NumElements(); i++)
	{
		if (strcmp(pcScene->macMaterials.Get(i)->szName, szName) == 0)
		{
			return (int32)i;
		}
	}
	return OLD_DRAGON_NO_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void CopyMaxFaces(COldDragonMesh* pcSource, COldDragonScene* pcSourceScene, COldDragonMesh* pcDest, COldDragonScene* pcDestScene)
{
	size			i;
	SOldDragonFace	sFace;

	for (i = 0; i < pcSource->macUVs.NumElements(); i++)
	{
		pcDest->macUVs.Add(pcSource->macUVs.Get(i));
	}
	for (i = 0; i < pcSource->macFaces.NumElements(); i++)
	{
		sFace = *pcSource->macFaces.Get(i);
		if (sFace.iMaterial != OLD_DRAGON_NO_INDEX)
		{
			sFace.iMaterial = FindOldDragonMaterial(pcDestScene, pcSourceScene->macMaterials.Get((size)sFace.iMaterial)->szName);
		}
		pcDest->macFaces.Add(&sFace);
	}
	if (pcDest->mszName[0] == 0)
	{
		strcpy(pcDest->mszName, pcSource->mszName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh only takes faces from a Max mesh with the same positions in the
// same order, so faces can never land on the wrong mesh or the wrong file.
//
//////////////////////////////////////////////////////////////////////////
bool AddOldDragonMaxFaces(const char* szMaxFilename, COldDragonScene* pcScene, size* puiMeshesFilled)
{
	COldDragonScene	cMaxScene;
	size			i;
	size			j;
	COldDragonMesh*	pcMesh;
	COldDragonMesh*	pcSource;
	bool			bResult;

	*puiMeshesFilled = 0;
	cMaxScene.Init();
	bResult = ReadOldDragonMaxScene(szMaxFilename, &cMaxScene);
	if (bResult)
	{
		for (i = 0; i < pcScene->macMeshes.NumElements(); i++)
		{
			pcMesh = pcScene->macMeshes.Get(i);
			if (pcMesh->macFaces.NumElements() != 0)
			{
				continue;
			}
			for (j = 0; j < cMaxScene.macMeshes.NumElements(); j++)
			{
				pcSource = cMaxScene.macMeshes.Get(j);
				if (SameOldDragonPositions(pcMesh, pcSource))
				{
					CopyMaxFaces(pcSource, &cMaxScene, pcMesh, pcScene);
					(*puiMeshesFilled)++;
					break;
				}
			}
		}
	}
	cMaxScene.Kill();
	return bResult;
}

