#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/DataOrderers.h"
#include "TestLib/Assert.h"
#include "TestIndexObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectAddConstructors(void)
{
	gcObjects.AddConstructor<CTestSaveableObject1>();
	gcObjects.AddConstructor<CTestObject>();
	gcObjects.AddConstructor<CPointerContainer>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectPut(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pIndex->Put("shnork", OMalloc<CTestObject>(&sNotifier1));
		pIndex->Put("shnerp", OMalloc<CTestObject>(&sNotifier2));
		pIndex->Put("booger", OMalloc<CTestObject>(&sNotifier3));

		AssertSize(3, pIndex->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		pIndex = NULL;
		AssertSize(6, gcObjects.NumMemoryIndexes());

		pIndex = pRoot->GetFirst<CIndexObject>();
		pTest1 = pIndex->Get("booger");
		AssertPointer(&sNotifier3, pTest1->mpsFreedNotifier);
		pTest2 = pIndex->Get("shnerp");
		AssertPointer(&sNotifier2, pTest2->mpsFreedNotifier);
		pTest3 = pIndex->Get("shnork");
		AssertPointer(&sNotifier1, pTest3->mpsFreedNotifier);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectDetachInHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pIndex->Put("shnork", OMalloc<CTestObject>(&sNotifier1));
		pIndex->Put("shnerp", OMalloc<CTestObject>(&sNotifier2));
		pIndex->Put("booger", OMalloc<CTestObject>(&sNotifier3));
		AssertFalse(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);

		AssertSize(3, pIndex->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());


		pRoot->RemoveAll();
		pIndex = NULL;
		AssertSize(2, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertTrue(sNotifier3.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectIterateSafeNull(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		SIndexTreeMemoryIterator	sIter;
		uint8						auiKey[8];
		size						uiKeySize;
		bool						bExists;
		CPointer					pPtr;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pTest3 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest1 = OMalloc<CTestObject>(&sNotifier3);
		pIndex->Put("shnork", pTest3);
		pIndex->Put("shniqu", Null<CObject>());
		pIndex->Put("shnerp", pTest2);
		pIndex->Put("booger", pTest1);
		pIndex->Put("bozzul", Null<CObject>());

		AssertSize(5, pIndex->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		memset(auiKey, 0, 8);
		pPtr = pIndex->StartIteration(&sIter, auiKey, &uiKeySize, 8, &bExists);
		AssertTrue(bExists);
		AssertString("booger", auiKey);
		AssertPointer(&pTest1, &pPtr);

		memset(auiKey, 0, 8);
		pPtr = pIndex->Iterate(&sIter, auiKey, &uiKeySize, 8, &bExists);
		AssertTrue(bExists);
		AssertString("bozzul", auiKey);
		AssertNull(&pPtr);

		memset(auiKey, 0, 8);
		pPtr = pIndex->Iterate(&sIter, auiKey, &uiKeySize, 8, &bExists);
		AssertTrue(bExists);
		AssertString("shnerp", auiKey);
		AssertPointer(&pTest2, &pPtr);

		memset(auiKey, 0, 8);
		pPtr = pIndex->Iterate(&sIter, auiKey, &uiKeySize, 8, &bExists);
		AssertTrue(bExists);
		AssertString("shniqu", auiKey);
		AssertNull(&pPtr);

		memset(auiKey, 0, 8);
		pPtr = pIndex->Iterate(&sIter, auiKey, &uiKeySize, 8, &bExists);
		AssertTrue(bExists);
		AssertString("shnork", auiKey);
		AssertPointer(&pTest3, &pPtr);

		memset(auiKey, 0, 8);
		pPtr = pIndex->Iterate(&sIter, auiKey, &uiKeySize, 8, &bExists);
		sIter.Kill();
		AssertFalse(bExists);
		AssertNull(&pPtr);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectIterateUnsafeNull(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>						pRoot;
		Ptr<CIndexObject>				pIndex;
		Ptr<CTestObject>				pTest1;
		Ptr<CTestObject>				pTest2;
		Ptr<CTestObject>				pTest3;
		SIndexTreeMemoryUnsafeIterator	sIter;
		bool							bExists;
		CObject*						pcObject;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pTest3 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest1 = OMalloc<CTestObject>(&sNotifier3);
		pIndex->Put("shnork", pTest3);
		pIndex->Put("shniqu", Null<CObject>());
		pIndex->Put("shnerp", pTest2);
		pIndex->Put("booger", pTest1);
		pIndex->Put("bozzul", Null<CObject>());

		AssertSize(5, pIndex->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		bExists = pIndex->GetIndexForTesting()->StartIteration(&sIter, (CUnknown**)&pcObject);
		AssertTrue(bExists);
		AssertPointer(&pTest1, pcObject);

		bExists = pIndex->GetIndexForTesting()->Iterate(&sIter, (CUnknown**)&pcObject);
		AssertTrue(bExists);
		AssertNull(pcObject);

		bExists = pIndex->GetIndexForTesting()->Iterate(&sIter, (CUnknown**)&pcObject);
		AssertTrue(bExists);
		AssertPointer(&pTest2, pcObject);

		bExists = pIndex->GetIndexForTesting()->Iterate(&sIter, (CUnknown**)&pcObject);
		AssertTrue(bExists);
		AssertNull(pcObject);

		bExists = pIndex->GetIndexForTesting()->Iterate(&sIter, (CUnknown**)&pcObject);
		AssertTrue(bExists);
		AssertPointer(&pTest3, pcObject);

		bExists = pIndex->GetIndexForTesting()->Iterate(&sIter, (CUnknown**)&pcObject);
		AssertFalse(bExists);
		AssertNull(pcObject);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectRemove(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pIndex->Put("shnork", OMalloc<CTestObject>(&sNotifier3));
		pIndex->Put("shnerp", OMalloc<CTestObject>(&sNotifier2));
		pIndex->Put("booger", OMalloc<CTestObject>(&sNotifier1));
		AssertSize(3, pIndex->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		pIndex->Remove("shnerp");
		AssertSize(2, pIndex->NumElements());
		AssertSize(5, gcObjects.NumMemoryIndexes());
		AssertFalse(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);

		pTest1 = pIndex->Get("booger");
		pIndex->Remove("booger");
		AssertSize(1, pIndex->NumElements());
		AssertSize(5, gcObjects.NumMemoryIndexes());
		AssertFalse(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectDetachOnStack(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		bool							bExists;
		SIndexTreeMemoryUnsafeIterator	sIter;
		CTestObject*					pcObject;

		{
			CIndexObject			cIndex;

			cIndex.Init();
			cIndex.Put("shnork", OMalloc<CTestObject>(&sNotifier1));
			cIndex.Put("shnerp", OMalloc<CTestObject>(&sNotifier2));
			cIndex.Put("booger", OMalloc<CTestObject>(&sNotifier3));
			AssertFalse(sNotifier1.bFreed);
			AssertFalse(sNotifier2.bFreed);
			AssertFalse(sNotifier3.bFreed);
			AssertSize(3, cIndex.NumElements());
			AssertSize(3, gcObjects.NumMemoryIndexes());

			bExists = cIndex.GetIndexForTesting()->StartIteration(&sIter, (CUnknown**)&pcObject);
			while (bExists)
			{
				AssertIntHex(0x7070707, pcObject->mi);
				bExists = cIndex.GetIndexForTesting()->Iterate(&sIter, (CUnknown**)&pcObject);
			}
		}

		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertTrue(sNotifier3.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectGetPointerTos(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		CArrayEmbeddedObjectPtr		apcTos;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pTest3 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest1 = OMalloc<CTestObject>(&sNotifier3);
		pIndex->Put("shnork", pTest3);
		pIndex->Put("shniqu", Null<CObject>());
		pIndex->Put("shnerp", pTest2);
		pIndex->Put("booger", pTest1);
		pIndex->Put("bozzul", Null<CObject>());

		AssertSize(5, pIndex->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		apcTos.Init();
		pIndex->GetPointerTos(&apcTos);
		AssertSize(3, apcTos.NumElements());
		AssertTrue(apcTos.ContainsVoidPtr(&pTest1));
		AssertTrue(apcTos.ContainsVoidPtr(&pTest2));
		AssertTrue(apcTos.ContainsVoidPtr(&pTest3));
		apcTos.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectPointerFromsHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pTest3 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest1 = OMalloc<CTestObject>(&sNotifier3);
		pIndex->Put("booger", pTest1);
		pIndex->Put("shnerp", pTest2);
		pIndex->Put("shnork", pTest3);
		AssertSize(1, pTest1->NumHeapFroms());
		AssertSize(1, pTest2->NumHeapFroms());
		AssertSize(1, pTest3->NumHeapFroms());
		AssertSize(3, pIndex->NumElements());

		pIndex->Remove("booger");
		AssertSize(0, pTest1->NumHeapFroms());
		AssertSize(1, pTest2->NumHeapFroms());
		AssertSize(1, pTest3->NumHeapFroms());
		AssertSize(2, pIndex->NumElements());

		pIndex->RemoveAll();
		AssertSize(0, pTest1->NumHeapFroms());
		AssertSize(0, pTest2->NumHeapFroms());
		AssertSize(0, pTest3->NumHeapFroms());
		AssertSize(0, pIndex->NumElements());

		pIndex->Put("abcde", pTest1);
		AssertSize(1, pTest1->NumHeapFroms());
		AssertSize(0, pTest2->NumHeapFroms());
		AssertSize(0, pTest3->NumHeapFroms());
		AssertSize(1, pIndex->NumElements());

		pIndex->Put("abcdf", pTest1);
		AssertSize(2, pTest1->NumHeapFroms());
		AssertSize(0, pTest2->NumHeapFroms());
		AssertSize(0, pTest3->NumHeapFroms());
		AssertSize(2, pIndex->NumElements());

		pIndex->Remove("abcde");
		AssertSize(1, pTest1->NumHeapFroms());
		AssertSize(0, pTest2->NumHeapFroms());
		AssertSize(0, pTest3->NumHeapFroms());
		AssertSize(1, pIndex->NumElements());

		pIndex->Remove("abcdf");
		AssertSize(0, pTest1->NumHeapFroms());
		AssertSize(0, pTest2->NumHeapFroms());
		AssertSize(0, pTest3->NumHeapFroms());
		AssertSize(0, pIndex->NumElements());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectPointerFromsStack(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		CIndexObject				cIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;

		gcObjects.ValidateObjectsConsistency();

		cIndex.Init();
		pTest3 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest1 = OMalloc<CTestObject>(&sNotifier3);
		cIndex.Put("booger", pTest1);
		cIndex.Put("shnerp", pTest2);
		cIndex.Put("shnork", pTest3);
		AssertSize(2, pTest1->NumStackFroms());
		AssertSize(2, pTest2->NumStackFroms());
		AssertSize(2, pTest3->NumStackFroms());
		AssertSize(3, cIndex.NumElements());

		cIndex.Remove("booger");
		AssertSize(1, pTest1->NumStackFroms());
		AssertSize(2, pTest2->NumStackFroms());
		AssertSize(2, pTest3->NumStackFroms());
		AssertSize(2, cIndex.NumElements());

		cIndex.RemoveAll();
		AssertSize(1, pTest1->NumStackFroms());
		AssertSize(1, pTest2->NumStackFroms());
		AssertSize(1, pTest3->NumStackFroms());
		AssertSize(0, cIndex.NumElements());

		cIndex.Put("abcde", pTest1);
		AssertSize(2, pTest1->NumStackFroms());
		AssertSize(1, pTest2->NumStackFroms());
		AssertSize(1, pTest3->NumStackFroms());
		AssertSize(1, cIndex.NumElements());

		cIndex.Put("abcdf", pTest1);
		AssertSize(3, pTest1->NumStackFroms());
		AssertSize(1, pTest2->NumStackFroms());
		AssertSize(1, pTest3->NumStackFroms());
		AssertSize(2, cIndex.NumElements());

		cIndex.Remove("abcde");
		AssertSize(2, pTest1->NumStackFroms());
		AssertSize(1, pTest2->NumStackFroms());
		AssertSize(1, pTest3->NumStackFroms());
		AssertSize(1, cIndex.NumElements());

		cIndex.Remove("abcdf");
		AssertSize(1, pTest1->NumStackFroms());
		AssertSize(1, pTest2->NumStackFroms());
		AssertSize(1, pTest3->NumStackFroms());
		AssertSize(0, cIndex.NumElements());

		pTest1 = NULL;
		pTest2 = NULL;
		pTest3 = NULL;
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectOnStackKillContained(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		CIndexObject		cIndex;
		Ptr<CTestObject>	pTest1;
		Ptr<CTestObject>	pTest2;
		CPointer			pGot;

		cIndex.Init();
		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		cIndex.Put("one", pTest1);
		cIndex.Put("two", pTest2);
		AssertSize(2, cIndex.NumElements());
		AssertSize(4, gcStackPointers.NumElements());

		pTest1->Kill();
		AssertTrue(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertSize(1, cIndex.NumElements());
		pGot = cIndex.Get("one");
		AssertTrue(pGot.IsNull());
		pGot = cIndex.Get("two");
		AssertPointer(pTest2.Object(), pGot.Object());
		pGot = NULL;
		AssertSize(2, gcStackPointers.NumElements());

		cIndex.Kill();
		AssertSize(1, gcStackPointers.NumElements());
		AssertFalse(sNotifier2.bFreed);
	}
	AssertTrue(sNotifier2.bFreed);
	AssertSize(0, gcStackPointers.NumElements());
	AssertLong(0, gcObjects.NumMemoryIndexes());
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectKillContainedHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CIndexObject>	pIndex;
		Ptr<CTestObject>	pTest1;
		Ptr<CTestObject>	pTest2;
		CPointer			pGot;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);
		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pIndex->Put("one", pTest1);
		pIndex->Put("two", pTest2);
		AssertSize(2, pIndex->NumElements());

		pTest1->Kill();
		AssertTrue(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertSize(1, pIndex->NumElements());
		pGot = pIndex->Get("one");
		AssertTrue(pGot.IsNull());
		pGot = pIndex->Get("two");
		AssertPointer(pTest2.Object(), pGot.Object());
		pGot = NULL;
		AssertSize(1, pTest2->NumHeapFroms());
		gcObjects.ValidateObjectsConsistency();

		pTest2 = NULL;
		pIndex = NULL;
		AssertFalse(sNotifier2.bFreed);
		AssertTrue(pRoot->RemoveAll());
		AssertTrue(sNotifier2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectPutOverwrite(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		bool						bResult;

		pRoot = ORoot();
		pIndex = OMalloc<CIndexObject>();
		pRoot->Add(pIndex);

		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		bResult = pIndex->Put("1", pTest2);
		gcObjects.ValidateObjectsConsistency();
		AssertTrue(bResult);
		AssertSize(1, pIndex->NumElements());
		AssertSize(5, gcObjects.NumMemoryIndexes());
		pTest2 = NULL;
		AssertFalse(sNotifier2.bFreed);

		bResult = pIndex->Put("1", pTest1);
		gcObjects.ValidateObjectsConsistency();
		AssertTrue(bResult);
		AssertSize(1, pIndex->NumElements());
		AssertSize(4, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectSerialisation()
{
	CFileUtil	cFileUtil;
	char		szDirectory[] = "Output" _FS_ "TestIndexObject";

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	DataOrderersInit();
	ObjectsInit();
	TestIndexObjectAddConstructors();
	{
		Ptr<CPointerContainer>			pContainer1;
		Ptr<CPointerContainer>			pContainer2;
		Ptr<CPointerContainer>			pContainer3;
		Ptr<CTestObject>				pObject;
		Ptr<CIndexObject>				pIndexObject;
		CExternalObjectSerialiser		cSerialiser;
		CChunkFileObjectWriter			cWriter;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		pObject = OMalloc<CTestObject>();
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pContainer3 = OMalloc<CPointerContainer>(Null());
		pIndexObject = ONMalloc<CIndexObject>("Orbis");
		pIndexObject->Put("Hun", pContainer1);
		pIndexObject->Put("Koya", pContainer2);
		pIndexObject->Put("Tokka", pContainer3);

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertSize(3, pIndexObject->NumElements());

		cWriter.Init(szDirectory, "", "File");
		cSerialiser.Init(&cWriter);
		cSerialiser.Write(&pIndexObject);
		cSerialiser.Kill();
		cWriter.Kill();
	}
	ObjectsKill(false);
	ObjectsInit();
	{
		CPointer						pPointer1;
		CPointer						pPointer2;
		CPointer						pPointer3;
		Ptr<CIndexObject>				pIndexObject;
		CChunkFileObjectWriter			cWriter;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		cReader.Init(szDirectory, "File");
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pIndexObject = cDeserialiser.Read("Orbis");
		AssertNotNull(&pIndexObject);
		pPointer1 = pIndexObject->Get("Hun");
		pPointer2 = pIndexObject->Get("Koya");
		pPointer3 = pIndexObject->Get("Tokka");
		AssertTrue(pPointer1.IsNotNull());
		AssertTrue(pPointer2.IsNotNull());
		AssertTrue(pPointer3.IsNotNull());
		AssertString("CPointerContainer", pPointer1->ClassName());
		AssertString("CPointerContainer", pPointer2->ClassName());
		AssertString("CPointerContainer", pPointer3->ClassName());

		Ptr<CPointerContainer>			pContainer1;
		Ptr<CPointerContainer>			pContainer2;
		Ptr<CPointerContainer>			pContainer3;
		Ptr<CTestObject>				pObject;
		pContainer1 = pPointer1;
		pContainer2 = pPointer2;
		pContainer3 = pPointer3;
		AssertTrue(pContainer1->mp.IsNotNull());
		AssertTrue(pContainer2->mp.IsNotNull());
		AssertTrue(pContainer3->mp.IsNull());
		AssertString("CPointerContainer", pContainer1->mp->ClassName());
		AssertString("CTestObject", pContainer2->mp->ClassName());
		AssertPointer(&pContainer1->mp, &pContainer2);

		cDeserialiser.Kill();
		cReader.Kill();

		pIndexObject = NULL;
	}
	ObjectsFlush();
	ObjectsKill(false);
	DataOrderersKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectConstructorExists(void)
{
	CConstructors*	pcConstructors;
	CIndexObject*	pcIndexObject;

	ObjectsInit();

	pcConstructors = gcObjects.GetConstructors();
	pcIndexObject = pcConstructors->Get<CIndexObject>();
	AssertNotNull(pcIndexObject);
	AssertString("CIndexObject", pcIndexObject->ClassName());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectClassExists(void)
{
	CClasses*		pcClasses;
	CClass*			pcClass;

	ObjectsInit();

	CIndexObject	cIndexObject;

	pcClasses = gcObjects.GetClasses();
	pcClass = pcClasses->Get(cIndexObject.ClassName());
	AssertNotNull(pcClass);
	AssertString("CIndexObject", pcClass->GetName());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectMorphInto(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CIndexObject>			pIndex1;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestTriPointerObject>	pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestTriPointerObject>	pTest4;
		bool						bResult;
		int64						oid;
		size						uiRemapped;

		pRoot = ORoot();
		pIndex1 = OMalloc<CIndexObject>(IKR_Yes);
		pRoot->Add(pIndex1);

		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestTriPointerObject>(&sNotifier2);
		pTest3 = OMalloc<CTestObject>(&sNotifier3);
		pTest4 = OMalloc<CTestTriPointerObject>(&sNotifier4);
		oid = 1;
		bResult = pIndex1->Put((uint8*)&oid, sizeof(int64), pTest1);
		AssertTrue(bResult);
		oid = 2;
		bResult = pIndex1->Put((uint8*)&oid, sizeof(int64), pTest2);
		
		CIndexObject		cIndex2;

		cIndex2.Init(IKR_Yes);
		cIndex2.Put("3", pTest2);
		cIndex2.Put("4", pTest3);
		cIndex2.Put("5", pTest2);

		pTest4->mpObject1 = pTest2;
		pTest4->mpObject2 = pTest2;

		pTest2->mpObject1 = pTest1;

		Ptr<CTestObjectWithFields>	pTest5;
		CPointer					pPtr;

		pTest5 = OMalloc<CTestObjectWithFields>();

		AssertFalse(sNotifier2.bFreed);
		uiRemapped = pTest2.MorphInto(&pTest5);
		//2 (cIndex2) +
		//1 (pIndex1) +
		//1 (pTest2)  +
		//2 (pTest4->mpObject1, pTest4->mpObject2) =
		//6
		AssertSize(6, uiRemapped);
		AssertPointer(&pTest5, &pTest2);
		AssertString("CTestObjectWithFields", pTest5->ClassName());
		AssertTrue(sNotifier2.bFreed);
		AssertPointer(&pTest5, &pTest4->mpObject1);
		AssertPointer(&pTest5, &pTest4->mpObject2);
		pPtr = pIndex1->Get((uint8*)&oid, sizeof(int64));
		AssertPointer(&pTest5, &pPtr);
		pPtr = cIndex2.Get("3");
		AssertPointer(&pTest5, &pPtr);
		pPtr = cIndex2.Get("4");
		AssertPointer(&pTest3, &pPtr);
		pPtr = cIndex2.Get("5");
		AssertPointer(&pTest5, &pPtr);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectEmbeddedOnHeapKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		Ptr<CTestEmbeddedIndex>		pEmbedded;
		Ptr<CTestObject>			pTest;
		CTestObject*				pcTest;

		pEmbedded = OMalloc<CTestEmbeddedIndex>(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		pEmbedded->mcIndex.Put("1", pTest);
		pEmbedded->mcIndex.Put("2", pTest);

		AssertSize(2, pTest.NumHeapFroms());
		AssertSize(1, pTest.NumStackFroms());

		pcTest = &pTest;
		pTest = NULL;
		AssertFalse(sFreedNotifier1.bFreed);
		AssertFalse(sFreedNotifier2.bFreed);
		AssertSize(2, pcTest->NumHeapFroms());
		AssertSize(0, pcTest->NumStackFroms());

		pEmbedded = NULL;
		AssertTrue(sFreedNotifier1.bFreed);
		AssertTrue(sFreedNotifier2.bFreed);
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectEmbeddedOnStackKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		CTestEmbeddedIndex		cEmbedded;
		Ptr<CTestObject>		pTest;
		CTestObject*			pcTest;

		cEmbedded.Init(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		cEmbedded.mcIndex.Put("1", pTest);
		cEmbedded.mcIndex.Put("2", pTest);

		AssertSize(3, pTest.NumStackFroms());
		AssertSize(0, pTest.NumHeapFroms());
		pcTest = &pTest;

		pTest = NULL;
		AssertSize(0, pTest.NumStackFroms());
		AssertSize(0, pcTest->NumHeapFroms());
		AssertSize(2, pcTest->NumStackFroms());

		AssertFalse(sFreedNotifier1.bFreed);
		AssertFalse(sFreedNotifier2.bFreed);
	}
	AssertTrue(sFreedNotifier1.bFreed);
	AssertTrue(sFreedNotifier2.bFreed);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObjectEmbeddedInObjectsKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();

	Ptr<CTestEmbeddedIndex>		pEmbedded;
	Ptr<CTestObject>			pTest;
	CTestObject*				pcTest;
	Ptr<CRoot>					pRoot;

	pRoot = ORoot();
	pEmbedded = OMalloc<CTestEmbeddedIndex>(&sFreedNotifier1);
	pRoot->Add(pEmbedded);
	pTest = OMalloc<CTestObject>(&sFreedNotifier2);
	pEmbedded->mcIndex.Put("1", pTest);
	pEmbedded->mcIndex.Put("2", pTest);

	AssertSize(2, pTest.NumHeapFroms());
	AssertSize(1, pTest.NumStackFroms());
	pcTest = &pTest;

	pTest = NULL;
	pEmbedded = NULL;
	AssertSize(2, pcTest->NumHeapFroms());
	AssertSize(0, pcTest->NumStackFroms());

	AssertFalse(sFreedNotifier1.bFreed);
	AssertFalse(sFreedNotifier2.bFreed);

	ObjectsFlush();
	ObjectsKill(false);

	AssertTrue(sFreedNotifier1.bFreed);
	AssertTrue(sFreedNotifier2.bFreed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestIndexObjectConstructorExists();
	TestIndexObjectClassExists();
	TestIndexObjectPut();
	TestIndexObjectDetachInHeap();
	TestIndexObjectIterateSafeNull();
	TestIndexObjectIterateUnsafeNull();
	TestIndexObjectRemove();
	TestIndexObjectDetachOnStack();
	TestIndexObjectGetPointerTos();
	TestIndexObjectPointerFromsHeap();
	TestIndexObjectPointerFromsStack();
	TestIndexObjectOnStackKillContained();
	TestIndexObjectKillContainedHeap();
	TestIndexObjectPutOverwrite();
	TestIndexObjectSerialisation();
	TestIndexObjectMorphInto();
	TestIndexObjectEmbeddedOnHeapKill();
	TestIndexObjectEmbeddedOnStackKill();
	TestIndexObjectEmbeddedInObjectsKill();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

