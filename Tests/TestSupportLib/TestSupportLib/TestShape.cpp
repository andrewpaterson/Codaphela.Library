#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/Numbers.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/HalfSpace.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The shape's gradient is a unit and agrees with central differences at a
// point away from its creases.
//
//////////////////////////////////////////////////////////////////////////
static void AssertShapeGradient(CShape* pcShape, SFloat32Vec3* psPosition, float32 fX, float32 fY, float32 fZ)
{
	SFloat32Vec3	sGradient;
	SFloat32Vec3	sDifferences;

	pcShape->Gradient(&sGradient, psPosition);
	AssertFloat(fX, sGradient.x, 3);
	AssertFloat(fY, sGradient.y, 3);
	AssertFloat(fZ, sGradient.z, 3);

	pcShape->CShape::Gradient(&sDifferences, psPosition);
	AssertFloat(sGradient.x, sDifferences.x, 2);
	AssertFloat(sGradient.y, sDifferences.y, 2);
	AssertFloat(sGradient.z, sDifferences.z, 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere's distance from its surface, negative inside and zero on it,
// which is outside; its gradient points straight out; its bounds hold it.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeSphere(void)
{
	Ptr<CSphere>	pSphere;
	SFloat32Vec3	sCenter;
	SFloat32Vec3	sTest;
	SFloat32Bounds	sBounds;

	sCenter.Init(1, 2, 3);
	pSphere = OMalloc<CSphere>(&sCenter, 2.0f);
	ORoot()->Add(pSphere);

	sTest.Init(1, 2, 8);
	AssertFloat(3.0f, pSphere->Distance(&sTest), 3);
	AssertFalse(pSphere->Contains(&sTest));
	AssertShapeGradient(&pSphere, &sTest, 0, 0, 1);

	sTest.Init(2, 2, 3);
	AssertFloat(-1.0f, pSphere->Distance(&sTest), 3);
	AssertTrue(pSphere->Contains(&sTest));
	AssertShapeGradient(&pSphere, &sTest, 1, 0, 0);

	sTest.Init(1, 0, 3);
	AssertFloat(0.0f, pSphere->Distance(&sTest), 3);
	AssertFalse(pSphere->Contains(&sTest));
	AssertShapeGradient(&pSphere, &sTest, 0, -1, 0);

	pSphere->GetBounds(&sBounds);
	AssertFloat(-1.0f, sBounds.sMinimum.x, 3);
	AssertFloat(5.0f, sBounds.sMaximum.z, 3);
	AssertTrue(pSphere->IsExact());

	ORoot()->Remove(pSphere);
	pSphere = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An 8 x 4 x 2 box turned 30 degrees about y and moved: its gradient points
// out of the face a point is beyond or nearest, and from the nearest edge
// past an edge.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeBox(void)
{
	Ptr<CBox>		pBox;
	SFloat32Box		sBox;
	SFloat4x4		sMatrix;
	SFloat32Vec3	sCenter;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sTest;
	SFloat32Vec3	sEdge;
	SFloat32Bounds	sBounds;

	Float4x4RotationY(&sMatrix, Deg2Rad(30.0f));
	asAxes[0].Init(sMatrix.x.x, sMatrix.x.y, sMatrix.x.z);
	asAxes[1].Init(sMatrix.y.x, sMatrix.y.y, sMatrix.y.z);
	asAxes[2].Init(sMatrix.z.x, sMatrix.z.y, sMatrix.z.z);
	sCenter.Init(10, -5, 3);
	sBox.Init(&sCenter, &asAxes[0], 4.0f, &asAxes[1], 2.0f, &asAxes[2], 1.0f);

	pBox = OMalloc<CBox>(&sBox);
	ORoot()->Add(pBox);

	//Beyond the long axis's far face.
	sTest = sCenter + asAxes[0] * 7.0f + asAxes[1] * 0.5f;
	AssertFloat(3.0f, pBox->Distance(&sTest), 3);
	AssertShapeGradient(&pBox, &sTest, asAxes[0].x, asAxes[0].y, asAxes[0].z);

	//Past the edge between the long and middle axes' far faces, 3 and 4 beyond them.
	sTest = sCenter + asAxes[0] * 7.0f + asAxes[1] * 6.0f;
	AssertFloat(5.0f, pBox->Distance(&sTest), 3);
	sEdge = asAxes[0] * 0.6f + asAxes[1] * 0.8f;
	AssertShapeGradient(&pBox, &sTest, sEdge.x, sEdge.y, sEdge.z);

	//Inside, nearest the short axis's near face.
	sTest = sCenter - asAxes[2] * 0.75f + asAxes[0] * 0.5f;
	AssertFloat(-0.25f, pBox->Distance(&sTest), 3);
	AssertTrue(pBox->Contains(&sTest));
	AssertShapeGradient(&pBox, &sTest, -asAxes[2].x, -asAxes[2].y, -asAxes[2].z);

	//On a face is outside.
	sTest = sCenter + asAxes[2] * 1.0f;
	AssertFloat(0.0f, pBox->Distance(&sTest), 3);
	AssertFalse(pBox->Contains(&sTest));

	pBox->GetBounds(&sBounds);
	AssertFloat(-7.0f, sBounds.sMinimum.y, 3);
	AssertFloat(-3.0f, sBounds.sMaximum.y, 3);

	ORoot()->Remove(pBox);
	pBox = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Everything behind a plane: its gradient is the plane's normal everywhere,
// and its bounds are everything.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeHalfSpace(void)
{
	Ptr<CHalfSpace>	pHalfSpace;
	SFloat32Plane	sPlane;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sTest;
	SFloat32Bounds	sBounds;

	sNormal.Init(0, 3, 4);
	sPoint.Init(0, 3, 4);
	sPlane.Init(&sNormal, &sPoint);
	pHalfSpace = OMalloc<CHalfSpace>(&sPlane);
	ORoot()->Add(pHalfSpace);

	sTest.Init(7, 6, 8);
	AssertFloat(5.0f, pHalfSpace->Distance(&sTest), 3);
	AssertShapeGradient(&pHalfSpace, &sTest, 0, 0.6f, 0.8f);
	sTest.Init(-7, 0, 0);
	AssertFloat(-5.0f, pHalfSpace->Distance(&sTest), 3);
	AssertTrue(pHalfSpace->Contains(&sTest));
	AssertShapeGradient(&pHalfSpace, &sTest, 0, 0.6f, 0.8f);
	AssertFalse(pHalfSpace->Contains(&sPoint));

	pHalfSpace->GetBounds(&sBounds);
	AssertTrue(sBounds.sMinimum.x <= -FLOAT32_BOUNDS_LARGEST);
	AssertTrue(sBounds.sMaximum.z >= FLOAT32_BOUNDS_LARGEST);

	ORoot()->Remove(pHalfSpace);
	pHalfSpace = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestShapeAddConstructors(void)
{
	gcObjects.AddConstructor<CSphere>();
	gcObjects.AddConstructor<CBox>();
	gcObjects.AddConstructor<CHalfSpace>();
}


//////////////////////////////////////////////////////////////////////////
//
// A chunk file holds the object written to it last, so each shape has a
// file of its own.
//
//////////////////////////////////////////////////////////////////////////
static bool TestShapeWrite(char* szDirectory, CShape* pcShape)
{
	CExternalObjectSerialiser	cSerialiser;
	CChunkFileObjectWriter		cWriter;
	bool						bResult;

	cWriter.Init(szDirectory, "", pcShape->GetName());
	cSerialiser.Init(&cWriter);
	bResult = cSerialiser.Write(pcShape);
	cSerialiser.Kill();
	cWriter.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static CBaseObject* TestShapeRead(char* szDirectory, const char* szName)
{
	CExternalObjectDeserialiser		cDeserialiser;
	CChunkFileSystemObjectReader	cReader;
	CBaseObject*					pcObject;

	cReader.Init(szDirectory, szName);
	cDeserialiser.Init(&cReader, false, &gcObjects);
	pcObject = cDeserialiser.Read(szName);
	cDeserialiser.Kill();
	cReader.Kill();
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// Each shape is the same once saved and read back.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeSaveLoad(void)
{
	char		szDirectory[] = "Output" _FS_ "TestShape";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CSphere>	pSphere;
		Ptr<CBox>		pBox;
		Ptr<CHalfSpace>	pHalfSpace;
		SFloat32Vec3	sCenter;
		SFloat32Vec3	asPoints[2];
		SFloat32Box		sBox;
		SFloat32Plane	sPlane;

		TestShapeAddConstructors();
		sCenter.Init(1, 2, 3);
		pSphere = gcObjects.Malloc<CSphere>("Sphere");
		pSphere->Init(&sCenter, 2.5f);

		asPoints[0].Init(-1, 2, 3);
		asPoints[1].Init(5, 4, 4);
		sBox.Init(asPoints, sizeof(SFloat32Vec3), 2);
		pBox = gcObjects.Malloc<CBox>("Box");
		pBox->Init(&sBox);

		sPlane.Init(&sCenter, 7.0f);
		pHalfSpace = gcObjects.Malloc<CHalfSpace>("HalfSpace");
		pHalfSpace->Init(&sPlane);

		AssertTrue(TestShapeWrite(szDirectory, &pSphere));
		AssertTrue(TestShapeWrite(szDirectory, &pBox));
		AssertTrue(TestShapeWrite(szDirectory, &pHalfSpace));
	}
	ObjectsDiscard();
	ObjectsKill();

	ObjectsInit();
	{
		Ptr<CSphere>	pSphere;
		Ptr<CBox>		pBox;
		Ptr<CHalfSpace>	pHalfSpace;

		TestShapeAddConstructors();
		pSphere = TestShapeRead(szDirectory, "Sphere");
		pBox = TestShapeRead(szDirectory, "Box");
		pHalfSpace = TestShapeRead(szDirectory, "HalfSpace");

		AssertNotNull(&pSphere);
		AssertFloat(2.0f, pSphere->msSphere.sCenter.y, 3);
		AssertFloat(2.5f, pSphere->msSphere.fRadius, 3);

		AssertNotNull(&pBox);
		AssertFloat(2.0f, pBox->msBox.sCenter.x, 3);
		AssertFloat(3.5f, pBox->msBox.sCenter.z, 3);
		AssertFloat(1.0f, pBox->msBox.asAxes[0].x, 3);
		AssertFloat(1.0f, pBox->msBox.asAxes[1].y, 3);
		AssertFloat(1.0f, pBox->msBox.asAxes[2].z, 3);
		AssertFloat(3.0f, pBox->msBox.afHalfLengths[0], 3);
		AssertFloat(0.5f, pBox->msBox.afHalfLengths[2], 3);

		AssertNotNull(&pHalfSpace);
		AssertFloat(3.0f, pHalfSpace->msPlane.sNormal.z, 3);
		AssertFloat(7.0f, pHalfSpace->msPlane.fDistance, 3);

		pSphere = NULL;
		pBox = NULL;
		pHalfSpace = NULL;
	}
	ObjectsDiscard();
	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestShape(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestShapeSphere();
	TestShapeBox();
	TestShapeHalfSpace();
	ObjectsDiscard();
	ObjectsKill(false);

	TestShapeSaveLoad();
	DataIOKill();

	TestStatistics();
}
