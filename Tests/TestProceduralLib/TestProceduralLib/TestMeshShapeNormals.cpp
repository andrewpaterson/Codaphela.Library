#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/ArrayInt.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshMergeNormals.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/ShapeSmoothUnion.h"
#include "ProceduralLib/MeshShapeEditor.h"
#include "ProceduralLib/MeshShapeNormals.h"
#include "SupportLib/MeshUnwrap.h"
#include "SupportLib/MeshUVMeasures.h"
#include "TestLib/Assert.h"
#include "MeshMeasures.h"


//////////////////////////////////////////////////////////////////////////
//
// How many corners have more than one normal among the face corners on
// them: where the shading has an edge.
//
//////////////////////////////////////////////////////////////////////////
static int TestMeshShapeNormalsSplit(CMesh* pcMesh)
{
	CArrayLargeInt		aiFirst;
	SMeshNormalFace*	psNormalFace;
	int*				piCorners;
	int					iFace;
	int					iCorner;
	int					k;
	int					iSplit;

	aiFirst.Init();
	aiFirst.Resize((lsize)pcMesh->NumCorners());
	aiFirst.SetArrayValues(-1);
	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
		piCorners = pcMesh->GetFaceCorners(iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			iCorner = piCorners[k];
			if (aiFirst.GetValue((lsize)iCorner) == -1)
			{
				aiFirst.SetValue((lsize)iCorner, psNormalFace->aiCornerNormals[k]);
			}
			else if (aiFirst.GetValue((lsize)iCorner) != psNormalFace->aiCornerNormals[k])
			{
				aiFirst.SetValue((lsize)iCorner, -2);
			}
		}
	}
	iSplit = 0;
	for (iCorner = 0; iCorner < pcMesh->NumCorners(); iCorner++)
	{
		if (aiFirst.GetValue((lsize)iCorner) == -2)
		{
			iSplit++;
		}
	}
	aiFirst.Kill();
	return iSplit;
}


//////////////////////////////////////////////////////////////////////////
//
// The normal a face corner has: its own, or its face's flat one.
//
//////////////////////////////////////////////////////////////////////////
static SFloat32Vec3* TestMeshShapeNormalsCorner(CMesh* pcMesh, int iFace, int k)
{
	int		iNormal;

	iNormal = pcMesh->mcNormals.mcFaces.Get(iFace)->aiCornerNormals[k];
	if (iNormal < 0)
	{
		iNormal = iFace;
	}
	return pcMesh->mcNormals.mcNormals.Get(iNormal);
}


