#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterLib/RasterMesh.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshBuild(void)
{
	CMesh				cMesh;
	CRasterMesh			cRaster;
	SMeshNormalFace*	psNormalFace;
	SMeshFaceColour*	psColourFace;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	int					i;

	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	cMesh.mcColours.mbInUse = true;
	cMesh.mcColours.mcColours.Add(Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)255));
	cMesh.mcColours.mcColours.Add(Set32BitColour((uint8)0, (uint8)0, (uint8)255, (uint8)128));

	//A 4 x 2 quad, from (10, 20, 30), and a triangle with no corner normals that shares none of its corners.
	cMesh.AddCorner(10.0f, 20.0f, 30.0f);
	cMesh.AddCorner(14.0f, 20.0f, 30.0f);
	cMesh.AddCorner(14.0f, 22.0f, 30.0f);
	cMesh.AddCorner(10.0f, 22.0f, 30.0f);
	cMesh.AddCorner(10.0f, 20.0f, 31.0f);
	cMesh.AddCorner(14.0f, 20.0f, 31.0f);
	cMesh.AddCorner(14.0f, 22.0f, 31.0f);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(0, 2, 3);
	cMesh.AddFace(4, 5, 6);

	//Corner normals are after the three flat ones.
	cMesh.AddNormal(0.0f, 0.0f, 1.0f);
	for (i = 0; i < 2; i++)
	{
		psNormalFace = cMesh.mcNormals.mcFaces.Get(i);
		psNormalFace->aiCornerNormals[0] = 3;
		psNormalFace->aiCornerNormals[1] = 3;
		psNormalFace->aiCornerNormals[2] = 3;

		psColourFace = cMesh.mcColours.mcFaces.Get(i);
		psColourFace->aiCornerColours[0] = 0;
	}
	psColourFace = cMesh.mcColours.mcFaces.Get(2);
	psColourFace->aiCornerColours[0] = 1;

	cRaster.Init();
	cRaster.Build(&cMesh);
	AssertFalse(cMesh.GetConnectivity()->IsGenerated());

	//The quad shares corners 0 and 2.  The triangle has its own.
	AssertSize(7, cRaster.masVertices.NumElements());
	AssertSize(2, cRaster.masOpaque.NumElements());
	AssertSize(1, cRaster.masTransparent.NumElements());

	psTriangle = cRaster.masOpaque.Get(1);
	AssertInt(0, psTriangle->aiVertex[0]);
	AssertInt(2, psTriangle->aiVertex[1]);
	AssertInt(3, psTriangle->aiVertex[2]);
	AssertInt((int32)0xFFFF0000, (int32)psTriangle->uiColour);

	psTriangle = cRaster.masTransparent.Get(0);
	AssertInt((int32)0x800000FF, (int32)psTriangle->uiColour);

	//Centred on (12, 21, 30.5) with its half width of 2 made RASTER_MESH_HALF_EXTENT.
	psVertex = cRaster.masVertices.Get(0);
	AssertInt(-RASTER_MESH_HALF_EXTENT, psVertex->sPosition.x);
	AssertInt(-RASTER_MESH_HALF_EXTENT / 2, psVertex->sPosition.y);
	AssertInt(-RASTER_MESH_HALF_EXTENT / 4, psVertex->sPosition.z);
	AssertInt(0, psVertex->sNormal.x.miRaw);
	AssertInt(0, psVertex->sNormal.y.miRaw);
	AssertInt(16384, psVertex->sNormal.z.miRaw);

	psVertex = cRaster.masVertices.Get(2);
	AssertInt(RASTER_MESH_HALF_EXTENT, psVertex->sPosition.x);
	AssertInt(RASTER_MESH_HALF_EXTENT / 2, psVertex->sPosition.y);

	//The triangle uses its flat normal, which faces out of the plane.
	psVertex = cRaster.masVertices.Get(6);
	AssertInt(16384, psVertex->sNormal.z.miRaw);

	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshDifferentNormals(void)
{
	CMesh				cMesh;
	CRasterMesh			cRaster;

	//The faces share an edge but not their flat normals, so the shared corners must not be merged.
	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	cMesh.AddCorner(0.0f, 0.0f, 0.0f);
	cMesh.AddCorner(1.0f, 0.0f, 0.0f);
	cMesh.AddCorner(0.0f, 1.0f, 0.0f);
	cMesh.AddCorner(0.0f, 0.0f, 1.0f);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(1, 0, 3);

	cRaster.Init();
	cRaster.Build(&cMesh);
	AssertSize(6, cRaster.masVertices.NumElements());
	AssertInt(16384, cRaster.masVertices.Get(0)->sNormal.z.miRaw);
	AssertInt(16384, cRaster.masVertices.Get(3)->sNormal.y.miRaw);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshEmpty(void)
{
	CMesh		cMesh;
	CRasterMesh	cRaster;

	cMesh.Init();
	cRaster.Init();
	cRaster.Build(&cMesh);
	AssertSize(0, cRaster.masVertices.NumElements());
	AssertSize(0, cRaster.masOpaque.NumElements());
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// iVertex's position and a normal facing +Z.
//
//////////////////////////////////////////////////////////////////////////
static void TestRasterAddVertex(CRasterMesh* pcRaster, int32 x, int32 y, int32 z)
{
	SRasterVertex*	psVertex;

	psVertex = pcRaster->masVertices.Add();
	psVertex->sPosition.Init(x, y, z);
	psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestRasterAddTriangle(CArrayRasterTriangle* pasTriangles, int32 i0, int32 i1, int32 i2, uint32 uiColour)
{
	SRasterTriangle*	psTriangle;

	psTriangle = pasTriangles->Add();
	psTriangle->aiVertex[0] = i0;
	psTriangle->aiVertex[1] = i1;
	psTriangle->aiVertex[2] = i2;
	psTriangle->uiColour = uiColour;
}


//////////////////////////////////////////////////////////////////////////
//
// Built where it is and at the scale given, not centred or fitted.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshBuildScaled(void)
{
	CMesh		cMesh;
	CRasterMesh	cRaster;

	cMesh.Init();
	cMesh.AddCorner(10.0f, 20.0f, 30.0f);
	cMesh.AddCorner(14.0f, 20.0f, 30.0f);
	cMesh.AddCorner(14.0f, 22.0f, 30.0f);
	cMesh.AddFace(0, 1, 2);

	cRaster.Init();
	cRaster.Build(&cMesh, 100);
	AssertSize(3, cRaster.masVertices.NumElements());
	AssertInt(1000, cRaster.masVertices.Get(0)->sPosition.x);
	AssertInt(2000, cRaster.masVertices.Get(0)->sPosition.y);
	AssertInt(3000, cRaster.masVertices.Get(0)->sPosition.z);
	AssertInt(1400, cRaster.masVertices.Get(1)->sPosition.x);
	AssertInt(1000, cRaster.msMin.x);
	AssertInt(1400, cRaster.msMax.x);
	AssertInt(2200, cRaster.msMax.y);
	AssertSize(1, cRaster.NumTriangles());
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshRemoveDegenerate(void)
{
	CRasterMesh	cRaster;

	cRaster.Init();
	TestRasterAddVertex(&cRaster, 0, 0, 0);
	TestRasterAddVertex(&cRaster, 100, 0, 0);
	TestRasterAddVertex(&cRaster, 0, 100, 0);
	TestRasterAddVertex(&cRaster, 200, 0, 0);
	TestRasterAddVertex(&cRaster, 0, 0, 0);

	TestRasterAddTriangle(&cRaster.masOpaque, 0, 1, 2, 0xFF111111);		//Kept.
	TestRasterAddTriangle(&cRaster.masOpaque, 0, 1, 3, 0xFF222222);		//In a line.
	TestRasterAddTriangle(&cRaster.masOpaque, 0, 4, 1, 0xFF333333);		//Two corners in the same place.
	TestRasterAddTriangle(&cRaster.masOpaque, 1, 0, 2, 0xFF444444);		//Kept: wound the other way isn't degenerate.
	TestRasterAddTriangle(&cRaster.masTransparent, 1, 1, 2, 0x80555555);	//One vertex twice.
	TestRasterAddTriangle(&cRaster.masTransparent, 0, 1, 2, 0x80666666);	//Kept.

	AssertSize(3, cRaster.RemoveDegenerate());
	AssertSize(2, cRaster.masOpaque.NumElements());
	AssertSize(1, cRaster.masTransparent.NumElements());
	AssertIntHex((int32)0xFF111111, (int32)cRaster.masOpaque.Get(0)->uiColour);
	AssertIntHex((int32)0xFF444444, (int32)cRaster.masOpaque.Get(1)->uiColour);
	AssertIntHex((int32)0x80666666, (int32)cRaster.masTransparent.Get(0)->uiColour);
	AssertSize(0, cRaster.RemoveDegenerate());
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What's saved loads the same, but not as the mesh of another source, nor
// from no file.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshFile(void)
{
	CRasterMesh	cRaster;
	CRasterMesh	cLoaded;
	CFileUtil	cFileUtil;

	cFileUtil.TouchDir("Output");
	cRaster.Init();
	TestRasterAddVertex(&cRaster, 1, 2, 3);
	TestRasterAddVertex(&cRaster, 100, 0, 0);
	TestRasterAddVertex(&cRaster, 0, 100, 0);
	TestRasterAddTriangle(&cRaster.masOpaque, 0, 1, 2, 0xFF112233);
	TestRasterAddTriangle(&cRaster.masTransparent, 0, 2, 1, 0x80445566);
	cRaster.msMin.Init(0, 0, 0);
	cRaster.msMax.Init(100, 100, 3);
	AssertTrue(cRaster.Save("Output" _FS_ "Mesh.rastermesh", 0x1234567812345678ULL));

	cLoaded.Init();
	AssertFalse(cLoaded.Load("Output" _FS_ "NoSuchMesh.rastermesh", 0x1234567812345678ULL));
	AssertFalse(cLoaded.Load("Output" _FS_ "Mesh.rastermesh", 0x1234567812345679ULL));
	AssertSize(0, cLoaded.masVertices.NumElements());
	AssertTrue(cLoaded.Load("Output" _FS_ "Mesh.rastermesh", 0x1234567812345678ULL));
	AssertSize(3, cLoaded.masVertices.NumElements());
	AssertSize(1, cLoaded.masOpaque.NumElements());
	AssertSize(1, cLoaded.masTransparent.NumElements());
	AssertInt(2, cLoaded.masVertices.Get(0)->sPosition.y);
	AssertIntHex((int32)0xFF112233, (int32)cLoaded.masOpaque.Get(0)->uiColour);
	AssertIntHex((int32)0x80445566, (int32)cLoaded.masTransparent.Get(0)->uiColour);
	AssertInt(100, cLoaded.msMax.x);
	cLoaded.Kill();
	cRaster.Kill();
	cFileUtil.Delete("Output" _FS_ "Mesh.rastermesh");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMesh(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasterMeshBuild();
	TestRasterMeshDifferentNormals();
	TestRasterMeshEmpty();
	TestRasterMeshBuildScaled();
	TestRasterMeshRemoveDegenerate();
	TestRasterMeshFile();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

