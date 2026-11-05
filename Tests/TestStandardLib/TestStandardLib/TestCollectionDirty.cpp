#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/DataOrderers.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/SetObject.h"
#include "StandardLib/MapObject.h"
#include "StandardLib/IndexObject.h"
#include "StandardLib/Root.h"
#include "TestLib/Assert.h"


//A collection loaded from the database is dirty once what's in it changes, so the change is saved.  Loading it and
//reading its members leave it clean.


#define COLLECTION_DIRTY_DATABASE	"Output" _FS_ "CollectionDirty"


enum ECollectionChange
{
	CC_ArrayAdd,
	CC_ArrayAddNull,
	CC_ArrayInsert,
	CC_ArraySet,
	CC_ArrayRemove,
	CC_ArrayRemoveAt,
	CC_ArrayRemoveEnd,
	CC_ArrayRemoveAll,
	CC_SetAdd,
	CC_SetRemove,
	CC_MapPut,
	CC_MapReplace,
	CC_MapRemove,
	CC_IndexPut,
	CC_IndexReplace,
	CC_IndexRemove,
	CC_RootAdd,
	CC_RootRemove,
	CC_NumChanges
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyOpen(CCodabase** ppcDatabase, CSequence** ppcSequence)
{
	*ppcSequence = CSequenceFactory::Create(COLLECTION_DIRTY_DATABASE, DS_No);
	*ppcDatabase = CCodabaseFactory::Create(COLLECTION_DIRTY_DATABASE, IWT_No, DS_No);
	(*ppcDatabase)->Open();
	ObjectsInit(*ppcDatabase, *ppcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyClose(CCodabase** ppcDatabase, CSequence** ppcSequence)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;

	pcDatabase = *ppcDatabase;
	pcSequence = *ppcSequence;
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	*ppcDatabase = NULL;
	*ppcSequence = NULL;
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Root -> "Dirty/Array" [A, B, C]
//      -> "Dirty/Set"   {A}
//      -> "Dirty/Map"   {KeyA: A}
//      -> "Dirty/Index" {"a": A}
//      -> D, E
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyBuild(void)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(COLLECTION_DIRTY_DATABASE);
	AssertTrue(cFileUtil.TouchDir(COLLECTION_DIRTY_DATABASE));

	TestCollectionDirtyOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CRoot>			pRoot;
		Ptr<CArrayObject>	pArray;
		Ptr<CSetObject>		pSet;
		Ptr<CMapObject>		pMap;
		Ptr<CIndexObject>	pIndex;
		Ptr<CString>		pA;
		Ptr<CString>		pB;
		Ptr<CString>		pC;
		Ptr<CString>		pD;
		Ptr<CString>		pE;
		Ptr<CString>		pKeyA;
		Ptr<CString>		pKeyB;

		pRoot = ORoot();
		pA = ONMalloc<CString>("Dirty/A", "A");
		pB = ONMalloc<CString>("Dirty/B", "B");
		pC = ONMalloc<CString>("Dirty/C", "C");
		pD = ONMalloc<CString>("Dirty/D", "D");
		pE = ONMalloc<CString>("Dirty/E", "E");
		pKeyA = ONMalloc<CString>("Dirty/KeyA", "KeyA");
		pKeyB = ONMalloc<CString>("Dirty/KeyB", "KeyB");

		pArray = ONMalloc<CArrayObject>("Dirty/Array");
		pArray->Add(pA);
		pArray->Add(pB);
		pArray->Add(pC);
		pSet = ONMalloc<CSetObject>("Dirty/Set");
		pSet->Add(pA);
		pMap = ONMalloc<CMapObject>("Dirty/Map");
		pMap->Put(pKeyA, pA);
		pIndex = ONMalloc<CIndexObject>("Dirty/Index");
		pIndex->Put("a", pA);

		pRoot->Add(pArray);
		pRoot->Add(pSet);
		pRoot->Add(pMap);
		pRoot->Add(pIndex);
		pRoot->Add(pD);
		pRoot->Add(pE);
		pRoot->Add(pKeyB);
		ObjectsFlush();
	}
	TestCollectionDirtyClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* TestCollectionDirtyCollectionName(ECollectionChange eChange)
{
	if (eChange <= CC_ArrayRemoveAll)
	{
		return "Dirty/Array";
	}
	else if (eChange <= CC_SetRemove)
	{
		return "Dirty/Set";
	}
	else if (eChange <= CC_MapRemove)
	{
		return "Dirty/Map";
	}
	else if (eChange <= CC_IndexRemove)
	{
		return "Dirty/Index";
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What's in the array, as "A,B,C" ("-" for an empty place).
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyDescribeArray(Ptr<CArrayObject> pArray, CChars* psz)
{
	size			i;
	Ptr<CString>	pString;

	psz->Init();
	for (i = 0; i < pArray->NumElements(); i++)
	{
		if (i > 0)
		{
			psz->Append(',');
		}
		pString = pArray->Get(i);
		if (pString.IsNull())
		{
			psz->Append('-');
		}
		else
		{
			psz->Append(pString->Text());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a collection holds, read before changing it: its members are read
// (made not hollow), which mustn't make it dirty.
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyReadMembers(CPointer& pCollection)
{
	CChars				sz;
	Ptr<CArrayObject>	pArray;
	Ptr<CMapObject>		pMap;
	Ptr<CIndexObject>	pIndex;
	Ptr<CSetObject>		pSet;
	Ptr<CString>		pKeyA;
	Ptr<CString>		pString;

	if (strcmp(pCollection.GetName(), "Dirty/Array") == 0)
	{
		pArray = pCollection;
		TestCollectionDirtyDescribeArray(pArray, &sz);
		AssertString("A,B,C", sz.Text());
		sz.Kill();
	}
	else if (strcmp(pCollection.GetName(), "Dirty/Set") == 0)
	{
		pSet = pCollection;
		AssertSize(1, pSet->NumElements());
		pString = pSet->UnsafeGet(0);
		AssertString("A", pString->Text());
	}
	else if (strcmp(pCollection.GetName(), "Dirty/Map") == 0)
	{
		pMap = pCollection;
		pKeyA = gcObjects.Get("Dirty/KeyA");
		pString = pMap->Get(pKeyA);
		AssertString("A", pString->Text());
	}
	else if (strcmp(pCollection.GetName(), "Dirty/Index") == 0)
	{
		pIndex = pCollection;
		pString = pIndex->Get("a");
		AssertString("A", pString->Text());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyChange(ECollectionChange eChange, CPointer& pCollection, Ptr<CRoot> pRoot)
{
	Ptr<CArrayObject>	pArray;
	Ptr<CSetObject>		pSet;
	Ptr<CMapObject>		pMap;
	Ptr<CIndexObject>	pIndex;
	CPointer			pA;
	CPointer			pB;
	CPointer			pD;
	CPointer			pE;
	CPointer			pKeyA;
	CPointer			pKeyB;

	pA = gcObjects.Get("Dirty/A");
	pB = gcObjects.Get("Dirty/B");
	pD = gcObjects.Get("Dirty/D");
	pE = gcObjects.Get("Dirty/E");
	pKeyA = gcObjects.Get("Dirty/KeyA");
	pKeyB = gcObjects.Get("Dirty/KeyB");
	pArray = pCollection;
	pSet = pCollection;
	pMap = pCollection;
	pIndex = pCollection;

	switch (eChange)
	{
	case CC_ArrayAdd:
		AssertTrue(pArray->Add(pD));
		break;
	case CC_ArrayAddNull:
		AssertTrue(pArray->CArrayCommonObject::Add((CEmbeddedObject*)NULL));
		break;
	case CC_ArrayInsert:
		AssertTrue(pArray->Insert(1, pD));
		break;
	case CC_ArraySet:
		AssertTrue(pArray->Set(1, pD));
		break;
	case CC_ArrayRemove:
		AssertTrue(pArray->Remove(pB));
		break;
	case CC_ArrayRemoveAt:
		AssertTrue(pArray->Remove((size)0));
		break;
	case CC_ArrayRemoveEnd:
		AssertTrue(pArray->RemoveEnd(1));
		break;
	case CC_ArrayRemoveAll:
		AssertTrue(pArray->RemoveAll());
		break;
	case CC_SetAdd:
		AssertTrue(pSet->Add(pD));
		break;
	case CC_SetRemove:
		AssertTrue(pSet->Remove(pA));
		break;
	case CC_MapPut:
		AssertTrue(pMap->Put(pKeyB, pD));
		break;
	case CC_MapReplace:
		AssertTrue(pMap->Put(pKeyA, pD));
		break;
	case CC_MapRemove:
		AssertTrue(pMap->Remove(pKeyA));
		break;
	case CC_IndexPut:
		AssertTrue(pIndex->Put("b", pD));
		break;
	case CC_IndexReplace:
		AssertTrue(pIndex->Put("a", pD));
		break;
	case CC_IndexRemove:
		AssertTrue(pIndex->Remove("a"));
		break;
	case CC_RootAdd:
		pRoot->Add(pA);
		break;
	case CC_RootRemove:
		AssertTrue(pRoot->Remove(pE));
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The change, read back after reopening the database.
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyAssertSaved(ECollectionChange eChange, Ptr<CRoot> pRoot, size uiRootObjectsBefore)
{
	CChars				sz;
	Ptr<CArrayObject>	pArray;
	Ptr<CSetObject>		pSet;
	Ptr<CMapObject>		pMap;
	Ptr<CIndexObject>	pIndex;
	CPointer			pA;
	CPointer			pD;
	CPointer			pE;
	Ptr<CString>		pKeyA;
	Ptr<CString>		pKeyB;
	Ptr<CString>		pString;
	const char*			szExpected;

	pA = gcObjects.Get("Dirty/A");
	pD = gcObjects.Get("Dirty/D");
	pE = gcObjects.Get("Dirty/E");
	pKeyA = gcObjects.Get("Dirty/KeyA");
	pKeyB = gcObjects.Get("Dirty/KeyB");

	szExpected = NULL;
	switch (eChange)
	{
	case CC_ArrayAdd:
		szExpected = "A,B,C,D";
		break;
	case CC_ArrayAddNull:
		szExpected = "A,B,C,-";
		break;
	case CC_ArrayInsert:
		szExpected = "A,D,B,C";
		break;
	case CC_ArraySet:
		szExpected = "A,D,C";
		break;
	case CC_ArrayRemove:
		szExpected = "A,C";
		break;
	case CC_ArrayRemoveAt:
		szExpected = "C,B";  //Arrays don't keep their order by default: the last fills the gap.
		break;
	case CC_ArrayRemoveEnd:
		szExpected = "A";
		break;
	case CC_ArrayRemoveAll:
		szExpected = "";
		break;
	default:
		break;
	}
	if (szExpected)
	{
		pArray = pRoot->Get("Dirty/Array");
		TestCollectionDirtyDescribeArray(pArray, &sz);
		AssertString(szExpected, sz.Text());
		sz.Kill();
		return;
	}

	switch (eChange)
	{
	case CC_SetAdd:
		pSet = pRoot->Get("Dirty/Set");
		AssertSize(2, pSet->NumElements());
		AssertTrue(pSet->Contains(pA));
		AssertTrue(pSet->Contains(pD));
		break;
	case CC_SetRemove:
		pSet = pRoot->Get("Dirty/Set");
		AssertSize(0, pSet->NumElements());
		break;
	case CC_MapPut:
		pMap = pRoot->Get("Dirty/Map");
		AssertSize(2, pMap->NumElements());
		pString = pMap->Get(pKeyA);
		AssertString("A", pString->Text());
		pString = pMap->Get(pKeyB);
		AssertString("D", pString->Text());
		break;
	case CC_MapReplace:
		pMap = pRoot->Get("Dirty/Map");
		AssertSize(1, pMap->NumElements());
		pString = pMap->Get(pKeyA);
		AssertString("D", pString->Text());
		break;
	case CC_MapRemove:
		pMap = pRoot->Get("Dirty/Map");
		AssertSize(0, pMap->NumElements());
		break;
	case CC_IndexPut:
		pIndex = pRoot->Get("Dirty/Index");
		AssertSize(2, pIndex->NumElements());
		pString = pIndex->Get("a");
		AssertString("A", pString->Text());
		pString = pIndex->Get("b");
		AssertString("D", pString->Text());
		break;
	case CC_IndexReplace:
		pIndex = pRoot->Get("Dirty/Index");
		AssertSize(1, pIndex->NumElements());
		pString = pIndex->Get("a");
		AssertString("D", pString->Text());
		break;
	case CC_IndexRemove:
		pIndex = pRoot->Get("Dirty/Index");
		AssertSize(0, pIndex->NumElements());
		break;
	case CC_RootAdd:
		AssertSize(uiRootObjectsBefore + 1, pRoot->NumObjects());
		AssertTrue(pRoot->TestGetSet()->Contains(pA));
		break;
	case CC_RootRemove:
		AssertSize(uiRootObjectsBefore - 1, pRoot->NumObjects());
		AssertFalse(pRoot->TestGetSet()->Contains(pE));
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyOneChange(ECollectionChange eChange)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	size		uiRootObjects;
	const char*	szCollection;

	TestCollectionDirtyBuild();

	TestCollectionDirtyOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CRoot>		pRoot;
		CPointer		pCollection;
		CBaseObject*	pcDirty;

		pRoot = ORoot();
		uiRootObjects = pRoot->NumObjects();
		szCollection = TestCollectionDirtyCollectionName(eChange);
		if (szCollection)
		{
			pCollection = pRoot->Get(szCollection);
			AssertTrue(pCollection.IsNotNull());
			AssertNotNull(pCollection.Dereference());
			pcDirty = pCollection.BaseObject();
		}
		else
		{
			pcDirty = pRoot->TestGetSet();
		}

		AssertFalse(pcDirty->IsDirty());
		if (szCollection)
		{
			TestCollectionDirtyReadMembers(pCollection);
		}
		AssertFalse(pcDirty->IsDirty());

		TestCollectionDirtyChange(eChange, pCollection, pRoot);
		AssertTrue(pcDirty->IsDirty());

		ObjectsFlush();
		AssertFalse(pcDirty->IsDirty());
	}
	TestCollectionDirtyClose(&pcDatabase, &pcSequence);

	TestCollectionDirtyOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CRoot>	pRoot;

		pRoot = ORoot();
		TestCollectionDirtyAssertSaved(eChange, pRoot, uiRootObjects);
		ObjectsFlush();
	}
	TestCollectionDirtyClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// Without a change nothing is saved: a collection loaded, read and
// flushed is still clean, and the next session reads what was built.
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirtyNoChange(void)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	const char*	aszCollections[] = { "Dirty/Array", "Dirty/Set", "Dirty/Map", "Dirty/Index", NULL };
	size		i;

	TestCollectionDirtyBuild();

	TestCollectionDirtyOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CRoot>	pRoot;
		CPointer	pCollection;

		pRoot = ORoot();
		AssertFalse(pRoot->TestGetSet()->IsDirty());
		for (i = 0; aszCollections[i] != NULL; i++)
		{
			pCollection = pRoot->Get(aszCollections[i]);
			AssertNotNull(pCollection.Dereference());
			AssertFalse(pCollection.BaseObject()->IsDirty());
			TestCollectionDirtyReadMembers(pCollection);
			AssertFalse(pCollection.BaseObject()->IsDirty());
		}
		AssertFalse(pRoot->TestGetSet()->IsDirty());
		ObjectsFlush();
	}
	TestCollectionDirtyClose(&pcDatabase, &pcSequence);

	TestCollectionDirtyOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CRoot>	pRoot;
		CPointer	pCollection;

		pRoot = ORoot();
		for (i = 0; aszCollections[i] != NULL; i++)
		{
			pCollection = pRoot->Get(aszCollections[i]);
			AssertNotNull(pCollection.Dereference());
			TestCollectionDirtyReadMembers(pCollection);
		}
		ObjectsFlush();
	}
	TestCollectionDirtyClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCollectionDirty(void)
{
	CFileUtil	cFileUtil;
	int			i;

	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();
	DataOrderersInit();

	TestCollectionDirtyNoChange();
	for (i = 0; i < CC_NumChanges; i++)
	{
		TestCollectionDirtyOneChange((ECollectionChange)i);
	}

	cFileUtil.RemoveDir(COLLECTION_DIRTY_DATABASE);

	DataOrderersKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}
