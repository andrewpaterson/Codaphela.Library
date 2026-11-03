#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "SupportLib/MeshCacher.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountMeshMemoryObjects(const char* szClassName)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	CPointer			p;
	size				uiCount;

	uiCount = 0;
	oi = gcObjects.StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		p = gcObjects.TestGetFromMemory(oi);
		if (StringCompare(p->ClassName(), szClassName) == 0)
		{
			uiCount++;
		}
		oi = gcObjects.IterateMemory(&sIter);
	}
	sIter.Kill();
	p = NULL;
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshCacheFreesTypeIndices(void)
{
	CMesh				cMesh;
	SFloat32Vec3		sXDirection;
	SFloat32Vec3		sYDirection;
	SFloat32Vec3		sStart;
	CMeshPlaneEditor*	pcPlaneEditor;
	CMeshEditor			cMeshEditor;
	Ptr<CMeshCacher>	pCacher;
	size				uiVertexArrays;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sStart.Init(0, 0, 0);

	pcPlaneEditor = cMeshEditor.AddModifier<CMeshPlaneEditor>();
	pcPlaneEditor->Init(&sXDirection, &sYDirection, &sStart, 2, 2);

	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CMeshCacher"));

	//Touching the mesh caches it.  The type indices only live while it does.
	cMeshEditor.Touch();
	uiVertexArrays = cMesh.mcCache.mcMeshVerticesArrays.NumElements();
	AssertTrue(uiVertexArrays > 0);
	AssertSize(uiVertexArrays, CountMeshMemoryObjects("CMeshVertexArray"));
	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CMeshCacher"));

	//While the cacher is alive it and its type indices are objects in memory and letting go of it frees them.
	pCacher = OMalloc<CMeshCacher>(&cMesh);
	AssertTrue(pCacher->Index());
	AssertSize(1, CountMeshMemoryObjects("CMeshCacher"));
	AssertSize(pCacher->maTypeIndices.NumElements(), CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertTrue(pCacher->maTypeIndices.NumElements() > 0);
	AssertTrue(pCacher->Index());
	AssertSize(pCacher->maTypeIndices.NumElements(), CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertTrue(pCacher->Vertex());
	pCacher = NULL;
	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CMeshCacher"));

	cMeshEditor.Kill();
	cMesh.Kill();
	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CMeshVertexArray"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshCache(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestMeshCacheFreesTypeIndices();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

