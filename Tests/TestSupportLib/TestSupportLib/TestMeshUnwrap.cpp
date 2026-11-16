#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshSphereEditor.h"
#include "SupportLib/MeshTorusEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshUnwrap.h"
#include "SupportLib/MeshUVMeasures.h"
#include "TestLib/Assert.h"


//The spheres' and tori's segments around and along.  The W65C816 unwraps coarser ones (half the faces), as all its float
//arithmetic is in software: the finer ones take 35.7G cycles there.  The bounds below hold for both.
#ifdef __W65C816__
#define TEST_MESH_UNWRAP_SPHERE		16, 8
#define TEST_MESH_UNWRAP_TORUS		24, 8
#else
#define TEST_MESH_UNWRAP_SPHERE		24, 12
#define TEST_MESH_UNWRAP_TORUS		32, 12
#endif // __W65C816__


//////////////////////////////////////////////////////////////////////////
//
// Every face has UVs, none is turned over or outside, no two overlap, and
// the layout is whole pages, its size on the layer.
//
//////////////////////////////////////////////////////////////////////////
static void AssertUnwrapped(CMesh* pcMesh, CMeshUnwrap* pcUnwrap)
{
	AssertTrue(pcMesh->mcUVs.InUse());
	AssertInt(0, CountMeshUVFacesMissing(pcMesh, 0));
	AssertInt(0, CountMeshUVFacesTurned(pcMesh, 0));
	AssertInt(0, CountMeshUVsOutside(pcMesh, 0));
	AssertInt(0, CountMeshUVOverlaps(pcMesh, 0));
	AssertTrue(pcUnwrap->miLayoutWidth > 0);
	AssertInt(0, pcUnwrap->miLayoutWidth % MESH_UNWRAP_PAGE);
	AssertInt(0, pcUnwrap->miLayoutHeight % MESH_UNWRAP_PAGE);
	AssertInt(pcUnwrap->miLayoutWidth, pcMesh->mcUVs.mcLayers.Get(0)->miLayoutWidth);
	AssertInt(pcUnwrap->miLayoutHeight, pcMesh->mcUVs.mcLayers.Get(0)->miLayoutHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// A box 2 by 1 by 0.5 at 64 texels to the unit, box projected: six
// charts, one for each side, none stretched, all at 64 texels to the
// unit.  Its normals (made
// before the unwrap) are kept.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrapBox(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sZ;
	SFloat32Vec3		sStart;
	SMeshUVStretch		sStretch;

	sX.Init(2, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 0.5f);
	sStart.Init(-1, -0.5f, -0.25f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 2, 2, 2);
	pEditor->AddModifier<CMeshFlatFaces>();
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(64.0f, MUC_Box, MUF_Projected);
	pMesh = pEditor->GetOutput();

	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertInt(6, pUnwrap->miNumCharts);
	AssertTrue(pMesh->mcNormals.InUse());
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 64.0f);
	AssertInt(pMesh->NumFaces(), sStretch.iNumFaces);
	AssertFloat(1.0f, sStretch.fWorstAngle, 4);
	AssertFloat(1.0f, sStretch.fLeastArea, 4);
	AssertFloat(1.0f, sStretch.fMostArea, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere box projected: six charts, one about each axis, stretched as
// box projection stretches a slope (at most root 3 between two directions, and never
// more texels than the density).  A torus: unwrapped as well, in more
// charts.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrapCurved(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sCentre;
	SMeshUVStretch		sStretch;

	sAxis.Init(0, 0, 1);
	sCentre.Init(0.3f, -0.2f, 0.1f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 1.0f, TEST_MESH_UNWRAP_SPHERE);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f, MUC_Box, MUF_Projected);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertInt(6, pUnwrap->miNumCharts);
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 100.0f);
	AssertTrue(sStretch.fWorstAngle < 1.75f);
	AssertTrue(sStretch.fMostArea < 1.001f);
	AssertTrue(sStretch.fLeastArea > 0.5f);

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshTorusEditor>(&sAxis, &sCentre, 2.0f, 0.5f, TEST_MESH_UNWRAP_TORUS);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f, MUC_Box, MUF_Projected);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertTrue(pUnwrap->miNumCharts > 6);
}


