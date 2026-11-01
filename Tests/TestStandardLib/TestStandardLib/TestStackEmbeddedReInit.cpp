#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "ObjectTestSetup.h"
#include "ObjectTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackEmbeddedReInitOnStack(void)
{
	ObjectsInit();

	{
		CEmbeddedContainer	cContainer;
		CEmbeddedTest*		pcOne;
		CEmbeddedTest*		pcTwo;

		pcOne = &cContainer.mcOne;
		pcTwo = &cContainer.mcTwo;

		cContainer.Init();
		AssertSize(0, gcObjects.NumMemoryIndexes());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		AssertSize(3, cContainer.CountNumEmbedded());
		cContainer.Kill();
		AssertSize(0, pcOne->miAmANumber);
		AssertSize(0, pcTwo->miAmANumber);
		AssertSize(0, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		AssertSize(3, cContainer.CountNumEmbedded());
		AssertSize(0, gcObjects.NumMemoryIndexes());

		cContainer.Init();
		AssertSize(0, gcObjects.NumMemoryIndexes());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		AssertSize(3, cContainer.CountNumEmbedded());
		cContainer.Kill();
		AssertSize(0, pcOne->miAmANumber);
		AssertSize(0, pcTwo->miAmANumber);
		AssertSize(0, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		AssertSize(3, cContainer.CountNumEmbedded());
		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackEmbeddedReInitOnStackPointedTo(void)
{
	ObjectsInit();

	{
		CEmbeddedContainer	cContainer;
		CEmbeddedTest*		pcOne;
		CEmbeddedTest*		pcTwo;
		CTestObject			cTest;
		Ptr<CTestObject>	pTest;

		cContainer.Init();
		pcOne = &cContainer.mcOne;
		pcTwo = &cContainer.mcTwo;

		pTest = OMalloc<CTestObject>();
		pTest->mpObject = &cContainer;
		cTest.Init();
		cTest.mpObject = &cContainer;
		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNotNull());
		AssertTrue(cTest.mpObject.IsNotNull());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		cContainer.Kill();
		AssertSize(0, pcOne->miAmANumber);
		AssertSize(0, pcTwo->miAmANumber);
		AssertSize(0, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNull());
		AssertTrue(cTest.mpObject.IsNull());

		cContainer.Init();
		pcOne = &cContainer.mcOne;
		pcTwo = &cContainer.mcTwo;
		pTest->mpObject = &cContainer;
		cTest.mpObject = &cContainer;
		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNotNull());
		AssertTrue(cTest.mpObject.IsNotNull());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		cContainer.Kill();
		AssertSize(0, pcOne->miAmANumber);
		AssertSize(0, pcTwo->miAmANumber);
		AssertSize(0, cContainer.mi);
		AssertSize(3, cContainer.NumEmbedded());
		AssertSize(1, gcObjects.NumMemoryIndexes());
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackEmbeddedReInitOnHeap(void)
{
	ObjectsInit();

	{
		Ptr<CEmbeddedContainer>		pContainer;
		CEmbeddedTest*				pcOne;
		CEmbeddedTest*				pcTwo;

		pContainer = OMalloc<CEmbeddedContainer>();
		pcOne = &pContainer->mcOne;
		pcTwo = &pContainer->mcTwo;

		AssertSize(1, gcObjects.NumMemoryIndexes());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, pContainer->mi);
		AssertSize(3, pContainer->NumEmbedded());
		pContainer->Kill();
		AssertTrue(pContainer.IsNull());
		AssertSize(0, gcObjects.NumMemoryIndexes());

		pContainer = OMalloc<CEmbeddedContainer>();
		pcOne = &pContainer->mcOne;
		pcTwo = &pContainer->mcTwo;
		AssertSize(1, gcObjects.NumMemoryIndexes());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, pContainer->mi);
		AssertSize(3, pContainer->NumEmbedded());
		pContainer->Kill();
		AssertTrue(pContainer.IsNull());
		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackEmbeddedReInitOnHeapPointedTo(void)
{
	ObjectsInit();

	{
		Ptr<CEmbeddedContainer>	pContainer;
		CEmbeddedTest*			pcOne;
		CEmbeddedTest*			pcTwo;
		CTestObject				cTest;
		Ptr<CTestObject>		pTest;

		pContainer = OMalloc<CEmbeddedContainer>();
		pcOne = &pContainer->mcOne;
		pcTwo = &pContainer->mcTwo;

		pTest = OMalloc<CTestObject>();
		pTest->mpObject = pContainer;
		cTest.Init();
		cTest.mpObject = pContainer;
		AssertSize(2, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNotNull());
		AssertTrue(cTest.mpObject.IsNotNull());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, pContainer->mi);
		AssertSize(3, pContainer->NumEmbedded());
		pContainer->Kill();
		AssertTrue(pContainer.IsNull());
		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNull());
		AssertTrue(cTest.mpObject.IsNull());

		pContainer = OMalloc<CEmbeddedContainer>();
		pcOne = &pContainer->mcOne;
		pcTwo = &pContainer->mcTwo;
		pTest->mpObject = pContainer;
		cTest.mpObject = pContainer;
		AssertSize(2, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNotNull());
		AssertTrue(cTest.mpObject.IsNotNull());

		AssertInt((int32)85, pcOne->miAmANumber);
		AssertInt((int32)85, pcTwo->miAmANumber);
		AssertSize(73, pContainer->mi);
		AssertSize(3, pContainer->NumEmbedded());
		pContainer->Kill();
		AssertTrue(pContainer.IsNull());
		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertTrue(pTest->mpObject.IsNull());
		AssertTrue(cTest.mpObject.IsNull());
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackEmbeddedReInit(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	//These Embeddeds are refering to objects declared on the Stack.
	//I wish I would name things properly.  They're not refering to
	//re-initialisation of objects embedded in other objects.
	TestStackEmbeddedReInitOnStack();
	TestStackEmbeddedReInitOnStackPointedTo();
	TestStackEmbeddedReInitOnHeap();
	TestStackEmbeddedReInitOnHeapPointedTo();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

