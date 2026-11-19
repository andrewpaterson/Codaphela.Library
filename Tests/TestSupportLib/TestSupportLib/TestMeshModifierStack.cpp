#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshSphereEditor.h"
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
	p = NULL;
	return uiCount;
}


//Returns a new mesh of the one corner it is given, moved.
class CMeshTestMoveModifier : public CMeshModifier
{
CONSTRUCTABLE(CMeshTestMoveModifier);
DESTRUCTABLE(CMeshTestMoveModifier);
public:
	float32	mfScale;
	float32	mfAdd;
	int		miCalls;

	void Init(float32 fScale, float32 fAdd)
	{
		PreInit();
		mfScale = fScale;
		mfAdd = fAdd;
		miCalls = 0;
		PostInit();
	}

	void Class(void) override
	{
		CMeshModifier::Class();
		U_Float32(mfScale);
		U_Float32(mfAdd);
		U_SInt(miCalls);
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
DESTRUCTABLE(CMeshTestSameModifier);
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
	CMeshModifierStack			cStack;
	Ptr<CMesh>					pSource;
	Ptr<CMesh>					pOutput;
	Ptr<CMeshTestMoveModifier>	pDouble;
	Ptr<CMeshTestMoveModifier>	pAdd;

	pSource = TestMeshOneCorner(3);
	cStack.Init(pSource);
	pDouble = cStack.AddModifier<CMeshTestMoveModifier>(2.0f, 0.0f);
	pAdd = cStack.AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);

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
	CMeshModifierStack			cStack;
	Ptr<CMesh>					pOutput;
	Ptr<CMeshTestMoveModifier>	pAdd;
	Ptr<CMeshTestMoveModifier>	pDouble;

	cStack.Init(TestMeshOneCorner(3));
	pAdd = cStack.AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);

	pOutput = cStack.GetOutput();
	AssertTrue(cStack.GetOutput() == pOutput);
	AssertInt(1, pAdd->miCalls);

	cStack.Touch();
	AssertFloat(4, cStack.GetOutput()->GetPosition(0)->x, 3);
	AssertFalse(cStack.GetOutput() == pOutput);
	AssertInt(2, pAdd->miCalls);

	pDouble = cStack.AddModifier<CMeshTestMoveModifier>(2.0f, 0.0f);
	AssertFloat(8, cStack.GetOutput()->GetPosition(0)->x, 3);
	AssertInt(3, pAdd->miCalls);

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
	AssertInt(0, cStack.NumModifiers());

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
	CMeshModifierStack			cStack;
	Ptr<CMesh>					pOldSource;
	Ptr<CMesh>					pOutput;
	Ptr<CMeshTestMoveModifier>	pAdd;

	pOldSource = TestMeshOneCorner(3);
	cStack.Init(pOldSource);
	pAdd = cStack.AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);
	pOutput = cStack.GetOutput();
	AssertInt(1, pAdd->miCalls);

	cStack.Flatten();

	AssertTrue(cStack.GetSource() == pOutput);
	AssertTrue(cStack.GetOutput() == pOutput);
	AssertInt(0, cStack.NumModifiers());
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
	Ptr<CMeshEditor>			pEditor;
	Ptr<CMesh>					pOutput;
	Ptr<CMeshTestMoveModifier>	pAdd;
	SFloat32Vec3				sPosition;
	size						uiStructure;
	size						uiShape;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(3, 0, 0);
	pAdd = pEditor->AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);

	pOutput = pEditor->GetOutput();
	AssertFloat(4, pOutput->GetPosition(0)->x, 3);
	AssertFloat(3, pEditor->mpMesh->GetPosition(0)->x, 3);
	AssertTrue(pEditor->GetOutput() == pOutput);
	AssertInt(1, pAdd->miCalls);

	uiStructure = pEditor->NumStructureChanges();
	uiShape = pEditor->NumShapeChanges();
	sPosition.Init(5, 0, 0);
	pEditor->MovePosition(0, &sPosition);
	AssertSize(uiStructure, pEditor->NumStructureChanges());
	AssertSize(uiShape + 1, pEditor->NumShapeChanges());
	pEditor->GetOutput();
	AssertInt(2, pAdd->miCalls);

	pEditor->AddCorner(1, 1, 1);
	AssertSize(uiStructure + 1, pEditor->NumStructureChanges());
	AssertSize(uiShape + 1, pEditor->NumShapeChanges());
	pOutput = pEditor->GetOutput();
	AssertInt(3, pAdd->miCalls);
	AssertTrue(pEditor->GetOutput() == pOutput);
	AssertInt(3, pAdd->miCalls);

	//Let go, it's made again from the same mesh.
	pEditor->ClearOutput();
	pOutput = NULL;
	AssertSize(1, CountMeshMemoryObjects("CMesh"));
	AssertFloat(4, pEditor->GetOutput()->GetPosition(0)->x, 3);
	AssertInt(4, pAdd->miCalls);
}


