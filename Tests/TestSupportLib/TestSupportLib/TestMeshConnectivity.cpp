#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "TestLib/Assert.h"


void AssertCornerFaces(CMeshCorner* pcCorner, size iNumFaces, ...);
void AssertCornerEdges(CMeshCorner* pcCorner, size iNumEdges, ...);
void AssertEdgeFaces(CMeshEdge* pcEdge, size iFace1, size iFace2);
void AssertEdgeCorners(CMeshEdge* pcEdge, size iCorner1, size iCorner2);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeConnectivity(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sXDirection;
	SFloat32Vec3				sYDirection;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	CMeshPlaneEditor*	pcPlaneEditor;
	CMeshEditor			cMeshEditor;
	CMeshConnectivity*	pcConn;
	size				i;
	CCornerEdgesMap*	pcCornerEdgesMap;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sZDirection.Init(0, 0, 1);
	sStart.Init(-0.5f, -0.5f, -0.5f);

	pcPlaneEditor = cMeshEditor.AddModifier<CMeshPlaneEditor>();
	pcPlaneEditor->Init(&sXDirection, &sYDirection, &sStart, 1, 2);

	cMeshEditor.Collapse();

	pcConn = cMesh.GetConnectivity();
	AssertSize(9, pcConn->NumEdges());
	AssertSize(6, pcConn->NumCorners());
	AssertSize(4, pcConn->mcFaces.NumElements());

	AssertCornerFaces(pcConn->GetCorner(0),  1,  0);
	AssertCornerFaces(pcConn->GetCorner(1),  2,  0, 1);
	AssertCornerFaces(pcConn->GetCorner(2),  3,  0, 1, 2);
	AssertCornerFaces(pcConn->GetCorner(3),  3,  1, 2, 3);
	AssertCornerFaces(pcConn->GetCorner(4),  2,  2, 3);
	AssertCornerFaces(pcConn->GetCorner(5),  1,  3);

	AssertCornerEdges(pcConn->GetCorner(0),  2,  0, 2);
	AssertCornerEdges(pcConn->GetCorner(1),  3,  0, 1, 3);
	AssertCornerEdges(pcConn->GetCorner(2),  4,  1, 2, 4, 6);
	AssertCornerEdges(pcConn->GetCorner(3),  4,  3, 4, 5, 7);
	AssertCornerEdges(pcConn->GetCorner(4),  3,  5, 6, 8);
	AssertCornerEdges(pcConn->GetCorner(5),  2,  7, 8);

	for (i = 0; i < pcConn->NumCorners(); i++)
	{
		pcCornerEdgesMap = pcConn->GetCornerEdgesMap((int)i);
		AssertSize(pcCornerEdgesMap->maiEdgeIndex.NumElements(), pcCornerEdgesMap->maiLargerCornerIndex.NumElements());
	}

	AssertEdgeCorners(pcConn->GetEdge(0), 0, 1);
	AssertEdgeCorners(pcConn->GetEdge(1), 1, 2);
	AssertEdgeCorners(pcConn->GetEdge(2), 0, 2);
	AssertEdgeCorners(pcConn->GetEdge(3), 1, 3);
	AssertEdgeCorners(pcConn->GetEdge(4), 2, 3);
	AssertEdgeCorners(pcConn->GetEdge(5), 3, 4);
	AssertEdgeCorners(pcConn->GetEdge(6), 2, 4);
	AssertEdgeCorners(pcConn->GetEdge(7), 3, 5);
	AssertEdgeCorners(pcConn->GetEdge(8), 4, 5);

	AssertEdgeFaces(pcConn->GetEdge(0), 0, -1);
	AssertEdgeFaces(pcConn->GetEdge(1), 0, 1);
	AssertEdgeFaces(pcConn->GetEdge(2), 0, -1);
	AssertEdgeFaces(pcConn->GetEdge(3), 1, -1);
	AssertEdgeFaces(pcConn->GetEdge(4), 1, 2);
	AssertEdgeFaces(pcConn->GetEdge(5), 2, 3);
	AssertEdgeFaces(pcConn->GetEdge(6), 2, -1);
	AssertEdgeFaces(pcConn->GetEdge(7), 3, -1);
	AssertEdgeFaces(pcConn->GetEdge(8), 3, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCornerFaces(CMeshCorner* pcCorner, size iNumFaces, ...)
{
	va_list		vaMarker;
	size			i;
	size			iFaceNum;

	AssertSize(iNumFaces, pcCorner->maiFace.NumElements());
	if (iNumFaces != pcCorner->maiFace.NumElements())
	{
		return;
	}

	va_start(vaMarker, iNumFaces);
	for (i = 0; i < iNumFaces; i++)
	{
		iFaceNum = (size)va_arg(vaMarker, int);  //The face numbers are passed as int literals.
		AssertSize(iFaceNum, pcCorner->maiFace.GetValue(i));
	}
	va_end(vaMarker);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCornerEdges(CMeshCorner* pcCorner, size iNumEdges, ...)
{
	va_list		vaMarker;
	size			i;
	size			iEdgeNum;

	AssertSize(iNumEdges, pcCorner->maiEdge.NumElements());
	if (iNumEdges != pcCorner->maiEdge.NumElements())
	{
		return;
	}

	va_start(vaMarker, iNumEdges);
	for (i = 0; i < iNumEdges; i++)
	{
		iEdgeNum = (size)va_arg(vaMarker, int);  //The edge numbers are passed as int literals.
		AssertSize(iEdgeNum, pcCorner->maiEdge.GetValue(i));
	}
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertEdgeFaces(CMeshEdge* pcEdge, size iFace1, size iFace2)
{
	AssertSize(iFace1, pcEdge->maiFace[0]);
	AssertSize(iFace2, pcEdge->maiFace[1]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertEdgeCorners(CMeshEdge* pcEdge, size iCorner1, size iCorner2)
{
	AssertSize(iCorner1, pcEdge->maiCorner[0]);
	AssertSize(iCorner2, pcEdge->maiCorner[1]);
}


//////////////////////////////////////////////////////////////////////////
//
// A face's matrix count is the most of any of its three corners and reading it leaves the corners as they were.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshFaceTypesFromSkin(void)
{
	CMesh			cMesh;
	CMeshFace*		pcFace;
	SSkinWeight*	psWeight;
	int				i;

	cMesh.Init();
	cMesh.mcSkin.mbInUse = true;
	for (i = 0; i < 3; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);

	for (i = 0; i < 2; i++)
	{
		psWeight = cMesh.mcSkin.mcSkinVerts.Get(2)->mcWeights.Add();
		psWeight->iMatrix = i;
		psWeight->fWeight = 0.5f;
	}

	cMesh.mcFaceTypes.ClearFaceTypes(1);
	cMesh.mcFaceTypes.GenerateFromSkin(&cMesh);

	pcFace = cMesh.GetFace(0);
	AssertInt(0, pcFace->maiCorner[0]);
	AssertInt(1, pcFace->maiCorner[1]);
	AssertInt(2, pcFace->maiCorner[2]);
	AssertInt(2 << MESH_FACE_MATRIX_SHIFT, (int)cMesh.mcFaceTypes.mcFaces.Get(0)->iFlags1);
}


//////////////////////////////////////////////////////////////////////////
//
// A flipped quad's return says which of the quad's corners are in each of its two faces.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorAddFlippedQuad(void)
{
	CMesh		cMesh;
	CMeshEditor	cMeshEditor;
	CMFRet		r;
	int			i;
	int			iCorner1;
	int			iCorner2;
	int			iCorner3;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	for (i = 0; i < 4; i++)
	{
		cMeshEditor.AddCorner((float32)i, 0, 0);
	}

	r = cMeshEditor.AddQuad(0, 1, 2, 3, 0, true);
	AssertInt(2, r.NumFaces());

	r.Unpack1(&iCorner1, &iCorner2, &iCorner3);
	AssertInt(0, iCorner1);
	AssertInt(3, iCorner2);
	AssertInt(1, iCorner3);

	r.Unpack2(&iCorner1, &iCorner2, &iCorner3);
	AssertInt(1, iCorner1);
	AssertInt(3, iCorner2);
	AssertInt(2, iCorner3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivitySaveLoad(void)
{
	CMesh				cMesh;
	CMemoryFile			cMemory;
	CObjectWriter		cWriter;
	CObjectReader		cReader;
	CMeshConnectivity	cLoaded;
	int					i;

	cMesh.Init();
	for (i = 0; i < 4; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(2, 1, 3);

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);
	AssertTrue(cMesh.GetConnectivity()->Save(&cWriter));
	cWriter.Kill();

	cMemory.Seek(0, EFSO_SET);
	cReader.Init(&cMemory, NULL);
	AssertTrue(cLoaded.Load(&cReader));
	cReader.Kill();
	cMemory.Close();
	cMemory.Kill();

	AssertFalse(cLoaded.IsGenerated());
	AssertSize(4, cLoaded.NumCorners());
	AssertSize(2, cLoaded.NumFaces());
	AssertInt(3, cLoaded.GetFaceCorners(1)[2]);
	AssertFalse(cLoaded.IsGenerated());

	AssertSize(5, cLoaded.NumEdges());
	AssertTrue(cLoaded.IsGenerated());
	AssertCornerFaces(cLoaded.GetCorner(1),  2,  0, 1);
	AssertCornerEdges(cLoaded.GetCorner(1),  3,  0, 1, 3);
	AssertEdgeFaces(cLoaded.GetEdge(1), 0, 1);

	cLoaded.AddFace(0, 3, 1);
	AssertSize(6, cLoaded.NumEdges());
	AssertCornerFaces(cLoaded.GetCorner(3),  2,  1, 2);

	cLoaded.Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Adding only records, and reading a face's corners or counting corners and faces doesn't need anything generated.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivityGeneratedWhenNeeded(void)
{
	CMesh				cMesh;
	CMeshConnectivity*	pcConn;
	int					i;

	cMesh.Init();
	pcConn = cMesh.GetConnectivity();
	AssertTrue(pcConn->IsGenerated());

	for (i = 0; i < 4; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);

	AssertFalse(pcConn->IsGenerated());
	AssertInt(4, cMesh.NumCorners());
	AssertInt(1, cMesh.NumFaces());
	AssertInt(2, cMesh.GetFaceCorners(0)[2]);
	AssertFalse(pcConn->IsGenerated());

	AssertInt(3, cMesh.NumEdges());
	AssertTrue(pcConn->IsGenerated());

	cMesh.AddFace(2, 1, 3);
	AssertFalse(pcConn->IsGenerated());
	AssertCornerFaces(cMesh.GetCorner(1),  2,  0, 1);
	AssertTrue(pcConn->IsGenerated());
	AssertEdgeFaces(cMesh.GetEdge(1), 0, 1);
	AssertInt(1, cMesh.GetFace(1)->maiEdge[0]);

	cMesh.ClearConnectivity();
	AssertTrue(pcConn->IsGenerated());
	AssertInt(0, cMesh.NumCorners());
	AssertInt(0, cMesh.NumFaces());
	AssertInt(0, cMesh.NumEdges());

	cMesh.AddCorner(0, 0, 0);
	cMesh.AddCorner(1, 0, 0);
	cMesh.AddCorner(2, 0, 0);
	cMesh.AddFace(0, 1, 2);
	AssertInt(3, cMesh.NumEdges());
}


//////////////////////////////////////////////////////////////////////////
//
// Generating after each face gives the same connectivity as generating once after them all.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivityGeneratedInSteps(void)
{
	CMesh	cSteps;
	CMesh	cOnce;
	int		i;
	int		j;
	int		iFaces[4][3] = {{0, 1, 2}, {2, 1, 3}, {2, 3, 4}, {0, 2, 4}};

	cSteps.Init();
	cOnce.Init();
	for (i = 0; i < 5; i++)
	{
		cSteps.AddCorner((float32)i, 0, 0);
		cOnce.AddCorner((float32)i, 0, 0);
	}
	for (i = 0; i < 4; i++)
	{
		cSteps.AddFace(iFaces[i][0], iFaces[i][1], iFaces[i][2]);
		cOnce.AddFace(iFaces[i][0], iFaces[i][1], iFaces[i][2]);
		cSteps.NumEdges();
	}

	AssertInt(cOnce.NumEdges(), cSteps.NumEdges());
	for (i = 0; i < cOnce.NumEdges(); i++)
	{
		AssertEdgeCorners(cSteps.GetEdge(i), cOnce.GetEdge(i)->maiCorner[0], cOnce.GetEdge(i)->maiCorner[1]);
		AssertEdgeFaces(cSteps.GetEdge(i), cOnce.GetEdge(i)->maiFace[0], cOnce.GetEdge(i)->maiFace[1]);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < NUM_FACE_EDGES; j++)
		{
			AssertInt(cOnce.GetFace(i)->maiEdge[j], cSteps.GetFace(i)->maiEdge[j]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The editor still says how many edges each face added, and faces added directly to the mesh aren't counted as its own.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorNewEdges(void)
{
	CMesh		cMesh;
	CMeshEditor	cMeshEditor;
	int			i;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	for (i = 0; i < 5; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);

	AssertInt(2, cMeshEditor.AddFace(2, 1, 3).NewEdges());
	AssertInt(2, cMeshEditor.AddFace(3, 1, 4).NewEdges());
	AssertInt(7, cMesh.NumEdges());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivity(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshEdgeConnectivity();
	TestMeshFaceTypesFromSkin();
	TestMeshEditorAddFlippedQuad();
	TestMeshConnectivitySaveLoad();
	TestMeshConnectivityGeneratedWhenNeeded();
	TestMeshConnectivityGeneratedInSteps();
	TestMeshEditorNewEdges();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

