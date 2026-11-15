#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/Torus.h"
#include "SupportLib/ShapeIntersection.h"
#include "ProceduralLib/MarchingCubes.h"
#include "TestLib/Assert.h"
#include "MeshMeasures.h"
#include "TestGyroid.h"
#include "IsosurfaceMeshes.h"


//////////////////////////////////////////////////////////////////////////
//
// No corners inside or all inside is nothing.  One corner inside is one
// triangle across its three edges, anticlockwise seen from outside: x, y,
// z.  In every case each triangle is on edges whose ends differ, each
// such edge has a triangle on it, and the edges between a case's
// triangles are each used once each way, so the triangles fit together.
//
//////////////////////////////////////////////////////////////////////////
void TestMarchingCubesCases(void)
{
	CMarchingCubes		cMarchingCubes;
	SMarchingCubesCase*	psCase;
	int					iInside;
	int					iFace;
	int					i;
	int					iEdge;
	int					iNext;
	int					aiUsed[MARCHING_CUBES_EDGES];
	int					aaiJoins[MARCHING_CUBES_EDGES][MARCHING_CUBES_EDGES];
	bool				bCrosses;
	bool				bOnCrossings;
	bool				bAllUsed;
	bool				bFits;
	int					iMostFaces;

	cMarchingCubes.Init();
	AssertInt(0, cMarchingCubes.masCases[0].iNumFaces);
	AssertInt(0, cMarchingCubes.masCases[255].iNumFaces);
	psCase = &cMarchingCubes.masCases[1];
	AssertInt(1, psCase->iNumFaces);
	AssertInt(0, psCase->aaiFaces[0][0]);
	AssertInt(4, psCase->aaiFaces[0][1]);
	AssertInt(8, psCase->aaiFaces[0][2]);
	AssertInt(6, CMarchingCubes::CornersEdge(1, 3));
	AssertInt(-1, CMarchingCubes::CornersEdge(0, 3));
	AssertInt(1, CMarchingCubes::EdgeStartCorner(9));
	AssertInt(7, CMarchingCubes::EdgeEndCorner(11));

	bOnCrossings = true;
	bAllUsed = true;
	bFits = true;
	iMostFaces = 0;
	for (iInside = 0; iInside < MARCHING_CUBES_CASES; iInside++)
	{
		psCase = &cMarchingCubes.masCases[iInside];
		if (psCase->iNumFaces > iMostFaces)
		{
			iMostFaces = psCase->iNumFaces;
		}
		for (i = 0; i < MARCHING_CUBES_EDGES; i++)
		{
			aiUsed[i] = 0;
			for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
			{
				aaiJoins[i][iEdge] = 0;
			}
		}

		for (iFace = 0; iFace < psCase->iNumFaces; iFace++)
		{
			for (i = 0; i < 3; i++)
			{
				iEdge = psCase->aaiFaces[iFace][i];
				iNext = psCase->aaiFaces[iFace][(i + 1) % 3];
				aiUsed[iEdge]++;
				aaiJoins[iEdge][iNext]++;
				bCrosses = ((iInside >> CMarchingCubes::EdgeStartCorner(iEdge)) & 1) != ((iInside >> CMarchingCubes::EdgeEndCorner(iEdge)) & 1);
				if (!bCrosses)
				{
					bOnCrossings = false;
				}
			}
		}

		for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
		{
			bCrosses = ((iInside >> CMarchingCubes::EdgeStartCorner(iEdge)) & 1) != ((iInside >> CMarchingCubes::EdgeEndCorner(iEdge)) & 1);
			if (bCrosses && (aiUsed[iEdge] == 0))
			{
				bAllUsed = false;
			}
			for (iNext = 0; iNext < MARCHING_CUBES_EDGES; iNext++)
			{
				//A join used twice the same way is a fold; one with no partner the other way is on the cube's face, where it's
				//shared with the next cube.  Inside the cube, a fan's diagonals go each way once.
				if (aaiJoins[iEdge][iNext] > 1)
				{
					bFits = false;
				}
			}
		}
	}
	AssertTrue(bOnCrossings);
	AssertTrue(bAllUsed);
	AssertTrue(bFits);
	AssertTrue(iMostFaces <= MARCHING_CUBES_MAX_FACES);
	cMarchingCubes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere, a cylinder and a torus: closed, a ball or a ring, facing out,
// with every corner on the surface (they're the grid's crossings).
//
//////////////////////////////////////////////////////////////////////////
void TestMarchingCubesSmooth(void)
{
	Ptr<CSphere>	pSphere;
	Ptr<CCylinder>	pCylinder;
	Ptr<CTorus>		pTorus;
	Ptr<CMesh>		pMesh;
	SFloat32Vec3	sCentre;
	SFloat32Vec3	sAxis;

	sCentre.Init(0.1f, 0.2f, 0.3f);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	pMesh = MarchingCubesMesh(&pSphere, 0.25f);
	AssertTrue(pMesh->NumFaces() > 500);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pSphere));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pSphere) < 0.0001f);

	sAxis.Init(1, 2, 3);
	pCylinder = OMalloc<CCylinder>(&sAxis, &sCentre, 1.5f, 3.0f);
	pMesh = MarchingCubesMesh(&pCylinder, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pCylinder));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pCylinder) < 0.0001f);

	pTorus = OMalloc<CTorus>(&sAxis, &sCentre, 2.0f, 0.75f);
	pMesh = MarchingCubesMesh(&pTorus, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(0, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pTorus));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pTorus) < 0.0001f);
}


//////////////////////////////////////////////////////////////////////////
//
// A gyroid cut by a sphere winds through the cells every way, the cases
// with two inside corners across a face from each other too.  It's still
// closed, and the gyroid's faces face out of it.  Faces near the sphere
// are left out of that: marching cubes bevels the crease where the two
// meet, often at a sharp angle, and a bevel's middle can be won by the
// other shape.
//
//////////////////////////////////////////////////////////////////////////
void TestMarchingCubesGyroid(void)
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
	pMesh = MarchingCubesMesh(&pShape, 0.2f);
	AssertTrue(pMesh->NumFaces() > 1000);
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
	AssertTrue(iGyroidFaces > 1000);
	AssertInt(0, iFacingIn);
}


//////////////////////////////////////////////////////////////////////////
//
// A box off the grid's planes is closed, faces out and has its corners on
// it, but its edges and corners are cut off: no corner of the mesh comes
// within a tenth of a cell of the box's.  This is what dual contouring is
// to do better.
//
//////////////////////////////////////////////////////////////////////////
void TestMarchingCubesBox(void)
{
	Ptr<CBox>		pBox;
	Ptr<CMesh>		pMesh;
	SFloat32Box		sBox;
	SFloat32Vec3	sCentre;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sCorner;
	float32			fNearest;
	int				i;

	sCentre.Init(0.05f, 0.03f, 0.02f);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.1f, &asAxes[1], 0.9f, &asAxes[2], 0.7f);
	pBox = OMalloc<CBox>(&sBox);
	pMesh = MarchingCubesMesh(&pBox, 0.25f);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pBox));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pBox) < 0.0001f);

	fNearest = FLOAT32_BOUNDS_LARGEST;
	for (i = 0; i < 8; i++)
	{
		sBox.GetCorner(&sCorner, i);
		fNearest = fminf(fNearest, NearestMeshCorner(&pMesh, &sCorner));
	}
	AssertTrue(fNearest > 0.025f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMarchingCubes(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMarchingCubesCases();
	TestMarchingCubesSmooth();
	TestMarchingCubesGyroid();
	TestMarchingCubesBox();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