//////////////////////////////////////////////////////////////////////////
//
// The modifier counter goes up when what the modifiers do changes, so that
// what uses the output can tell, but not when the output is only let go or
// made again because the mesh changed.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorModifierChanges(void)
{
	Ptr<CMeshEditor>			pEditor;
	Ptr<CMeshTestMoveModifier>	pAdd;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(3, 0, 0);
	AssertSize(0, pEditor->NumModifierChanges());

	pAdd = pEditor->AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);
	AssertSize(1, pEditor->NumModifierChanges());

	pEditor->GetOutput();
	pEditor->ClearOutput();
	pEditor->AddCorner(1, 1, 1);
	pEditor->GetOutput();
	AssertSize(1, pEditor->NumModifierChanges());
	AssertInt(2, pAdd->miCalls);

	pEditor->mcModifiers.Touch();
	AssertSize(2, pEditor->NumModifierChanges());

	pEditor->Collapse();
	AssertSize(3, pEditor->NumModifierChanges());
	AssertInt(0, pEditor->mcModifiers.NumModifiers());
}


//////////////////////////////////////////////////////////////////////////
//
// An output made elsewhere is the editor's until something changes: it
// counts as a change of the modifiers, no modifier is called for it, and
// once it's let go, or the mesh has changed, the editor makes its own.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorSetOutput(void)
{
	Ptr<CMeshEditor>			pEditor;
	Ptr<CMeshTestMoveModifier>	pAdd;
	Ptr<CMesh>					pMade;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(3, 0, 0);
	pAdd = pEditor->AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);
	AssertSize(1, pEditor->NumModifierChanges());

	pMade = TestMeshOneCorner(4);
	pEditor->SetOutput(pMade);
	AssertSize(2, pEditor->NumModifierChanges());
	AssertTrue(pEditor->GetOutput() == pMade);
	AssertTrue(pEditor->GetOutput() == pMade);
	AssertInt(0, pAdd->miCalls);

	//Let go, the editor makes it.
	pEditor->ClearOutput();
	AssertSize(2, pEditor->NumModifierChanges());
	AssertFalse(pEditor->GetOutput() == pMade);
	AssertFloat(4, pEditor->GetOutput()->GetPosition(0)->x, 3);
	AssertInt(1, pAdd->miCalls);

	//Given after the mesh changed, it is of the mesh as it is now; another change of the mesh and it is made again.
	pEditor->AddCorner(1, 1, 1);
	pEditor->SetOutput(pMade);
	AssertTrue(pEditor->GetOutput() == pMade);
	AssertInt(1, pAdd->miCalls);
	pEditor->AddCorner(2, 2, 2);
	AssertFalse(pEditor->GetOutput() == pMade);
	AssertInt(2, pAdd->miCalls);

	//A setting changed after it was given: made again.
	pEditor->SetOutput(pMade);
	pEditor->mcModifiers.Touch();
	AssertFalse(pEditor->GetOutput() == pMade);
	AssertInt(3, pAdd->miCalls);
}


