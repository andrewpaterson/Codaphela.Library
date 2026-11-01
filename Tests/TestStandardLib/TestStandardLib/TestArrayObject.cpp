#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "TestLib/Assert.h"
#include "TestArrayObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectAddConstructors(void)
{
	gcObjects.AddConstructor<CTestSaveableObject1>();
	gcObjects.AddConstructor<CTestObject>();
	gcObjects.AddConstructor<CPointerContainer>();
	gcObjects.AddConstructor<CTestTriPointerObject>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectAdd(void)
{
	ObjectsInit();

	Ptr<CArray<CTestSaveableObject1>> pacStuff = OMalloc<CArray<CTestSaveableObject1>>();
	pacStuff->Add(OMalloc<CTestSaveableObject1>());

	Ptr<CTestSaveableObject1> pSaveable = pacStuff->Get(0);
	AssertTrue(pSaveable.IsNotNull());
	AssertInt((int32)7, pSaveable->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectGet(void)
{
	ObjectsInit();

	Ptr<CArray<CTestSaveableObject1>> pacStuff = OMalloc<CArray<CTestSaveableObject1>>();
	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Add(pSaveable);

	AssertSize(2, pacStuff->NumElements());
	AssertInt((int32)7, pacStuff->Get(0)->miInt);
	AssertInt((int32)3, pacStuff->Get(1)->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectInsert(void)
{
	ObjectsInit();

	Ptr<CArray<CTestSaveableObject1>> pacStuff = OMalloc<CArray<CTestSaveableObject1>>();
	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Insert(0, pSaveable);

	AssertSize(2, pacStuff->NumElements());
	AssertInt((int32)3, pacStuff->Get(0)->miInt);
	AssertInt((int32)7, pacStuff->Get(1)->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectAddAll(void)
{
	ObjectsInit();

	Ptr<CArray<CTestSaveableObject1>> pacStuff = OMalloc<CArray<CTestSaveableObject1>>();
	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Insert(0, pSaveable);

	Ptr<CArray<CTestSaveableObject1>> pacMore = OMalloc<CArray<CTestSaveableObject1>>();
	pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 5;

	AssertSize(0, pSaveable->NumHeapFroms());
	pacStuff->Insert(0, pSaveable);
	AssertSize(1, pSaveable->NumHeapFroms());
	gcObjects.ValidateObjectsConsistency();

	pacMore->AddAll(pacStuff);

	AssertSize(3, pacMore->NumElements());
	AssertInt((int32)5, pacMore->Get(0)->miInt);
	AssertInt((int32)3, pacMore->Get(1)->miInt);
	AssertInt((int32)7, pacMore->Get(2)->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectRemove(void)
{
	ObjectsInit();

	Ptr<CRoot> pRoot = ORoot();

	Ptr<CArray<CTestSaveableObject1>> pacStuff = OMalloc<CArray<CTestSaveableObject1>>();
	pRoot->Add(pacStuff);

	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Insert(0, pSaveable);
	pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 5;
	pacStuff->Insert(0, pSaveable);

	pacStuff->RemoveAt(1);

	AssertSize(2, pacStuff->NumElements());
	AssertInt((int32)5, pacStuff->Get(0)->miInt);
	AssertInt((int32)7, pacStuff->Get(1)->miInt);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectSneakyOnStack(void)
{
	ObjectsInit();

	CTestObject					aArray;
	STestObjectFreedNotifier	sFreed;
	SStackPointer*				psPointer;
	CBaseObject*				pcObject;

	AssertSize(0, gcStackPointers.NumElements());

	Ptr<CTestSimpleObject> pTestObject = OMalloc<CTestSimpleObject>(&sFreed, 23);
	AssertSize(1, gcStackPointers.NumElements());
	psPointer = gcStackPointers.Get(0)->Get();
	AssertTrue(psPointer->meType == SPT_Pointer);
	pcObject = psPointer->u.pcPointer->BaseObject();
	//The pointer 'pTestObject' is on the stack and pointes to a CTestSimpleObject.
	AssertPointer(pTestObject.BaseObject(), pcObject);

	AssertSize(1, gcStackPointers.NumElements());
	aArray.Init(pTestObject, NULL);
	AssertSize(2, gcStackPointers.NumElements());

	AssertFalse(sFreed.bFreed);
	pTestObject = NULL;
	AssertFalse(sFreed.bFreed);
	AssertSize(1, gcStackPointers.NumElements());

	aArray.Kill();
	AssertSize(0, gcStackPointers.NumElements());
	AssertTrue(sFreed.bFreed);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectOnStackKill(void)
{
	ObjectsInit();

	CArray<CTestSimpleObject>	aArray;
	STestObjectFreedNotifier	sFreed;
	SStackPointer*				psPointer;
	CBaseObject*				pcObject;

	AssertSize(0, gcStackPointers.NumElements());

	Ptr<CTestSimpleObject> pTestObject = OMalloc<CTestSimpleObject>(&sFreed, 23);
	AssertSize(1, gcStackPointers.NumElements());
	psPointer = gcStackPointers.Get(0)->Get();;
	AssertTrue(psPointer->meType == SPT_Pointer);
	pcObject = psPointer->u.pcPointer->BaseObject();
	//The pointer 'pTestObject' is on the stack and pointes to a CTestSimpleObject.
	AssertPointer(pTestObject.BaseObject(), pcObject);

	AssertSize(1, gcStackPointers.NumElements());
	aArray.Init();
	aArray.Add(pTestObject);
	AssertSize(2, gcStackPointers.NumElements());

	AssertFalse(sFreed.bFreed);
	pTestObject = NULL;
	AssertFalse(sFreed.bFreed);
	AssertSize(1, gcStackPointers.NumElements());

	aArray.Kill();
	AssertSize(0, gcStackPointers.NumElements());
	AssertTrue(sFreed.bFreed);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectOnStackRemoveObject(void)
{
	ObjectsInit();

	CArray<CTestSimpleObject>	aArray;
	STestObjectFreedNotifier	sFreed;
	SStackPointer*				psPointer;
	CBaseObject*				pcObject;

	AssertSize(0, gcStackPointers.NumElements());
	AssertSize(0, gcObjects.NumMemoryIndexes());

	Ptr<CTestSimpleObject> pTestObject = OMalloc<CTestSimpleObject>(&sFreed, 23);
	AssertSize(1, gcStackPointers.NumElements());
	AssertSize(1, gcObjects.NumMemoryIndexes());
	psPointer = gcStackPointers.Get(0)->Get();;
	AssertTrue(psPointer->meType == SPT_Pointer);
	pcObject = psPointer->u.pcPointer->BaseObject();
	//The pointer 'pTestObject' is on the stack and pointes to a CTestSimpleObject.
	AssertPointer(pTestObject.BaseObject(), pcObject);

	AssertSize(1, gcStackPointers.NumElements());
	aArray.Init();
	AssertSize(1, gcStackPointers.NumElements());
	aArray.Add(pTestObject);
	AssertSize(2, gcStackPointers.NumElements());
	AssertSize(1, aArray.NumElements());
	AssertSize(1, gcObjects.NumMemoryIndexes());

	aArray.Remove(pTestObject);
	AssertSize(1, gcStackPointers.NumElements());
	AssertSize(0, aArray.NumElements());
	AssertSize(1, gcObjects.NumMemoryIndexes());
	AssertFalse(sFreed.bFreed);

	pTestObject = NULL;
	AssertTrue(sFreed.bFreed);
	AssertSize(0, gcStackPointers.NumElements());
	AssertSize(0, gcObjects.NumMemoryIndexes());

	aArray.Kill();
	AssertSize(0, gcStackPointers.NumElements());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectRemoveDuplicates(void)
{
	STestObjectFreedNotifier	sFreed;
	STestObjectFreedNotifier	sOtherFreed;
	STestObjectFreedNotifier	sOnlyInArrayFreed;
	CBaseObject*				pcOnlyInArray;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CArrayObject>	pArray;
		Ptr<CTestObject>	pObject;
		Ptr<CTestObject>	pOther;
		Ptr<CTestObject>	pOnlyInArray;

		pRoot = ORoot();
		pArray = OMalloc<CArrayObject>();
		pRoot->Add(pArray);
		pObject = OMalloc<CTestObject>(&sFreed);
		pOther = OMalloc<CTestObject>(&sOtherFreed);

		pArray->Add(pObject);
		pArray->Add(pOther);
		pArray->Add(pObject);
		pArray->Add(pObject);
		AssertSize(4, pArray->NumElements());
		AssertSize(3, pObject->NumHeapFroms());
		AssertSize(1, pOther->NumHeapFroms());

		//Remove takes out every copy of the Object, so it must take out the from for every copy.
		AssertTrue(pArray->Remove(pObject));
		AssertSize(1, pArray->NumElements());
		AssertPointer(&pOther, pArray->Get(0).Object());
		AssertSize(0, pObject->NumHeapFroms());
		AssertFalse(pObject->ContainsFrom(&pArray));
		AssertSize(1, pOther->NumHeapFroms());
		AssertFalse(sFreed.bFreed);

		//With no froms left the Object is freed when the stack lets go of it.
		pObject = NULL;
		AssertTrue(sFreed.bFreed);
		AssertFalse(sOtherFreed.bFreed);

		//An Object only held by the array is freed by removing its last copy, not before.
		pOnlyInArray = OMalloc<CTestObject>(&sOnlyInArrayFreed);
		pArray->Add(pOnlyInArray);
		pArray->Add(pOnlyInArray);
		pcOnlyInArray = &pOnlyInArray;
		pOnlyInArray = NULL;
		AssertFalse(sOnlyInArrayFreed.bFreed);
		AssertSize(2, pcOnlyInArray->NumHeapFroms());
		AssertTrue(((CArrayCommonObject*)&pArray)->Remove(pcOnlyInArray));
		AssertTrue(sOnlyInArrayFreed.bFreed);
		AssertSize(1, pArray->NumElements());

		pRoot = NULL;
		pArray = NULL;
		pOther = NULL;
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectOnStackRemoveDuplicates(void)
{
	STestObjectFreedNotifier	sFreed;

	ObjectsInit();
	{
		CArray<CTestSimpleObject>	aArray;
		Ptr<CTestSimpleObject>		pTestObject;

		pTestObject = OMalloc<CTestSimpleObject>(&sFreed, 23);
		AssertSize(1, gcStackPointers.NumElements());

		aArray.Init();
		aArray.Add(pTestObject);
		aArray.Add(pTestObject);
		aArray.Add(pTestObject);
		AssertSize(3, aArray.NumElements());
		AssertSize(4, gcStackPointers.NumElements());
		AssertSize(4, pTestObject->NumStackFroms());

		//Every copy's stack from goes with the copies.
		AssertTrue(aArray.Remove(pTestObject));
		AssertSize(0, aArray.NumElements());
		AssertSize(1, gcStackPointers.NumElements());
		AssertSize(1, pTestObject->NumStackFroms());
		AssertFalse(sFreed.bFreed);

		pTestObject = NULL;
		AssertTrue(sFreed.bFreed);
		AssertSize(0, gcStackPointers.NumElements());

		aArray.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectOnStackKillContainedHeapObject(void)
{
	STestObjectFreedNotifier	sFreed;

	ObjectsInit();
	{
		CArray<CTestSimpleObject>	aArray;

		Ptr<CTestSimpleObject> pTestObject = OMalloc<CTestSimpleObject>(&sFreed, 23);
		aArray.Init();
		aArray.Add(pTestObject);
		AssertSize(1, aArray.NumElements());
		AssertSize(2, gcStackPointers.NumElements());

		pTestObject->Kill();
		AssertTrue(sFreed.bFreed);
		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertNull(&pTestObject);
		AssertSize(0, aArray.NumElements());

		aArray.Kill();
		AssertSize(0, gcStackPointers.NumElements());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectOnStackKillContainedStackObject(void)
{
	STestObjectFreedNotifier	sFreed;

	ObjectsInit();
	{
		CArray<CTestSimpleObject>	aArray;
		CTestSimpleObject			cTestObject;

		cTestObject.Init(&sFreed, 23);
		aArray.Init();
		aArray.Add(&cTestObject);
		AssertSize(1, aArray.NumElements());

		cTestObject.Kill();
		AssertTrue(sFreed.bFreed);
		AssertSize(0, aArray.NumElements());

		aArray.Kill();
		AssertSize(0, gcStackPointers.NumElements());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectKillContainedHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CArray<CTestObject>>	pArray;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pGot;

		pRoot = ORoot();
		pArray = OMalloc<CArray<CTestObject>>();
		pRoot->Add(pArray);
		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pArray->Add(pTest1);
		pArray->Add(pTest2);
		AssertSize(2, pArray->NumElements());

		pTest1->Kill();
		AssertTrue(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertSize(1, pArray->NumElements());
		pGot = pArray->Get(0);
		AssertPointer(&pTest2, &pGot);
		pGot = NULL;
		AssertSize(1, pTest2->NumHeapFroms());
		gcObjects.ValidateObjectsConsistency();

		pTest2 = NULL;
		pArray = NULL;
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
void TestArrayObjectKillAllWithNull(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CArray<CTestObject>>	pArray;
		Ptr<CTestObject>			pTest;
		Ptr<CTestObject>			pNull;

		pRoot = ORoot();
		pArray = OMalloc<CArray<CTestObject>>();
		pRoot->Add(pArray);

		pTest = OMalloc<CTestObject>(&sNotifier1);
		pArray->Add(pTest);
		pArray->Add(pNull);
		pTest = OMalloc<CTestObject>(&sNotifier2);
		pArray->Add(pTest);
		pTest = OMalloc<CTestObject>(&sNotifier3);
		pArray->Add(pTest);
		pTest = NULL;
		AssertSize(4, pArray->NumElements());

		pArray->KillAll();
		AssertTrue(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertTrue(sNotifier3.bFreed);
		gcObjects.ValidateObjectsConsistency();

		pArray = NULL;
		AssertTrue(pRoot->RemoveAll());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectConstructorExists(void)
{
	CConstructors*	pcConstructors;
	CArrayObject*	pcArray;

	ObjectsInit();

	pcConstructors = gcObjects.GetConstructors();
	pcArray = pcConstructors->Get<CArrayObject>();
	AssertNotNull(pcArray);
	AssertString("CArrayObject", pcArray->ClassName());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectClassExists(void)
{
	CClasses*		pcClasses;
	CClass*			pcClass;

	ObjectsInit();

	CArrayObject	cArray;

	pcClasses = gcObjects.GetClasses();
	pcClass = pcClasses->Get(cArray.ClassName());
	AssertNotNull(pcClass);
	AssertString("CArrayObject", pcClass->GetName());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectExternalSerialisation(void)
{
	CFileUtil	cFileUtil;
	char		szDirectory[] = "Output" _FS_ "TestArrayObject";

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	TestArrayObjectAddConstructors();
	{
		Ptr<CPointerContainer>			pContainer1;
		Ptr<CPointerContainer>			pContainer2;
		Ptr<CPointerContainer>			pContainer3;
		Ptr<CTestObject>				pObject;
		Ptr<CArrayObject>				pArray;
		CExternalObjectSerialiser		cSerialiser;
		CChunkFileObjectWriter			cWriter;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		pObject = OMalloc<CTestObject>();
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pContainer3 = OMalloc<CPointerContainer>(Null());
		pArray = ONMalloc<CArrayObject>("Orbis");
		pArray->Add(pContainer1);
		pArray->Add(pContainer2);
		pArray->Add(pContainer3);

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertSize(3, pArray->NumElements());

		cWriter.Init(szDirectory, "", "File");
		cSerialiser.Init(&cWriter);
		cSerialiser.Write(&pArray);
		cSerialiser.Kill();
		cWriter.Kill();
	}
	ObjectsKill(false);
	ObjectsInit();
	{
		CPointer						pPointer1;
		CPointer						pPointer2;
		CPointer						pPointer3;
		CPointer						pPointer4;
		Ptr<CArrayObject>				pArray;
		CChunkFileObjectWriter			cWriter;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		cReader.Init(szDirectory, "File");
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pArray = cDeserialiser.Read("Orbis");
		AssertNotNull(&pArray);
		pPointer1 = pArray->Get(0);
		pPointer2 = pArray->Get(1);
		pPointer3 = pArray->Get(2);
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

		pArray = NULL;
	}
	ObjectsFlush();
	ObjectsKill(false);

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


struct STriOi
{
	OIndex	oi1;
	OIndex	oi2;
	OIndex	oi3;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectInternalSerialisation(size uiNumArrayItems)
{
	CFileUtil			cFileUtil;
	CCodabase*			pcDatabase;
	CSequence*			pcSequence;
	char				szDirectory[] = "Output" _FS_ "ArrayObjectInternalSerialisation";
	bool				bResult;
	CIndexTreeMemory	cMemory;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	uint		uiIndex;
	CArrayInt	aiKeyNames;
	CRandom		cRandom;
	uint		uiCount;
	size		ui;

	cMemory.Init(IKR_Yes);

	cRandom.Init(32280975);
	aiKeyNames.Init();
	for (uiCount = 0; uiCount < uiNumArrayItems; uiCount++)
	{
		aiKeyNames.Add(uiCount);
	}
	aiKeyNames.Shuffle(&cRandom);
	cRandom.Kill();
	AssertSize(uiNumArrayItems, aiKeyNames.NumElements());

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CRoot>					pRoot;
		Ptr<CArrayObject>			pArray;
		Ptr<CTestObject>			pKey;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;
		Ptr<CTestTriPointerObject>	pValue5;
		bool						bResult;
		size						ui;
		OIndex						oi;
		STriOi*						psTriOi;
		STriOi						sTriOi;

		pRoot = ORoot();
		pArray = ONMalloc<CArrayObject>("Array");
		pRoot->Add(pArray);

		gcObjects.DisableValidation();

		pValue1 = NULL;
		pValue2 = NULL;
		pValue3 = NULL;

		bResult = true;
		for (ui = 0; ui < uiNumArrayItems; ui++)
		{
			if (ui % 2 == 0)
			{
				pValue3 = pValue2;
				pValue2 = pValue1;
				pValue1 = OMalloc<CTestObject>();
				bResult = pArray->Add(pValue1);
			}
			else
			{
				pValue5 = OMalloc<CTestTriPointerObject>();
				pValue5->mpObject1 = pValue1;
				pValue5->mpObject2 = pValue2;
				pValue5->mpObject3 = pValue3;
				bResult = pArray->Add(pValue5);

				oi = pValue5->GetIndex();
				psTriOi = (STriOi*)cMemory.Put((uint8*)&oi, sizeof(OIndex), NULL, sizeof(STriOi));
				psTriOi->oi1 = pValue1.GetIndex();
				psTriOi->oi2 = pValue2.GetIndex();
				psTriOi->oi3 = pValue3.GetIndex();
			}
			if (!bResult)
			{
				AssertTrue(bResult);
				break;
			}
		}
		AssertTrue(bResult);

		pValue1 = NULL;
		pValue2 = NULL;
		pValue3 = NULL;

		bResult = true;
		for (ui = 0; ui < uiNumArrayItems; ui++)
		{
			uiIndex = aiKeyNames.GetValue(ui);
			if (uiIndex % 2 == 0)
			{
				pValue1 = pArray->Get((size)uiIndex);
				bResult = pValue1.IsNotNull();
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
				bResult = StringCompare("CTestObject", pValue1->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
			else
			{
				pValue5 = pArray->Get((size)uiIndex);
				bResult = pValue5.IsNotNull();
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
				bResult = StringCompare("CTestTriPointerObject", pValue5->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}

				oi = pValue5->GetIndex();
				bResult = cMemory.Get((uint8*)&oi, sizeof(OIndex), &sTriOi, NULL, sizeof(STriOi));
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}

				pValue1 = gcObjects.Get(sTriOi.oi1);
				pValue2 = gcObjects.Get(sTriOi.oi2);
				pValue3 = gcObjects.Get(sTriOi.oi3);

				bResult  = &pValue5->mpObject1 == &pValue1;
				bResult &= &pValue5->mpObject2 == &pValue2;
				bResult &= &pValue5->mpObject3 == &pValue3;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
		}
		AssertTrue(bResult);
	}

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	AssertLong(uiNumArrayItems + 3LL, pcDatabase->NumIndices());
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		TestArrayObjectAddConstructors();
		AssertLong(uiNumArrayItems + 3LL, pcDatabase->NumIndices());

		AssertTrue(gcObjects.Contains("Array"));

		Ptr<CArrayObject>			pArray;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;
		Ptr<CTestTriPointerObject>	pValue5;
		bool						bResult;
		OIndex						oi;
		STriOi						sTriOi;

		pArray = gcObjects.Get("Array");
		AssertTrue(pArray.IsNotNull());
		AssertSize(uiNumArrayItems, pArray->NumElements());

		pValue1 = NULL;
		pValue2 = NULL;
		pValue3 = NULL;

		bResult = true;
		for (ui = 0; ui < uiNumArrayItems; ui++)
		{
			uiIndex = aiKeyNames.GetValue(ui);
			if (uiIndex % 2 == 0)
			{
				pValue3 = pValue2;
				pValue2 = pValue1;
				pValue1 = pArray->Get((size)uiIndex);
				bResult = pValue1.IsNotNull();
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
				bResult = StringCompare("CTestObject", pValue1->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
			else
			{
				pValue5 = pArray->Get((size)uiIndex);
				bResult = pValue5.IsNotNull();
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
				bResult = StringCompare("CTestTriPointerObject", pValue5->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}

				oi = pValue5->GetIndex();
				bResult = cMemory.Get((uint8*)&oi, sizeof(OIndex), &sTriOi, NULL, sizeof(STriOi));
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}

				pValue1 = gcObjects.Get(sTriOi.oi1);
				pValue2 = gcObjects.Get(sTriOi.oi2);
				pValue3 = gcObjects.Get(sTriOi.oi3);

				bResult = &pValue5->mpObject1 == &pValue1;
				bResult &= &pValue5->mpObject2 == &pValue2;
				bResult &= &pValue5->mpObject3 == &pValue3;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
		}
		AssertTrue(bResult);

		aiKeyNames.Kill();
	}
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectEmbeddedOnHeapKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		Ptr<CTestEmbeddedArray>		pEmbedded;
		Ptr<CTestObject>			pTest;
		CTestObject*				pcTest;

		pEmbedded = OMalloc<CTestEmbeddedArray>(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		pEmbedded->mcArray.Add(pTest);
		pEmbedded->mcArray.Add(pTest);

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
void TestArrayObjectEmbeddedOnStackKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		CTestEmbeddedArray			cEmbedded;
		Ptr<CTestObject>			pTest;
		CTestObject*				pcTest;

		cEmbedded.Init(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		cEmbedded.mcArray.Add(pTest);
		cEmbedded.mcArray.Add(pTest);

		AssertSize(3, pTest.NumStackFroms());
		AssertSize(0, pTest.NumHeapFroms());
		pcTest = &pTest;

		pTest = NULL;
		AssertSize(0, pTest.NumStackFroms());
		AssertSize(0, pcTest->NumHeapFroms());
		AssertSize(2, pcTest->NumStackFroms());
	}
	AssertTrue(sFreedNotifier1.bFreed);
	AssertTrue(sFreedNotifier2.bFreed);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectEmbeddedInObjectsKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();

	Ptr<CTestEmbeddedArray>		pEmbedded;
	Ptr<CTestObject>			pTest;
	CTestObject*				pcTest;
	Ptr<CRoot>					pRoot;

	pRoot = ORoot();
	pEmbedded = OMalloc<CTestEmbeddedArray>(&sFreedNotifier1);
	pRoot->Add(pEmbedded);
	pTest = OMalloc<CTestObject>(&sFreedNotifier2);
	pEmbedded->mcArray.Add(pTest);
	pEmbedded->mcArray.Add(pTest);

	AssertSize(2, pTest.NumHeapFroms());
	AssertSize(1, pTest.NumStackFroms());
	pcTest = &pTest;

	pTest = NULL;
	pEmbedded = NULL;
	AssertSize(2, pcTest->NumHeapFroms());
	AssertSize(0, pcTest->NumStackFroms());

	ObjectsFlush();
	ObjectsKill(false);

	AssertTrue(sFreedNotifier1.bFreed);
	AssertTrue(sFreedNotifier2.bFreed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestArrayObjectAdd();
	TestArrayObjectGet();
	TestArrayObjectInsert();
	TestArrayObjectAddAll();
	TestArrayObjectRemove();
	TestArrayObjectSneakyOnStack();
	TestArrayObjectOnStackKill();
	TestArrayObjectOnStackRemoveObject();
	TestArrayObjectRemoveDuplicates();
	TestArrayObjectOnStackRemoveDuplicates();
	TestArrayObjectOnStackKillContainedHeapObject();
	TestArrayObjectOnStackKillContainedStackObject();
	TestArrayObjectKillContainedHeap();
	TestArrayObjectKillAllWithNull();
	TestArrayObjectConstructorExists();
	TestArrayObjectClassExists();
	TestArrayObjectExternalSerialisation();
	TestArrayObjectInternalSerialisation(4);
	TestArrayObjectInternalSerialisation(10000);
	TestArrayObjectEmbeddedOnHeapKill();
	TestArrayObjectEmbeddedOnStackKill();
	TestArrayObjectEmbeddedInObjectsKill();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

