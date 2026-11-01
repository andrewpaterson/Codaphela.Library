#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshPolygonsAddFace(void)
{
	DataIOInit();
	ObjectsInit();
	{
		CMesh			cMesh;
		CMeshEditor		cMeshEditor;
		CMeshPolygon* pcPolygon;

		cMesh.Init();
		cMeshEditor.Init(&cMesh);

		cMesh.AddCorner(0, 0, 0);
		cMesh.AddCorner(1, 0, 0);
		cMesh.AddCorner(1, 1, 0);
		cMesh.AddCorner(0, 1, 0);

		AssertSize(0, cMeshEditor.mcPolygons.mcPolygons.NumElements());

		cMeshEditor.AddFace(0, 1, 2, 1, true, true, false);

		AssertSize(3, cMeshEditor.mcEdgeVisibility.mcEdges.NumElements());
		AssertChar(1, cMeshEditor.mcEdgeVisibility.GetEdge(0));
		AssertChar(1, cMeshEditor.mcEdgeVisibility.GetEdge(1));
		AssertChar(0, cMeshEditor.mcEdgeVisibility.GetEdge(2));

		AssertSize(1, cMeshEditor.mcPolygons.mcPolygons.NumElements());

		pcPolygon = cMeshEditor.mcPolygons.mcPolygons.Get(0);

		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(0));
		AssertSize(1, pcPolygon->maiFaces.NumElements());
		AssertInt(0, pcPolygon->maiFaces.GetValue(0));

		cMeshEditor.AddFace(0, 2, 3, 1, false, true, true);

		AssertSize(5, cMeshEditor.mcEdgeVisibility.mcEdges.NumElements());
		AssertChar(1, cMeshEditor.mcEdgeVisibility.GetEdge(0));
		AssertChar(1, cMeshEditor.mcEdgeVisibility.GetEdge(1));
		AssertChar(0, cMeshEditor.mcEdgeVisibility.GetEdge(2));
		AssertChar(1, cMeshEditor.mcEdgeVisibility.GetEdge(3));
		AssertChar(1, cMeshEditor.mcEdgeVisibility.GetEdge(4));

		AssertSize(1, cMeshEditor.mcPolygons.mcPolygons.NumElements());

		pcPolygon = cMeshEditor.mcPolygons.mcPolygons.Get(0);

		AssertSize(2, cMeshEditor.mcPolygons.maiFacesToPolygons.NumElements());
		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(0));
		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(1));
		AssertSize(2, pcPolygon->maiFaces.NumElements());
		AssertInt(0, pcPolygon->maiFaces.GetValue(0));
		AssertInt(1, pcPolygon->maiFaces.GetValue(1));
	}
	ObjectsKill(false);
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshPolygonsAddDiscontiguousFace(bool bOutEdgeValue)
{
	DataIOInit();
	ObjectsInit();
	{
		CMesh			cMesh;
		CMeshEditor		cMeshEditor;
		CMeshPolygon* pcPolygon;

		cMesh.Init();
		cMeshEditor.Init(&cMesh);

		cMesh.AddCorner(0, 0, 0);
		cMesh.AddCorner(1, 0, 0);
		cMesh.AddCorner(1, 1, 0);
		cMesh.AddCorner(0, 1, 0);
		cMesh.AddCorner(-0.5f, 0.5f, 0);
		cMesh.AddCorner(0.5f, 1.5f, 0);

		AssertSize(0, cMeshEditor.mcPolygons.mcPolygons.NumElements());

		cMeshEditor.AddFace(0, 1, 2, 1, bOutEdgeValue, bOutEdgeValue, false);
		cMeshEditor.AddFace(0, 3, 4, 1, false, bOutEdgeValue, bOutEdgeValue);
		cMeshEditor.AddFace(5, 3, 2, 1, bOutEdgeValue, false, bOutEdgeValue);

		AssertSize(9, cMeshEditor.mcEdgeVisibility.mcEdges.NumElements());
		AssertSize(3, cMeshEditor.mcPolygons.mcPolygons.NumElements());

		cMeshEditor.AddFace(0, 2, 3, 1, false, false, false);

		AssertSize(9, cMeshEditor.mcEdgeVisibility.mcEdges.NumElements());
		AssertSize(1, cMeshEditor.mcPolygons.mcPolygons.NumElements());

		AssertSize(4, cMeshEditor.mcPolygons.maiFacesToPolygons.NumElements());
		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(0));
		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(1));
		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(2));
		AssertInt(0, cMeshEditor.mcPolygons.maiFacesToPolygons.GetValue(3));

		pcPolygon = cMeshEditor.mcPolygons.Get(0);
		AssertSize(4, pcPolygon->maiFaces.NumElements());

		//The order isn't important.  We must not have lost any faces though.
		AssertInt(0, pcPolygon->maiFaces.GetValue(0));
		AssertInt(3, pcPolygon->maiFaces.GetValue(1));
		AssertInt(1, pcPolygon->maiFaces.GetValue(2));
		AssertInt(2, pcPolygon->maiFaces.GetValue(3));

		cMeshEditor.AddFace(5, 4, 3, 2, true, true, true);

		AssertSize(10, cMeshEditor.mcEdgeVisibility.mcEdges.NumElements());
		AssertSize(2, cMeshEditor.mcPolygons.mcPolygons.NumElements());
	}
	ObjectsKill(false);
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshPolygonAdjacency(void)
{
	DataIOInit();
	ObjectsInit();
	{
		CMeshEditor			cMeshEditor;
		CMesh				cMesh;
		CMeshPlaneEditor* pcPlane;
		CArrayInt			aiAdjacent;

		cMesh.Init();
		cMeshEditor.Init(&cMesh);

		pcPlane = cMeshEditor.AddModifier<CMeshPlaneEditor>();
		SFloat32Vec3	sXDirection(3, 0, 0);
		SFloat32Vec3	sYDirection(0, 3, 0);
		SFloat32Vec3	sStart(0, 0, 0);
		pcPlane->Init(&sXDirection, &sYDirection, &sStart, 3, 3);

		cMeshEditor.Collapse();

		aiAdjacent.Init();
		cMeshEditor.mcPolygons.GetAdjacentPolygons(cMesh.GetConnectivity(), 4, &aiAdjacent);

		AssertSize(4, aiAdjacent.NumElements());
		AssertInt(1, aiAdjacent.GetValue(0));
		AssertInt(3, aiAdjacent.GetValue(1));
		AssertInt(5, aiAdjacent.GetValue(2));
		AssertInt(7, aiAdjacent.GetValue(3));

		aiAdjacent.Kill();

		aiAdjacent.Init();
		cMeshEditor.mcPolygons.GetAdjacentPolygons(cMesh.GetConnectivity(), 0, &aiAdjacent);

		AssertSize(2, aiAdjacent.NumElements());
		AssertInt(1, aiAdjacent.GetValue(0));
		AssertInt(3, aiAdjacent.GetValue(1));

		aiAdjacent.Kill();
	}
	ObjectsKill(false);
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshPolygons(void)
{
	BeginTests();

	TestMeshPolygonsAddFace();
	TestMeshPolygonsAddDiscontiguousFace(true);
	TestMeshPolygonsAddDiscontiguousFace(false);
	TestMeshPolygonAdjacency();

	TestStatistics();
}

