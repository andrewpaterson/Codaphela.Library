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
	CMeshCacher			cCacher;
	size				uiVertexArrays;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sStart.Init(0, 0, 0);

	pcPlaneEditor = cMeshEditor.AddModifier<CMeshPlaneEditor>();
	pcPlaneEditor->Init(&sXDirection, &sYDirection, &sStart, 2, 2);

	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CArray"));

	//Touching the mesh caches it.  The type indices only live while it does.
	cMeshEditor.Touch();
	uiVertexArrays = cMesh.mcCache.mcMeshVerticesArrays.NumElements();
	AssertTrue(uiVertexArrays > 0);
	AssertSize(uiVertexArrays, CountMeshMemoryObjects("CMeshVertexArray"));
	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CArray"));

	//While the cacher is alive its array and type indices are objects in memory and killing it frees them.
	cCacher.Init(&cMesh);
	AssertTrue(cCacher.Index());
	AssertSize(1, CountMeshMemoryObjects("CArray"));
	AssertSize(cCacher.mpTypeIndices->NumElements(), CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertTrue(cCacher.mpTypeIndices->NumElements() > 0);
	AssertTrue(cCacher.Vertex());
	cCacher.Kill();
	AssertSize(0, CountMeshMemoryObjects("CMeshTypeIndices"));
	AssertSize(0, CountMeshMemoryObjects("CArray"));

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

