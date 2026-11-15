#include <math.h>
#include "BaseLib/Float32Vec3.h"
#include "MeshMeasures.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshOpenEdges(CMesh* pcMesh)
{
	int			i;
	int			iNumEdges;
	int			iOpen;
	CMeshEdge*	pcEdge;

	iOpen = 0;
	iNumEdges = pcMesh->NumEdges();
	for (i = 0; i < iNumEdges; i++)
	{
		pcEdge = pcMesh->GetEdge(i);
		if ((pcEdge->maiFace[0] == -1) || (pcEdge->maiFace[1] == -1))
		{
			iOpen++;
		}
	}
	return iOpen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int MeshEulerCharacteristic(CMesh* pcMesh)
{
	return pcMesh->NumCorners() - pcMesh->NumEdges() + pcMesh->NumFaces();
}


//////////////////////////////////////////////////////////////////////////
//
// A face has no area when twice its area is under a millionth of its
// longest side squared.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshFaceHasArea(CMesh* pcMesh, int iFace, SFloat32Vec3* psNormal)
{
	int*			piCorners;
	SFloat32Vec3	sSide;
	float32			fLongest;
	float32			fLength;
	int				i;

	piCorners = pcMesh->GetFaceCorners(iFace);
	fLongest = 0.0f;
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		sSide = *pcMesh->GetPosition(piCorners[i]) - *pcMesh->GetPosition(piCorners[(i + 1) % NUM_FACE_CORNERS]);
		fLength = (float32)sSide.SquareMagnitude();
		if (fLength > fLongest)
		{
			fLongest = fLength;
		}
	}
	return psNormal->Magnitude() > (fLongest * 1e-6f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshFacesFacingIn(CMesh* pcMesh, CShape* pcShape)
{
	int				i;
	int				iNumFaces;
	int				iFacingIn;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sMiddle;
	SFloat32Vec3	sGradient;

	iFacingIn = 0;
	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		MeshFaceNormal(pcMesh, i, &sNormal, &sMiddle);
		if (MeshFaceHasArea(pcMesh, i, &sNormal))
		{
			pcShape->Gradient(&sGradient, &sMiddle);
			if (Float32Vec3Dot(&sNormal, &sGradient) <= 0.0f)
			{
				iFacingIn++;
			}
		}
	}
	return iFacingIn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshFacesWithNoArea(CMesh* pcMesh)
{
	int				i;
	int				iNumFaces;
	int				iNoArea;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sMiddle;

	iNoArea = 0;
	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		MeshFaceNormal(pcMesh, i, &sNormal, &sMiddle);
		if (!MeshFaceHasArea(pcMesh, i, &sNormal))
		{
			iNoArea++;
		}
	}
	return iNoArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 FurthestMeshCornerFromSurface(CMesh* pcMesh, CShape* pcShape)
{
	int		i;
	int		iNumCorners;
	float32	fFurthest;
	float32	fDistance;

	fFurthest = 0.0f;
	iNumCorners = pcMesh->NumCorners();
	for (i = 0; i < iNumCorners; i++)
	{
		fDistance = fabsf(pcShape->Distance(pcMesh->GetPosition(i)));
		if (fDistance > fFurthest)
		{
			fFurthest = fDistance;
		}
	}
	return fFurthest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 NearestMeshCorner(CMesh* pcMesh, SFloat32Vec3* psPoint)
{
	int				i;
	int				iNumCorners;
	float32			fNearest;
	float32			fDistance;
	SFloat32Vec3	sOffset;

	fNearest = FLOAT32_BOUNDS_LARGEST;
	iNumCorners = pcMesh->NumCorners();
	for (i = 0; i < iNumCorners; i++)
	{
		sOffset = *pcMesh->GetPosition(i) - *psPoint;
		fDistance = sOffset.Magnitude();
		if (fDistance < fNearest)
		{
			fNearest = fDistance;
		}
	}
	return fNearest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GrownShapeBounds(SFloat32Bounds* psDest, CShape* pcShape, float32 fMargin)
{
	SFloat32Vec3	sMargin;

	pcShape->GetBounds(psDest);
	sMargin.Init(fMargin, fMargin, fMargin);
	psDest->sMinimum -= sMargin;
	psDest->sMaximum += sMargin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MeshFaceNormal(CMesh* pcMesh, int iFace, SFloat32Vec3* psNormal, SFloat32Vec3* psMiddle)
{
	int*			piCorners;
	SFloat32Vec3	sVec1;
	SFloat32Vec3	sVec2;

	piCorners = pcMesh->GetFaceCorners(iFace);
	sVec1 = *pcMesh->GetPosition(piCorners[0]) - *pcMesh->GetPosition(piCorners[1]);
	sVec2 = *pcMesh->GetPosition(piCorners[2]) - *pcMesh->GetPosition(piCorners[1]);
	Float32Vec3Cross(psNormal, &sVec2, &sVec1);

	*psMiddle = *pcMesh->GetPosition(piCorners[0]) + *pcMesh->GetPosition(piCorners[1]);
	psMiddle->Add(pcMesh->GetPosition(piCorners[2]));
	*psMiddle /= 3.0f;
}

