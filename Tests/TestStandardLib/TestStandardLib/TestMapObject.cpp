#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/DataOrderers.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "TestLib/Assert.h"
#include "TestMapObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectAddConstructors(void)
{
	gcObjects.AddConstructor<CTestSaveableObject1>();
	gcObjects.AddConstructor<CTestObject>();
	gcObjects.AddConstructor<CPointerContainer>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectPut(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;
		Ptr<CTestObject>			pKey3;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pKey1 = ONMalloc<CTestObject>("shnork", &sNotifier4);
		pKey2 = ONMalloc<CTestObject>("shnerp", &sNotifier5);
		pKey3 = ONMalloc<CTestObject>("booger", &sNotifier6);
		pMap->Put(pKey1, OMalloc<CTestObject>(&sNotifier1));
		pMap->Put(pKey2, OMalloc<CTestObject>(&sNotifier2));
		pMap->Put(pKey3, OMalloc<CTestObject>(&sNotifier3));

		AssertSize(3, pMap->NumElements());
		AssertSize(9, gcObjects.NumMemoryIndexes());

		pMap = NULL;
		AssertSize(9, gcObjects.NumMemoryIndexes());

		pMap = pRoot->GetFirst<CMapObject>();
		pValue1 = pMap->Get(pKey3);
		AssertPointer(&sNotifier3, pValue1->mpsFreedNotifier);
		pValue2 = pMap->Get(pKey2);
		AssertPointer(&sNotifier2, pValue2->mpsFreedNotifier);
		pValue3 = pMap->Get(pKey1);
		AssertPointer(&sNotifier1, pValue3->mpsFreedNotifier);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectDetachInHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pMap->Put(ONMalloc<CTestObject>("shnork", &sNotifier4), OMalloc<CTestObject>(&sNotifier1));
		pMap->Put(ONMalloc<CTestObject>("shnerp", &sNotifier5), OMalloc<CTestObject>(&sNotifier2));
		pMap->Put(ONMalloc<CTestObject>("booger", &sNotifier6), OMalloc<CTestObject>(&sNotifier3));
		AssertFalse(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
		AssertFalse(sNotifier4.bFreed);
		AssertFalse(sNotifier5.bFreed);
		AssertFalse(sNotifier6.bFreed);

		AssertSize(3, pMap->NumElements());
		AssertSize(9, gcObjects.NumMemoryIndexes());

		pRoot->RemoveAll();
		pMap = NULL;
		AssertSize(2, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertTrue(sNotifier3.bFreed);
		AssertTrue(sNotifier4.bFreed);
		AssertTrue(sNotifier5.bFreed);
		AssertTrue(sNotifier6.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectKillOnStack(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;

	ObjectsInit();
	{
		CMapObject					cMap;

		cMap.Init();
		cMap.Put(ONMalloc<CTestObject>("shnork", &sNotifier4), OMalloc<CTestObject>(&sNotifier1));
		cMap.Put(ONMalloc<CTestObject>("shnerp", &sNotifier5), OMalloc<CTestObject>(&sNotifier2));
		cMap.Put(ONMalloc<CTestObject>("booger", &sNotifier6), OMalloc<CTestObject>(&sNotifier3));
		AssertFalse(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
		AssertFalse(sNotifier4.bFreed);
		AssertFalse(sNotifier5.bFreed);
		AssertFalse(sNotifier6.bFreed);

		AssertSize(3, cMap.NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		cMap.Kill();

		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertTrue(sNotifier3.bFreed);
		AssertTrue(sNotifier4.bFreed);
		AssertTrue(sNotifier5.bFreed);
		AssertTrue(sNotifier6.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


CMapEntry TestMapObjectReturnMapEntry(STestObjectFreedNotifier* psNotifier3, STestObjectFreedNotifier* psNotifier4)
{
	CMapEntry			cEntry2;
	Ptr<CTestObject>	pValue3;
	Ptr<CTestObject>	pKey1;

	pValue3 = OMalloc<CTestObject>(psNotifier3);
	pKey1 = ONMalloc<CTestObject>("shnork", psNotifier4);

	cEntry2.Init(pKey1, pValue3);
	AssertTrue(cEntry2.ValidatePointersEmbedded());
	return cEntry2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectMapEntry(void)
{
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;

	ObjectsInit();
	{
		CMapEntry					cEntry1;

		cEntry1 = TestMapObjectReturnMapEntry(&sNotifier3, &sNotifier4);
		AssertTrue(cEntry1.ValidatePointersEmbedded());
		AssertFalse(sNotifier3.bFreed);
		AssertFalse(sNotifier4.bFreed);
	}
	AssertTrue(sNotifier3.bFreed);
	AssertTrue(sNotifier4.bFreed);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectIterate(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;
	STestObjectFreedNotifier	sNotifier7;
	STestObjectFreedNotifier	sNotifier8;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;
		Ptr<CTestObject>			pKey3;
		Ptr<CTestObject>			pKey4;
		Ptr<CTestObject>			pKey5;
		SMapIterator				sIter;
		CMapEntry					cEntry;
#ifdef BUILD_64
		const char*					aszNames[5] = { "shnork", "shniqu", "shnerp", "booger", "bozzul" };
		void*						apvValues[5];
		bool						abFound[5] = { false, false, false, false, false };
		size						i;
		size						iExpected;
		char*						szName;
#endif // BUILD_64

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pValue3 = OMalloc<CTestObject>(&sNotifier1);
		pValue2 = OMalloc<CTestObject>(&sNotifier2);
		pValue1 = OMalloc<CTestObject>(&sNotifier3);
		pKey1 = ONMalloc<CTestObject>("shnork", &sNotifier4);
		pKey2 = ONMalloc<CTestObject>("shniqu", &sNotifier5);
		pKey3 = ONMalloc<CTestObject>("shnerp", &sNotifier6);
		pKey4 = ONMalloc<CTestObject>("booger", &sNotifier7);
		pKey5 = ONMalloc<CTestObject>("bozzul", &sNotifier8);
		pMap->Put(pKey1, pValue3);
		pMap->Put(pKey2, Null<CObject>());
		pMap->Put(pKey3, pValue2);
		pMap->Put(pKey4, pValue1);
		pMap->Put(pKey5, Null<CObject>());

		AssertSize(5, pMap->NumElements());
		AssertSize(11, gcObjects.NumMemoryIndexes());

		//Iteration order depends on allocation order in GeneralMemory.  Which should be stable.
		cEntry.Init();
#ifdef BUILD_64
		//The map is sorted by key address and 64 bit allocations are not in address order, so the entries can come in any order.
		apvValues[0] = &pValue3;
		apvValues[1] = NULL;
		apvValues[2] = &pValue2;
		apvValues[3] = &pValue1;
		apvValues[4] = NULL;
		cEntry = pMap->StartIteration(&sIter);
		for (i = 0; i < 5; i++)
		{
			AssertTrue(cEntry.Exists());
			szName = cEntry.Key().GetName();
			for (iExpected = 0; iExpected < 5; iExpected++)
			{
				if (strcmp(aszNames[iExpected], szName) == 0)
				{
					break;
				}
			}
			AssertTrue(iExpected < 5);
			AssertFalse(abFound[iExpected]);
			abFound[iExpected] = true;
			AssertPointer(apvValues[iExpected], &cEntry.Value());
			if (i < 4)
			{
				cEntry = pMap->Iterate(&sIter);
			}
		}
#else
		cEntry = pMap->StartIteration(&sIter);
		AssertTrue(cEntry.Exists());
		AssertString("shnork", cEntry.Key().GetName());
		AssertPointer(&pValue3, &cEntry.Value());

		cEntry = pMap->Iterate(&sIter);
		AssertTrue(cEntry.Exists());
		AssertString("shniqu", cEntry.Key().GetName());
		AssertNull(&cEntry.Value());

		cEntry = pMap->Iterate(&sIter);
		AssertTrue(cEntry.Exists());
		AssertString("shnerp", cEntry.Key().GetName());
		AssertPointer(&pValue2, &cEntry.Value());

		cEntry = pMap->Iterate(&sIter);
		AssertTrue(cEntry.Exists());
		AssertString("booger", cEntry.Key().GetName());
		AssertPointer(&pValue1, &cEntry.Value());

		cEntry = pMap->Iterate(&sIter);		
		AssertTrue(cEntry.Exists());
		AssertString("bozzul", cEntry.Key().GetName());
		AssertNull(&cEntry.Value());
#endif // BUILD_64

		cEntry = pMap->Iterate(&sIter);
		AssertFalse(cEntry.Exists());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectRemove(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;
		Ptr<CTestObject>			pKey3;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pKey1 = ONMalloc<CTestObject>("shnork", &sNotifier4);
		pKey2 = ONMalloc<CTestObject>("shnerp", &sNotifier5);
		pKey3 = ONMalloc<CTestObject>("booger", &sNotifier6);
		pMap->Put(pKey1, OMalloc<CTestObject>(&sNotifier3));
		pMap->Put(pKey2, OMalloc<CTestObject>(&sNotifier2));
		pMap->Put(pKey3, OMalloc<CTestObject>(&sNotifier1));

		AssertSize(3, pMap->NumElements());
		AssertSize(9, gcObjects.NumMemoryIndexes());

		pMap->Remove(pKey2);
		AssertSize(2, pMap->NumElements());
		AssertSize(8, gcObjects.NumMemoryIndexes());
		AssertFalse(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
		AssertFalse(sNotifier4.bFreed);
		AssertFalse(sNotifier5.bFreed);
		AssertFalse(sNotifier6.bFreed);
		gcObjects.ValidateObjectsConsistency();
		pKey2 = NULL;
		AssertTrue(sNotifier5.bFreed);

		pValue1 = pMap->Get(pKey3);
		pMap->Remove(pKey3);
		AssertSize(1, pMap->NumElements());
		AssertSize(7, gcObjects.NumMemoryIndexes());
		AssertFalse(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
		AssertFalse(sNotifier4.bFreed);
		AssertTrue(sNotifier5.bFreed);
		AssertFalse(sNotifier6.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectDetachOnStack(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;

	ObjectsInit();
	{
		bool						bExists;
		SMapIterator				sIter;
		CTestObject*				pcKey;
		CTestObject*				pcValue;

		{
			CMapObject			cMap;

			cMap.Init();
			cMap.Put(ONMalloc<CTestObject>("shnork", &sNotifier4), OMalloc<CTestObject>(&sNotifier1));
			cMap.Put(ONMalloc<CTestObject>("shnerp", &sNotifier5), OMalloc<CTestObject>(&sNotifier2));
			cMap.Put(ONMalloc<CTestObject>("booger", &sNotifier6), OMalloc<CTestObject>(&sNotifier3));
			AssertFalse(sNotifier1.bFreed);
			AssertFalse(sNotifier2.bFreed);
			AssertFalse(sNotifier3.bFreed);
			AssertFalse(sNotifier4.bFreed);
			AssertFalse(sNotifier5.bFreed);
			AssertFalse(sNotifier6.bFreed);
			AssertSize(3, cMap.NumElements());
			AssertSize(6, gcObjects.NumMemoryIndexes());

			bExists = cMap.GetUnknownMap()->StartIteration(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
			while (bExists)
			{
				AssertIntHex(0x7070707, pcKey->mi);
				AssertIntHex(0x7070707, pcValue->mi);
				bExists = cMap.GetUnknownMap()->Iterate(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
			}
		}

		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier1.bFreed);
		AssertTrue(sNotifier2.bFreed);
		AssertTrue(sNotifier3.bFreed);
		AssertTrue(sNotifier4.bFreed);
		AssertTrue(sNotifier5.bFreed);
		AssertTrue(sNotifier6.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectGetPointerTos(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;
	STestObjectFreedNotifier	sNotifier7;
	STestObjectFreedNotifier	sNotifier8;

	ObjectsInit();
	{
		Ptr<CRoot>							pRoot;
		Ptr<CMapObject>						pMap;
		Ptr<CTestObject>					pValue1;
		Ptr<CTestObject>					pValue2;
		Ptr<CTestObject>					pValue3;
		Ptr<CTestObject>					pKey1;
		Ptr<CTestObject>					pKey2;
		Ptr<CTestObject>					pKey3;
		Ptr<CTestObject>					pKey4;
		Ptr<CTestObject>					pKey5;
		CArrayEmbeddedObjectPtr		apcTos;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pValue3 = OMalloc<CTestObject>(&sNotifier1);
		pValue2 = OMalloc<CTestObject>(&sNotifier2);
		pValue1 = OMalloc<CTestObject>(&sNotifier3);
		pKey1 = ONMalloc<CTestObject>("shnork", &sNotifier4);
		pKey2 = ONMalloc<CTestObject>("shniqu", &sNotifier5);
		pKey3 = ONMalloc<CTestObject>("shnerp", &sNotifier6);
		pKey4 = ONMalloc<CTestObject>("booger", &sNotifier7);
		pKey5 = ONMalloc<CTestObject>("bozzul", &sNotifier8);

		pMap->Put(pKey1, pValue3);
		pMap->Put(pKey2, Null<CObject>());
		pMap->Put(pKey3, pValue2);
		pMap->Put(pKey4, pValue1);
		pMap->Put(pKey5, Null<CObject>());

		AssertSize(5, pMap->NumElements());
		AssertSize(11, gcObjects.NumMemoryIndexes());

		apcTos.Init();
		pMap->GetPointerTos(&apcTos);
		AssertSize(8, apcTos.NumElements());
		AssertTrue(apcTos.ContainsVoidPtr(&pValue1));
		AssertTrue(apcTos.ContainsVoidPtr(&pValue2));
		AssertTrue(apcTos.ContainsVoidPtr(&pValue3));
		AssertTrue(apcTos.ContainsVoidPtr(&pKey1));
		AssertTrue(apcTos.ContainsVoidPtr(&pKey2));
		AssertTrue(apcTos.ContainsVoidPtr(&pKey3));
		AssertTrue(apcTos.ContainsVoidPtr(&pKey4));
		AssertTrue(apcTos.ContainsVoidPtr(&pKey5));
		apcTos.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectPointerFromsHeap(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;
	STestObjectFreedNotifier	sNotifier7;
	STestObjectFreedNotifier	sNotifier8;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;
		Ptr<CTestObject>			pKey3;
		Ptr<CTestObject>			pKey4;
		Ptr<CTestObject>			pKey5;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pValue3 = OMalloc<CTestObject>(&sNotifier1);
		pValue2 = OMalloc<CTestObject>(&sNotifier2);
		pValue1 = OMalloc<CTestObject>(&sNotifier3);
		pKey1 = ONMalloc<CTestObject>("shnork", &sNotifier4);
		pKey2 = ONMalloc<CTestObject>("shnerp", &sNotifier5);
		pKey3 = ONMalloc<CTestObject>("booger", &sNotifier6);
		pMap->Put(pKey1, pValue1);
		pMap->Put(pKey2, pValue2);
		pMap->Put(pKey3, pValue3);
		AssertSize(1, pValue1->NumHeapFroms());
		AssertSize(1, pValue2->NumHeapFroms());
		AssertSize(1, pValue3->NumHeapFroms());
		AssertSize(1, pKey1->NumHeapFroms());
		AssertSize(1, pKey2->NumHeapFroms());
		AssertSize(1, pKey3->NumHeapFroms());
		AssertSize(3, pMap->NumElements());

		pMap->Remove(pKey3);
		AssertSize(1, pValue1->NumHeapFroms());
		AssertSize(1, pValue2->NumHeapFroms());
		AssertSize(0, pValue3->NumHeapFroms());
		AssertSize(1, pKey1->NumHeapFroms());
		AssertSize(1, pKey2->NumHeapFroms());
		AssertSize(0, pKey3->NumHeapFroms());
		AssertSize(2, pMap->NumElements());

		pMap->RemoveAll();
		AssertSize(0, pValue1->NumHeapFroms());
		AssertSize(0, pValue2->NumHeapFroms());
		AssertSize(0, pValue3->NumHeapFroms());
		AssertSize(0, pKey1->NumHeapFroms());
		AssertSize(0, pKey2->NumHeapFroms());
		AssertSize(0, pKey3->NumHeapFroms());
		AssertSize(0, pMap->NumElements());

		pKey4 = ONMalloc<CTestObject>("abcde", &sNotifier7);
		pMap->Put(pKey4, pValue1);
		AssertSize(1, pValue1->NumHeapFroms());
		AssertSize(0, pValue2->NumHeapFroms());
		AssertSize(0, pValue3->NumHeapFroms());
		AssertSize(1, pKey4->NumHeapFroms());
		AssertSize(1, pMap->NumElements());

		pKey5 = ONMalloc<CTestObject>("abcdf", &sNotifier8);
		pMap->Put(pKey5, pValue1);
		AssertSize(2, pValue1->NumHeapFroms());
		AssertSize(0, pValue2->NumHeapFroms());
		AssertSize(0, pValue3->NumHeapFroms());
		AssertSize(1, pKey4->NumHeapFroms());
		AssertSize(1, pKey5->NumHeapFroms());
		AssertSize(2, pMap->NumElements());

		pMap->Remove(pKey4);
		AssertSize(1, pValue1->NumHeapFroms());
		AssertSize(0, pValue2->NumHeapFroms());
		AssertSize(0, pValue3->NumHeapFroms());
		AssertSize(0, pKey4->NumHeapFroms());
		AssertSize(1, pKey5->NumHeapFroms());
		AssertSize(1, pMap->NumElements());

		pMap->Remove(pKey5);
		AssertSize(0, pValue1->NumHeapFroms());
		AssertSize(0, pValue2->NumHeapFroms());
		AssertSize(0, pValue3->NumHeapFroms());
		AssertSize(0, pKey4->NumHeapFroms());
		AssertSize(0, pKey5->NumHeapFroms());
		AssertSize(0, pMap->NumElements());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectPointerFromsStack(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;
	STestObjectFreedNotifier	sNotifier7;
	STestObjectFreedNotifier	sNotifier8;

	ObjectsInit();
	{
		CMapObject					cMap;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;
		Ptr<CTestObject>			pKey3;
		Ptr<CTestObject>			pKey4;
		Ptr<CTestObject>			pKey5;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;
		CArrayStackPointerE32			apcFroms;
		SStackPointer*				psStack;

		cMap.Init();
		pValue3 = OMalloc<CTestObject>(&sNotifier1);
		pValue2 = OMalloc<CTestObject>(&sNotifier2);
		pValue1 = OMalloc<CTestObject>(&sNotifier3);
		pKey1 = ONMalloc<CTestObject>("shnork", &sNotifier4);
		pKey2 = ONMalloc<CTestObject>("shnerp", &sNotifier5);
		pKey3 = ONMalloc<CTestObject>("booger", &sNotifier6);
		cMap.Put(pKey1, pValue1);
		cMap.Put(pKey2, pValue2);
		cMap.Put(pKey3, pValue3);
		AssertSize(2, pValue1->NumStackFroms());
		AssertSize(2, pValue2->NumStackFroms());
		AssertSize(2, pValue3->NumStackFroms());
		AssertSize(2, pKey1->NumStackFroms());
		AssertSize(2, pKey2->NumStackFroms());
		AssertSize(2, pKey3->NumStackFroms());
		AssertSize(3, cMap.NumElements());
		apcFroms.Init();
		pValue1->GetStackFroms(&apcFroms);
		psStack = apcFroms.Get(0);
		AssertTrue(psStack->meType == SPT_Pointer);
		AssertPointer(pValue1.This(), psStack->u.pcPointer);
		psStack = apcFroms.Get(1);
		AssertTrue(psStack->meType == SPT_Collection);
		AssertPointer(&cMap, psStack->u.pcCollection);
		apcFroms.Kill();

		cMap.Remove(pKey3);
		AssertSize(2, pValue1->NumStackFroms());
		AssertSize(2, pValue2->NumStackFroms());
		AssertSize(1, pValue3->NumStackFroms());
		AssertSize(2, pKey1->NumStackFroms());
		AssertSize(2, pKey2->NumStackFroms());
		AssertSize(1, pKey3->NumStackFroms());
		AssertSize(2, cMap.NumElements());

		cMap.RemoveAll();
		AssertSize(1, pValue1->NumStackFroms());
		AssertSize(1, pValue2->NumStackFroms());
		AssertSize(1, pValue3->NumStackFroms());
		AssertSize(1, pKey1->NumStackFroms());
		AssertSize(1, pKey2->NumStackFroms());
		AssertSize(1, pKey3->NumStackFroms());
		AssertSize(0, cMap.NumElements());

		pKey4 = ONMalloc<CTestObject>("abcde", &sNotifier7);
		cMap.Put(pKey4, pValue1);
		AssertSize(2, pValue1->NumStackFroms());
		AssertSize(1, pValue2->NumStackFroms());
		AssertSize(1, pValue3->NumStackFroms());
		AssertSize(2, pKey4->NumStackFroms());
		AssertSize(1, cMap.NumElements());

		pKey5 = ONMalloc<CTestObject>("abcdf", &sNotifier8);
		cMap.Put(pKey5, pValue1);
		AssertSize(3, pValue1->NumStackFroms());
		AssertSize(1, pValue2->NumStackFroms());
		AssertSize(1, pValue3->NumStackFroms());
		AssertSize(2, pKey4->NumStackFroms());
		AssertSize(2, pKey5->NumStackFroms());
		AssertSize(2, cMap.NumElements());

		cMap.Remove(pKey4);
		AssertSize(2, pValue1->NumStackFroms());
		AssertSize(1, pValue2->NumStackFroms());
		AssertSize(1, pValue3->NumStackFroms());
		AssertSize(1, pKey4->NumStackFroms());
		AssertSize(2, pKey5->NumStackFroms());
		AssertSize(1, cMap.NumElements());

		cMap.Remove(pKey5);
		AssertSize(1, pValue1->NumStackFroms());
		AssertSize(1, pValue2->NumStackFroms());
		AssertSize(1, pValue3->NumStackFroms());
		AssertSize(1, pKey4->NumStackFroms());
		AssertSize(1, pKey5->NumStackFroms());
		AssertSize(0, cMap.NumElements());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectPutOverwrite(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		bool						bResult;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pKey1 = ONMalloc<CTestObject>("1", &sNotifier3);
		pValue2 = OMalloc<CTestObject>(&sNotifier2);
		pValue1 = OMalloc<CTestObject>(&sNotifier1);
		bResult = pMap->Put(pKey1, pValue2);
		gcObjects.ValidateObjectsConsistency();
		AssertTrue(bResult);
		AssertSize(1, pMap->NumElements());
		AssertSize(6, gcObjects.NumMemoryIndexes());
		pValue2 = NULL;
		AssertFalse(sNotifier2.bFreed);

		bResult = pMap->Put(pKey1, pValue1);
		gcObjects.ValidateObjectsConsistency();
		AssertTrue(bResult);
		AssertSize(1, pMap->NumElements());
		AssertSize(5, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifier2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectExternalSerialisation()
{
	CFileUtil	cFileUtil;
	char		szDirectory[] = "Output" _FS_ "MapObjectExternalSerialisation";

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	DataOrderersInit();
	ObjectsInit();
	TestMapObjectAddConstructors();
	{
		Ptr<CPointerContainer>			pContainer1;
		Ptr<CPointerContainer>			pContainer2;
		Ptr<CPointerContainer>			pContainer3;
		Ptr<CTestObject>				pObject;
		Ptr<CMapObject>					pMapObject;
		CExternalObjectSerialiser		cSerialiser;
		CChunkFileObjectWriter			cWriter;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		pObject = OMalloc<CTestObject>();
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pContainer3 = OMalloc<CPointerContainer>(Null());
		pMapObject = ONMalloc<CMapObject>("Orbis");
		pMapObject->Put(OMalloc<CString>("Hun"), pContainer1);
		pMapObject->Put(OMalloc<CString>("Koya"), pContainer2);
		pMapObject->Put(OMalloc<CString>("Tokka"), pContainer3);

		AssertLong(8, gcObjects.NumMemoryIndexes());
		AssertSize(3, pMapObject->NumElements());

		cWriter.Init(szDirectory, "", "File");
		cSerialiser.Init(&cWriter);
		cSerialiser.Write(&pMapObject);
		cSerialiser.Kill();
		cWriter.Kill();
	}
	ObjectsKill(false);
	ObjectsInit();
	{
		Ptr<CString>					pKey1;
		Ptr<CString>					pKey2;
		Ptr<CString>					pKey3;
		Ptr<CPointerContainer>			pValue1;
		Ptr<CPointerContainer>			pValue2;
		Ptr<CPointerContainer>			pValue3;
		Ptr<CMapObject>					pMapObject;
		CChunkFileObjectWriter			cWriter;
		CExternalObjectDeserialiser		cDeserialiser;
		CChunkFileSystemObjectReader 	cReader;
		SMapIterator					sIter;
		CMapEntry						cEntry;

		cReader.Init(szDirectory, "File");
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pMapObject = cDeserialiser.Read("Orbis");
		AssertNotNull(&pMapObject);
		AssertLong(8, gcObjects.NumMemoryIndexes());
		AssertSize(3, pMapObject->NumElements());
		cDeserialiser.Kill();
		cReader.Kill();

		cEntry = pMapObject->StartIteration(&sIter);
		pKey1 = cEntry.Key();
		AssertTrue(pKey1.IsNotNull());
		AssertString("Hun", pKey1->Text());
		cEntry = pMapObject->Iterate(&sIter);
		pKey2 = cEntry.Key();
		AssertTrue(pKey2.IsNotNull());
		AssertString("CString", pKey1->ClassName());
		AssertString("Koya", pKey2->Text());
		cEntry = pMapObject->Iterate(&sIter);
		pKey3 = cEntry.Key();
		AssertTrue(pKey3.IsNotNull());
		AssertString("CString", pKey2->ClassName());
		AssertString("CString", pKey3->ClassName());
		AssertString("Tokka", pKey3->Text());
		pValue1 = pMapObject->Get(pKey1);
		pValue2 = pMapObject->Get(pKey2);
		pValue3 = pMapObject->Get(pKey3);
		AssertTrue(pValue1.IsNotNull());
		AssertTrue(pValue2.IsNotNull());
		AssertTrue(pValue3.IsNotNull());
		AssertString("CPointerContainer", pValue1->ClassName());
		AssertString("CPointerContainer", pValue2->ClassName());
		AssertString("CPointerContainer", pValue3->ClassName());

		Ptr<CPointerContainer>	pContainer1;
		Ptr<CPointerContainer>	pContainer2;
		Ptr<CPointerContainer>	pContainer3;
		Ptr<CTestObject>		pObject;

		pContainer1 = pValue1;
		pContainer2 = pValue2;
		pContainer3 = pValue3;
		AssertTrue(pContainer1->mp.IsNotNull());
		AssertTrue(pContainer2->mp.IsNotNull());
		AssertTrue(pContainer3->mp.IsNull());
		AssertString("CPointerContainer", pContainer1->mp->ClassName());
		AssertString("CTestObject", pContainer2->mp->ClassName());
		AssertPointer(&pContainer1->mp, &pContainer2);

		pMapObject = NULL;
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
void TestMapObjectConstructorExists(void)
{
	CConstructors*	pcConstructors;
	CMapObject*		pcMapObject;

	ObjectsInit();

	pcConstructors = gcObjects.GetConstructors();
	pcMapObject = pcConstructors->Get<CMapObject>();
	AssertNotNull(pcMapObject);
	AssertString("CMapObject", pcMapObject->ClassName());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectClassExists(void)
{
	CClasses*				pcClasses;
	CClass*					pcClass;
	CMapUnknownUnknown*		pcMapUnknownUnknown;

	ObjectsInit();
	{
		CMapObject		cMapObject;
		CMapPtrPtr*		pcMapPtrPtr;

		pcClasses = gcObjects.GetClasses();
		pcClass = pcClasses->Get(cMapObject.ClassName());
		AssertNotNull(pcClass);
		AssertString("CMapObject", pcClass->GetName());
		pcMapUnknownUnknown = cMapObject.GetUnknownMap();
		AssertNotNull(pcMapUnknownUnknown);
		pcMapPtrPtr = pcMapUnknownUnknown->GetPointerMap();
		AssertNotNull(pcMapPtrPtr);
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectMorphInto(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;
	STestObjectFreedNotifier	sNotifier5;
	STestObjectFreedNotifier	sNotifier6;
	STestObjectFreedNotifier	sNotifier7;
	STestObjectFreedNotifier	sNotifier8;
	STestObjectFreedNotifier	sNotifier9;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap1;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;
		Ptr<CTestObject>			pKey3;
		Ptr<CTestObject>			pKey4;
		Ptr<CTestObject>			pKey5;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestTriPointerObject>	pValue2;
		Ptr<CTestObject>			pValue3;
		Ptr<CTestTriPointerObject>	pValue4;
		bool						bResult;
		size						uiRemapped;

		pRoot = ORoot();
		pMap1 = OMalloc<CMapObject>();
		pRoot->Add(pMap1);

		pValue1 = OMalloc<CTestObject>(&sNotifier1);
		pValue2 = OMalloc<CTestTriPointerObject>(&sNotifier2);
		pValue3 = OMalloc<CTestObject>(&sNotifier3);
		pValue4 = OMalloc<CTestTriPointerObject>(&sNotifier4);
		pKey1 = ONMalloc<CTestObject>("1", &sNotifier5);
		pKey2 = ONMalloc<CTestObject>("2", &sNotifier6);
		bResult = pMap1->Put(pKey1, pValue1);
		AssertTrue(bResult);
		bResult = pMap1->Put(pKey2, pValue2);
		
		CMapObject		cMap2;

		pKey3 = ONMalloc<CTestObject>("3", &sNotifier7);
		pKey4 = ONMalloc<CTestObject>("4", &sNotifier8);
		pKey5 = ONMalloc<CTestObject>("5", &sNotifier9);

		cMap2.Init();
		cMap2.Put(pKey3, pValue2);
		cMap2.Put(pKey4, pValue3);
		cMap2.Put(pKey5, pValue2);

		pValue4->mpObject1 = pValue2;
		pValue4->mpObject2 = pValue2;

		pValue2->mpObject1 = pValue1;

		Ptr<CTestObjectWithFields>	pValue5;
		CPointer					pPtr;

		pValue5 = OMalloc<CTestObjectWithFields>();

		AssertFalse(sNotifier2.bFreed);
		uiRemapped = pValue2.MorphInto(&pValue5);
		//2 (cMap2) +
		//1 (pMap1) +
		//1 (pValue2)  +
		//2 (pValue4->mpObject1, pValue4->mpObject2) =
		//6
		AssertSize(6, uiRemapped);
		AssertPointer(&pValue5, &pValue2);
		AssertString("CTestObjectWithFields", pValue5->ClassName());
		AssertTrue(sNotifier2.bFreed);
		AssertPointer(&pValue5, &pValue4->mpObject1);
		AssertPointer(&pValue5, &pValue4->mpObject2);
		pPtr = pMap1->Get(pKey2);
		AssertPointer(&pValue5, &pPtr);
		pPtr = cMap2.Get(pKey3);
		AssertPointer(&pValue5, &pPtr);
		pPtr = cMap2.Get(pKey4);
		AssertPointer(&pValue3, &pPtr);
		pPtr = cMap2.Get(pKey5);
		AssertPointer(&pValue5, &pPtr);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectInternalSerialisation(size uiNumMapItems)
{
	CFileUtil	cFileUtil;
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	char		szDirectory[] = "Output" _FS_ "MapObjectInternalSerialisation";
	bool		bResult;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CRoot>					pRoot;
		Ptr<CMapObject>				pMap;
		Ptr<CTestObject>			pKey;
		Ptr<CTestObject>			pValue1;
		Ptr<CTestObject>			pValue2;
		Ptr<CTestObject>			pValue3;
		Ptr<CTestTriPointerObject>	pValue5;
		bool						bResult;
		uint						uiCount;
		size						ui;
		SMapIterator				sIter;
		CMapEntry					cEntry;

		pRoot = ORoot();
		pMap = ONMalloc<CMapObject>("Map");
		pRoot->Add(pMap);

		gcObjects.DisableValidation();

		bResult = true;
		for (ui = 0; ui < uiNumMapItems; ui++)
		{
			if (ui % 2 == 0)
			{
				pValue3 = pValue2;
				pValue2 = pValue1;
				pValue1 = OMalloc<CTestObject>();
			}
			else
			{
				pValue5 = OMalloc<CTestTriPointerObject>();
				pValue5->mpObject1 = pValue1;
				pValue5->mpObject2 = pValue2;
				pValue5->mpObject2 = pValue3;

			}
			pKey = ONMalloc<CTestObject>(SizeToString(ui));
			bResult = pMap->Put(pKey, pValue1);
			if (!bResult)
			{
				AssertTrue(bResult);
				break;
			}
		}
		AssertTrue(bResult);

		uiCount = 0;
		cEntry.Init();
		cEntry = pMap->StartIteration(&sIter);
		while (cEntry.Exists())
		{
			uiCount++;
			cEntry = pMap->Iterate(&sIter);
		}

		AssertSize(uiNumMapItems, uiCount);
	}

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	AssertLong(uiNumMapItems + uiNumMapItems / 2 + 3, pcDatabase->NumIndices());
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		CArrayInt	aiKeyNames;
		CRandom		cRandom;
		uint		uiCount;
		size		ui;
		bool		bResult;

		cRandom.Init(89073456);
		aiKeyNames.Init();
		for (uiCount = 0; uiCount < uiNumMapItems; uiCount++)
		{
			aiKeyNames.Add(uiCount);
		}
		aiKeyNames.Shuffle(&cRandom);
		cRandom.Kill();
		AssertSize(uiNumMapItems, aiKeyNames.NumElements());
		
		TestMapObjectAddConstructors();
		AssertLong(uiNumMapItems + uiNumMapItems / 2 + 3, pcDatabase->NumIndices());

		AssertTrue(gcObjects.Contains("Map"));

		Ptr<CMapObject>		pMap;
		Ptr<CTestObject>	pKey;
		CPointer			pValue;
		uint				uiName;
		SMapIterator		sIter;
		CMapEntry			cEntry;

		pMap = gcObjects.Get("Map");
		AssertTrue(pMap.IsNotNull());
		AssertSize(uiNumMapItems, pMap->NumElements());
		AssertFalse(pMap->IsSorted());

		bResult = true;
		for (ui = 0; ui < uiNumMapItems; ui++)
		{
			uiName = aiKeyNames.GetValue(ui);
			pKey = gcObjects.Get(SizeToString(uiName));
			bResult = pKey.IsNotNull();
			if (!bResult)
			{
				AssertTrue(bResult);
				break;
			}
			pValue = pMap->Get(pKey);
			bResult = pValue.IsNotNull();
			if (!bResult)
			{
				AssertTrue(bResult);
				break;
			}
		}
		AssertTrue(bResult);
		aiKeyNames.Kill();

		uiCount = 0;
		cEntry.Init();
		cEntry = pMap->StartIteration(&sIter);
		while (cEntry.Exists())
		{
			uiCount++;
			cEntry = pMap->Iterate(&sIter);
		}

		AssertSize(uiNumMapItems, uiCount);
	}
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectEmbeddedOnHeapKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;
	STestObjectFreedNotifier	sFreedNotifier3;
	STestObjectFreedNotifier	sFreedNotifier4;

	ObjectsInit();
	{
		Ptr<CTestEmbeddedMap>		pEmbedded;
		Ptr<CTestObject>			pTest;
		CTestObject*				pcTest;
		Ptr<CTestObject>			pKey1;
		Ptr<CTestObject>			pKey2;

		pEmbedded = OMalloc<CTestEmbeddedMap>(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		pKey1 = OMalloc<CTestObject>(&sFreedNotifier3);
		pKey2 = OMalloc<CTestObject>(&sFreedNotifier4);
		pEmbedded->mcMap.Put(pKey1, pTest);
		pEmbedded->mcMap.Put(pKey2, pTest);

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
void TestMapObjectEmbeddedOnStackKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;
	STestObjectFreedNotifier	sFreedNotifier3;
	STestObjectFreedNotifier	sFreedNotifier4;

	ObjectsInit();
	{
		CTestEmbeddedMap		cEmbedded;
		Ptr<CTestObject>		pTest;
		CTestObject*			pcTest;
		Ptr<CTestObject>		pKey1;
		Ptr<CTestObject>		pKey2;

		cEmbedded.Init(&sFreedNotifier1);
		pTest = OMalloc<CTestObject>(&sFreedNotifier2);
		pKey1 = OMalloc<CTestObject>(&sFreedNotifier3);
		pKey2 = OMalloc<CTestObject>(&sFreedNotifier4);
		cEmbedded.mcMap.Put(pKey1, pTest);
		cEmbedded.mcMap.Put(pKey2, pTest);

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
void TestMapObjectEmbeddedInObjectsKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;
	STestObjectFreedNotifier	sFreedNotifier3;
	STestObjectFreedNotifier	sFreedNotifier4;

	ObjectsInit();

	Ptr<CTestEmbeddedMap>		pEmbedded;
	Ptr<CTestObject>			pTest;
	CTestObject*				pcTest;
	Ptr<CTestObject>			pKey1;
	Ptr<CTestObject>			pKey2;
	Ptr<CRoot>					pRoot;

	pRoot = ORoot();
	pEmbedded = OMalloc<CTestEmbeddedMap>(&sFreedNotifier1);
	pRoot->Add(pEmbedded);
	pTest = OMalloc<CTestObject>(&sFreedNotifier2);
	pKey1 = OMalloc<CTestObject>(&sFreedNotifier3);
	pKey2 = OMalloc<CTestObject>(&sFreedNotifier4);
	pEmbedded->mcMap.Put(pKey1, pTest);
	pEmbedded->mcMap.Put(pKey2, pTest);

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
void TestMapObjectOnStackKillContained(void)
{
	STestObjectFreedNotifier	sNotifierKey1;
	STestObjectFreedNotifier	sNotifierValue1;
	STestObjectFreedNotifier	sNotifierKey2;
	STestObjectFreedNotifier	sNotifierValue2;

	ObjectsInit();
	{
		CMapObject			cMap;
		Ptr<CTestObject>	pKey1;
		Ptr<CTestObject>	pValue1;
		Ptr<CTestObject>	pKey2;
		Ptr<CTestObject>	pValue2;

		cMap.Init();
		pKey1 = OMalloc<CTestObject>(&sNotifierKey1);
		pValue1 = OMalloc<CTestObject>(&sNotifierValue1);
		pKey2 = OMalloc<CTestObject>(&sNotifierKey2);
		pValue2 = OMalloc<CTestObject>(&sNotifierValue2);
		cMap.Put(pKey1, pValue1);
		cMap.Put(pKey2, pValue2);
		AssertSize(2, cMap.NumElements());
		AssertSize(8, gcStackPointers.NumElements());

		//Kill a key.  The whole entry goes.
		pKey1->Kill();
		AssertTrue(sNotifierKey1.bFreed);
		AssertFalse(sNotifierValue1.bFreed);
		AssertSize(1, cMap.NumElements());
		AssertSize(1, pValue1->NumStackFroms());
		AssertSize(5, gcStackPointers.NumElements());

		//Kill a value.  The entry keeps its key.
		pValue2->Kill();
		AssertTrue(sNotifierValue2.bFreed);
		AssertFalse(sNotifierKey2.bFreed);
		AssertSize(1, cMap.NumElements());
		AssertTrue(cMap.Get(pKey2).IsNull());
		AssertSize(2, pKey2->NumStackFroms());

		cMap.Kill();
		AssertSize(1, pKey2->NumStackFroms());
		AssertFalse(sNotifierKey2.bFreed);
	}
	AssertTrue(sNotifierKey2.bFreed);
	AssertTrue(sNotifierValue1.bFreed);
	AssertSize(0, gcStackPointers.NumElements());
	AssertLong(0, gcObjects.NumMemoryIndexes());
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectKillAllWithNull(void)
{
	STestObjectFreedNotifier	sNotifierKey1;
	STestObjectFreedNotifier	sNotifierValue1;
	STestObjectFreedNotifier	sNotifierKey2;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CMapObject>		pMap;
		Ptr<CTestObject>	pKey;
		Ptr<CTestObject>	pValue;
		Ptr<CTestObject>	pNull;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pKey = OMalloc<CTestObject>(&sNotifierKey1);
		pValue = OMalloc<CTestObject>(&sNotifierValue1);
		pMap->Put(pKey, pValue);
		pKey = OMalloc<CTestObject>(&sNotifierKey2);
		pMap->Put(pKey, pNull);
		pKey = NULL;
		pValue = NULL;
		AssertSize(2, pMap->NumElements());

		pMap->KillAll();
		AssertTrue(sNotifierKey1.bFreed);
		AssertTrue(sNotifierValue1.bFreed);
		AssertTrue(sNotifierKey2.bFreed);
		AssertSize(0, pMap->NumElements());
		gcObjects.ValidateObjectsConsistency();

		pMap = NULL;
		AssertTrue(pRoot->RemoveAll());
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObjectKillContainedHeap(void)
{
	STestObjectFreedNotifier	sNotifierKey1;
	STestObjectFreedNotifier	sNotifierValue1;
	STestObjectFreedNotifier	sNotifierKey2;
	STestObjectFreedNotifier	sNotifierValue2;
	STestObjectFreedNotifier	sNotifierKey3;
	STestObjectFreedNotifier	sNotifierValue3;
	STestObjectFreedNotifier	sNotifierBoth;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CMapObject>		pMap;
		Ptr<CTestObject>	pKey1;
		Ptr<CTestObject>	pValue1;
		Ptr<CTestObject>	pKey2;
		Ptr<CTestObject>	pValue2;
		Ptr<CTestObject>	pKey3;
		Ptr<CTestObject>	pValue3;
		Ptr<CTestObject>	pBoth;

		pRoot = ORoot();
		pMap = OMalloc<CMapObject>();
		pRoot->Add(pMap);

		pKey1 = OMalloc<CTestObject>(&sNotifierKey1);
		pValue1 = OMalloc<CTestObject>(&sNotifierValue1);
		pKey2 = OMalloc<CTestObject>(&sNotifierKey2);
		pValue2 = OMalloc<CTestObject>(&sNotifierValue2);
		pKey3 = OMalloc<CTestObject>(&sNotifierKey3);
		pValue3 = OMalloc<CTestObject>(&sNotifierValue3);
		pBoth = OMalloc<CTestObject>(&sNotifierBoth);

		pMap->Put(pKey1, pValue1);
		pMap->Put(pKey2, pValue2);
		pMap->Put(pBoth, pValue3);
		pMap->Put(pKey3, pBoth);
		AssertSize(4, pMap->NumElements());
		AssertSize(2, pBoth->NumHeapFroms());
		gcObjects.ValidateObjectsConsistency();

		//Kill a key.  The whole entry goes and the value is no longer pointed to by the map.
		pKey1->Kill();
		AssertTrue(sNotifierKey1.bFreed);
		AssertFalse(sNotifierValue1.bFreed);
		AssertSize(3, pMap->NumElements());
		AssertSize(0, pValue1->NumHeapFroms());
		gcObjects.ValidateObjectsConsistency();

		//Kill a value.  The map must not be left pointing to it.
		pValue2->Kill();
		AssertTrue(sNotifierValue2.bFreed);
		AssertFalse(sNotifierKey2.bFreed);
		AssertTrue(pMap->Get(pKey2).IsNull());
		gcObjects.ValidateObjectsConsistency();

		//Kill an Object that is both a key and a value.
		pBoth->Kill();
		AssertTrue(sNotifierBoth.bFreed);
		AssertFalse(sNotifierKey3.bFreed);
		AssertFalse(sNotifierValue3.bFreed);
		AssertTrue(pMap->Get(pKey3).IsNull());
		AssertSize(0, pValue3->NumHeapFroms());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestMapObjectConstructorExists();
	TestMapObjectClassExists();
	TestMapObjectPut();
	TestMapObjectDetachInHeap();
	TestMapObjectKillOnStack();
	TestMapObjectMapEntry();
	TestMapObjectIterate();
	TestMapObjectRemove();
	TestMapObjectDetachOnStack();
	TestMapObjectGetPointerTos();
	TestMapObjectPointerFromsHeap();
	TestMapObjectKillContainedHeap();
	TestMapObjectOnStackKillContained();
	TestMapObjectKillAllWithNull();
	TestMapObjectPointerFromsStack();
	TestMapObjectPutOverwrite();
	TestMapObjectExternalSerialisation();
	TestMapObjectMorphInto();
	TestMapObjectInternalSerialisation(4);
	TestMapObjectInternalSerialisation(3000);
	TestMapObjectEmbeddedOnHeapKill();
	TestMapObjectEmbeddedOnStackKill();
	TestMapObjectEmbeddedInObjectsKill();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