//////////////////////////////////////////////////////////////////////////
//
// Five spheres smoothly joined, as the isosurface page drops them: merging
// flat normals at 30 degrees splits 4 corners, where coarse faces fan
// through more than that, and the shape's normals split none: every face
// corner has the shape's gradient at its corner, none a flat normal.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeNormalsSmooth(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CShape>			pShape;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		asCentres[5];
	SFloat32Vec3		sGradient;
	SFloat32Vec3*		psNormal;
	SMeshNormalFace*	psNormalFace;
	int*				piCorners;
	int					i;
	int					iFace;
	int					k;
	int					iFlat;
	float32				fWorst;
	float32				fOff;

	asCentres[0].Init(-1.5f, 0.2f, 0);
	asCentres[1].Init(-0.3f, -0.4f, 0);
	asCentres[2].Init(0.9f, 0.5f, 0);
	asCentres[3].Init(1.8f, -0.6f, 0);
	asCentres[4].Init(0.2f, 1.4f, 0);
	pShape = OMalloc<CSphere>(&asCentres[0], 1.0f);
	for (i = 1; i < 5; i++)
	{
		pShape = OMalloc<CShapeSmoothUnion>(pShape, OMalloc<CSphere>(&asCentres[i], 1.0f), 0.5f);
	}

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pShape, 0.05f, 0.8f);
	pEditor->AddModifier<CMeshFlatFaces>();
	pEditor->AddModifier<CMeshMergeNormals>(MESH_SHAPE_SPLIT_ANGLE);
	pMesh = pEditor->GetOutput();
	AssertTrue(TestMeshShapeNormalsSplit(&pMesh) > 0);

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pShape, 0.05f, 0.8f);
	pEditor->AddModifier<CMeshShapeNormals>(pShape);
	pMesh = pEditor->GetOutput();
	AssertTrue(pMesh->mcNormals.InUse());
	AssertTrue(pMesh->NumFaces() > 5000);
	AssertInt(0, TestMeshShapeNormalsSplit(&pMesh));

	fWorst = 0.0f;
	iFlat = 0;
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		psNormalFace = pMesh->mcNormals.mcFaces.Get(iFace);
		piCorners = pMesh->GetFaceCorners(iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			if (psNormalFace->aiCornerNormals[k] < pMesh->NumFaces())
			{
				iFlat++;
			}
			psNormal = TestMeshShapeNormalsCorner(&pMesh, iFace, k);
			pShape->Gradient(&sGradient, pMesh->GetPosition(piCorners[k]));
			sGradient /= (float32)sGradient.Magnitude();
			sGradient -= *psNormal;
			fOff = (float32)sGradient.Magnitude();
			if (fOff > fWorst)
			{
				fWorst = fOff;
			}
		}
	}
	AssertInt(0, iFlat);
	AssertTrue(fWorst < 0.00001f);
}


//////////////////////////////////////////////////////////////////////////
//
// A turned box's creases stay sharp: every face corner's normal is its
// face's, though the gradient at a box corner is only one face's.  A
// cylinder's caps' normals are its axis, and its side's are across it,
// right up to its rims.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeNormalsCreases(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CBox>			pBox;
	Ptr<CCylinder>		pCylinder;
	Ptr<CMesh>			pMesh;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sFaceNormal;
	SFloat32Vec3		sMiddle;
	SFloat32Vec3*		psNormal;
	int					iFace;
	int					k;
	float32				fWorst;
	float32				fOff;
	float32				fAlong;

	sCentre.Init(0.05f, 0.03f, 0.02f);
	asAxes[0].Init(2, 1, 2);
	asAxes[0] /= 3.0f;
	asAxes[1].Init(-2, 2, 1);
	asAxes[1] /= 3.0f;
	Float32Vec3Cross(&asAxes[2], &asAxes[0], &asAxes[1]);
	sBox.Init(&sCentre, &asAxes[0], 1.1f, &asAxes[1], 0.9f, &asAxes[2], 0.7f);
	pBox = OMalloc<CBox>(&sBox);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pBox, 0.0625f, 0.5f);
	pEditor->AddModifier<CMeshShapeNormals>(pBox);
	pMesh = pEditor->GetOutput();
	fWorst = 0.0f;
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		MeshFaceNormal(&pMesh, iFace, &sFaceNormal, &sMiddle);
		sFaceNormal /= (float32)sFaceNormal.Magnitude();
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psNormal = TestMeshShapeNormalsCorner(&pMesh, iFace, k);
			fOff = 1.0f - Float32Vec3Dot(psNormal, &sFaceNormal);
			if (fOff > fWorst)
			{
				fWorst = fOff;
			}
		}
	}
	AssertTrue(fWorst < 0.0001f);

	sAxis.Init(0, 0, 1);
	sCentre.Init(0.1f, -0.2f, -1.0f);
	pCylinder = OMalloc<CCylinder>(&sAxis, &sCentre, 1.3f, 2.0f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pCylinder, 0.0625f, 0.5f);
	pEditor->AddModifier<CMeshShapeNormals>(pCylinder);
	pMesh = pEditor->GetOutput();
	fWorst = 0.0f;
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		MeshFaceNormal(&pMesh, iFace, &sFaceNormal, &sMiddle);
		sFaceNormal /= (float32)sFaceNormal.Magnitude();
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psNormal = TestMeshShapeNormalsCorner(&pMesh, iFace, k);
			fAlong = fabsf(psNormal->z);
			fOff = (fabsf(sFaceNormal.z) > 0.9f) ? (1.0f - fAlong) : fAlong;
			if (fOff > fWorst)
			{
				fWorst = fOff;
			}
		}
	}
	AssertTrue(fWorst < 0.0001f);
}


