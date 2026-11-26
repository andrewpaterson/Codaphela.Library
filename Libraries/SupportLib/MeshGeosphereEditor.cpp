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
#include <stdlib.h>
#include "BaseLib/Numbers.h"
#include "MeshEditor.h"
#include "MeshGeosphereEditor.h"


//The icosahedron's faces: the five around the top pole (corner 0), the ten around its middle between the upper ring
//(corners 1 to 5) and the lower (6 to 10), and the five around the bottom pole (11).
static const int gaaiIcosahedronFaces[GEOSPHERE_ICOSAHEDRON_FACES][3] =
{
	{0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 1},
	{1, 6, 2}, {2, 6, 7}, {2, 7, 3}, {3, 7, 8}, {3, 8, 4}, {4, 8, 9}, {4, 9, 5}, {5, 9, 10}, {5, 10, 1}, {1, 10, 6},
	{11, 7, 6}, {11, 8, 7}, {11, 9, 8}, {11, 10, 9}, {11, 6, 10},
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psCentre, float32 fRadius, int iSegments)
{
	PreInit();

	msZDirection.Init(psZDirection);
	msZDirection.Normalize();
	msCentre.Init(psCentre);
	mfRadius = fRadius;
	miSegments = iSegments;
	mbFlipFaces = false;
	miFaceName = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::Class(void)
{
	CMeshSourceModifier::Class();
	U_3Float32(msZDirection);
	U_3Float32(msCentre);
	U_Float32(mfRadius);
	U_SInt(miSegments);
	U_Bool(mbFlipFaces);
	U_SInt(miFaceName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::SetFlipFaces(bool bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::SetFaceName(int iFaceName)
{
	miFaceName = iFaceName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshGeosphereEditor::ExpectedFaceChunkSize(void)
{
	return GEOSPHERE_ICOSAHEDRON_FACES * miSegments * miSegments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshGeosphereEditor::ExpectedCornerChunkSize(void)
{
	return 10 * miSegments * miSegments + 2;
}


//////////////////////////////////////////////////////////////////////////
//
// Unit length.  The upper ring is at a height of 1/sqrt(5) and the lower
// one, turned a tenth of the way around, at -1/sqrt(5).
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::IcosahedronCorners(SFloat32Vec3* asCorners)
{
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sAlong;
	float32			fHeight;
	float32			fRingRadius;
	float32			fFifthRad;
	int				k;

	WedgeDirection(&sAcross, &msZDirection);
	fHeight = 1.0f / sqrtf(5.0f);
	fRingRadius = 2.0f / sqrtf(5.0f);
	fFifthRad = (N_PI*2.0f) / 5.0f;

	asCorners[0].Init(&msZDirection);
	for (k = 0; k < 5; k++)
	{
		sAlong.Init(&msZDirection);
		sAlong *= fHeight;
		Float32Vec3RotateAboutNormalisedAxis(&asCorners[1 + k], sAcross, fFifthRad * k, msZDirection);
		asCorners[1 + k] *= fRingRadius;
		asCorners[1 + k].Add(&sAlong);

		sAlong.Init(&msZDirection);
		sAlong *= -fHeight;
		Float32Vec3RotateAboutNormalisedAxis(&asCorners[6 + k], sAcross, fFifthRad * (k + 0.5f), msZDirection);
		asCorners[6 + k] *= fRingRadius;
		asCorners[6 + k].Add(&sAlong);
	}
	asCorners[11].Init(&msZDirection);
	asCorners[11] *= -1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// The corner iStep of miSegments along the icosahedron's edge from iFrom to
// iTo.  An edge's corners are numbered from its lower numbered end, after
// the icosahedron's own.
//
//////////////////////////////////////////////////////////////////////////
int CMeshGeosphereEditor::EdgeCorner(int aaiEdges[GEOSPHERE_ICOSAHEDRON_EDGES][2], int iFrom, int iTo, int iStep)
{
	int		e;
	int		iFirst;

	for (e = 0; e < GEOSPHERE_ICOSAHEDRON_EDGES; e++)
	{
		iFirst = GEOSPHERE_ICOSAHEDRON_CORNERS + (e * (miSegments - 1));
		if ((aaiEdges[e][0] == iFrom) && (aaiEdges[e][1] == iTo))
		{
			return iFirst + (iStep - 1);
		}
		else if ((aaiEdges[e][0] == iTo) && (aaiEdges[e][1] == iFrom))
		{
			return iFirst + (miSegments - iStep - 1);
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::AddCorner(CMeshEditor* pcMeshEditor, SFloat32Vec3* asDirections, int iCorner, SFloat32Vec3* psDirection)
{
	SFloat32Vec3	sPosition;

	asDirections[iCorner].Init(psDirection);
	asDirections[iCorner].Normalize();

	sPosition.Init(&asDirections[iCorner]);
	sPosition *= mfRadius;
	sPosition.Add(&msCentre);
	pcMeshEditor->AddCorner(&sPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// Turned so that its normal (as CMeshNormals makes it) points away from the
// centre, unless the faces are flipped.
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::AddOutwardFace(CMeshEditor* pcMeshEditor, SFloat32Vec3* asDirections, int iStartPosition, int iCorner1, int iCorner2, int iCorner3)
{
	SFloat32Vec3	sVec1;
	SFloat32Vec3	sVec2;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sMiddle;
	bool			bInward;

	sVec1 = asDirections[iCorner1] - asDirections[iCorner2];
	sVec2 = asDirections[iCorner3] - asDirections[iCorner2];
	Float32Vec3Cross(&sNormal, &sVec2, &sVec1);
	sMiddle = asDirections[iCorner1] + asDirections[iCorner2] + asDirections[iCorner3];
	bInward = Float32Vec3Dot(&sNormal, &sMiddle) < 0.0f;

	pcMeshEditor->AddFace(iStartPosition + iCorner1, iStartPosition + iCorner2, iStartPosition + iCorner3, miFaceName, true, true, true, bInward != mbFlipFaces);
}


//////////////////////////////////////////////////////////////////////////
//
// The icosahedron's corners, then each of its edges' corners, then each of
// its faces' corners and faces.  A face's corners are found from a
// triangle of them, iSegments + 1 along each side.
//
//////////////////////////////////////////////////////////////////////////
void CMeshGeosphereEditor::Generate(CMeshEditor* pcMeshEditor)
{
	SFloat32Vec3	asIcosahedron[GEOSPHERE_ICOSAHEDRON_CORNERS];
	int				aaiEdges[GEOSPHERE_ICOSAHEDRON_EDGES][2];
	SFloat32Vec3*	asDirections;
	int*			paiTriangle;
	SFloat32Vec3	sDirection;
	SFloat32Vec3	sPart;
	int				iStartPosition;
	int				iNumEdges;
	int				iNumCorners;
	int				iCorner;
	int				iA;
	int				iB;
	int				iC;
	int				iFrom;
	int				iTo;
	int				f;
	int				e;
	int				c;
	int				i;
	int				j;
	int				k;
	int				t;
	int				n;

	n = miSegments;
	iStartPosition = pcMeshEditor->NumCorners();
	asDirections = (SFloat32Vec3*)malloc(sizeof(SFloat32Vec3) * ExpectedCornerChunkSize());
	paiTriangle = (int*)malloc(sizeof(int) * (n + 1) * (n + 1));

	IcosahedronCorners(asIcosahedron);
	for (c = 0; c < GEOSPHERE_ICOSAHEDRON_CORNERS; c++)
	{
		AddCorner(pcMeshEditor, asDirections, c, &asIcosahedron[c]);
	}
	iNumCorners = GEOSPHERE_ICOSAHEDRON_CORNERS;

	iNumEdges = 0;
	for (f = 0; f < GEOSPHERE_ICOSAHEDRON_FACES; f++)
	{
		for (k = 0; k < 3; k++)
		{
			iFrom = gaaiIcosahedronFaces[f][k];
			iTo = gaaiIcosahedronFaces[f][(k + 1) % 3];
			if (iFrom > iTo)
			{
				t = iFrom;
				iFrom = iTo;
				iTo = t;
			}
			for (e = 0; e < iNumEdges; e++)
			{
				if ((aaiEdges[e][0] == iFrom) && (aaiEdges[e][1] == iTo))
				{
					break;
				}
			}
			if (e == iNumEdges)
			{
				aaiEdges[iNumEdges][0] = iFrom;
				aaiEdges[iNumEdges][1] = iTo;
				iNumEdges++;

				for (t = 1; t < n; t++)
				{
					sDirection = asIcosahedron[iFrom] * ((float32)(n - t) / n);
					sPart = asIcosahedron[iTo] * ((float32)t / n);
					sDirection.Add(&sPart);
					AddCorner(pcMeshEditor, asDirections, iNumCorners, &sDirection);
					iNumCorners++;
				}
			}
		}
	}

	for (f = 0; f < GEOSPHERE_ICOSAHEDRON_FACES; f++)
	{
		iA = gaaiIcosahedronFaces[f][0];
		iB = gaaiIcosahedronFaces[f][1];
		iC = gaaiIcosahedronFaces[f][2];

		//paiTriangle[i + j * (n + 1)] is the corner i of n from A towards B and j of n from A towards C.
		for (j = 0; j <= n; j++)
		{
			for (i = 0; i + j <= n; i++)
			{
				k = n - i - j;
				if (k == n)
				{
					iCorner = iA;
				}
				else if (i == n)
				{
					iCorner = iB;
				}
				else if (j == n)
				{
					iCorner = iC;
				}
				else if (j == 0)
				{
					iCorner = EdgeCorner(aaiEdges, iA, iB, i);
				}
				else if (i == 0)
				{
					iCorner = EdgeCorner(aaiEdges, iA, iC, j);
				}
				else if (k == 0)
				{
					iCorner = EdgeCorner(aaiEdges, iB, iC, j);
				}
				else
				{
					sDirection = asIcosahedron[iA] * ((float32)k / n);
					sPart = asIcosahedron[iB] * ((float32)i / n);
					sDirection.Add(&sPart);
					sPart = asIcosahedron[iC] * ((float32)j / n);
					sDirection.Add(&sPart);
					iCorner = iNumCorners;
					AddCorner(pcMeshEditor, asDirections, iCorner, &sDirection);
					iNumCorners++;
				}
				paiTriangle[i + j * (n + 1)] = iCorner;
			}
		}

		for (j = 0; j < n; j++)
		{
			for (i = 0; i + j < n; i++)
			{
				AddOutwardFace(pcMeshEditor, asDirections, iStartPosition, paiTriangle[i + j * (n + 1)], paiTriangle[(i + 1) + j * (n + 1)], paiTriangle[i + (j + 1) * (n + 1)]);
				if (i + j + 1 < n)
				{
					AddOutwardFace(pcMeshEditor, asDirections, iStartPosition, paiTriangle[(i + 1) + j * (n + 1)], paiTriangle[(i + 1) + (j + 1) * (n + 1)], paiTriangle[i + (j + 1) * (n + 1)]);
				}
			}
		}
	}

	free(paiTriangle);
	free(asDirections);
}

