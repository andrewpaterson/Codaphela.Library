#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/MapObject.h"
#include "StandardLib/ObjectFileGeneral.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/MultiFileObjectWriter.h"
#include "StandardLib/ObjectReaderSimpleDisk.h"
#include "StandardLib/Root.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


//An object that can't be read from the database leaves nothing half read behind.  Whatever pointed to it points to a
//hollow with its index and name again, so using it later reads it again: once it can be read, it is.


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


#define READ_FAILURE_DIRECTORY		"Output" _FS_ "ObjectReadFailure"
#define READ_FAILURE_DATABASE		READ_FAILURE_DIRECTORY _FS_ "Database"
#define READ_FAILURE_FILES			READ_FAILURE_DIRECTORY _FS_ "Files"
#define READ_FAILURE_MAX_DATA		8192


//How a stored object is broken.
enum ETestReadFailure
{
	TRF_ObjectMagic,	//Fails before the object is allocated.
	TRF_Length,			//Fails after its fields are loaded (and what it points to made hollow).
	TRF_HeapFromsMagic,	//Fails reading what points to it, after it was loaded.
	TRF_NumFailures
};


//The indices of the stored objects.
OIndex	goiReadFailureShared;
OIndex	goiReadFailureHolder;
OIndex	goiReadFailureContainer;
OIndex	goiReadFailureArray;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* TestReadFailureName(ETestReadFailure eFailure)
{
	switch (eFailure)
	{
	case TRF_ObjectMagic:
		return "ObjectMagic";
	case TRF_Length:
		return "Length";
	case TRF_HeapFromsMagic:
		return "HeapFromsMagic";
	default:
		return "?";
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureAddConstructors(void)
{
	gcObjects.AddConstructor<CTestSaveableObject1>();
	gcObjects.AddConstructor<CTestSaveableObject2>();
	gcObjects.AddConstructor<CEmbeddedTest>();
	gcObjects.AddConstructor<CEmbeddedContainer>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureOpen(CCodabase** ppcDatabase, CSequence** ppcSequence)
{
	*ppcSequence = CSequenceFactory::Create(READ_FAILURE_DATABASE, DS_No);
	*ppcDatabase = CCodabaseFactory::Create(READ_FAILURE_DATABASE, IWT_No, DS_No);
	(*ppcDatabase)->Open();
	ObjectsInit(*ppcDatabase, *ppcSequence);
	TestReadFailureAddConstructors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureClose(CCodabase** ppcDatabase, CSequence** ppcSequence)
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
// Root -> "Fail/Holder"  mp1 -> "Fail/Shared" -> mpObject -> "Other"
//                        mp2 -> "Fail/Container"'s mcTwo
//      -> "Fail/Array"   [Shared, Container]
//      -> "Fail/Map"     {"Fail/Key": Shared}
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureBuildDatabase(void)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(READ_FAILURE_DIRECTORY);
	AssertTrue(cFileUtil.TouchDir(READ_FAILURE_DATABASE));
	AssertTrue(cFileUtil.TouchDir(READ_FAILURE_FILES));

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CTestSaveableObject1>	pShared;
		Ptr<CString>				pOther;
		Ptr<CEmbeddedContainer>		pContainer;
		Ptr<CArrayObject>			pArray;
		Ptr<CMapObject>				pMap;
		Ptr<CString>				pKey;

		pRoot = ORoot();
		pHolder = ONMalloc<CTestSaveableObject2>("Fail/Holder", "Holder");
		pShared = ONMalloc<CTestSaveableObject1>("Fail/Shared");
		pOther = OMalloc<CString>("Other");
		pContainer = ONMalloc<CEmbeddedContainer>("Fail/Container");
		pArray = ONMalloc<CArrayObject>("Fail/Array");
		pMap = ONMalloc<CMapObject>("Fail/Map");
		pKey = ONMalloc<CString>("Fail/Key", "Key");

		pShared->miInt = 89;
		pShared->mszString.Set("Shared");
		pShared->mpObject = pOther;
		pContainer->mi = 42;
		pContainer->mcTwo.miAmANumber = 22;

		pHolder->mp1 = pShared;
		pHolder->mp2 = &pContainer->mcTwo;
		pArray->Add(pShared);
		pArray->Add(pContainer);
		pRoot->Add(pHolder);
		pRoot->Add(pArray);
		pMap->Put(pKey, pShared);
		pRoot->Add(pMap);

		goiReadFailureShared = pShared->GetIndex();
		goiReadFailureHolder = pHolder->GetIndex();
		goiReadFailureContainer = pContainer->GetIndex();
		goiReadFailureArray = pArray->GetIndex();
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size TestReadFailureFind(char* pcData, size uiSize, const char* szMagic)
{
	size	i;

	for (i = 0; i + 4 <= uiSize; i++)
	{
		if (memcmp(&pcData[i], szMagic, 4) == 0)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Breaks the object stored at oi.  Its bytes are kept in pacOriginal to
// put back.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureBreak(CCodabase* pcDatabase, OIndex oi, ETestReadFailure eFailure, CArrayChar* pacOriginal)
{
	char	acData[READ_FAILURE_MAX_DATA];
	lsize	uiSize;
	size	uiMagic;
	int32	iObjectMagic;
	int32	iHeapFromsMagic;

	iObjectMagic = OBJECT_DATA;
	iHeapFromsMagic = OBJECT_FROM_HEAP;

	AssertTrue(pcDatabase->Get(oi, &uiSize, acData, READ_FAILURE_MAX_DATA));
	AssertTrue(uiSize <= READ_FAILURE_MAX_DATA);
	pacOriginal->Init();
	pacOriginal->AddNum((size)uiSize);
	memcpy(pacOriginal->GetData(), acData, (size)uiSize);

	if (eFailure == TRF_HeapFromsMagic)
	{
		uiMagic = TestReadFailureFind(acData, (size)uiSize, (char*)&iHeapFromsMagic);
		AssertTrue(uiMagic != ARRAY_ELEMENT_NOT_FOUND);
		acData[uiMagic] ^= 0x5A;
	}
	else
	{
		uiMagic = TestReadFailureFind(acData, (size)uiSize, (char*)&iObjectMagic);
		AssertTrue((uiMagic != ARRAY_ELEMENT_NOT_FOUND) && (uiMagic > 0));
		if (eFailure == TRF_ObjectMagic)
		{
			acData[uiMagic] ^= 0x5A;
		}
		else
		{
			//The most significant byte of the length, just before the magic.
			acData[uiMagic - 1] ^= 0x40;
		}
	}
	AssertTrue(pcDatabase->Set(oi, acData, uiSize));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureMend(CCodabase* pcDatabase, OIndex oi, CArrayChar* pacOriginal)
{
	AssertTrue(pcDatabase->Set(oi, pacOriginal->GetData(), pacOriginal->NumElements()));
	pacOriginal->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureAssertShared(Ptr<CTestSaveableObject1> pShared)
{
	Ptr<CString>	pOther;

	AssertTrue(pShared.IsNotNull());
	AssertNotNull(pShared.Dereference());  //A pointer to a hollow is read when it's used.
	AssertFalse(pShared.IsHollow());
	AssertString("Fail/Shared", pShared.GetName());
	AssertLong(goiReadFailureShared, pShared.GetIndex());
	AssertInt(89, pShared->miInt);
	AssertString("Shared", pShared->mszString.Text());
	pOther = pShared->mpObject;
	AssertString("Other", pOther->Text());
}


//////////////////////////////////////////////////////////////////////////
//
// The hollow pointer pObject points to: the object's index and name, and
// what points to it.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureAssertHollow(CPointer& pObject, OIndex oi, const char* szName, CBaseObject* pcFrom)
{
	CBaseObject*	pcHollow;

	AssertTrue(pObject.IsNotNull());
	AssertTrue(pObject.IsHollow());
	pcHollow = pObject.Object()->GetEmbeddingContainer();
	AssertTrue(pcHollow->IsHollow());
	AssertLong(oi, pcHollow->GetIndex());
	AssertString(szName, pcHollow->GetName());
	AssertPointer(pcHollow, gcObjects.GetMemory()->Get(oi));
	if (pcFrom)
	{
		AssertSize(1, pObject.Object()->NumHeapFroms());
		AssertPointer(pcFrom, pObject.Object()->GetHeapFrom(0));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Get by name or by index of an object nobody in memory points to: it
// fails cleanly (no name or index left in memory, nothing else left), it
// fails again the same way, and once mended it's read.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureGet(ETestReadFailure eFailure, bool bByName)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];
	size		uiNumIndexes;
	size		uiNumNames;
	size		i;

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		CPointer					pObject;
		Ptr<CTestSaveableObject1>	pShared;

		uiNumIndexes = gcObjects.NumMemoryIndexes();
		uiNumNames = gcObjects.NumMemoryNames();

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		for (i = 0; i < 3; i++)
		{
			StartCapturingLog(&cLog, &sLogConfig);
			if (bByName)
			{
				pObject = gcObjects.Get("Fail/Shared");
			}
			else
			{
				pObject = gcObjects.Get(goiReadFailureShared);
			}
			StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);

			AssertTrue(pObject.IsNull());
			AssertTrue(strstr(szLog, "already exists") == NULL);
			AssertNull(gcObjects.GetMemory()->Get("Fail/Shared"));
			AssertNull(gcObjects.GetMemory()->Get(goiReadFailureShared));
			AssertSize(uiNumIndexes, gcObjects.NumMemoryIndexes());
			AssertSize(uiNumNames, gcObjects.NumMemoryNames());
		}

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
		if (bByName)
		{
			pShared = gcObjects.Get("Fail/Shared");
		}
		else
		{
			pShared = gcObjects.Get(goiReadFailureShared);
		}
		TestReadFailureAssertShared(pShared);
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A loaded object's pointer to the one that can't be read: dereferencing
// it fails, and it still points to a hollow for it, that the holder
// points to.  Again and again, and nothing builds up.  Mended, it's read
// through the same pointer.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailurePointer(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];
	size		uiNumIndexes;
	size		uiNumNames;
	int			iDistToRoot;
	size		i;

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CTestSaveableObject1>	pShared;
		CEmbeddedObject*			pcLoaded;

		pHolder = gcObjects.Get("Fail/Holder");
		AssertTrue(pHolder.IsNotNull());
		TestReadFailureAssertHollow(pHolder->mp1, goiReadFailureShared, "Fail/Shared", &pHolder);
		iDistToRoot = pHolder->mp1.GetDistToRoot();
		uiNumIndexes = gcObjects.NumMemoryIndexes();
		uiNumNames = gcObjects.NumMemoryNames();

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		for (i = 0; i < 5; i++)
		{
			StartCapturingLog(&cLog, &sLogConfig);
			pcLoaded = pHolder->mp1.Dereference();
			StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);

			AssertNull(pcLoaded);
			AssertTrue(strstr(szLog, "already exists") == NULL);
			TestReadFailureAssertHollow(pHolder->mp1, goiReadFailureShared, "Fail/Shared", &pHolder);
			AssertInt(iDistToRoot, pHolder->mp1.GetDistToRoot());
			AssertSize(2, pHolder->NumPointerTos());
			AssertSize(uiNumIndexes, gcObjects.NumMemoryIndexes());
			AssertSize(uiNumNames, gcObjects.NumMemoryNames());
		}

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
		pShared = pHolder->mp1;
		TestReadFailureAssertShared(pShared);
		AssertPointer(&pShared, &pHolder->mp1);
		AssertFalse(pHolder->mp1.IsHollow());
		AssertSize(1, pShared->NumHeapFroms());
		AssertPointer(&pHolder, pShared->GetHeapFrom(0));
		AssertPointer(&pShared, gcObjects.GetMemory()->Get("Fail/Shared"));
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer on the stack to the hollow: the same as a pointer in an
// object.  Both are left pointing to the same hollow, and both point to
// the object once it's read.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureStackPointer(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		CPointer					pOnStack;
		Ptr<CTestSaveableObject1>	pShared;

		pHolder = gcObjects.Get("Fail/Holder");
		pOnStack = pHolder->mp1;
		AssertTrue(pOnStack.IsHollow());
		AssertPointer(pOnStack.Object(), pHolder->mp1.Object());

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		StartCapturingLog(&cLog, &sLogConfig);
		AssertNull(pOnStack.Dereference());
		StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
		AssertTrue(pOnStack.IsHollow());
		AssertPointer(pOnStack.Object(), pHolder->mp1.Object());
		AssertLong(goiReadFailureShared, pOnStack.GetIndex());
		AssertTrue(pOnStack.Object()->HasStackFroms());
		TestReadFailureAssertHollow(pHolder->mp1, goiReadFailureShared, "Fail/Shared", &pHolder);

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
		AssertNotNull(pOnStack.Dereference());
		AssertFalse(pOnStack.IsHollow());
		AssertPointer(pOnStack.Object(), pHolder->mp1.Object());
		pShared = pOnStack;
		TestReadFailureAssertShared(pShared);
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer to an embedded object of the object that can't be read: left
// pointing to the hollow's embedded object, and to the object's embedded
// object once it's read.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureEmbedded(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];
	size		i;

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CEmbeddedContainer>		pContainer;
		CBaseObject*				pcHollow;
		CEmbeddedObject*			pcHollowTwo;

		pHolder = gcObjects.Get("Fail/Holder");
		AssertTrue(pHolder->mp2.IsHollow());
		pcHollow = pHolder->mp2.Object()->GetEmbeddingContainer();
		AssertLong(goiReadFailureContainer, pcHollow->GetIndex());
		AssertPointer(pcHollow->GetEmbeddedObject(2), pHolder->mp2.Object());

		TestReadFailureBreak(pcDatabase, goiReadFailureContainer, eFailure, &acOriginal);
		for (i = 0; i < 3; i++)
		{
			StartCapturingLog(&cLog, &sLogConfig);
			AssertNull(pHolder->mp2.Dereference());
			StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
			AssertTrue(pHolder->mp2.IsHollow());
			pcHollow = pHolder->mp2.Object()->GetEmbeddingContainer();
			AssertTrue(pcHollow->IsHollow());
			AssertLong(goiReadFailureContainer, pcHollow->GetIndex());
			AssertString("Fail/Container", pcHollow->GetName());
			pcHollowTwo = pcHollow->GetEmbeddedObject(2);
			AssertPointer(pcHollowTwo, pHolder->mp2.Object());
			AssertSize(1, pcHollowTwo->NumHeapFroms());
			AssertPointer(&pHolder, pcHollowTwo->GetHeapFrom(0));
		}

		TestReadFailureMend(pcDatabase, goiReadFailureContainer, &acOriginal);
		AssertNotNull(pHolder->mp2.Dereference());
		AssertFalse(pHolder->mp2.IsHollow());
		pContainer = gcObjects.Get("Fail/Container");
		AssertInt(42, pContainer->mi);
		AssertInt(22, pContainer->mcTwo.miAmANumber);
		AssertPointer(&pContainer->mcTwo, pHolder->mp2.Object());
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// An array element that can't be read: getting it fails and it's left
// hollow; once mended it's read.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureArray(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];
	size		i;

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CArrayObject>			pArray;
		Ptr<CTestSaveableObject1>	pShared;
		CPointer					pElement;
		CBaseObject*				pcHollow;

		pArray = gcObjects.Get("Fail/Array");
		AssertSize(2, pArray->NumElements());
		pcHollow = pArray->UnsafeGetObject(0);
		AssertTrue(pcHollow->IsHollow());

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		for (i = 0; i < 3; i++)
		{
			StartCapturingLog(&cLog, &sLogConfig);
			pElement = pArray->Get(0);
			StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
			AssertTrue(pElement.IsNull());
			pcHollow = pArray->UnsafeGetObject(0);
			AssertNotNull(pcHollow);
			AssertTrue(pcHollow->IsHollow());
			AssertLong(goiReadFailureShared, pcHollow->GetIndex());
			AssertString("Fail/Shared", pcHollow->GetName());
			AssertSize(1, pcHollow->NumHeapFroms());
			AssertPointer(&pArray, pcHollow->GetHeapFrom(0));
			AssertSize(2, pArray->NumElements());
		}

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
		pShared = pArray->Get(0);
		TestReadFailureAssertShared(pShared);
		AssertPointer(&pShared, pArray->UnsafeGetObject(0));
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A map value that can't be read: getting it fails and the map still has
// it (as a hollow); once mended it's read.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureMap(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];
	size		uiNumIndexes;
	size		i;

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CMapObject>				pMap;
		CPointer					pKey;
		CPointer					pValue;
		Ptr<CTestSaveableObject1>	pShared;
		CBaseObject*				pcHollow;

		pMap = gcObjects.Get("Fail/Map");
		pKey = gcObjects.Get("Fail/Key");
		AssertSize(1, pMap->NumElements());
		AssertTrue(pMap->Contains(pKey));
		uiNumIndexes = gcObjects.NumMemoryIndexes();

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		for (i = 0; i < 3; i++)
		{
			StartCapturingLog(&cLog, &sLogConfig);
			pValue = pMap->Get(pKey);
			StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
			AssertTrue(pValue.IsNull());
			AssertSize(1, pMap->NumElements());
			AssertTrue(pMap->Contains(pKey));
			pcHollow = gcObjects.GetMemory()->Get(goiReadFailureShared);
			AssertNotNull(pcHollow);
			AssertTrue(pcHollow->IsHollow());
			AssertString("Fail/Shared", pcHollow->GetName());
			AssertSize(1, pcHollow->NumHeapFroms());
			AssertPointer(&pMap, pcHollow->GetHeapFrom(0));
			AssertSize(uiNumIndexes, gcObjects.NumMemoryIndexes());
		}

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
		pShared = pMap->Get(pKey);
		TestReadFailureAssertShared(pShared);
		AssertPointer(&pShared, gcObjects.GetMemory()->Get(goiReadFailureShared));
		AssertSize(1, pShared->NumHeapFroms());
		AssertPointer(&pMap, pShared->GetHeapFrom(0));
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// Several pointers to the object that can't be read (an object, the
// array, the stack): every one is left pointing to the one hollow.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureManyPointers(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CArrayObject>			pArray;
		CPointer					pOnStack;
		Ptr<CTestSaveableObject1>	pShared;
		CBaseObject*				pcHollow;

		pHolder = gcObjects.Get("Fail/Holder");
		pArray = gcObjects.Get("Fail/Array");
		pOnStack = pHolder->mp1;
		AssertPointer(pHolder->mp1.Object(), pArray->UnsafeGetObject(0));

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		StartCapturingLog(&cLog, &sLogConfig);
		AssertNull(pHolder->mp1.Dereference());
		StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);

		pcHollow = pHolder->mp1.BaseObject();
		AssertTrue(pcHollow->IsHollow());
		AssertPointer(pcHollow, pArray->UnsafeGetObject(0));
		AssertPointer(pcHollow, pOnStack.Object());
		AssertSize(2, pcHollow->NumHeapFroms());
		AssertTrue(pcHollow->HasStackFroms());
		AssertPointer(pcHollow, gcObjects.GetMemory()->Get(goiReadFailureShared));

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
		pShared = pArray->Get(0);
		TestReadFailureAssertShared(pShared);
		AssertPointer(&pShared, pHolder->mp1.Object());
		AssertPointer(&pShared, pOnStack.Object());
		AssertSize(2, pShared->NumHeapFroms());
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// Flushing after a failure saves nothing over the stored object: mended
// after the flush (and a reopen), it's read as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureFlush(ETestReadFailure eFailure)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CArrayChar	acOriginal;
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLog[2048];

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CRoot>					pRoot;

		pRoot = ORoot();
		pHolder = pRoot->Get("Fail/Holder");
		pHolder->msz.Set("Changed");
		pHolder->SetDirty(true);

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		StartCapturingLog(&cLog, &sLogConfig);
		AssertNull(pHolder->mp1.Dereference());
		StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
		ObjectsFlush();

		TestReadFailureMend(pcDatabase, goiReadFailureShared, &acOriginal);
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CTestSaveableObject1>	pShared;

		pHolder = gcObjects.Get("Fail/Holder");
		AssertString("Changed", pHolder->msz.Text());
		pShared = pHolder->mp1;
		TestReadFailureAssertShared(pShared);

		pHolder->msz.Set("Holder");
		pHolder->SetDirty(true);
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// An external read replaces "Fail/Shared", which can't be read from the
// database: the replacement takes its name and index and saves over it,
// and the stored holder points to it.  With the holder loaded the hollow
// it points to is replaced; without, the index comes from the database.
//
//////////////////////////////////////////////////////////////////////////
void TestReadFailureReplacedByExternal(ETestReadFailure eFailure, bool bHolderLoaded)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CArrayChar					acOriginal;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLog[2048];
	CMultiFileObjectWriter		cWriter;
	CObjectReaderSimpleDisk		cReader;
	CExternalObjectSerialiser	cSerialiser;
	CExternalObjectDeserialiser	cDeserialiser;
	CFileUtil					cFileUtil;

	//The external file, written from memory alone.
	cFileUtil.RemoveDir(READ_FAILURE_FILES);
	AssertTrue(cFileUtil.TouchDir(READ_FAILURE_FILES));
	ObjectsInit();
	TestReadFailureAddConstructors();
	{
		Ptr<CTestSaveableObject1>	pShared;

		pShared = ONMalloc<CTestSaveableObject1>("Fail/Shared");
		pShared->miInt = 7;
		pShared->mszString.Set("External");
		cWriter.Init(READ_FAILURE_FILES, "");
		cSerialiser.Init(&cWriter);
		AssertTrue(cSerialiser.Write(&pShared));
		cSerialiser.Kill();
		cWriter.Kill();
	}
	ObjectsKill();

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject1>	pShared;
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CRoot>					pRoot;

		pRoot = ORoot();
		if (bHolderLoaded)
		{
			pHolder = pRoot->Get("Fail/Holder");
			AssertTrue(pHolder->mp1.IsHollow());
		}

		TestReadFailureBreak(pcDatabase, goiReadFailureShared, eFailure, &acOriginal);
		acOriginal.Kill();

		cReader.Init(READ_FAILURE_FILES);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		StartCapturingLog(&cLog, &sLogConfig);
		pShared = cDeserialiser.Read("Fail/Shared");
		StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
		cDeserialiser.Kill();
		cReader.Kill();

		AssertTrue(pShared.IsNotNull());
		AssertTrue(strstr(szLog, "already exists") == NULL);
		AssertInt(7, pShared->miInt);
		AssertString("Fail/Shared", pShared.GetName());
		AssertLong(goiReadFailureShared, pShared.GetIndex());
		AssertPointer(&pShared, gcObjects.GetMemory()->Get("Fail/Shared"));
		if (bHolderLoaded)
		{
			AssertPointer(&pShared, pHolder->mp1.Object());
		}
		else
		{
			//Rooted so it's saved.  (Adding to the loaded root's set doesn't mark the set dirty, so the set isn't.)
			pRoot->Add(pShared);
		}
		StartCapturingLog(&cLog, &sLogConfig);
		ObjectsFlush();
		StopCapturingLog(&cLog, &sLogConfig, szLog, 2048);
		AssertString("", szLog);
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);

	TestReadFailureOpen(&pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CTestSaveableObject1>	pShared;
		Ptr<CRoot>					pRoot;

		pRoot = ORoot();
		pHolder = pRoot->Get("Fail/Holder");
		pShared = pHolder->mp1;
		AssertTrue(pShared.IsNotNull());
		AssertNotNull(pShared.Dereference());
		AssertLong(goiReadFailureShared, pShared.GetIndex());
		AssertInt(7, pShared->miInt);
		AssertString("External", pShared->mszString.Text());
		AssertPointer(&pShared, gcObjects.Get("Fail/Shared").Object());

		//Put it back as it was for the next tests.
		pShared->miInt = 89;
		pShared->mszString.Set("Shared");
		pShared->mpObject = OMalloc<CString>("Other");
		pShared->SetDirty(true);
		ObjectsFlush();
	}
	TestReadFailureClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReadFailure(void)
{
	CFileUtil	cFileUtil;
	int			i;

	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestReadFailureBuildDatabase();
	for (i = 0; i < TRF_NumFailures; i++)
	{
		TestReadFailureGet((ETestReadFailure)i, true);
		TestReadFailureGet((ETestReadFailure)i, false);
		TestReadFailurePointer((ETestReadFailure)i);
		TestReadFailureStackPointer((ETestReadFailure)i);
		TestReadFailureEmbedded((ETestReadFailure)i);
		TestReadFailureArray((ETestReadFailure)i);
		TestReadFailureMap((ETestReadFailure)i);
		TestReadFailureManyPointers((ETestReadFailure)i);
		TestReadFailureFlush((ETestReadFailure)i);
		TestReadFailureReplacedByExternal((ETestReadFailure)i, true);
		TestReadFailureReplacedByExternal((ETestReadFailure)i, false);
	}

	cFileUtil.RemoveDir(READ_FAILURE_DIRECTORY);

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}
