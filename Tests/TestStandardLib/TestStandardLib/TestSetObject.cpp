#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/DebugOutput.h"
#include "StandardLib/HollowObject.h"
#include "StandardLib/Set.h"
#include "TestLib/Assert.h"
#include "TestSetObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectAddConstructors(void)
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
void TestSetObjectAdd(void)
{
	ObjectsInit();

	Ptr<CSet<CTestSaveableObject1>> pacStuff = OMalloc<CSet<CTestSaveableObject1>>(false);
	pacStuff->Add(OMalloc<CTestSaveableObject1>());

	Ptr<CTestSaveableObject1> pSaveable = pacStuff->UnsafeGet(0);
	AssertTrue(pSaveable.IsNotNull());
	AssertInt((int32)7, pSaveable->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectGet(void)
{
	ObjectsInit();

	Ptr<CSet<CTestSaveableObject1>> pacStuff = OMalloc<CSet<CTestSaveableObject1>>(false);
	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Add(pSaveable);

	AssertSize(2, pacStuff->NumElements());
	AssertInt((int32)7, pacStuff->UnsafeGet(0)->miInt);
	AssertInt((int32)3, pacStuff->UnsafeGet(1)->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectAddAll(void)
{
	ObjectsInit();

	Ptr<CSet<CTestSaveableObject1>> pacStuff = OMalloc<CSet<CTestSaveableObject1>>(false);
	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Add(pSaveable);

	Ptr<CSet<CTestSaveableObject1>> pacMore = OMalloc<CSet<CTestSaveableObject1>>(false);
	pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 5;
	pacStuff->Add(pSaveable);

	pacMore->AddAll(pacStuff);

	AssertSize(3, pacMore->NumElements());
	AssertInt((int32)7, pacMore->UnsafeGet(0)->miInt);
	AssertInt((int32)3, pacMore->UnsafeGet(1)->miInt);
	AssertInt((int32)5, pacMore->UnsafeGet(2)->miInt);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectRemove(void)
{
	ObjectsInit();

	Ptr<CRoot> pRoot = ORoot();

	Ptr<CSet<CTestSaveableObject1>> pacStuff = OMalloc<CSet<CTestSaveableObject1>>(false);
	pRoot->Add(pacStuff);

	pacStuff->Add(OMalloc<CTestSaveableObject1>());
	Ptr<CTestSaveableObject1> pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 3;
	pacStuff->Add(pSaveable);
	pSaveable = OMalloc<CTestSaveableObject1>();
	pSaveable->miInt = 5;
	pacStuff->Add(pSaveable);

	pacStuff->Remove(pSaveable);

	AssertSize(2, pacStuff->NumElements());
	AssertInt((int32)7, pacStuff->UnsafeGet(0)->miInt);
	AssertInt((int32)3, pacStuff->UnsafeGet(1)->miInt);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectRemoveUnlessLowest(Ptr<CSet<CTestObject>> pSet, Ptr<CTestObject>& pObject, Ptr<CTestObject>& pLowest)
{
	if (&pObject != &pLowest)
	{
		AssertTrue(pSet->Remove(pObject));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectRemoveSortedWithNulls(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CSet<CTestObject>>		pSet;
		Ptr<CTestObject>			pObject1;
		Ptr<CTestObject>			pObject2;
		Ptr<CTestObject>			pObject3;
		Ptr<CTestObject>			pObject4;
		Ptr<CTestObject>			pLowest;
		Ptr<CTestObject>			pNew;

		pRoot = ORoot();
		pSet = OMalloc<CSet<CTestObject>>();
		pRoot->Add(pSet);

		pObject1 = OMalloc<CTestObject>();
		pObject2 = OMalloc<CTestObject>();
		pObject3 = OMalloc<CTestObject>();
		pObject4 = OMalloc<CTestObject>();
		pSet->Add(pObject1);
		pSet->Add(pObject2);
		pSet->Add(pObject3);
		pSet->Add(pObject4);
		AssertSize(4, pSet->NumElements());

		//Removing from a set that ignores NULLs leaves NULLs behind.  Remove every Object except the one sorted first so the NULLs are after it.
		pLowest = pSet->UnsafeGet(0);
		TestSetObjectRemoveUnlessLowest(pSet, pObject1, pLowest);
		TestSetObjectRemoveUnlessLowest(pSet, pObject2, pLowest);
		TestSetObjectRemoveUnlessLowest(pSet, pObject3, pLowest);
		TestSetObjectRemoveUnlessLowest(pSet, pObject4, pLowest);
		AssertSize(1, pSet->NumElements());

		AssertTrue(pSet->Contains(pLowest));
		AssertTrue(pSet->Remove(pLowest));
		AssertSize(0, pSet->NumElements());
		AssertFalse(pSet->Contains(pLowest));

		pNew = OMalloc<CTestObject>();
		AssertTrue(pSet->Add(pObject2));
		AssertTrue(pSet->Add(pNew));
		AssertTrue(pSet->Add(pLowest));
		AssertSize(3, pSet->NumElements());
		AssertTrue(pSet->Contains(pObject2));
		AssertTrue(pSet->Contains(pNew));
		AssertTrue(pSet->Contains(pLowest));
		AssertFalse(pSet->Contains(pObject3));
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectOnStackKillContained(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		CSetObject			cSet;
		Ptr<CTestObject>	pTest1;
		Ptr<CTestObject>	pTest2;

		cSet.Init();
		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		cSet.Add(pTest1);
		cSet.Add(pTest2);
		AssertSize(2, cSet.NumElements());
		AssertSize(4, gcStackPointers.NumElements());

		pTest1->Kill();
		AssertTrue(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertSize(1, cSet.NumElements());
		AssertTrue(cSet.Contains(pTest2));
		AssertSize(2, gcStackPointers.NumElements());
		AssertLong(1, gcObjects.NumMemoryIndexes());

		cSet.Kill();
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
void TestSetObjectKillContainedHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;

	ObjectsInit();
	{
		Ptr<CRoot>				pRoot;
		Ptr<CSet<CTestObject>>	pSet;
		Ptr<CTestObject>		pTest1;
		Ptr<CTestObject>		pTest2;

		pRoot = ORoot();
		pSet = OMalloc<CSet<CTestObject>>();
		pRoot->Add(pSet);
		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pSet->Add(pTest1);
		pSet->Add(pTest2);
		AssertSize(2, pSet->NumElements());

		pTest1->Kill();
		AssertTrue(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertSize(1, pSet->NumElements());
		AssertTrue(pSet->Contains(pTest2));
		AssertSize(1, pTest2->NumStackFroms());
		AssertSize(1, pTest2->NumHeapFroms());
		gcObjects.ValidateObjectsConsistency();

		pTest2 = NULL;
		pSet = NULL;
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
void TestSetObjectRemoveDuringIterationSorted(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CSet<CTestObject>>		pSet;
		Ptr<CTestObject>			apObjects[6];
		Ptr<CTestObject>			pObject;
		SSetIterator				sIter;
		bool						bExists;
		size						i;
		size						uiVisited;
		size						auiTimesVisited[6];
		bool						abRemoved[6];

		pRoot = ORoot();
		pSet = OMalloc<CSet<CTestObject>>();
		pRoot->Add(pSet);
		for (i = 0; i < 6; i++)
		{
			apObjects[i] = OMalloc<CTestObject>();
			pSet->Add(apObjects[i]);
			auiTimesVisited[i] = 0;
			abRemoved[i] = false;
		}
		AssertSize(6, pSet->NumElements());

		//Remove every second Object visited.  The set must not be searched while iterating because searching can clean the NULLs left by removing.
		uiVisited = 0;
		pObject = pSet->StartIteration(&sIter, &bExists);
		while (bExists)
		{
			for (i = 0; i < 6; i++)
			{
				if (&apObjects[i] == &pObject)
				{
					auiTimesVisited[i]++;
					if (uiVisited % 2 == 0)
					{
						pSet->RemoveDuringIteration(&sIter);
						abRemoved[i] = true;
					}
					break;
				}
			}
			uiVisited++;

			pObject = pSet->Iterate(&sIter, &bExists);
		}

		AssertSize(6, uiVisited);
		for (i = 0; i < 6; i++)
		{
			AssertSize(1, auiTimesVisited[i]);
		}
		AssertSize(3, pSet->NumElements());
		for (i = 0; i < 6; i++)
		{
			AssertBool(!abRemoved[i], pSet->Contains(apObjects[i]));
			AssertSize(abRemoved[i] ? 0 : 1, apObjects[i]->NumHeapFroms());
		}
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectKillCyclic(void)
{
	ObjectsInit();

	Ptr<CTestObject>			pTest1;
	Ptr<CTestObject>			pTest2;
	Ptr<CTestObject>			pTest3;
	Ptr<CRoot>					pRoot;
	Ptr<CSetObject>				pSet;

	pTest1 = OMalloc<CTestObject>();
	pTest2 = OMalloc<CTestObject>();
	pTest1->mpTest = pTest2;
	pTest2->mpTest = pTest1;
	pTest3 = OMalloc<CTestObject>();
	pTest3->mpTest = pTest2;
	pSet = OMalloc<CSetObject>(false);
	pRoot = ORoot();
	pRoot->Add(pSet);
	pSet->Add(pTest3);

	AssertLong(6, gcObjects.NumMemoryIndexes());
	AssertInt(5, pTest1->GetDistToRoot());
	AssertInt(4, pTest2->GetDistToRoot());
	AssertInt(3, pTest3->GetDistToRoot());

	pSet->Kill();
	AssertLong(5, gcObjects.NumMemoryIndexes());
	AssertNull(&pSet);
	AssertInt(UNATTACHED_DIST_TO_ROOT, pTest1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pTest2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pTest3->GetDistToRoot());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectKillAll(void)
{
	ObjectsInit();

	Ptr<CPointerContainer>		pContainer1;
	Ptr<CPointerContainer>		pContainer2;
	Ptr<CTestObject>			pObject;
	Ptr<CRoot>					pRoot;
	Ptr<CSetObject>				pSet;

	pObject = OMalloc<CTestObject>();
	pContainer2 = OMalloc<CPointerContainer>(pObject);
	pContainer1 = OMalloc<CPointerContainer>(pContainer2);
	pSet = OMalloc<CSetObject>(false);
	pRoot = ORoot();
	pRoot->Add(pSet);
	pSet->Add(pContainer1);

	AssertLong(6, gcObjects.NumMemoryIndexes());
	AssertSize(1, pSet->NumElements());

	pSet->KillAll();
	AssertSize(0, pSet->NumElements());
	AssertLong(5, gcObjects.NumMemoryIndexes());
	AssertNotNull(&pSet);
	AssertNull(&pContainer1);
	AssertInt(UNATTACHED_DIST_TO_ROOT, pContainer2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pObject->GetDistToRoot());

	pSet->Kill();

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectRemoveAll(void)
{
	ObjectsInit();

	Ptr<CPointerContainer>		pContainer1;
	Ptr<CPointerContainer>		pContainer2;
	Ptr<CTestObject>			pObject;
	Ptr<CRoot>					pRoot;
	Ptr<CSetObject>				pSet;

	pObject = OMalloc<CTestObject>();
	pContainer2 = OMalloc<CPointerContainer>(pObject);
	pContainer1 = OMalloc<CPointerContainer>(pContainer2);
	pSet = OMalloc<CSetObject>(false);
	pRoot = ORoot();
	pRoot->Add(pSet);
	pSet->Add(pContainer1);

	AssertLong(6, gcObjects.NumMemoryIndexes());
	AssertSize(1, pSet->NumElements());

	pSet->RemoveAll();
	AssertSize(0, pSet->NumElements());
	AssertLong(6, gcObjects.NumMemoryIndexes());
	AssertNotNull(&pSet);
	AssertInt(UNATTACHED_DIST_TO_ROOT, pContainer1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pContainer2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pObject->GetDistToRoot());

	pSet->Kill();

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectSerialisation(void)
{
	ObjectsInit();
	TestSetObjectAddConstructors();

	Ptr<CPointerContainer>			pContainer1;
	Ptr<CPointerContainer>			pContainer2;
	Ptr<CTestObject>				pObject;
	Ptr<CSetObject>					pSet;
	CExternalObjectSerialiser		cSerialiser;
	CChunkFileObjectWriter			cWriter;
	CFileUtil						cFileUtil;
	char							szDirectory[] = "Output" _FS_ "TestSetObject";
	CExternalObjectDeserialiser		cDeserialiser;
	CChunkFileSystemObjectReader 	cReader;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pObject = OMalloc<CTestObject>();
	pContainer2 = OMalloc<CPointerContainer>(pObject);
	pContainer1 = OMalloc<CPointerContainer>(pContainer2);
	pSet = ONMalloc<CSetObject>("Burke", false);
	pSet->Add(pContainer1);
	pSet->Add(pContainer2);

	AssertLong(4, gcObjects.NumMemoryIndexes());
	AssertSize(2, pSet->NumElements());

	cWriter.Init(szDirectory, "", "File");
	cSerialiser.Init(&cWriter);
	cSerialiser.Write(&pSet);
	cSerialiser.Kill();
	cWriter.Kill();

	ObjectsKill(false);
	AssertNull(&pSet);
	ObjectsInit();

	cReader.Init(szDirectory, "File");
	cDeserialiser.Init(&cReader, false, &gcObjects);
	pSet = cDeserialiser.Read("Burke");
	AssertNotNull(&pSet);
	cDeserialiser.Kill();
	cReader.Kill();

	pSet = NULL;

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
void TestSetObjectObjectInternalSerialisation(size uiNumSetItems)
{
	CFileUtil			cFileUtil;
	CCodabase*			pcDatabase;
	CSequence*			pcSequence;
	char				szDirectory[] = "Output" _FS_ "SetObjectInternalSerialisation";
	bool				bResult;
	CIndexTreeMemory	cIndexTriIndices;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	CArrayInt	aiKeyNames;
	CRandom		cRandom;
	uint		uiCount;

	cIndexTriIndices.Init(IKR_Yes);

	cRandom.Init(32280975);
	aiKeyNames.Init();
	for (uiCount = 0; uiCount < uiNumSetItems; uiCount++)
	{
		aiKeyNames.Add(uiCount);
	}
	aiKeyNames.Shuffle(&cRandom);
	cRandom.Kill();
	AssertSize(uiNumSetItems, aiKeyNames.NumElements());

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CRoot>					pRoot;
		Ptr<CSetObject>				pSet;
		Ptr<CTestObject>			pKey;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;
		Ptr<CTestTriPointerObject>	pValue5;
		CPointer					pObject;
		bool						bResult;
		size						ui;
		OIndex						oi;
		STriOi*						psTriOi;
		STriOi						sTriOi;
		SSetIterator				sIter;
		bool						bExists;

		pRoot = ORoot();
		pSet = ONMalloc<CSetObject>("Set");
		pRoot->Add(pSet);

		gcObjects.DisableValidation();

		pValue1 = NULL;
		pValue2 = NULL;
		pValue3 = NULL;

		AssertTrue(pSet->IsMustSort());

		bResult = true;
		for (ui = 0; ui < uiNumSetItems; ui++)
		{
			if (ui % 2 == 0)
			{
				pValue3 = pValue2;
				pValue2 = pValue1;
				pValue1 = ONMalloc<CTestObject>(SizeToString(ui));
				bResult = pSet->Add(pValue1);
			}
			else
			{
				pValue5 = ONMalloc<CTestTriPointerObject>(SizeToString(ui));
				pValue5->mpObject1 = pValue1;
				pValue5->mpObject2 = pValue2;
				pValue5->mpObject3 = pValue3;
				bResult = pSet->Add(pValue5);

				oi = pValue5->GetIndex();
				psTriOi = (STriOi*)cIndexTriIndices.Put((uint8*)&oi, sizeof(OIndex), NULL, sizeof(STriOi));
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
		pObject = pSet->StartIteration(&sIter);
		while (pObject.IsNotNull())
		{
			oi = pObject->GetIndex();
			bExists = cIndexTriIndices.Get((uint8*)&oi, sizeof(OIndex), &sTriOi, NULL, sizeof(STriOi));
			if (!bExists)
			{
				bResult = StringCompare("CTestObject", pObject->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
			else
			{
				bResult = StringCompare("CTestTriPointerObject", pObject->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}

				pValue1 = gcObjects.Get(sTriOi.oi1);
				pValue2 = gcObjects.Get(sTriOi.oi2);
				pValue3 = gcObjects.Get(sTriOi.oi3);

				pValue5 = pObject;
				bResult  = &pValue5->mpObject1 == &pValue1;
				bResult &= &pValue5->mpObject2 == &pValue2;
				bResult &= &pValue5->mpObject3 == &pValue3;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
			pObject = pSet->Iterate(&sIter);
		}
		AssertTrue(bResult);
	}

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	AssertLong(uiNumSetItems + 3LL, pcDatabase->NumIndices());
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		TestSetObjectAddConstructors();
		AssertLong(uiNumSetItems + 3LL, pcDatabase->NumIndices());

		AssertTrue(gcObjects.Contains("Set"));

		Ptr<CSetObject>				pSet;
		Ptr<CHollowObject>			pHollow;
		bool						bResult;
		OIndex						oi;
		STriOi						sTriOi;
		SSetIterator				sIter;
		CPointer					pObject;
		bool						bExists;
		CIndexTreeMemory			cIndexObjectPointers;
		char*						szName;
		size						uiLength;
		CHollowObject*				pcHollow;
		CObject*					pcObject;

		pSet = gcObjects.Get("Set");
		AssertTrue(pSet.IsNotNull());
		AssertSize(uiNumSetItems, pSet->NumElements());

		cIndexObjectPointers.Init();

		AssertTrue(pSet->IsMustSort());
		AssertFalse(pSet->IsSorted());

		bResult = true;
		pObject = pSet->StartIteration(&sIter);
		AssertTrue(pSet->IsSorted());
		AssertTrue(pSet->CalculateIsSorted());
		while (pObject.IsNotNull())
		{
			bResult = pObject.IsHollow();
			if (!bResult)
			{
				AssertTrue(bResult);
				break;
			}
			pHollow = pObject;
			pcHollow = ((CHollowObject*)pHollow.Object());
			bResult = pcHollow->IsFatHollow();
			if (!bResult)
			{
				AssertTrue(bResult);
				break;
			}
			szName = pHollow.GetName();
			uiLength = StrLen(szName);
			
			cIndexObjectPointers.Put((uint8*)szName, uiLength, &pcHollow, sizeof(CHollowObject*));

			oi = pObject->GetIndex();
			bResult = pObject.IsHollow();
			if (bResult)
			{
				AssertFalse(bResult);
				break;
			}

			bExists = cIndexTriIndices.Get((uint8*)&oi, sizeof(OIndex), &sTriOi, NULL, sizeof(STriOi));
			if (!bExists)
			{
				bResult = StringCompare("CTestObject", pObject->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
			else
			{
				bResult = StringCompare("CTestTriPointerObject", pObject->ClassName()) == 0;
				if (!bResult)
				{
					AssertTrue(bResult);
					break;
				}
			}
			pObject = pSet->Iterate(&sIter);
		}
		AssertTrue(bResult);
		AssertTrue(pSet->IsSorted());
		AssertTrue(pSet->CalculateIsSorted());

		bResult = true;
		pObject = pSet->StartIteration(&sIter);
		while (pObject.IsNotNull())
		{
			bResult = pObject.IsHollow();
			if (bResult)
			{
				AssertFalse(bResult);
				break;
			}
			szName = pObject.GetName();
			uiLength = StrLen(szName);

			bExists = cIndexObjectPointers.Get((uint8*)szName, uiLength, &pcObject, NULL, sizeof(CObject*));
			bResult = pObject.Object() == pcObject;
			if (bResult)
			{
				AssertPointer(pObject.Object(), pcObject);
				break;
			}
			pObject = pSet->Iterate(&sIter);
		}
		AssertTrue(bResult);

		cIndexObjectPointers.Kill();
		aiKeyNames.Kill();
	}
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	cIndexTriIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectObjectMorphSetEntry(size uiNumElements)
{
	ObjectsInit();
	{
		size										ui;
		Ptr<CSetObject>								pSet;
		Ptr<CTestSaveableObject1>					pSaveable;
		CArrayTemplate<STestObjectFreedNotifier>	asFreedNotifiers;
		STestObjectFreedNotifier*					psFreedNotifier;
		Ptr<>										pMiddle;
		size										uiHalfWay;
		Ptr<CTestEmbeddedObjectWithFields>			pComplex;
		size										uiNumRemapped;
		Ptr<CRoot>									pRoot;
		size										uiNumSaveableObject;
		size										uiNumEmbeddedObject;
		SSetIterator								sIter;
		Ptr<>										pObject;
		bool										bResult;
		int											iCompare;
		Ptr<CTestObject>							pTestObject;

		uiHalfWay = uiNumElements / 2;
		asFreedNotifiers.Init();
		for (ui = 0; ui < uiNumElements; ui++)
		{
			asFreedNotifiers.Add();
		}

		gcObjects.DisableValidation();

		pRoot = ORoot();
		pSet = OMalloc<CSetObject>();
		pRoot->Add(pSaveable);
		for (ui = 0; ui < uiNumElements; ui++)
		{
			pSaveable = OMalloc<CTestSaveableObject1>();
			pSaveable->miInt = (int)(uiNumElements - ui);
			pSaveable->mszString.Init(SizeToString(ui));
			psFreedNotifier = asFreedNotifiers.Get(ui);
			pSaveable->mpObject = OMalloc<CTestObject>(psFreedNotifier);
			pSet->Add(pSaveable);

			if (ui == uiHalfWay)
			{
				pMiddle = pSaveable;
			}
		}
		pSaveable = NULL;

		AssertTrue(pSet->IsSorted());
		AssertTrue(pSet->CalculateIsSorted());

		AssertSize(1, pMiddle.NumHeapFroms());
		AssertSize(1, pMiddle.NumStackFroms());

		pComplex = ONMalloc<CTestEmbeddedObjectWithFields>("Complex");
		AssertSize(2 * uiNumElements + 1 + 3, gcObjects.NumMemoryIndexes());

		gcObjects.EnableValidation();

		uiNumRemapped = pMiddle.MorphInto(&pComplex);
		AssertSize(2, uiNumRemapped);
		AssertSize(2 * (uiNumElements - 1) + 1 + 3, gcObjects.NumMemoryIndexes());

		bResult = true;
		for (ui = 0; ui < uiNumElements; ui++)
		{
			psFreedNotifier = asFreedNotifiers.Get(ui);
			if (ui != uiHalfWay)
			{
				if (psFreedNotifier->bFreed)
				{
					AssertFalse(psFreedNotifier->bFreed);
					break;
				}
			}
			else
			{
				if (!psFreedNotifier->bFreed)
				{
					AssertTrue(psFreedNotifier->bFreed);
					break;
				}
			}
		}
		AssertTrue(bResult);

		AssertFalse(pSet->IsSorted());
		AssertFalse(pSet->CalculateIsSorted());

		pSet->Contains(pComplex);

		AssertTrue(pSet->IsSorted());
		AssertTrue(pSet->CalculateIsSorted());

		AssertPointer(&pComplex, &pMiddle);

		uiNumSaveableObject = 0;
		uiNumEmbeddedObject = 0;

		pObject = pSet->StartIteration(&sIter);
		while (pObject.IsNotNull())
		{
			iCompare = StringCompare("CTestSaveableObject1", pObject.ClassName());
			if (iCompare == 0)
			{
				uiNumSaveableObject++;
			}

			iCompare = StringCompare("CTestEmbeddedObjectWithFields", pObject.ClassName());
			if (iCompare == 0)
			{
				uiNumEmbeddedObject++;
			}

			pObject = pSet->Iterate(&sIter);
		}

		AssertSize(uiNumElements - 1, uiNumSaveableObject);
		AssertSize(1, uiNumEmbeddedObject);

		AssertSize(2 * (uiNumElements - 1) + 1 + 3, gcObjects.NumMemoryIndexes());

		pTestObject = OMalloc<CTestObject>();

		uiNumRemapped = pSet.MorphInto(&pTestObject);
		AssertSize(1, uiNumRemapped);
		AssertSize(1 + 3, gcObjects.NumMemoryIndexes());

		bResult = true;
		for (ui = 0; ui < uiNumElements; ui++)
		{
			psFreedNotifier = asFreedNotifiers.Get(ui);
			if (!psFreedNotifier->bFreed)
			{
				AssertTrue(psFreedNotifier->bFreed);
				break;
			}
		}
		AssertTrue(bResult);

		asFreedNotifiers.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetObjectEmbeddedOnHeapKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		Ptr<CTestEmbeddedSet>		pEmbedded;
		Ptr<CTestObject>			pTest;
		CTestObject*				pcTest;

		pEmbedded = OMalloc<CTestEmbeddedSet>(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		pEmbedded->mcSet.Add(pTest);
		pEmbedded->mcSet.Add(pTest);

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
void TestSetObjectEmbeddedOnStackKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		CTestEmbeddedSet		cEmbedded;
		Ptr<CTestObject>		pTest;
		CTestObject*			pcTest;

		cEmbedded.Init(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		cEmbedded.mcSet.Add(pTest);
		cEmbedded.mcSet.Add(pTest);

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
void TestSetObjectEmbeddedInObjectsKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();

	Ptr<CTestEmbeddedSet>		pEmbedded;
	Ptr<CTestObject>			pTest;
	CTestObject*				pcTest;
	Ptr<CRoot>					pRoot;

	pRoot = ORoot();
	pEmbedded = OMalloc<CTestEmbeddedSet>(&sFreedNotifier1);
	pRoot->Add(pEmbedded);
	pTest = OMalloc<CTestObject>(&sFreedNotifier2);
	pEmbedded->mcSet.Add(pTest);
	pEmbedded->mcSet.Add(pTest);

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
void TestSetObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestSetObjectAdd();
	TestSetObjectGet();
	TestSetObjectAddAll();
	TestSetObjectRemove();
	TestSetObjectRemoveSortedWithNulls();
	TestSetObjectOnStackKillContained();
	TestSetObjectKillContainedHeap();
	TestSetObjectRemoveDuringIterationSorted();
	TestSetObjectKillCyclic();
	TestSetObjectKillAll();
	TestSetObjectRemoveAll();
	TestSetObjectSerialisation();
	TestSetObjectObjectInternalSerialisation(4);
	TestSetObjectObjectInternalSerialisation(10000);
	TestSetObjectObjectMorphSetEntry(10);
	TestSetObjectObjectMorphSetEntry(2000);
	TestSetObjectEmbeddedOnHeapKill();
	TestSetObjectEmbeddedOnStackKill();
	TestSetObjectEmbeddedInObjectsKill();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

