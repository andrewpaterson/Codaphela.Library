#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshConeEditor.h"
#include "SupportLib/MeshCylinderEditor.h"
#include "SupportLib/MeshTorusEditor.h"
#include "SupportLib/MeshTubeEditor.h"
#include "SupportLib/Capsule.h"
#include "SupportLib/Cone.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/Torus.h"
#include "SupportLib/Tube.h"
#include "TestLib/Assert.h"


void			AssertShapeGradient(CShape* pcShape, SFloat32Vec3* psPosition, float32 fX, float32 fY, float32 fZ);
bool			TestShapeWrite(char* szDirectory, CShape* pcShape);
CBaseObject*	TestShapeRead(char* szDirectory, const char* szName);


//////////////////////////////////////////////////////////////////////////
//
// The axis the shapes are turned around, a unit along (1, 2, 2), and where
// they start.
//
//////////////////////////////////////////////////////////////////////////
static void TestRevolvedShapeAxis(SFloat32Vec3* psAxis, SFloat32Vec3* psStart)
{
	psAxis->Init(1.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f);
	psStart->Init(1, -2, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Every corner of the mesh its editor made with the same values is on the
// shape's surface and in its bounds.
//
//////////////////////////////////////////////////////////////////////////
static void AssertShapeOnMesh(CShape* pcShape, CMesh* pcMesh)
{
	int				i;
	float32			fLargest;
	float32			fDistance;
	SFloat32Bounds	sBounds;

	AssertTrue(pcMesh->NumCorners() > 0);
	pcShape->GetBounds(&sBounds);
	fLargest = 0.0f;
	for (i = 0; i < pcMesh->NumCorners(); i++)
	{
		fDistance = fabsf(pcShape->Distance(pcMesh->GetPosition(i)));
		if (fDistance > fLargest)
		{
			fLargest = fDistance;
		}
		AssertTrue(sBounds.Distance(pcMesh->GetPosition(i)) <= 0.001f);
	}
	AssertTrue(fLargest <= 0.001f);
}


//////////////////////////////////////////////////////////////////////////
//
// A point fRadial out from the axis (towards (2, -1, 0) / sqrt 5, across
// it) and fAlong it from the start.
//
//////////////////////////////////////////////////////////////////////////
static void TestRevolvedShapePoint(SFloat32Vec3* psDest, float32 fRadial, float32 fAlong)
{
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sAcross;

	TestRevolvedShapeAxis(&sAxis, &sStart);
	sAcross.Init(2.0f, -1.0f, 0.0f);
	sAcross.Normalize();
	*psDest = sStart + sAxis * fAlong + sAcross * fRadial;
}


//////////////////////////////////////////////////////////////////////////
//
// A gradient fOut across the axis and fUp along it.
//
//////////////////////////////////////////////////////////////////////////
static void AssertRevolvedGradient(CShape* pcShape, SFloat32Vec3* psPosition, float32 fOut, float32 fUp)
{
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sExpected;

	TestRevolvedShapeAxis(&sAxis, &sStart);
	sAcross.Init(2.0f, -1.0f, 0.0f);
	sAcross.Normalize();
	sExpected = sAcross * fOut + sAxis * fUp;
	AssertShapeGradient(pcShape, psPosition, sExpected.x, sExpected.y, sExpected.z);
}


//////////////////////////////////////////////////////////////////////////
//
// Radius 2, length 5: its editor's mesh is on it, and its distances and
// gradients from its side, its end and past its rim are right.  Along z
// from the origin, its bounds are exactly it, and its side is outside it.
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShapeCylinder(void)
{
	Ptr<CCylinder>	pCylinder;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sTest;
	CMesh			cMesh;
	CMeshEditor		cEditor;
	SFloat32Bounds	sBounds;

	TestRevolvedShapeAxis(&sAxis, &sStart);
	pCylinder = OMalloc<CCylinder>(&sAxis, &sStart, 2.0f, 5.0f);
	ORoot()->Add(pCylinder);

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshCylinderEditor>()->Init(&sAxis, &sStart, 2.0f, 5.0f, 24, 2, 3);
	cEditor.Collapse();
	AssertShapeOnMesh(&pCylinder, &cMesh);
	cEditor.Kill();
	cMesh.Kill();

	TestRevolvedShapePoint(&sTest, 3.0f, 2.5f);
	AssertFloat(1.0f, pCylinder->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCylinder, &sTest, 1.0f, 0.0f);
	TestRevolvedShapePoint(&sTest, 0.5f, 6.0f);
	AssertFloat(1.0f, pCylinder->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCylinder, &sTest, 0.0f, 1.0f);
	TestRevolvedShapePoint(&sTest, 1.0f, 2.5f);
	AssertFloat(-1.0f, pCylinder->Distance(&sTest), 3);
	AssertTrue(pCylinder->Contains(&sTest));
	AssertRevolvedGradient(&pCylinder, &sTest, 1.0f, 0.0f);
	TestRevolvedShapePoint(&sTest, 5.0f, 9.0f);
	AssertFloat(5.0f, pCylinder->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCylinder, &sTest, 0.6f, 0.8f);

	ORoot()->Remove(pCylinder);
	pCylinder = NULL;

	sAxis.Init(0, 0, 1);
	sStart.Init(0, 0, 0);
	pCylinder = OMalloc<CCylinder>(&sAxis, &sStart, 2.0f, 5.0f);
	pCylinder->GetBounds(&sBounds);
	AssertFloat(-2.0f, sBounds.sMinimum.x, 4);
	AssertFloat(-2.0f, sBounds.sMinimum.y, 4);
	AssertFloat(0.0f, sBounds.sMinimum.z, 4);
	AssertFloat(2.0f, sBounds.sMaximum.y, 4);
	AssertFloat(5.0f, sBounds.sMaximum.z, 4);

	//On its side, exactly, is outside.
	sTest.Init(2.0f, 0.0f, 2.5f);
	AssertFloat(0.0f, pCylinder->Distance(&sTest), 6);
	AssertFalse(pCylinder->Contains(&sTest));
	pCylinder = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Radius 2 at its start and 1 at its end, length 4.  Its side leans in by
// 1 in 4, so its normal there is (4, 1) / sqrt 17.
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShapeCone(void)
{
	Ptr<CCone>		pCone;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sTest;
	CMesh			cMesh;
	CMeshEditor		cEditor;

	TestRevolvedShapeAxis(&sAxis, &sStart);
	pCone = OMalloc<CCone>(&sAxis, &sStart, 1.0f, 2.0f, 4.0f);
	ORoot()->Add(pCone);

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshConeEditor>()->Init(&sAxis, &sStart, 1.0f, 2.0f, 4.0f, 24, 2, 2, 3);
	cEditor.Collapse();
	AssertShapeOnMesh(&pCone, &cMesh);
	cEditor.Kill();
	cMesh.Kill();

	TestRevolvedShapePoint(&sTest, 0.0f, -1.0f);
	AssertFloat(1.0f, pCone->Distance(&sTest), 3);
	TestRevolvedShapePoint(&sTest, 0.5f, -1.0f);
	AssertRevolvedGradient(&pCone, &sTest, 0.0f, -1.0f);
	TestRevolvedShapePoint(&sTest, 0.5f, 5.0f);
	AssertFloat(1.0f, pCone->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCone, &sTest, 0.0f, 1.0f);
	TestRevolvedShapePoint(&sTest, 0.0f, 2.0f);
	AssertFloat(-1.4552f, pCone->Distance(&sTest), 3);
	AssertTrue(pCone->Contains(&sTest));
	TestRevolvedShapePoint(&sTest, 3.0f, 2.0f);
	AssertFloat(1.4552f, pCone->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCone, &sTest, 0.9701f, 0.2425f);

	ORoot()->Remove(pCone);
	pCone = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Walls from 1 out to 2, length 3: the hole is outside, its nearest wall
// the inner one.
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShapeTube(void)
{
	Ptr<CTube>		pTube;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sTest;
	CMesh			cMesh;
	CMeshEditor		cEditor;

	TestRevolvedShapeAxis(&sAxis, &sStart);
	pTube = OMalloc<CTube>(&sAxis, &sStart, 1.0f, 2.0f, 3.0f);
	ORoot()->Add(pTube);

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshTubeEditor>()->Init(&sAxis, &sStart, 1.0f, 2.0f, 3.0f, 24, 2, 3);
	cEditor.Collapse();
	AssertShapeOnMesh(&pTube, &cMesh);
	cEditor.Kill();
	cMesh.Kill();

	TestRevolvedShapePoint(&sTest, 0.5f, 1.5f);
	AssertFloat(0.5f, pTube->Distance(&sTest), 3);
	AssertFalse(pTube->Contains(&sTest));
	AssertRevolvedGradient(&pTube, &sTest, -1.0f, 0.0f);
	TestRevolvedShapePoint(&sTest, 1.5f, 1.4f);
	AssertFloat(-0.5f, pTube->Distance(&sTest), 3);
	AssertTrue(pTube->Contains(&sTest));
	TestRevolvedShapePoint(&sTest, 2.5f, 1.5f);
	AssertRevolvedGradient(&pTube, &sTest, 1.0f, 0.0f);
	TestRevolvedShapePoint(&sTest, 1.5f, -2.0f);
	AssertFloat(2.0f, pTube->Distance(&sTest), 3);
	AssertRevolvedGradient(&pTube, &sTest, 0.0f, -1.0f);

	ORoot()->Remove(pTube);
	pTube = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// 3 out to the middle of its tube, which is 1 across: its centre is 2 from
// it and the middle of its tube 1 inside.
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShapeTorus(void)
{
	Ptr<CTorus>		pTorus;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sCenter;
	SFloat32Vec3	sTest;
	CMesh			cMesh;
	CMeshEditor		cEditor;

	TestRevolvedShapeAxis(&sAxis, &sCenter);
	pTorus = OMalloc<CTorus>(&sAxis, &sCenter, 3.0f, 1.0f);
	ORoot()->Add(pTorus);

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshTorusEditor>()->Init(&sAxis, &sCenter, 3.0f, 1.0f, 24, 12);
	cEditor.Collapse();
	AssertShapeOnMesh(&pTorus, &cMesh);
	cEditor.Kill();
	cMesh.Kill();

	AssertFloat(2.0f, pTorus->Distance(&sCenter), 3);
	TestRevolvedShapePoint(&sTest, 3.0f, 0.0f);
	AssertFloat(-1.0f, pTorus->Distance(&sTest), 3);
	TestRevolvedShapePoint(&sTest, 5.0f, 0.0f);
	AssertFloat(1.0f, pTorus->Distance(&sTest), 3);
	AssertRevolvedGradient(&pTorus, &sTest, 1.0f, 0.0f);
	TestRevolvedShapePoint(&sTest, 3.0f, 2.0f);
	AssertRevolvedGradient(&pTorus, &sTest, 0.0f, 1.0f);
	TestRevolvedShapePoint(&sTest, 1.0f, 0.0f);
	AssertRevolvedGradient(&pTorus, &sTest, -1.0f, 0.0f);

	ORoot()->Remove(pTorus);
	pTorus = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Radius 1 around a line of length 4: round ends, straight side.  It has
// no mesh editor, so its surface is found by hand.
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShapeCapsule(void)
{
	Ptr<CCapsule>	pCapsule;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;
	SFloat32Vec3	sTest;
	SFloat32Bounds	sBounds;

	TestRevolvedShapeAxis(&sAxis, &sStart);
	pCapsule = OMalloc<CCapsule>(&sAxis, &sStart, 1.0f, 4.0f);
	ORoot()->Add(pCapsule);

	TestRevolvedShapePoint(&sTest, 0.0f, -1.0f);
	AssertFloat(0.0f, pCapsule->Distance(&sTest), 4);
	TestRevolvedShapePoint(&sTest, 1.0f, 2.0f);
	AssertFloat(0.0f, pCapsule->Distance(&sTest), 4);
	TestRevolvedShapePoint(&sTest, 0.6f, 4.8f);
	AssertFloat(0.0f, pCapsule->Distance(&sTest), 4);
	TestRevolvedShapePoint(&sTest, 3.0f, 2.0f);
	AssertFloat(2.0f, pCapsule->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCapsule, &sTest, 1.0f, 0.0f);
	TestRevolvedShapePoint(&sTest, 0.0f, 6.0f);
	AssertFloat(1.0f, pCapsule->Distance(&sTest), 3);
	AssertRevolvedGradient(&pCapsule, &sTest, 0.0f, 1.0f);
	TestRevolvedShapePoint(&sTest, 0.5f, 2.0f);
	AssertFloat(-0.5f, pCapsule->Distance(&sTest), 3);

	pCapsule->GetBounds(&sBounds);
	TestRevolvedShapePoint(&sTest, 0.0f, -1.0f);
	AssertTrue(sBounds.Distance(&sTest) <= 0.0001f);
	TestRevolvedShapePoint(&sTest, 1.0f, 4.0f);
	AssertTrue(sBounds.Distance(&sTest) <= 0.0001f);

	ORoot()->Remove(pCapsule);
	pCapsule = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Each is the same once saved and read back.
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShapeSaveLoad(void)
{
	char			szDirectory[] = "Output" _FS_ "TestRevolvedShape";
	CFileUtil		cFileUtil;
	SFloat32Vec3	sAxis;
	SFloat32Vec3	sStart;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));
	TestRevolvedShapeAxis(&sAxis, &sStart);

	ObjectsInit();
	{
		Ptr<CCone>		pCone;
		Ptr<CTorus>		pTorus;
		Ptr<CCapsule>	pCapsule;
		Ptr<CCylinder>	pCylinder;
		Ptr<CTube>		pTube;

		pCone = gcObjects.Malloc<CCone>("Cone");
		pCone->Init(&sAxis, &sStart, 1.0f, 2.0f, 4.0f);
		pTorus = gcObjects.Malloc<CTorus>("Torus");
		pTorus->Init(&sAxis, &sStart, 3.0f, 1.0f);
		pCapsule = gcObjects.Malloc<CCapsule>("Capsule");
		pCapsule->Init(&sAxis, &sStart, 1.0f, 4.0f);
		pCylinder = gcObjects.Malloc<CCylinder>("Cylinder");
		pCylinder->Init(&sAxis, &sStart, 2.0f, 5.0f);
		pTube = gcObjects.Malloc<CTube>("Tube");
		pTube->Init(&sAxis, &sStart, 1.0f, 2.0f, 3.0f);

		AssertTrue(TestShapeWrite(szDirectory, &pCone));
		AssertTrue(TestShapeWrite(szDirectory, &pTorus));
		AssertTrue(TestShapeWrite(szDirectory, &pCapsule));
		AssertTrue(TestShapeWrite(szDirectory, &pCylinder));
		AssertTrue(TestShapeWrite(szDirectory, &pTube));
	}
	ObjectsDiscard();
	ObjectsKill();

	ObjectsInit();
	{
		Ptr<CCone>		pCone;
		Ptr<CTorus>		pTorus;
		Ptr<CCapsule>	pCapsule;
		Ptr<CCylinder>	pCylinder;
		Ptr<CTube>		pTube;

		gcObjects.AddConstructor<CCone>();
		gcObjects.AddConstructor<CTorus>();
		gcObjects.AddConstructor<CCapsule>();
		gcObjects.AddConstructor<CCylinder>();
		gcObjects.AddConstructor<CTube>();

		pCone = TestShapeRead(szDirectory, "Cone");
		pTorus = TestShapeRead(szDirectory, "Torus");
		pCapsule = TestShapeRead(szDirectory, "Capsule");
		pCylinder = TestShapeRead(szDirectory, "Cylinder");
		pTube = TestShapeRead(szDirectory, "Tube");

		AssertNotNull(&pCone);
		AssertFloat(-2.0f, pCone->msStart.y, 3);
		AssertFloat(0.6667f, pCone->msAxis.z, 4);
		AssertFloat(4.0f, pCone->mfLength, 3);
		AssertFloat(1.0f, pCone->mfTopRadius, 3);
		AssertFloat(2.0f, pCone->mfBottomRadius, 3);
		AssertNotNull(&pTorus);
		AssertFloat(3.0f, pTorus->mfRadius, 3);
		AssertFloat(1.0f, pTorus->mfTubeRadius, 3);
		AssertFloat(0.0f, pTorus->mfLength, 3);
		AssertNotNull(&pCapsule);
		AssertFloat(1.0f, pCapsule->mfRadius, 3);
		AssertFloat(4.0f, pCapsule->mfLength, 3);
		AssertNotNull(&pCylinder);
		AssertFloat(2.0f, pCylinder->mfRadius, 3);
		AssertNotNull(&pTube);
		AssertFloat(1.0f, pTube->mfInnerRadius, 3);
		AssertFloat(2.0f, pTube->mfOuterRadius, 3);

		pCone = NULL;
		pTorus = NULL;
		pCapsule = NULL;
		pCylinder = NULL;
		pTube = NULL;
	}
	ObjectsDiscard();
	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRevolvedShape(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestRevolvedShapeCylinder();
	TestRevolvedShapeCone();
	TestRevolvedShapeTube();
	TestRevolvedShapeTorus();
	TestRevolvedShapeCapsule();
	ObjectsDiscard();
	ObjectsKill();

	TestRevolvedShapeSaveLoad();
	DataIOKill();

	TestStatistics();
}