//////////////////////////////////////////////////////////////////////////
//
// The five spheres meshed, given the shape's normals and unwrapped last
// (as the modifiers that rebuild faces refuse a mesh with UVs), in box
// charts, grown charts, and grown charts by LSCM and by ARAP: every face
// has UVs, none turned over, outside or overlapping, and its normals
// kept.  Box charts stretch a face's angles up to 1.7 times, grown ones
// laid across their normals 7.4 times, by LSCM 1.1 times, by ARAP 1.32;
// ARAP's texels per area are 0.76 to 1.20 of the density, LSCM's 0.63
// to 1.44.  (Those are Release x64's.  Debug grows 41 charts, not 50, as
// its floats differ: some more curved, so by ARAP up to 1.92 times and
// 0.52 to 1.23, by LSCM 0.56 to 1.80.)
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeNormalsUnwrapped(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CShape>			pShape;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		asCentres[5];
	SMeshUVStretch		sStretch;
	float32				afWorst[4];
	float32				afSpread[4];  //The most texels per area over the least.
	int					i;

	asCentres[0].Init(-1.5f, 0.2f, 0);
	asCentres[1].Init(-0.3f, -0.4f, 0);
	asCentres[2].Init(0.9f, 0.5f, 0);
	asCentres[3].Init(1.8f, -0.6f, 0);
	asCentres[4].Init(0.2f, 1.4f, 0);
	pShape = OMalloc<CSphere>(&asCentres[0], 1.0f);
	for (i = 1; i < 5; i++)
	{
		pShape = OMalloc<CShapeSmoothUnion>(pShape, OMalloc<CSphere>(&asCentres[i], 1.0f), 0.5f);
	}

	for (i = 0; i < 4; i++)
	{
		pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
		pEditor->AddModifier<CMeshShapeEditor>(pShape, 0.05f, 0.8f);
		pEditor->AddModifier<CMeshShapeNormals>(pShape);
		pUnwrap = pEditor->AddModifier<CMeshUnwrap>(64.0f, (i == 0) ? MUC_Box : MUC_Grown, (i < 2) ? MUF_Projected : ((i == 2) ? MUF_Conformal : MUF_Rigid));
		pMesh = pEditor->GetOutput();
		AssertTrue(pMesh->mcNormals.InUse());
		AssertTrue(pUnwrap->miNumCharts >= 6);
		AssertInt(0, CountMeshUVFacesMissing(&pMesh, 0));
		AssertInt(0, CountMeshUVFacesTurned(&pMesh, 0));
		AssertInt(0, CountMeshUVsOutside(&pMesh, 0));
		AssertInt(0, CountMeshUVOverlaps(&pMesh, 0));
		MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 64.0f);
		afWorst[i] = sStretch.fWorstAngle;
		afSpread[i] = sStretch.fMostArea / sStretch.fLeastArea;
	}
	AssertTrue(afWorst[0] < 2.0f);
	AssertTrue(afWorst[1] > 4.0f);
	AssertTrue(afWorst[2] < 1.25f);
	AssertTrue(afWorst[3] < 2.0f);
	AssertTrue(afSpread[3] < afSpread[2]);
	AssertTrue(afSpread[3] < 2.5f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeNormals(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshShapeNormalsSmooth();
	ObjectsFlush();
	TestMeshShapeNormalsCreases();
	ObjectsFlush();
	TestMeshShapeNormalsUnwrapped();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

