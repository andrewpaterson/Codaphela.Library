#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/ValueIndexedDataConfig.h"
#include "BaseLib/ValueNamedIndexesConfig.h"
#include "BaseLib/NamedIndexedHeader.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTestDoubleNamedString> SetupObjectsForDehollowfication(void)
{
	Ptr<CTestNamedString>			cNS1;
	Ptr<CTestNamedString>			cNS2;
	Ptr<CTestNamedString>			cNS3;
	Ptr<CTestNamedString>			cDiamond;
	Ptr<CTestDoubleNamedString>		pDouble;
	Ptr<CRoot>						pRoot;
	Ptr<CString>					cS1;
	Ptr<CString>					cS2;

	pRoot = ORoot();

	cS2 = OMalloc<CString>("CS2");
	cDiamond = ONMalloc<CTestNamedString>("Diamond", cS2, Null(), "Diamond");

	cS1 = OMalloc<CString>("CS1");
	cNS1 = ONMalloc<CTestNamedString>("NS1", cS1, cDiamond, "NS1");

	cNS2 = ONMalloc<CTestNamedString>("NS2", Null(), cDiamond, "NS2");

	cNS3 = ONMalloc<CTestNamedString>("NS3", Null(), cNS1, "NS3");


	pDouble = ONMalloc<CTestDoubleNamedString>("Double", Null(), cNS2, cNS3);

	pRoot->Add(pDouble);
	return pDouble;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetupObjectsConstructors(void)
{
	gcObjects.AddConstructor<CTestWithArray>();
	gcObjects.AddConstructor<CTestInteger>();
	gcObjects.AddConstructor<CTestNamedString>();
	gcObjects.AddConstructor<CTestDoubleNamedString>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsInMemoryIteration(void)
{
	SIndexesIterator	sIter;
	OIndex				oi;

	ObjectsInit();
	SetupObjectsForDehollowfication();

	oi = gcObjects.StartMemoryIteration(&sIter);
	AssertLong(1LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(2LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(3LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(4LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(5LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(6LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(7LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(8LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(9LL, oi);
	oi = gcObjects.IterateMemory(&sIter);
	sIter.Kill();
	AssertLong(INVALID_O_INDEX, oi);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsObjectSave(void)
{
	Ptr<CTestDoubleNamedString>			pDouble;
	CFileUtil							cFileUtil;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	char								szDirectory[] = "Output" _FS_ "ObjectSave";
	CCodabase*							pcDatabase;
	lsize								iSerialisedSize;
	CSequence*							pcSequence;

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pDouble = SetupObjectsForDehollowfication();

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());
	AssertTrue(pDouble.IsDirty());
	Pass();
	
	AssertTrue(pDouble.BaseObject()->Flush());
	AssertFalse(pDouble.IsDirty());

	AssertLong(1, pcDatabase->NumIndices());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(174, pDouble->SerialisedSize());
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());
	Pass();

	AssertTrue(pDouble.BaseObject()->Flush());
	AssertLong(1, pcDatabase->NumIndices());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(174, iSerialisedSize);
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());
	
	pDouble->mszString = OMalloc<CString>("A String");
	AssertTrue(pDouble.IsDirty());

	AssertTrue(pDouble.BaseObject()->Flush());
	AssertLong(1, pcDatabase->NumIndices());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(188, iSerialisedSize);
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());

	pDouble->mszString = OMalloc<CString>("Different Object");

	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(188, iSerialisedSize);
	AssertTrue(pDouble.BaseObject()->Flush());
	AssertLong(1, pcDatabase->NumIndices());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(188, iSerialisedSize);
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());

	ObjectsFlush(); //ObjectsFlush flushed dirty objects into the databsase.
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFlushNoClear(void)
{
	CFileUtil	cFileUtil;
	bool		bResult;
	CCodabase*	pcDatabase;
	char		szDirectory[] = "Output" _FS_ "Flush1";
	CSequence*	pcSequence;

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	SetupObjectsForDehollowfication();

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(0, pcDatabase->NumNames());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());
	
	bResult = ObjectsFlush();
	AssertTrue(bResult);

	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(6, pcDatabase->NumNames());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFlushDurable(void)
{
	bool								bResult;
	CFileUtil							cFileUtil;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	char								szDirectory[] = "Output" _FS_ "Flush2";
	CCodabase*							pcDatabase;
	CSequence*							pcSequence;

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsForDehollowfication();

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(0, pcDatabase->NumNames());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());

	bResult = ObjectsFlush();
	AssertTrue(bResult);

	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(6, pcDatabase->NumNames());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);


	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "48_0.DAT",     "Output" _FS_ "Flush2" _FS_ "48_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "56_0.DAT",     "Output" _FS_ "Flush2" _FS_ "56_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "68_0.DAT",     "Output" _FS_ "Flush2" _FS_ "68_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "89_0.DAT",     "Output" _FS_ "Flush2" _FS_ "89_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "93_0.DAT",     "Output" _FS_ "Flush2" _FS_ "93_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "101_0.DAT",    "Output" _FS_ "Flush2" _FS_ "101_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "102_0.DAT",    "Output" _FS_ "Flush2" _FS_ "102_0.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "Files.DAT",    "Output" _FS_ "Flush2" _FS_ "Files.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "Indicies.DAT", "Output" _FS_ "Flush2" _FS_ "Indicies.DAT");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "32_0.NAM",     "Output" _FS_ "Flush2" _FS_ "32_0.NAM");
	AssertFile("Input" _FS_ "Dehollowfication" _FS_ "Files.NAM",    "Output" _FS_ "Flush2" _FS_ "Files.NAM");

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsEvict(void)
{
	CFileUtil	cFileUtil;
	bool		bResult;
	CCodabase*	pcDatabase;
	char		szDirectory[] = "Output" _FS_ "Eviction";
	CSequence*	pcSequence;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsForDehollowfication();

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(0, pcDatabase->NumIndicesCached());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());

	bResult = ObjectsFlush();
	AssertTrue(bResult);

	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(9, pcDatabase->NumIndicesCached());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);


	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsForDehollowfication();

	bResult = ObjectsFlush();
	AssertTrue(bResult);

	AssertLong(9, pcDatabase->NumIndicesCached());
	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);


	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsForDehollowfication();

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());
	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(6, pcDatabase->NumNames());
	AssertLong(9, pcDatabase->NumIndicesCached());

	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);
	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertLong(0, gcObjects.NumMemoryNames());


	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);


	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));
	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsForDehollowfication();

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	//AssertLong(0, pcDatabase->NumIndicesCached());
	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertLong(0, gcObjects.NumMemoryNames());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsObjectKillInGraph(void)
{
	Ptr<CTestNamedString>	cNS1;
	Ptr<CTestNamedString>	cNS2;
	Ptr<CRoot>				pRoot;
	Ptr<CString>			cS1;
	Ptr<CString>			cS2;
	CTestNamedString*		pcNS2;
	CString*				pcS2;

	ObjectsInit();

	pRoot = ORoot();

	cS1 = OMalloc<CString>("CS1");
	cNS1 = ONMalloc<CTestNamedString>("NS1", cS1, Null(), "NS1");

	cS2 = OMalloc<CString>("CS2");
	cNS2 = ONMalloc<CTestNamedString>("NS2", cS2, cNS1, "NS2");

	pRoot->Add(cNS2);
	
	AssertPointer(cNS2->mpAnother.Object(), cNS1.Object());
	AssertLong(6, gcObjects.NumMemoryIndexes());
	pcNS2 = &cNS2;
	cNS2 = NULL;
	pRoot = NULL;
	cS1 = NULL;
	pcS2 = &cS2;
	cS2 = NULL;
	AssertLong(6, gcObjects.NumMemoryIndexes());

	cNS1->Kill();

	AssertNull(pcNS2->mpAnother.Object());
	AssertLong(4, gcObjects.NumMemoryIndexes());
	AssertPointer(pcNS2->mszString.Object(), pcS2);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsArrayKillInGraph(void)
{
	Ptr<CTestNamedString>	cNS1;
	Ptr<CArrayObject>		cA1;
	Ptr<CArrayObject>		cA2;
	Ptr<CRoot>				pRoot;
	Ptr<CString>			cS1;

	ObjectsInit();

	pRoot = ORoot();

	cS1 = OMalloc<CString>("CS1");
	cNS1 = ONMalloc<CTestNamedString>("NS1", cS1, Null(), "NS1");

	cA1 = OMalloc<CArrayObject>();
	cA2 = OMalloc<CArrayObject>();

	cA1->Add(cNS1);
	cA2->Add(cNS1);

	pRoot->Add(cA1);
	pRoot->Add(cA2);

	AssertLong(6, gcObjects.NumMemoryIndexes());

	cNS1 = NULL;
	pRoot = NULL;
	cS1 = NULL;

	cA1->Kill();
	AssertLong(5, gcObjects.NumMemoryIndexes());

	cA2->Kill();
	AssertLong(2, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsObjectKillInArrayInGraph(void)
{
	ObjectsInit();
	
	Ptr<CTestNamedString>	cNS1;
	Ptr<CArrayObject>		cA1;
	Ptr<CArrayObject>		cA2;
	Ptr<CRoot>				pRoot;
	Ptr<CString>			cS1;
		
	pRoot = ORoot();

	cS1 = OMalloc<CString>("CS1");
	cNS1 = ONMalloc<CTestNamedString>("NS1", cS1, Null(), "NS1");

	cA1 = OMalloc<CArrayObject>();
	cA2 = OMalloc<CArrayObject>();

	cA1->Add(cNS1);
	cA2->Add(cNS1);

	pRoot->Add(cA1);
	pRoot->Add(cA2);

	AssertSize(1, cA1->NumPointerTos());
	AssertSize(1, cA2->NumPointerTos());
	AssertLong(6, gcObjects.NumMemoryIndexes());
	pRoot = NULL;
	cS1 = NULL;
	AssertLong(6, gcObjects.NumMemoryIndexes());

	cNS1->Kill();
	AssertLong(4, gcObjects.NumMemoryIndexes());

	AssertSize(0, cA1->NumPointerTos());
	AssertSize(0, cA2->NumPointerTos());

	cA1 = NULL;
	cA2 = NULL;

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectDehollowfication(void)
{
	CFileUtil						cFileUtil;
	CPointer						pPointer;
	CTestDoubleNamedString*			pcInternal;
	Ptr<CTestDoubleNamedString>		pDouble;
	Ptr<CTestNamedString>			pSingle;
	size							iClassSize;
	OIndex							oiOld;
	OIndex							oiNew;
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	char							szDirectory[] = "Output" _FS_ "Dehollowfication";
	bool							bResult;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsForDehollowfication();
	
	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	AssertLong(9, pcDatabase->NumIndices());
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsConstructors();
	AssertLong(9, pcDatabase->NumIndices());

	AssertTrue(gcObjects.Contains("Double"));

	pPointer = gcObjects.Get("Double");
	AssertNotNull(pPointer.Object());
	AssertString("CTestDoubleNamedString", pPointer.ClassName());

	pcInternal = (CTestDoubleNamedString*)pPointer.Object();
	AssertTrue(pcInternal->mpSplit1.IsNotNull());
	AssertTrue(pcInternal->mpSplit1.IsHollow());
	AssertTrue(pcInternal->mpSplit2.IsNotNull());
	AssertTrue(pcInternal->mpSplit2.IsHollow());

	pDouble = pPointer;

	oiOld = pDouble->mpSplit1.GetIndex();
	AssertTrue(pcInternal->mpSplit1.IsHollow());  //Making sure we haven't de-hollowed the object by calling GetIndex.

	//Problem - An oi of 1 is briefly assigned to the de-hollowed object and then it is reassigned back to its original value.
	iClassSize = pDouble->mpSplit1->ClassSize();  //The method call - ClassSize() - is irrelevant as long as the -> operator on mpSplit1 is invoked.
	AssertTrue(pcInternal->mpSplit1.IsNotNull());
	AssertFalse(pcInternal->mpSplit1.IsHollow());
	AssertSize(sizeof(CTestNamedString), iClassSize);
	AssertString("CTestNamedString", pcInternal->mpSplit1.ClassName());
	oiNew = pDouble->mpSplit1.GetIndex();
	AssertLong(oiOld, oiNew);

	pSingle = pDouble->mpSplit2;
	AssertTrue(pcInternal->mpSplit2.IsNotNull());
	AssertTrue(pcInternal->mpSplit2.IsHollow());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFlushClearGetByOid(void)
{
	CFileUtil						cFileUtil;
	CPointer						pObject;
	Ptr<CTestDoubleNamedString>		pDouble;
	Ptr<CRoot>						pRoot;
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	char							szDirectory[] = "Output" _FS_ "Dehollowfication";
	bool							bResult;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No); 
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsConstructors();
	
	pRoot = ORoot();
	pDouble = ONMalloc<CTestDoubleNamedString>("Double");
	pRoot->Add(pDouble);
	AssertLong(0, pcDatabase->NumIndices());
	pObject = gcObjects.Get(3);
	AssertNotNull(pObject.Object());
	AssertString("CTestDoubleNamedString", pObject.ClassName());

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);
	AssertLong(3, pcDatabase->NumIndices());

	pObject = gcObjects.Get(3);
	AssertNotNull(pObject.Object());
	AssertString("CTestDoubleNamedString", pObject.ClassName());
	
	pRoot = ORoot();
	AssertString("CRoot", pRoot.ClassName());

	pObject = gcObjects.Get("Double");
	AssertNotNull(pObject.Object());
	AssertString("CTestDoubleNamedString", pObject.ClassName());
	AssertString("CRoot", pRoot.ClassName());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFlushClearGetByName(void)
{
	CFileUtil						cFileUtil;
	CPointer						pObject;
	Ptr<CTestDoubleNamedString>		pDouble;
	Ptr<CRoot>						pRoot;
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	char							szDirectory[] = "Output" _FS_ "Dehollowfication";
	bool							bResult;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupObjectsConstructors();

	pRoot = ORoot();
	pDouble = ONMalloc<CTestDoubleNamedString>("Double");
	pRoot->Add(pDouble);
	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(0, pcDatabase->NumNames());
	pObject = gcObjects.Get(3);
	AssertNotNull(pObject.Object());
	AssertString("CTestDoubleNamedString", pObject.ClassName());

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);
	AssertLong(3, pcDatabase->NumIndices());
	AssertLong(2, pcDatabase->NumNames());

	pObject = gcObjects.Get("Double");
	AssertNotNull(pObject.Object());
	AssertString("CTestDoubleNamedString", pObject.ClassName());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFlushRemovesStackPointers(void)
{
	CFileUtil						cFileUtil;
	Ptr<CTestDoubleNamedString>		pDouble;
	Ptr<CRoot>						pRoot;
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	char							szDirectory[] = "Output" _FS_ "ClearPointers";
	bool							bResult;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);


	SetupObjectsConstructors();

	pRoot = ORoot();

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);
	AssertLong(2, pcDatabase->NumIndices());

	AssertNull(&pRoot);

	pDouble = ONMalloc<CTestDoubleNamedString>("Double");
	AssertString("CTestDoubleNamedString", pDouble.ClassName());

	pRoot = ORoot();
	AssertString("CRoot", pRoot.ClassName());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjects(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectsObjectKillInGraph();
	TestObjectsArrayKillInGraph();
	TestObjectsObjectKillInArrayInGraph();
	TestObjectsInMemoryIteration();
	TestObjectsObjectSave();
	TestObjectsFlushRemovesStackPointers();
	TestObjectsFlushNoClear();
	TestObjectsFlushClearGetByOid();
	TestObjectsFlushClearGetByName();
	TestObjectsFlushDurable();
	TestObjectsEvict();
	TestObjectDehollowfication();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

