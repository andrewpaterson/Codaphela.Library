#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshMergeNormals.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/ShapeUnion.h"
#include "SupportLib/ShapeIntersection.h"
#include "TestLib/Assert.h"
#include "MeshMeasures.h"
#include "IsosurfaceMeshes.h"
#include "TestGyroid.h"


//////////////////////////////////////////////////////////////////////////
//
// A box turned well away from the axes, off the grid.
//
//////////////////////////////////////////////////////////////////////////
static void TestDualContouringTurnedBox(SFloat32Box* psBox)
{
	SFloat32Vec3	sCentre;
	SFloat32Vec3	asAxes[3];

	sCentre.Init(0.05f, 0.03f, 0.02f);
	asAxes[0].Init(2, 1, 2);
	asAxes[0] /= 3.0f;
	asAxes[1].Init(-2, 2, 1);
	asAxes[1] /= 3.0f;
	Float32Vec3Cross(&asAxes[2], &asAxes[0], &asAxes[1]);
	psBox->Init(&sCentre, &asAxes[0], 1.1f, &asAxes[1], 0.9f, &asAxes[2], 0.7f);
}


//////////////////////////////////////////////////////////////////////////
//
// A turned box's corners are on the mesh exactly, and it's closed and
// faces out.  Its edges are sharp but for cells holding pieces of two
// faces whose edge passes outside them, where the fit is clamped back in
// (or is the mass point) a little off the box: within 0.3 of a cell, its
// volume within 1% and its area to 2 decimals.  Marching cubes' bevels
// lose 4% of its volume.
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouringBox(void)
{
	Ptr<CBox>		pBox;
	Ptr<CMesh>		pMesh;
	Ptr<CMesh>		pMarched;
	SFloat32Box		sBox;
	SFloat32Vec3	sCorner;
	float32			fFurthest;
	int				i;

	TestDualContouringTurnedBox(&sBox);
	pBox = OMalloc<CBox>(&sBox);
	pMesh = DualContouringMesh(&pBox, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pBox));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pBox) < 0.075f);

	fFurthest = 0.0f;
	for (i = 0; i < 8; i++)
	{
		sBox.GetCorner(&sCorner, i);
		fFurthest = fmaxf(fFurthest, NearestMeshCorner(&pMesh, &sCorner));
	}
	AssertTrue(fFurthest < 0.0001f);
	AssertTrue(fabsf(MeshVolume(&pMesh) - 5.544f) < 0.055f);
	AssertFloat(19.12f, MeshArea(&pMesh), 2);

	pMarched = MarchingCubesMesh(&pBox, 0.25f);
	AssertTrue(MeshVolume(&pMarched) < 5.4f);
}


