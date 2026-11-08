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
	AssertInt(0, psVertex->sNormal.x);
	AssertInt(0, psVertex->sNormal.y);
	AssertInt(1 << RASTER_NORMAL_BITS, psVertex->sNormal.z);

	psVertex = cRaster.masVertices.Get(2);
	AssertInt(RASTER_MESH_HALF_EXTENT, psVertex->sPosition.x);
	AssertInt(RASTER_MESH_HALF_EXTENT / 2, psVertex->sPosition.y);

	//The triangle uses its flat normal, which faces out of the plane.
	psVertex = cRaster.masVertices.Get(6);
	AssertInt(1 << RASTER_NORMAL_BITS, psVertex->sNormal.z);

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
	AssertInt(1 << RASTER_NORMAL_BITS, cRaster.masVertices.Get(0)->sNormal.z);
	AssertInt(1 << RASTER_NORMAL_BITS, cRaster.masVertices.Get(3)->sNormal.y);
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

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