//////////////////////////////////////////////////////////////////////////
//
// Grown charts laid across their normals: a box's six faces are six
// charts, unstretched; a sphere is a few, each stretched where its faces
// slope from its normal (up to 4.6 times; LSCM keeps their shapes); a
// torus is clean, as charts that overlap themselves laid flat (round its
// inside) are grown again smaller.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrapGrown(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sZ;
	SFloat32Vec3		sStart;
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sCentre;
	SMeshUVStretch		sStretch;

	sX.Init(2, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 0.5f);
	sStart.Init(-1, -0.5f, -0.25f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 2, 2, 2);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(64.0f, MUC_Grown, MUF_Projected);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertInt(6, pUnwrap->miNumCharts);
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 64.0f);
	AssertFloat(1.0f, sStretch.fWorstAngle, 4);
	AssertFloat(1.0f, sStretch.fMostArea, 4);

	sAxis.Init(0, 0, 1);
	sCentre.Init(0.3f, -0.2f, 0.1f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 1.0f, TEST_MESH_UNWRAP_SPHERE);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f, MUC_Grown, MUF_Projected);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 100.0f);
	AssertTrue(pUnwrap->miNumCharts <= 12);
	AssertTrue(sStretch.fMeanAngle < 1.5f);

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshTorusEditor>(&sAxis, &sCentre, 2.0f, 0.5f, TEST_MESH_UNWRAP_TORUS);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f, MUC_Grown, MUF_Projected);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertTrue(pUnwrap->miNumCharts < 40);
}


//////////////////////////////////////////////////////////////////////////
//
// Charts laid flat by LSCM: a box's six faces as before, unstretched; a
// sphere and a torus in box charts and in grown charts, their faces'
// angles kept to within 1.15 times (laid across their normals, grown
// charts stretch the sphere's 4.6 times) and their texels per area near
// the density.  Their charts cover from 0.44 to 0.63 of their layouts
// (packed as rectangles, 0.31 to 0.53): the rest is gutters, the cells
// round charts' edges and whole pages (so a layout a page taller, as
// builds whose floats differ can pack, covers a tenth less).
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrapConformal(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sZ;
	SFloat32Vec3		sStart;
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sCentre;
	SMeshUVStretch		sStretch;
	float32				afCovered[4];
	int					i;

	sX.Init(2, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 0.5f);
	sStart.Init(-1, -0.5f, -0.25f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 2, 2, 2);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(64.0f, MUC_Grown, MUF_Conformal);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertInt(6, pUnwrap->miNumCharts);
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 64.0f);
	AssertFloat(1.0f, sStretch.fWorstAngle, 3);
	AssertFloat(1.0f, sStretch.fLeastArea, 3);
	AssertFloat(1.0f, sStretch.fMostArea, 3);

	sAxis.Init(0, 0, 1);
	sCentre.Init(0.3f, -0.2f, 0.1f);
	for (i = 0; i < 4; i++)
	{
		pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
		if (i < 2)
		{
			pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 1.0f, TEST_MESH_UNWRAP_SPHERE);
		}
		else
		{
			pEditor->AddModifier<CMeshTorusEditor>(&sAxis, &sCentre, 2.0f, 0.5f, TEST_MESH_UNWRAP_TORUS);
		}
		pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f, ((i % 2) == 0) ? MUC_Box : MUC_Grown, MUF_Conformal);
		pMesh = pEditor->GetOutput();
		AssertUnwrapped(&pMesh, &pUnwrap);
		MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 100.0f);
		AssertTrue(sStretch.fWorstAngle < 1.15f);
		AssertTrue(sStretch.fMeanAngle < 1.08f);
		AssertTrue(sStretch.fLeastArea > 0.7f);
		AssertTrue(sStretch.fMostArea < 1.6f);
		afCovered[i] = MeasureMeshUVArea(&pMesh, 0);
	}
	AssertTrue(afCovered[0] > 0.4f);
	AssertTrue(afCovered[1] > 0.4f);
	AssertTrue(afCovered[2] > 0.4f);
	AssertTrue(afCovered[3] > 0.4f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrap(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshUnwrapBox();
	ObjectsFlush();
	TestMeshUnwrapCurved();
	ObjectsFlush();
	TestMeshUnwrapGrown();
	ObjectsFlush();
	TestMeshUnwrapConformal();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

