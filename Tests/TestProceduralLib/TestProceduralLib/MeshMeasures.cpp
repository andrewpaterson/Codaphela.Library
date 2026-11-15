#include <math.h>
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/ArrayInt.h"
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
int CountMeshFacesFacingIn(CMesh* pcMesh, CShape* pcShape, float32 fLeastArea)
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
		if (MeshFaceHasArea(pcMesh, i, &sNormal) && (((float32)sNormal.Magnitude() * 0.5f) >= fLeastArea))
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
// Each corner's faces are labelled by fans: a face joins the fan of
// another face of the corner it shares an edge through the corner with,
// until nothing changes.  More than one label left is a pinch.
//
//////////////////////////////////////////////////////////////////////////
int CountMeshPinchedCorners(CMesh* pcMesh)
{
	CArrayLargeInt	aiFirstFace;
	CArrayLargeInt	aiNextFace;
	CArrayInt		aiFaces;
	CArrayInt		aiFans;
	int*		piCorners;
	int*		piOther;
	int			iNumFaces;
	int			iNumCorners;
	int			iFace;
	int			iCorner;
	int			iPinched;
	int			i;
	int			j;
	int			k;
	int			m;
	int			iFan;
	bool		bChanged;
	bool		bShared;

	iNumFaces = pcMesh->NumFaces();
	iNumCorners = pcMesh->NumCorners();
	aiFirstFace.Init();
	aiNextFace.Init();
	aiFirstFace.Resize((lsize)iNumCorners);
	aiFirstFace.SetArrayValues(-1);
	aiNextFace.Resize((lsize)(iNumFaces * NUM_FACE_CORNERS));
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pcMesh->GetFaceCorners(iFace);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			aiNextFace.SetValue((lsize)((iFace * NUM_FACE_CORNERS) + i), aiFirstFace.GetValue((lsize)piCorners[i]));
			aiFirstFace.SetValue((lsize)piCorners[i], (iFace * NUM_FACE_CORNERS) + i);
		}
	}

	iPinched = 0;
	aiFaces.Init();
	aiFans.Init();
	for (iCorner = 0; iCorner < iNumCorners; iCorner++)
	{
		aiFaces.ReInit();
		aiFans.ReInit();
		for (k = aiFirstFace.GetValue((lsize)iCorner); k != -1; k = aiNextFace.GetValue((lsize)k))
		{
			aiFans.Add((int)aiFaces.NumElements());
			aiFaces.Add(k / NUM_FACE_CORNERS);
		}

		do
		{
			bChanged = false;
			for (i = 0; i < (int)aiFaces.NumElements(); i++)
			{
				piCorners = pcMesh->GetFaceCorners(aiFaces.GetValue((size)i));
				for (j = i + 1; j < (int)aiFaces.NumElements(); j++)
				{
					if (aiFans.GetValue((size)i) == aiFans.GetValue((size)j))
					{
						continue;
					}
					piOther = pcMesh->GetFaceCorners(aiFaces.GetValue((size)j));
					bShared = false;
					for (k = 0; (k < NUM_FACE_CORNERS) && !bShared; k++)
					{
						for (m = 0; m < NUM_FACE_CORNERS; m++)
						{
							if ((piCorners[k] != iCorner) && (piCorners[k] == piOther[m]))
							{
								bShared = true;
								break;
							}
						}
					}
					if (bShared)
					{
						iFan = (aiFans.GetValue((size)i) < aiFans.GetValue((size)j)) ? aiFans.GetValue((size)i) : aiFans.GetValue((size)j);
						aiFans.SetValue((size)i, iFan);
						aiFans.SetValue((size)j, iFan);
						bChanged = true;
					}
				}
			}
		}
		while (bChanged);

		for (i = 1; i < (int)aiFans.NumElements(); i++)
		{
			if (aiFans.GetValue((size)i) != aiFans.GetValue(0))
			{
				iPinched++;
				break;
			}
		}
	}
	aiFans.Kill();
	aiFaces.Kill();
	aiNextFace.Kill();
	aiFirstFace.Kill();
	return iPinched;
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
float32 MeshVolume(CMesh* pcMesh)
{
	int				i;
	int				iNumFaces;
	int*			piCorners;
	SFloat32Vec3	sCross;
	float32			fVolume;

	fVolume = 0.0f;
	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		Float32Vec3Cross(&sCross, pcMesh->GetPosition(piCorners[1]), pcMesh->GetPosition(piCorners[2]));
		fVolume += Float32Vec3Dot(pcMesh->GetPosition(piCorners[0]), &sCross);
	}
	return fVolume / 6.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 MeshArea(CMesh* pcMesh)
{
	int				i;
	int				iNumFaces;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sMiddle;
	float32			fArea;

	fArea = 0.0f;
	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		MeshFaceNormal(pcMesh, i, &sNormal, &sMiddle);
		fArea += sNormal.Magnitude() * 0.5f;
	}
	return fArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshCornersOnCircle(CMesh* pcMesh, SFloat32Vec3* psCentre, SFloat32Vec3* psAxis, float32 fRadius, float32 fTolerance)
{
	int				i;
	int				iNumCorners;
	int				iOn;
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sAcross;
	float32			fAlong;
	float32			fOut;

	iOn = 0;
	iNumCorners = pcMesh->NumCorners();
	for (i = 0; i < iNumCorners; i++)
	{
		sOffset = *pcMesh->GetPosition(i) - *psCentre;
		fAlong = Float32Vec3Dot(&sOffset, psAxis);
		sAcross = sOffset - (*psAxis * fAlong);
		fOut = (float32)sAcross.Magnitude() - fRadius;
		if (((fAlong * fAlong) + (fOut * fOut)) < (fTolerance * fTolerance))
		{
			iOn++;
		}
	}
	return iOn;
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

