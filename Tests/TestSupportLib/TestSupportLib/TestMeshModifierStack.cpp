#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshModifierStack.h"
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


//Returns a new mesh of the one corner it is given, moved.
class CMeshTestMoveModifier : public CMeshModifier
{
CONSTRUCTABLE(CMeshTestMoveModifier);
public:
	float32	mfScale;
	float32	mfAdd;
	int		miCalls;

	void Init(float32 fScale, float32 fAdd)
	{
		mfScale = fScale;
		mfAdd = fAdd;
		miCalls = 0;
	}

	Ptr<CMesh> Modify(Ptr<CMesh> pMesh) override
	{
		Ptr<CMesh>	pOutput;

		miCalls++;
		pOutput = OMalloc<CMesh>();
		pOutput->AddCorner(pMesh->GetPosition(0)->x * mfScale + mfAdd, 0, 0);
		return pOutput;
	}
};


//Finds nothing to do.
class CMeshTestSameModifier : public CMeshModifier
{
CONSTRUCTABLE(CMeshTestSameModifier);
public:
	Ptr<CMesh> Modify(Ptr<CMesh> pMesh) override
	{
		return pMesh;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshOneCorner(float32 x)
{
	Ptr<CMesh>	pMesh;

	pMesh = OMalloc<CMesh>();
	pMesh->AddCorner(x, 0, 0);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// Each modifier gets the one before's output, the source stays as it was and only the source and the output are left.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshModifierStackOutput(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pSource;
	Ptr<CMesh>				pOutput;
	CMeshTestMoveModifier*	pcDouble;
	CMeshTestMoveModifier*	pcAdd;

	pSource = TestMeshOneCorner(3);
	cStack.Init(pSource);
	pcDouble = cStack.AddModifier<CMeshTestMoveModifier>();
	pcDouble->Init(2, 0);
	pcAdd = cStack.AddModifier<CMeshTestMoveModifier>();
	pcAdd->Init(1, 1);

	pOutput = cStack.GetOutput();
	AssertFloat(7, pOutput->GetPosition(0)->x, 3);
	AssertFloat(3, pSource->GetPosition(0)->x, 3);
	AssertTrue(cStack.GetSource() == pSource);
	AssertSize(2, CountMeshMemoryObjects("CMesh"));

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The output is kept until a modifier is added or the stack is touched, and each time it starts from the source.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshModifierStackKeepsOutput(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pOutput;
	CMeshTestMoveModifier*	pcAdd;
	CMeshTestMoveModifier*	pcDouble;

	cStack.Init(TestMeshOneCorner(3));
	pcAdd = cStack.AddModifier<CMeshTestMoveModifier>();
	pcAdd->Init(1, 1);

	pOutput = cStack.GetOutput();
	AssertTrue(cStack.GetOutput() == pOutput);
	AssertInt(1, pcAdd->miCalls);

	cStack.Touch();
	AssertFloat(4, cStack.GetOutput()->GetPosition(0)->x, 3);
	AssertFalse(cStack.GetOutput() == pOutput);
	AssertInt(2, pcAdd->miCalls);

	pcDouble = cStack.AddModifier<CMeshTestMoveModifier>();
	pcDouble->Init(2, 0);
	AssertFloat(8, cStack.GetOutput()->GetPosition(0)->x, 3);
	AssertInt(3, pcAdd->miCalls);

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A modifier with nothing to do, or no modifiers at all, gives the source itself.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshModifierStackNothingToDo(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;

	pSource = TestMeshOneCorner(3);
	cStack.Init(pSource);
	AssertTrue(cStack.GetOutput() == pSource);

	cStack.AddModifier<CMeshTestSameModifier>();
	AssertTrue(cStack.GetOutput() == pSource);
	AssertSize(1, CountMeshMemoryObjects("CMesh"));

	cStack.Flatten();
	AssertTrue(cStack.GetSource() == pSource);
	AssertInt(0, cStack.NumEditors());

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The output, without working it out again, becomes the source.  The modifiers go, and whoever kept the old source
// still has it as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshModifierStackFlatten(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pOldSource;
	Ptr<CMesh>				pOutput;
	CMeshTestMoveModifier*	pcAdd;

	pOldSource = TestMeshOneCorner(3);
	cStack.Init(pOldSource);
	pcAdd = cStack.AddModifier<CMeshTestMoveModifier>();
	pcAdd->Init(1, 1);
	pOutput = cStack.GetOutput();
	AssertInt(1, pcAdd->miCalls);

	cStack.Flatten();

	AssertTrue(cStack.GetSource() == pOutput);
	AssertTrue(cStack.GetOutput() == pOutput);
	AssertInt(0, cStack.NumEditors());
	AssertFloat(3, pOldSource->GetPosition(0)->x, 3);
	AssertFloat(4, cStack.GetSource()->GetPosition(0)->x, 3);

	pOldSource = NULL;
	pOutput = NULL;
	AssertSize(1, CountMeshMemoryObjects("CMesh"));

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With no modifiers an editor's output is its mesh, and nothing is copied.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorOutputIsMesh(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMesh>			pMesh;

	pMesh = OMalloc<CMesh>();
	pEditor = OMalloc<CMeshEditor>(pMesh);
	AssertTrue(pEditor->GetOutput() == pMesh);
	AssertTrue(pEditor->mcModifiers.GetSource() == pMesh);

	pEditor->AddCorner(3, 0, 0);
	AssertTrue(pEditor->GetOutput() == pMesh);
	AssertInt(1, pEditor->GetOutput()->NumCorners());
	AssertSize(1, CountMeshMemoryObjects("CMesh"));
}


//////////////////////////////////////////////////////////////////////////
//
// The counters say what changed, and the output is made again when one
// has.  CMesh::MovePosition does nothing yet, so only that the output is
// made again is looked at.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorOutputChanges(void)
{
	Ptr<CMeshEditor>		pEditor;
	Ptr<CMesh>				pOutput;
	CMeshTestMoveModifier*	pcAdd;
	SFloat32Vec3			sPosition;
	size					uiStructure;
	size					uiShape;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(3, 0, 0);
	pcAdd = pEditor->AddModifier<CMeshTestMoveModifier>();
	pcAdd->Init(1, 1);

	pOutput = pEditor->GetOutput();
	AssertFloat(4, pOutput->GetPosition(0)->x, 3);
	AssertFloat(3, pEditor->mpMesh->GetPosition(0)->x, 3);
	AssertTrue(pEditor->GetOutput() == pOutput);
	AssertInt(1, pcAdd->miCalls);

	uiStructure = pEditor->NumStructureChanges();
	uiShape = pEditor->NumShapeChanges();
	sPosition.Init(5, 0, 0);
	pEditor->MovePosition(0, &sPosition);
	AssertSize(uiStructure, pEditor->NumStructureChanges());
	AssertSize(uiShape + 1, pEditor->NumShapeChanges());
	pEditor->GetOutput();
	AssertInt(2, pcAdd->miCalls);

	pEditor->AddCorner(1, 1, 1);
	AssertSize(uiStructure + 1, pEditor->NumStructureChanges());
	AssertSize(uiShape + 1, pEditor->NumShapeChanges());
	pOutput = pEditor->GetOutput();
	AssertInt(3, pcAdd->miCalls);
	AssertTrue(pEditor->GetOutput() == pOutput);
	AssertInt(3, pcAdd->miCalls);

	//Let go, it's made again from the same mesh.
	pEditor->ClearOutput();
	pOutput = NULL;
	AssertSize(1, CountMeshMemoryObjects("CMesh"));
	AssertFloat(4, pEditor->GetOutput()->GetPosition(0)->x, 3);
	AssertInt(4, pcAdd->miCalls);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshModifierStack(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshModifierStackOutput();
	ObjectsFlush();
	TestMeshModifierStackKeepsOutput();
	ObjectsFlush();
	TestMeshModifierStackNothingToDo();
	ObjectsFlush();
	TestMeshModifierStackFlatten();
	ObjectsFlush();
	TestMeshEditorOutputIsMesh();
	ObjectsFlush();
	TestMeshEditorOutputChanges();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