//////////////////////////////////////////////////////////////////////////
//
// Flat normals merged where they're within 30 degrees: a sphere's are
// one at each corner, and a box's edges keep theirs apart, so it has many
// more normals than corners.  Few are far from all of its faces' (the
// faces at its corners clamped off it).
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouringBoxNormals(void)
{
	Ptr<CBox>			pBox;
	Ptr<CSphere>		pSphere;
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMesh>			pMesh;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3*		psNormal;
	SMeshNormalFace*	psNormalFace;
	float32				fBest;
	int					iAligned;
	int					iAcross;
	int					iTotal;
	int					iFace;
	int					iCorner;
	int					iAxis;

	TestDualContouringTurnedBox(&sBox);
	pBox = OMalloc<CBox>(&sBox);
	pEditor = OMalloc<CMeshEditor>(DualContouringMesh(&pBox, 0.25f));
	pEditor->AddModifier<CMeshFlatFaces>();
	pEditor->AddModifier<CMeshMergeNormals>(0.5236f);
	pMesh = pEditor->GetOutput();
	AssertTrue(pMesh->mcNormals.InUse());

	iAligned = 0;
	iAcross = 0;
	iTotal = 0;
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		psNormalFace = pMesh->mcNormals.mcFaces.Get(iFace);
		for (iCorner = 0; iCorner < NUM_FACE_CORNERS; iCorner++)
		{
			psNormal = pMesh->mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[iCorner]);
			fBest = 0.0f;
			for (iAxis = 0; iAxis < 3; iAxis++)
			{
				fBest = fmaxf(fBest, fabsf(Float32Vec3Dot(psNormal, &sBox.asAxes[iAxis])));
			}
			iTotal++;
			if (fBest > 0.9999f)
			{
				iAligned++;
			}
			if (fBest < 0.8f)
			{
				iAcross++;
			}
		}
	}
	AssertTrue(((int)pMesh->mcNormals.mcNormals.NumElements() - pMesh->NumFaces()) > (pMesh->NumCorners() * 14 / 10));
	AssertTrue(iAligned > (iTotal / 2));
	AssertTrue(iAcross < (iTotal / 20));

	sCentre.Init(0.1f, 0.2f, 0.3f);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	pEditor = OMalloc<CMeshEditor>(DualContouringMesh(&pSphere, 0.25f));
	pEditor->AddModifier<CMeshFlatFaces>();
	pEditor->AddModifier<CMeshMergeNormals>(0.5236f);
	pMesh = pEditor->GetOutput();
	AssertInt(pMesh->NumCorners(), (int)pMesh->mcNormals.mcNormals.NumElements() - pMesh->NumFaces());
}


//////////////////////////////////////////////////////////////////////////
//
// A cylinder's rims are sharp: 80 of the mesh's corners are on them,
// where marching cubes has 16, and its volume is within 0.5%, where
// marching cubes' is 1.6% short.
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouringCylinder(void)
{
	Ptr<CCylinder>	pCylinder;
	Ptr<CMesh>		pMesh;
	Ptr<CMesh>		pMarched;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sEnd;
	int				iOnRims;
	int				iMarchedOnRims;
	float32			fVolume;

	sAxis.Init(0, 0, 1);
	sStart.Init(0.05f, 0.07f, -1.37f);
	sEnd = sStart + (sAxis * 2.83f);
	pCylinder = OMalloc<CCylinder>(&sAxis, &sStart, 1.3f, 2.83f);
	pMesh = DualContouringMesh(&pCylinder, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pCylinder));
	iOnRims = CountMeshCornersOnCircle(&pMesh, &sStart, &sAxis, 1.3f, 0.02f) + CountMeshCornersOnCircle(&pMesh, &sEnd, &sAxis, 1.3f, 0.02f);

	pMarched = MarchingCubesMesh(&pCylinder, 0.25f);
	iMarchedOnRims = CountMeshCornersOnCircle(&pMarched, &sStart, &sAxis, 1.3f, 0.02f) + CountMeshCornersOnCircle(&pMarched, &sEnd, &sAxis, 1.3f, 0.02f);
	AssertTrue(iOnRims >= 60);
	AssertTrue(iMarchedOnRims <= 20);
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pCylinder) < 0.01f);
	fVolume = 3.14159265f * 1.3f * 1.3f * 2.83f;
	AssertTrue(fabsf(MeshVolume(&pMesh) - fVolume) < (fVolume * 0.005f));
	AssertTrue(fabsf(MeshVolume(&pMarched) - fVolume) > (fVolume * 0.01f));
}