//////////////////////////////////////////////////////////////////////////
//
// Scaling moves every position and is a change of shape, so the output is
// made again from the scaled mesh.  Normals stay as they were.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorScale(void)
{
	Ptr<CMeshEditor>			pEditor;
	Ptr<CMeshTestMoveModifier>	pAdd;
	SFloat32Vec3*				psPosition;
	SFloat32Vec3				sNormal;
	size						uiStructure;
	size						uiShape;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->mpMesh->mcNormals.mbInUse = true;
	pEditor->AddCorner(1, 2, 3);
	pEditor->AddCorner(4, 0, 0);
	pEditor->AddCorner(0, 4, 0);
	pEditor->AddFace(0, 1, 2);
	pAdd = pEditor->AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);
	pEditor->GetOutput();
	AssertInt(1, pAdd->miCalls);

	AssertLSize(1, pEditor->mpMesh->mcNormals.mcNormals.NumElements());
	sNormal = *pEditor->mpMesh->mcNormals.mcNormals.Get(0);

	uiStructure = pEditor->NumStructureChanges();
	uiShape = pEditor->NumShapeChanges();
	pEditor->Scale(1000.0f);
	AssertSize(uiStructure, pEditor->NumStructureChanges());
	AssertSize(uiShape + 1, pEditor->NumShapeChanges());

	psPosition = pEditor->mpMesh->GetPosition(0);
	AssertFloat(1000, psPosition->x, 3);
	AssertFloat(2000, psPosition->y, 3);
	AssertFloat(3000, psPosition->z, 3);
	AssertFloat(4000, pEditor->mpMesh->GetPosition(1)->x, 3);
	AssertFloat(sNormal.x, pEditor->mpMesh->mcNormals.mcNormals.Get(0)->x, 5);
	AssertFloat(sNormal.y, pEditor->mpMesh->mcNormals.mcNormals.Get(0)->y, 5);
	AssertFloat(sNormal.z, pEditor->mpMesh->mcNormals.mcNormals.Get(0)->z, 5);

	AssertFloat(1001, pEditor->GetOutput()->GetPosition(0)->x, 3);
	AssertInt(2, pAdd->miCalls);
}


//////////////////////////////////////////////////////////////////////////
//
// A modifier is an object its stack holds: GetModifier gives the one
// AddModifier made, a setting changed through it and Touch make the output
// again, and the modifiers go when their editor does.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorModifiersAreObjects(void)
{
	Ptr<CMeshEditor>			pEditor;
	Ptr<CMeshTestMoveModifier>	pAdd;
	SFloat32Vec3				sAxis;
	SFloat32Vec3				sCentre;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(3, 0, 0);
	pAdd = pEditor->AddModifier<CMeshTestMoveModifier>(1.0f, 1.0f);
	AssertInt(1, pEditor->mcModifiers.NumModifiers());
	AssertTrue(&pEditor->mcModifiers.GetModifier<CMeshTestMoveModifier>() == &pAdd);
	AssertNull(&pEditor->mcModifiers.GetModifier<CMeshTestSameModifier>());
	AssertFloat(4, pEditor->GetOutput()->GetPosition(0)->x, 3);

	pAdd->mfAdd = 10;
	pEditor->mcModifiers.Touch();
	AssertFloat(13, pEditor->GetOutput()->GetPosition(0)->x, 3);

	sAxis.Init(0, 0, 1);
	sCentre.Init(0, 0, 0);
	pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 1.0f, 4, 3);
	AssertInt(2, pEditor->mcModifiers.NumModifiers());
	AssertSize(1, CountMeshMemoryObjects("CMeshSphereEditor"));
	AssertSize(1, CountMeshMemoryObjects("CMeshTestMoveModifier"));

	pAdd = NULL;
	pEditor = NULL;
	AssertSize(0, CountMeshMemoryObjects("CMeshSphereEditor"));
	AssertSize(0, CountMeshMemoryObjects("CMeshTestMoveModifier"));
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
	ObjectsFlush();
	TestMeshEditorModifierChanges();
	ObjectsFlush();
	TestMeshEditorSetOutput();
	ObjectsFlush();
	TestMeshEditorScale();
	ObjectsFlush();
	TestMeshEditorModifiersAreObjects();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
