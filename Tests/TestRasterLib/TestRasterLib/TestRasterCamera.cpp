#include "BaseLib/GeometricTypes.h"
#include "RasterLib/RasterCamera.h"
#include "RasterLib/RasterMesh.h"
#include "TestLib/Assert.h"


#define TEST_HALF_PI	1.57079632679f


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertProject(int32 iExpectedX, int32 iExpectedY, CRasterCamera* pcCamera, int32 x, int32 y, int32 z)
{
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sScreen;

	sPosition.Init(x, y, z);
	pcCamera->Project(&sScreen, &sPosition);
	AssertInt(iExpectedX, sScreen.sPosition.x);
	AssertInt(iExpectedY, sScreen.sPosition.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraOrthographic(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sNear;
	SRasterScreenVertex	sFar;

	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(200, 100);
	cCamera.Prepare(200, 100);

	//The smaller side, 100 pixels or 1600 subpixels, fits the mesh.  y goes down the screen.
	AssertProject(1600, 800, &cCamera, 0, 0, 0);
	AssertProject(1600 + 800, 800, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);
	AssertProject(1600 - 800, 800, &cCamera, -RASTER_MESH_HALF_EXTENT, 0, 0);
	AssertProject(1600, 0, &cCamera, 0, RASTER_MESH_HALF_EXTENT, 0);

	sPosition.Init(0, 0, 100);
	cCamera.Project(&sNear, &sPosition);
	sPosition.Init(0, 0, -100);
	cCamera.Project(&sFar, &sPosition);
	AssertTrue(sNear.iDepth > sFar.iDepth);
	AssertTrue(sFar.iDepth > 0);

	//A quarter turn takes x to y.
	cCamera.SetOrbit(TEST_HALF_PI, 0.0f);
	cCamera.Prepare(200, 100);
	AssertProject(1600, 0, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);

	//A quarter tilt takes y out of the screen and z up it.
	cCamera.SetOrbit(0.0f, TEST_HALF_PI);
	cCamera.Prepare(200, 100);
	AssertProject(1600, 800, &cCamera, 0, RASTER_MESH_HALF_EXTENT, 0);
	AssertProject(1600, 1600, &cCamera, 0, 0, RASTER_MESH_HALF_EXTENT);

	cCamera.SetOrbit(0.0f, 0.0f);
	cCamera.SetPan(RASTER_MESH_HALF_EXTENT, 0);
	cCamera.Prepare(200, 100);
	AssertProject(1600 + 800, 800, &cCamera, 0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraPerspective(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sNear;
	SRasterScreenVertex	sFar;

	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 100);
	cCamera.Prepare(200, 100);

	AssertProject(1600, 800, &cCamera, 0, 0, 0);

	//At the distance of the eye's focus x / distance is exact: half of 100 pixels.
	AssertProject(1600 + 800, 800, &cCamera, 1 << 20, 0, 0);
	AssertProject(1600 - 800, 800, &cCamera, -(1 << 20), 0, 0);
	AssertProject(1600, 0, &cCamera, 0, 1 << 20, 0);

	//Nearer things are further from the centre and have a larger depth.
	sPosition.Init(1 << 20, 0, 1 << 20);
	cCamera.Project(&sNear, &sPosition);
	sPosition.Init(1 << 20, 0, -(1 << 20));
	cCamera.Project(&sFar, &sPosition);
	AssertTrue(sNear.sPosition.x > sFar.sPosition.x);
	AssertTrue(sNear.iDepth > sFar.iDepth);
	AssertTrue(sFar.iDepth > 0);

	//Behind the near plane is not drawable.
	sPosition.Init(0, 0, 1 << 21);
	cCamera.Project(&sNear, &sPosition);
	AssertInt(0, sNear.iDepth);
	sPosition.Init(0, 0, (1 << 21) + 1000);
	cCamera.Project(&sNear, &sPosition);
	AssertInt(0, sNear.iDepth);

	//Fit keeps a mesh at its largest on the screen.
	cCamera.Fit(100, 100);
	cCamera.Prepare(100, 100);
	sPosition.Init(RASTER_MESH_HALF_EXTENT, 0, RASTER_MESH_HALF_EXTENT);
	cCamera.Project(&sNear, &sPosition);
	AssertTrue(sNear.sPosition.x <= 100 << RASTER_SUBPIXEL_BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraViewToMesh(void)
{
	CRasterCamera	cCamera;
	SInt64Vec3		sView;
	SInt64Vec3		sMesh;

	cCamera.Init();
	cCamera.SetOrbit(TEST_HALF_PI, 0.0f);
	cCamera.Prepare(100, 100);

	//The mesh's x axis is the screen's y, so the screen's y comes back as the mesh's x.
	sView.Init(0, 16384, 0);
	cCamera.ViewToMesh(&sMesh, &sView);
	AssertLong(16384, sMesh.x);
	AssertTrue((sMesh.y >= -1) && (sMesh.y <= 1));
	AssertLong(0, sMesh.z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCamera(void)
{
	BeginTests();

	TestRasterCameraOrthographic();
	TestRasterCameraPerspective();
	TestRasterCameraViewToMesh();

	TestStatistics();
}