//////////////////////////////////////////////////////////////////////////
//
// A cylinder pushed up through a box's top: the seam where it leaves the
// box is sharp, as each crossing takes its normal from the shape that
// wins there, so 22 corners are on it, where marching cubes has none.
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouringSeam(void)
{
	Ptr<CShapeUnion>	pUnion;
	Ptr<CMesh>			pMesh;
	Ptr<CMesh>			pMarched;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sStart;
	SFloat32Vec3		sSeam;
	int					iOnSeam;
	int					iMarchedOnSeam;

	sCentre.Init(0.03f, 0.04f, -0.6f);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.5f, &asAxes[1], 1.5f, &asAxes[2], 0.8f);
	sAxis.Init(0, 0, 1);
	sStart.Init(0.1f, 0.05f, -0.5f);
	sSeam.Init(0.1f, 0.05f, 0.2f);

	pUnion = OMalloc<CShapeUnion>();
	pUnion->Add(OMalloc<CBox>(&sBox));
	pUnion->Add(OMalloc<CCylinder>(&sAxis, &sStart, 0.7f, 1.6f));
	pMesh = DualContouringMesh(&pUnion, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pUnion));
	iOnSeam = CountMeshCornersOnCircle(&pMesh, &sSeam, &sAxis, 0.7f, 0.02f);

	pMarched = MarchingCubesMesh(&pUnion, 0.25f);
	iMarchedOnSeam = CountMeshCornersOnCircle(&pMarched, &sSeam, &sAxis, 0.7f, 0.02f);
	AssertTrue(iOnSeam >= 15);
	AssertTrue(iMarchedOnSeam <= 3);
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pUnion) < 0.02f);
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere's vertices are on it, near enough: the fit of its tangent
// planes in a cell is off it by under the cell squared over its radius
// (0.03), and its volume is within 0.5%.
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouringSphere(void)
{
	Ptr<CSphere>	pSphere;
	Ptr<CMesh>		pMesh;
	SFloat32Vec3	sCentre;

	sCentre.Init(0.1f, 0.2f, 0.3f);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	pMesh = DualContouringMesh(&pSphere, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pSphere));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pSphere) < 0.015f);
	AssertTrue(fabsf(MeshVolume(&pMesh) - (4.18879f * 8.0f)) < (4.18879f * 8.0f * 0.005f));
}


//////////////////////////////////////////////////////////////////////////
//
// A gyroid cut by a sphere: closed, and its own faces face out but for a
// few (4 of 7000) where a cell holds two pieces of its surface and their
// one vertex folds a face over (Isosurface.txt's Known limits: manifold
// dual contouring splits such a vertex).  Faces near the sphere are left
// out, as in TestMarchingCubesGyroid.
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouringGyroid(void)
{
	Ptr<CShapeIntersection>	pShape;
	Ptr<CSphere>			pSphere;
	Ptr<CTestGyroid>		pGyroid;
	Ptr<CMesh>				pMesh;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			sNormal;
	SFloat32Vec3			sMiddle;
	SFloat32Vec3			sGradient;
	int						i;
	int						iGyroidFaces;
	int						iFacingIn;

	sCentre.Init(0.05f, -0.1f, 0.15f);
	pSphere = OMalloc<CSphere>(&sCentre, 3.0f);
	pGyroid = OMalloc<CTestGyroid>(2.0f);
	pShape = OMalloc<CShapeIntersection>();
	pShape->Add(pSphere);
	pShape->Add(pGyroid);
	pMesh = DualContouringMesh(&pShape, 0.2f);
	AssertTrue(pMesh->NumFaces() > 10000);
	AssertInt(0, CountMeshOpenEdges(&pMesh));

	iGyroidFaces = 0;
	iFacingIn = 0;
	for (i = 0; i < pMesh->NumFaces(); i++)
	{
		MeshFaceNormal(&pMesh, i, &sNormal, &sMiddle);
		if (pSphere->Distance(&sMiddle) < -0.2f)
		{
			iGyroidFaces++;
			pGyroid->Gradient(&sGradient, &sMiddle);
			if (Float32Vec3Dot(&sNormal, &sGradient) <= 0.0f)
			{
				iFacingIn++;
			}
		}
	}
	AssertTrue(iGyroidFaces > 5000);
	AssertTrue(iFacingIn <= 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDualContouring(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestDualContouringBox();
	TestDualContouringBoxNormals();
	TestDualContouringCylinder();
	TestDualContouringSeam();
	TestDualContouringSphere();
	TestDualContouringGyroid();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

