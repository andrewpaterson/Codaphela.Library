#include <stdlib.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterLib/RasterMesh.h"
#include "RasterRenderLib/RasterMeshBuild.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshBuild(void)
{
	CMesh				cMesh;
	Ptr<CRasterMesh>	pRaster;
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

	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &cMesh);
	AssertFalse(cMesh.GetConnectivity()->IsGenerated());

	//The quad shares corners 0 and 2.  The triangle has its own.
	AssertSize(7, pRaster->masVertices.NumElements());
	AssertSize(2, pRaster->masOpaque.NumElements());
	AssertSize(1, pRaster->masTransparent.NumElements());

	psTriangle = pRaster->masOpaque.Get(1);
	AssertInt(0, psTriangle->aiVertex[0]);
	AssertInt(2, psTriangle->aiVertex[1]);
	AssertInt(3, psTriangle->aiVertex[2]);
	AssertInt((int32)0xFFFF0000, (int32)psTriangle->uiColour);

	psTriangle = pRaster->masTransparent.Get(0);
	AssertInt((int32)0x800000FF, (int32)psTriangle->uiColour);

	//Centred on (12, 21, 30.5) with its half width of 2 made RASTER_MESH_HALF_EXTENT.
	psVertex = pRaster->masVertices.Get(0);
	AssertInt(-RASTER_MESH_HALF_EXTENT, psVertex->sPosition.x);
	AssertInt(-RASTER_MESH_HALF_EXTENT / 2, psVertex->sPosition.y);
	AssertInt(-RASTER_MESH_HALF_EXTENT / 4, psVertex->sPosition.z);
	AssertInt(0, psVertex->sNormal.x.miRaw);
	AssertInt(0, psVertex->sNormal.y.miRaw);
	AssertInt(16384, psVertex->sNormal.z.miRaw);

	psVertex = pRaster->masVertices.Get(2);
	AssertInt(RASTER_MESH_HALF_EXTENT, psVertex->sPosition.x);
	AssertInt(RASTER_MESH_HALF_EXTENT / 2, psVertex->sPosition.y);

	//The triangle uses its flat normal, which faces out of the plane.
	psVertex = pRaster->masVertices.Get(6);
	AssertInt(16384, psVertex->sNormal.z.miRaw);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshDifferentNormals(void)
{
	CMesh				cMesh;
	Ptr<CRasterMesh>	pRaster;

	//The faces share an edge but not their flat normals, so the shared corners must not be merged.
	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	cMesh.AddCorner(0.0f, 0.0f, 0.0f);
	cMesh.AddCorner(1.0f, 0.0f, 0.0f);
	cMesh.AddCorner(0.0f, 1.0f, 0.0f);
	cMesh.AddCorner(0.0f, 0.0f, 1.0f);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(1, 0, 3);

	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &cMesh);
	AssertSize(6, pRaster->masVertices.NumElements());
	AssertInt(16384, pRaster->masVertices.Get(0)->sNormal.z.miRaw);
	AssertInt(16384, pRaster->masVertices.Get(3)->sNormal.y.miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshEmpty(void)
{
	CMesh				cMesh;
	Ptr<CRasterMesh>	pRaster;

	cMesh.Init();
	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &cMesh);
	AssertSize(0, pRaster->masVertices.NumElements());
	AssertSize(0, pRaster->masOpaque.NumElements());
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
	CMesh				cMesh;
	Ptr<CRasterMesh>	pRaster;

	cMesh.Init();
	cMesh.AddCorner(10.0f, 20.0f, 30.0f);
	cMesh.AddCorner(14.0f, 20.0f, 30.0f);
	cMesh.AddCorner(14.0f, 22.0f, 30.0f);
	cMesh.AddFace(0, 1, 2);

	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &cMesh, 100);
	AssertSize(3, pRaster->masVertices.NumElements());
	AssertInt(1000, pRaster->masVertices.Get(0)->sPosition.x);
	AssertInt(2000, pRaster->masVertices.Get(0)->sPosition.y);
	AssertInt(3000, pRaster->masVertices.Get(0)->sPosition.z);
	AssertInt(1400, pRaster->masVertices.Get(1)->sPosition.x);
	AssertInt(1000, pRaster->msMin.x);
	AssertInt(1400, pRaster->msMax.x);
	AssertInt(2200, pRaster->msMax.y);
	AssertSize(1, pRaster->NumTriangles());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshRemoveDegenerate(void)
{
	Ptr<CRasterMesh>	pRaster;

	pRaster = OMalloc<CRasterMesh>();
	TestRasterAddVertex(&pRaster, 0, 0, 0);
	TestRasterAddVertex(&pRaster, 100, 0, 0);
	TestRasterAddVertex(&pRaster, 0, 100, 0);
	TestRasterAddVertex(&pRaster, 200, 0, 0);
	TestRasterAddVertex(&pRaster, 0, 0, 0);

	TestRasterAddTriangle(&pRaster->masOpaque, 0, 1, 2, 0xFF111111);		//Kept.
	TestRasterAddTriangle(&pRaster->masOpaque, 0, 1, 3, 0xFF222222);		//In a line.
	TestRasterAddTriangle(&pRaster->masOpaque, 0, 4, 1, 0xFF333333);		//Two corners in the same place.
	TestRasterAddTriangle(&pRaster->masOpaque, 1, 0, 2, 0xFF444444);		//Kept: wound the other way isn't degenerate.
	TestRasterAddTriangle(&pRaster->masTransparent, 1, 1, 2, 0x80555555);	//One vertex twice.
	TestRasterAddTriangle(&pRaster->masTransparent, 0, 1, 2, 0x80666666);	//Kept.

	AssertSize(3, pRaster->RemoveDegenerate());
	AssertSize(2, pRaster->masOpaque.NumElements());
	AssertSize(1, pRaster->masTransparent.NumElements());
	AssertIntHex((int32)0xFF111111, (int32)pRaster->masOpaque.Get(0)->uiColour);
	AssertIntHex((int32)0xFF444444, (int32)pRaster->masOpaque.Get(1)->uiColour);
	AssertIntHex((int32)0x80666666, (int32)pRaster->masTransparent.Get(0)->uiColour);
	AssertSize(0, pRaster->RemoveDegenerate());
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex's padding after its normal is saved with it, so it's zero
// whatever was in the memory before.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshPadding(void)
{
	CMesh				cMesh;
	Ptr<CRasterMesh>	pRaster;
	SRasterVertex*		psVertex;
	uint8*				puiPad;
	size				uiPad;
	size				i;
	size				j;

	cMesh.Init();
	cMesh.AddCorner(0.0f, 0.0f, 0.0f);
	cMesh.AddCorner(1.0f, 0.0f, 0.0f);
	cMesh.AddCorner(0.0f, 1.0f, 0.0f);
	cMesh.AddFace(0, 1, 2);

	pRaster = OMalloc<CRasterMesh>();
	for (i = 0; i < 3; i++)
	{
		psVertex = pRaster->masVertices.Add();
		memset(psVertex, 0xFF, sizeof(SRasterVertex));
	}
	BuildRasterMesh(&pRaster, &cMesh);
	AssertSize(3, pRaster->masVertices.NumElements());

	uiPad = sizeof(SRasterVertex) - (sizeof(SInt32Vec3) + sizeof(SRasterUnitVec3));
	for (i = 0; i < pRaster->masVertices.NumElements(); i++)
	{
		puiPad = (uint8*)(&pRaster->masVertices.Get(i)->sNormal + 1);
		for (j = 0; j < uiPad; j++)
		{
			AssertInt(0, puiPad[j]);
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//
// What's saved loads the same, but not as the mesh of another source, nor
// from no file.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMeshFile(void)
{
	Ptr<CRasterMesh>	pRaster;
	Ptr<CRasterMesh>	pLoaded;
	CFileUtil			cFileUtil;

	cFileUtil.TouchDir("Output");
	pRaster = OMalloc<CRasterMesh>();
	TestRasterAddVertex(&pRaster, 1, 2, 3);
	TestRasterAddVertex(&pRaster, 100, 0, 0);
	TestRasterAddVertex(&pRaster, 0, 100, 0);
	TestRasterAddTriangle(&pRaster->masOpaque, 0, 1, 2, 0xFF112233);
	TestRasterAddTriangle(&pRaster->masTransparent, 0, 2, 1, 0x80445566);
	pRaster->msMin.Init(0, 0, 0);
	pRaster->msMax.Init(100, 100, 3);
	AssertTrue(pRaster->WriteFile("Output" _FS_ "Mesh.rastermesh", 0x1234567812345678ULL));

	pLoaded = OMalloc<CRasterMesh>();
	AssertFalse(pLoaded->ReadFile("Output" _FS_ "NoSuchMesh.rastermesh", 0x1234567812345678ULL));
	AssertFalse(pLoaded->ReadFile("Output" _FS_ "Mesh.rastermesh", 0x1234567812345679ULL));
	AssertSize(0, pLoaded->masVertices.NumElements());
	AssertTrue(pLoaded->ReadFile("Output" _FS_ "Mesh.rastermesh", 0x1234567812345678ULL));
	AssertSize(3, pLoaded->masVertices.NumElements());
	AssertSize(1, pLoaded->masOpaque.NumElements());
	AssertSize(1, pLoaded->masTransparent.NumElements());
	AssertInt(2, pLoaded->masVertices.Get(0)->sPosition.y);
	AssertIntHex((int32)0xFF112233, (int32)pLoaded->masOpaque.Get(0)->uiColour);
	AssertIntHex((int32)0x80445566, (int32)pLoaded->masTransparent.Get(0)->uiColour);
	AssertInt(100, pLoaded->msMax.x);
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
	TestRasterMeshPadding();
	TestRasterMeshFile();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

