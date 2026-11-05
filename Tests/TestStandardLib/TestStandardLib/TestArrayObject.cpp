#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "TestLib/Assert.h"
#include "NamedObjectTestClasses.h"
#include "TestArrayObject.h"


void ValidateObjectsCapturingErrors(char* szErrors, size uiSize);  //TestEmbedded.cpp


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
	AssertTrue(psPointer->eType == SPT_Pointer);
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
	AssertTrue(psPointer->eType == SPT_Pointer);
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
	AssertTrue(psPointer->eType == SPT_Pointer);
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
// Setting an element to the object it already holds leaves it with one
// from (and attached), and setting another object releases it.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectSetSameObjectAgain(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		Ptr<CArrayObject>			pArray;
		Ptr<CTestObject>			pObject1;
		Ptr<CTestObject>			pObject2;

		pArray = OMalloc<CArrayObject>();
		ORoot()->Add(pArray);
		pObject1 = OMalloc<CTestObject>(&sNotifier1);
		pObject2 = OMalloc<CTestObject>(&sNotifier2);

		pArray->Add(pObject1);
		AssertTrue(pArray->Set(0, pObject1));
		gcObjects.ValidateObjectsConsistency();
		AssertSize(1, pObject1->NumHeapFroms());
		AssertInt(3, pObject1->GetDistToRoot());
		pObject1 = NULL;
		AssertFalse(sNotifier1.bFreed);

		AssertTrue(pArray->Set(0, pObject2));
		AssertTrue(sNotifier1.bFreed);
		AssertSize(1, pObject2->NumHeapFroms());
		AssertInt(3, pObject2->GetDistToRoot());
		gcObjects.ValidateObjectsConsistency();

		pObject2 = NULL;
		AssertFalse(sNotifier2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Setting an element again through a pointer held by the object itself:
// nothing else holds it, so taking its from away before setting it back
// would free it.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectSetSameObjectFromItsOwnPointer(void)
{
	STestObjectFreedNotifier	sNotifier;

	ObjectsInit();
	{
		Ptr<CArrayObject>			pArray;
		Ptr<CTestObject>			pObject;
		CTestObject*				pcObject;

		pArray = OMalloc<CArrayObject>();
		ORoot()->Add(pArray);
		pObject = OMalloc<CTestObject>(&sNotifier);
		pObject->mpTest = pObject;
		pArray->Add(pObject);
		pcObject = &pObject;
		pObject = NULL;

		AssertTrue(pArray->Set(0, pcObject->mpTest));
		AssertFalse(sNotifier.bFreed);
		AssertPointer(pcObject, pArray->Get(0).Object());
		AssertSize(2, pcObject->NumHeapFroms());
		AssertInt(3, pcObject->GetDistToRoot());
		gcObjects.ValidateObjectsConsistency();

		AssertTrue(pArray->Remove(0));
		AssertTrue(sNotifier.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();
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
// An array keeps the order its objects were added in, even after one of
// them is replaced (as loading a hollow object does).  Sorting it by
// pointer is for sets only.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectOrderKeptAfterRemap(void)
{
	char	szName[8];
	size	i;
	size	j;

	ObjectsInit();
	gcObjects.AddConstructor<CTestNamedObject>();
	{
		Ptr<CArrayObject>		pArray;
		Ptr<CTestNamedObject>	apNamed[3];
		Ptr<CTestNamedObject>	pTemp;
		Ptr<CTestNamedObject>	pReplaced;

		pArray = OMalloc<CArrayObject>();
		ORoot()->Add(pArray);
		for (i = 0; i < 3; i++)
		{
			sprintf(szName, "N%i", (int)i);
			apNamed[i] = ONMalloc<CTestNamedObject>(szName, (int)i);
		}

		//Highest address first, so the order isn't the pointers' order.
		for (i = 0; i < 3; i++)
		{
			for (j = i + 1; j < 3; j++)
			{
				if (apNamed[j].Object() > apNamed[i].Object())
				{
					pTemp = apNamed[i];
					apNamed[i] = apNamed[j];
					apNamed[j] = pTemp;
				}
			}
		}
		pTemp = NULL;
		for (i = 0; i < 3; i++)
		{
			pArray->Add(apNamed[i]);
		}
		j = (size)apNamed[1]->miNum;
		for (i = 0; i < 3; i++)
		{
			apNamed[i] = NULL;
		}

		sprintf(szName, "N%i", (int)j);
		pReplaced = gcObjects.AllocateReplacingNamed("CTestNamedObject", szName);
		pReplaced->Init((int)j);
		pTemp = OMalloc<CTestNamedObject>(3);
		pArray->Add(pTemp);
		pTemp = NULL;
		pReplaced = NULL;

		AssertSize(4, pArray->NumElements());
		AssertTrue(pArray->Get<CTestNamedObject>(0).Object() > pArray->Get<CTestNamedObject>(2).Object());
		AssertInt((int32)j, pArray->Get<CTestNamedObject>(1)->miNum);
		AssertInt((int32)3, pArray->Get<CTestNamedObject>(3)->miNum);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// RemoveAll takes every copy of each of another array's objects out.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectRemoveAllOfAnother(void)
{
	char	szErrors[1024];

	ObjectsInit();
	{
		Ptr<CArrayObject>	pArray;
		Ptr<CArrayObject>	pOther;
		Ptr<CTestObject>	pX;
		Ptr<CTestObject>	pY;
		Ptr<CTestObject>	pZ;
		Ptr<CTestObject>	pW;

		pArray = OMalloc<CArrayObject>();
		pOther = OMalloc<CArrayObject>();
		ORoot()->Add(pArray);
		ORoot()->Add(pOther);
		pX = OMalloc<CTestObject>();
		pY = OMalloc<CTestObject>();
		pZ = OMalloc<CTestObject>();
		pW = OMalloc<CTestObject>();
		pArray->Add(pX);
		pArray->Add(pY);
		pArray->Add(pZ);
		pArray->Add(pY);
		pOther->Add(pY);
		pOther->Add(pW);

		AssertTrue(pArray->RemoveAll(&pOther));
		AssertSize(2, pArray->NumElements());
		AssertPointer(pX.Object(), pArray->Get(0).Object());
		AssertPointer(pZ.Object(), pArray->Get(1).Object());
		AssertSize(1, pY->NumHeapFroms());
		AssertSize(2, pOther->NumElements());

		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A big array keeps the index of each of its members once one is remapped
// (as loading a hollow object does), and every change keeps it right:
// validating checks every index kept.  Inserting moves the members after
// it, so then the indices are made again.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayObjectMemberLocations(void)
{
	char				szErrors[1024];
	size				i;
	size				uiExpected;

	ObjectsInit();
	gcObjects.AddConstructor<CTestNamedObject>();
	{
		Ptr<CArrayObject>		pArray;
		Ptr<CArrayObject>		pOther;
		Ptr<CTestObject>		pShared;
		Ptr<CTestObject>		pObject;
		Ptr<CTestNamedObject>	pNamed;

		pArray = OMalloc<CArrayObject>();
		ORoot()->Add(pArray);
		for (i = 0; i < 100; i++)
		{
			pObject = OMalloc<CTestObject>();
			pArray->Add(pObject);
		}
		pShared = OMalloc<CTestObject>();
		pArray->Set(10, pShared);
		pArray->Set(20, pShared);
		pArray->Add(pShared);
		pNamed = ONMalloc<CTestNamedObject>("Named", 1);
		pArray->Set(30, pNamed);
		pArray->Set(31, pNamed);
		uiExpected = 101;

		pNamed = gcObjects.AllocateReplacingNamed("CTestNamedObject", "Named");
		pNamed->Init(2);
		AssertPointer(pNamed.Object(), pArray->Get(30).Object());
		AssertPointer(pNamed.Object(), pArray->Get(31).Object());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		//Adding at the end, taking the last and setting move nothing.
		pObject = OMalloc<CTestObject>();
		pArray->Add(pObject);
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);
		AssertTrue(pArray->Remove(uiExpected));
		pArray->Set(40, pObject);
		AssertSize(uiExpected, pArray->NumElements());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		pShared->Kill();
		uiExpected -= 3;
		AssertSize(uiExpected, pArray->NumElements());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		//Slots 10 and 20 were taken: Named moved from 30 and 31.  Remapping it makes the indices again.
		pNamed = gcObjects.AllocateReplacingNamed("CTestNamedObject", "Named");
		pNamed->Init(3);
		AssertPointer(pNamed.Object(), pArray->Get(28).Object());
		AssertPointer(pNamed.Object(), pArray->Get(29).Object());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		pArray->Insert(uiExpected, pObject);
		uiExpected++;
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);
		pArray->Insert(5, pObject);
		uiExpected++;
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);
		pNamed = gcObjects.AllocateReplacingNamed("CTestNamedObject", "Named");
		pNamed->Init(4);
		AssertPointer(pNamed.Object(), pArray->Get(29).Object());
		AssertPointer(pNamed.Object(), pArray->Get(30).Object());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		pArray->RemoveEnd(90);
		uiExpected = 90;
		AssertSize(uiExpected, pArray->NumElements());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		//Removing the last member moves nothing.
		pShared = OMalloc<CTestObject>();
		pArray->Add(pShared);
		AssertTrue(pArray->Remove(pShared));
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		AssertTrue(pArray->Remove(pObject));
		uiExpected -= 2;
		AssertSize(uiExpected, pArray->NumElements());
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		pOther = OMalloc<CArrayObject>();
		pOther->Add(pNamed);
		pObject = pArray->Get(0);
		pOther->Add(pObject);
		AssertTrue(pArray->RemoveAll(&pOther));
		uiExpected -= 3;
		AssertSize(uiExpected, pArray->NumElements());
		AssertFalse(pArray->Contains(pNamed));
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);
	}
	ObjectsFlush();
	ObjectsKill();
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
	TestArrayObjectSetSameObjectAgain();
	TestArrayObjectSetSameObjectFromItsOwnPointer();
	TestArrayObjectConstructorExists();
	TestArrayObjectClassExists();
	TestArrayObjectExternalSerialisation();
	TestArrayObjectInternalSerialisation(4);
	TestArrayObjectInternalSerialisation(10000);
	TestArrayObjectEmbeddedOnHeapKill();
	TestArrayObjectEmbeddedOnStackKill();
	TestArrayObjectEmbeddedInObjectsKill();
	TestArrayObjectOrderKeptAfterRemap();
	TestArrayObjectRemoveAllOfAnother();
	TestArrayObjectMemberLocations();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

