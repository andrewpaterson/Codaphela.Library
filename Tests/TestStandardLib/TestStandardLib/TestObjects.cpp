#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/ValueIndexedDataConfig.h"
#include "BaseLib/ValueNamedIndexesConfig.h"
#include "BaseLib/NamedIndexedHeader.h"
#include "BaseLib/LifeCycle.h"
#include "BaseLib/Logger.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/DependentWriteObject.h"
#include "StandardLib/HollowObject.h"
#include "StandardLib/ObjectFileGeneral.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);  //TestObjectFreeOrdering.cpp
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


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
	AssertLSize(146, pDouble->SerialisedSize());
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());
	Pass();

	AssertTrue(pDouble.BaseObject()->Flush());
	AssertLong(1, pcDatabase->NumIndices());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(146, iSerialisedSize);
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());
	
	pDouble->mszString = OMalloc<CString>("A String");
	AssertTrue(pDouble.IsDirty());

	AssertTrue(pDouble.BaseObject()->Flush());
	AssertLong(1, pcDatabase->NumIndices());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(160, iSerialisedSize);
	AssertLong(1, pcDatabase->NumDataCached(NamedIndexedHeaderSize(pDouble->GetName(), iSerialisedSize)));
	AssertLong(1, pcDatabase->NumDataCached());

	pDouble->mszString = OMalloc<CString>("Different Object");

	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(160, iSerialisedSize);
	AssertTrue(pDouble.BaseObject()->Flush());
	AssertLong(1, pcDatabase->NumIndices());
	iSerialisedSize = pDouble->SerialisedSize();
	AssertLSize(160, iSerialisedSize);
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
// When the walk count wraps the stamps of the objects in memory are
// cleared, so an old stamp can't look like it is from a new walk.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsWalkStampWrap(void)
{
	Ptr<CRoot>		pRoot;
	Ptr<CString>	pString;
	uint32			uiWalk;

	ObjectsInit();

	pRoot = ORoot();
	pString = OMalloc<CString>("Walked");
	pRoot->Add(pString);

	gcObjects.TestSetWalkStamp(0xFFFFFFF0);
	uiWalk = gcObjects.StartWalk();
	AssertInt((int)0xFFFFFFF8, (int)uiWalk);
	pRoot.BaseObject()->SetWalkStamp(uiWalk | 1);
	pString.BaseObject()->SetWalkStamp(uiWalk | 3);

	uiWalk = gcObjects.StartWalk();
	AssertInt(WALK_STAMP_STEP, (int)uiWalk);
	AssertInt(0, (int)pRoot.BaseObject()->GetWalkStamp());
	AssertInt(0, (int)pString.BaseObject()->GetWalkStamp());

	pString.BaseObject()->SetWalkStamp(uiWalk | 2);
	uiWalk = gcObjects.StartWalk();
	AssertInt(2 * WALK_STAMP_STEP, (int)uiWalk);
	AssertInt(WALK_STAMP_STEP | 2, (int)pString.BaseObject()->GetWalkStamp());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The magic values files already hold: changing one means old files can't
// be read.  They are the same on every compiler and target.
//
//////////////////////////////////////////////////////////////////////////
static void AssertObjectFileMagic(const char* szExpected, int32 iMagic)
{
	AssertTrue(memcmp(szExpected, &iMagic, 4) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFileMagic(void)
{
	AssertObjectFileMagic("BAS", BASIC_OBJECT_FILE);
	AssertObjectFileMagic("CHK", CHUNKED_OBJECT_FILE);
	AssertObjectFileMagic("DAT", OBJECT_DATA);
	AssertObjectFileMagic("HEA", OBJECT_FROM_HEAP);
	AssertObjectFileMagic("NUL", OBJECT_POINTER_NULL);
	AssertObjectFileMagic("IDX", OBJECT_POINTER_ID);
	AssertObjectFileMagic("NAM", OBJECT_POINTER_NAMED);
}


//////////////////////////////////////////////////////////////////////////
//
// Dependent objects sort by the object they are for.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsDependentWriteObjectCompare(void)
{
	CBaseObject*			apcObjects[2];
	CDependentWriteObject	cFirst;
	CDependentWriteObject	cSecond;

	cFirst.Init((CBaseObject*)&apcObjects[0], false);
	cSecond.Init((CBaseObject*)&apcObjects[1], true);
	AssertTrue(CompareDependentWriteObject(&cFirst, &cSecond) < 0);
	AssertTrue(CompareDependentWriteObject(&cSecond, &cFirst) > 0);

	cSecond.Init((CBaseObject*)&apcObjects[0], true);
	AssertInt(0, CompareDependentWriteObject(&cFirst, &cSecond));
}


//////////////////////////////////////////////////////////////////////////
//
// What the three callbacks saw last, and how many times they were called.
//
//////////////////////////////////////////////////////////////////////////
static CBaseObject*	gpcTestObjectsCallbackObject = NULL;
static size			guiTestObjectsAllocated = 0;
static size			guiTestObjectsHollows = 0;
static size			guiTestObjectsDestroyed = 0;


static void TestObjectsAllocated(CBaseObject* pcObject)
{
	gpcTestObjectsCallbackObject = pcObject;
	guiTestObjectsAllocated++;
}


static void TestObjectsHollowAllocated(CBaseObject* pcObject)
{
	gpcTestObjectsCallbackObject = pcObject;
	guiTestObjectsHollows++;
}


static void TestObjectsDestroyed(CBaseObject* pcObject)
{
	gpcTestObjectsCallbackObject = pcObject;
	guiTestObjectsDestroyed++;
}


//////////////////////////////////////////////////////////////////////////
//
// Each callback is called for its own event, with the object: a hollow is
// allocated too, so that's both of those.  None is set to begin with, and
// ObjectsInit forgets them.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsCallbacks(void)
{
	CHollowObject*	pcHollow;
	CBaseObject*	pcObject;

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		guiTestObjectsAllocated = 0;
		guiTestObjectsHollows = 0;
		guiTestObjectsDestroyed = 0;
		pObject = OMalloc<CTestObject>();
		pObject = NULL;
		AssertSize(0, guiTestObjectsAllocated);
		AssertSize(0, guiTestObjectsDestroyed);

		gcObjects.SetAllocationCallback(&TestObjectsAllocated);
		gcObjects.SetHollowAllocationCallback(&TestObjectsHollowAllocated);
		gcObjects.SetDestructionCallback(&TestObjectsDestroyed);

		pObject = OMalloc<CTestObject>();
		AssertSize(1, guiTestObjectsAllocated);
		AssertSize(0, guiTestObjectsHollows);
		AssertSize(0, guiTestObjectsDestroyed);
		AssertPointer(pObject.Object(), gpcTestObjectsCallbackObject);

		pcHollow = gcObjects.AllocateInternalHollow("", gcObjects.GetIndexGenerator()->GetNext(), 1, OBJECT_IDENTIFIER_SIZE_NOT_SET);
		AssertNotNull(pcHollow);
		AssertSize(2, guiTestObjectsAllocated);
		AssertSize(1, guiTestObjectsHollows);
		AssertPointer(pcHollow, gpcTestObjectsCallbackObject);

		pcObject = pObject.BaseObject();
		pObject = NULL;
		AssertSize(1, guiTestObjectsDestroyed);
		AssertPointer(pcObject, gpcTestObjectsCallbackObject);
	}
	ObjectsFlush();
	ObjectsKill();

	//The hollow was left for ObjectsKill to free.
	AssertSize(2, guiTestObjectsDestroyed);

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		pObject = OMalloc<CTestObject>();
		AssertSize(2, guiTestObjectsAllocated);
		pObject = NULL;
		AssertSize(2, guiTestObjectsDestroyed);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// ValidateEmpty says how many objects are left, and nothing if none are.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsValidateEmpty(void)
{
	CMemoryFile	cLog;
	SLogConfig	sLogConfig;
	char		szLogged[2048];

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		StartCapturingLog(&cLog, &sLogConfig);
		gcObjects.ValidateEmpty();
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertString("", szLogged);

		pObject = OMalloc<CTestObject>();
		StartCapturingLog(&cLog, &sLogConfig);
		gcObjects.ValidateEmpty();
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertNotNull(strstr(szLogged, "1 objects are still indexed."));

		pObject = NULL;
		StartCapturingLog(&cLog, &sLogConfig);
		gcObjects.ValidateEmpty();
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertString("", szLogged);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object that can't be saved.
//
//////////////////////////////////////////////////////////////////////////
class CTestSaveFails : public CObject
{
CONSTRUCTABLE(CTestSaveFails)
DESTRUCTABLE(CTestSaveFails)
public:
	void Init(void)
	{
		PreInit();
		PostInit();
	}

	void Class(void) override
	{
	}

	void Free(void) override
	{
	}

	bool Save(CObjectWriter* pcFile) override
	{
		return false;
	}

	bool Load(CObjectReader* pcFile) override
	{
		return false;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A failure to write or read an object says which object it was.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsWriteAndReadFailuresNameTheObject(void)
{
	CMemoryFile		cLog;
	SLogConfig		sLogConfig;
	char			szLogged[1024];
	CMemoryFile		cMemory;
	CObjectWriter	cWriter;
	CObjectReader	cReader;

	ObjectsInit();
	{
		Ptr<CTestSaveFails>	pObject;

		pObject = ONMalloc<CTestSaveFails>("Unsaveable");

		cMemory.Init();
		cMemory.Open(EFM_ReadWrite_Create);
		cWriter.Init(&cMemory, NULL);
		StartCapturingLog(&cLog, &sLogConfig);
		AssertFalse(cWriter.Write(&pObject));
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertNotNull(strstr(szLogged, "Could not Save() object ["));
		AssertNotNull(strstr(szLogged, "Unsaveable"));
		cMemory.Close();
		cWriter.Kill();
		cMemory.Kill();

		//Nothing to read.
		cMemory.Init();
		cMemory.Open(EFM_ReadWrite_Create);
		cReader.Init(&cMemory, NULL);
		StartCapturingLog(&cLog, &sLogConfig);
		AssertFalse(cReader.ReadHeapFroms(&pObject));
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertNotNull(strstr(szLogged, "Could not read object stream size"));
		AssertNotNull(strstr(szLogged, "Unsaveable"));
		cMemory.Close();
		cReader.Kill();
		cMemory.Kill();

		//A length, then something that isn't the froms' magic.
		cMemory.Init();
		cMemory.Open(EFM_ReadWrite_Create);
		cWriter.Init(&cMemory, NULL);
		AssertTrue(cWriter.WriteSize((lsize)8));
		AssertTrue(cWriter.WriteInt32(12345));
		AssertTrue(cMemory.Seek(0, EFSO_SET));
		cReader.Init(&cMemory, NULL);
		StartCapturingLog(&cLog, &sLogConfig);
		AssertFalse(cReader.ReadHeapFroms(&pObject));
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertNotNull(strstr(szLogged, "Could not read object froms magic"));
		AssertNotNull(strstr(szLogged, "Unsaveable"));
		cMemory.Close();
		cReader.Kill();
		cWriter.Kill();
		cMemory.Kill();

		pObject = NULL;
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An argument that counts its copies.
//
//////////////////////////////////////////////////////////////////////////
static size guiTestMallocArgumentCopies = 0;

class CTestMallocArgument
{
public:
	int32	miValue;

	CTestMallocArgument(int32 iValue)
	{
		miValue = iValue;
	}

	CTestMallocArgument(const CTestMallocArgument& cOther)
	{
		miValue = cOther.miValue;
		guiTestMallocArgumentCopies++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// An object with an Init for each kind of argument OMalloc passes on.
//
//////////////////////////////////////////////////////////////////////////
class CTestMallocArguments : public CObject
{
CONSTRUCTABLE(CTestMallocArguments)
DESTRUCTABLE(CTestMallocArguments)
public:
	int32	miValue;
	size	muiStackFromsInInit;

	void Set(int32 iValue)
	{
		PreInit();
		miValue = iValue;
		PostInit();
	}

	void Init(void)								{ Set(1); }
	void Init(CTestMallocArgument& cArgument)	{ cArgument.miValue = 7; Set(cArgument.miValue); }
	void Init(const CTestMallocArgument& cArgument, int32 iAdd)	{ Set(cArgument.miValue + iAdd); }
	void Init(CTestMallocArgument cArgument, bool bFlag)		{ Set(bFlag ? cArgument.miValue : 0); }
	void Init(int32 iFirst, int32 iSecond)		{ Set(iFirst * 100 + iSecond); }
	void Init(const char* sz)					{ Set(sz[1]); }
	void Init(char* sz)							{ sz[0] = 'Y'; Set(2); }
	void Init(int32* pai)						{ Set(pai[1]); }
	void Init(int32 (*fGet)(void), bool bFlag)	{ Set(fGet()); }

	void Init(Ptr<CTestObject> pObject)
	{
		muiStackFromsInInit = pObject.Object()->NumStackFroms();
		Set(3);
	}

	void Class(void) override
	{
	}

	void Free(void) override
	{
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 TestObjectsMallocGetValue(void)
{
	return 44;
}


//////////////////////////////////////////////////////////////////////////
//
// OMalloc and ONMalloc give Init the arguments they were given, not copies:
// an Init taking a reference changes the caller's variable, a const
// reference copies nothing, and a by value parameter is the one copy.  A
// stack pointer's only other stack from is Init's parameter.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsMallocArgumentsNotCopied(void)
{
	STestObjectFreedNotifier	sNotifier;
	CTestMallocArgument			cArgument(1);
	const CTestMallocArgument	cConstArgument(5);

	ObjectsInit();
	{
		Ptr<CTestMallocArguments>	pMade;
		Ptr<CTestObject>			pObject;

		guiTestMallocArgumentCopies = 0;
		pMade = OMalloc<CTestMallocArguments>(cArgument);
		AssertInt((int32)7, cArgument.miValue);
		AssertInt((int32)7, pMade->miValue);
		AssertSize(0, guiTestMallocArgumentCopies);

		cArgument.miValue = 1;
		pMade = ONMalloc<CTestMallocArguments>("Named", cArgument);
		AssertInt((int32)7, cArgument.miValue);
		AssertSize(0, guiTestMallocArgumentCopies);

		pMade = OMalloc<CTestMallocArguments>(cConstArgument, (int32)10);
		AssertInt((int32)15, pMade->miValue);
		AssertSize(0, guiTestMallocArgumentCopies);

		pMade = OMalloc<CTestMallocArguments>(cArgument, true);
		AssertInt((int32)7, pMade->miValue);
		AssertSize(1, guiTestMallocArgumentCopies);

		pObject = OMalloc<CTestObject>();
		AssertSize(1, pObject.Object()->NumStackFroms());
		pMade = OMalloc<CTestMallocArguments>(pObject);
		AssertSize(2, pMade->muiStackFromsInInit);
		AssertSize(1, pObject.Object()->NumStackFroms());

		//A temporary outlasts the call to Init, and is gone after it.
		pMade = OMalloc<CTestMallocArguments>(OMalloc<CTestObject>(&sNotifier));
		AssertSize(2, pMade->muiStackFromsInInit);
		AssertTrue(sNotifier.bFreed);

		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// What has no address, and what decays, still reaches an Init that takes it
// by value: literals, sums, string literals of any length, arrays (their
// elements, not a copy) and functions.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsMallocArgumentsValues(void)
{
	char	acChars[3];
	int32	aiValues[2];
	int32	i;

	ObjectsInit();
	{
		Ptr<CTestMallocArguments>	pMade;
		Ptr<CString>				pString;

		i = 4;
		pMade = OMalloc<CTestMallocArguments>(5, i + 2);
		AssertInt((int32)506, pMade->miValue);

		pMade = OMalloc<CTestMallocArguments>("AB");
		AssertInt((int32)'B', pMade->miValue);
		pMade = OMalloc<CTestMallocArguments>("CDEFG");
		AssertInt((int32)'D', pMade->miValue);

		acChars[0] = 'Z';
		acChars[1] = 'X';
		pMade = OMalloc<CTestMallocArguments>(acChars);
		AssertInt((int32)2, pMade->miValue);
		AssertInt((int32)'Y', (int32)acChars[0]);

		aiValues[1] = 33;
		pMade = OMalloc<CTestMallocArguments>(aiValues);
		AssertInt((int32)33, pMade->miValue);

		pMade = OMalloc<CTestMallocArguments>(TestObjectsMallocGetValue, true);
		AssertInt((int32)44, pMade->miValue);

		pString = OString("Door");
		AssertString("Door", pString->Text());
		pString = OString(pString);
		AssertString("Door", pString->Text());

		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// LifeAlloc gives Init the arguments it was given too.
//
//////////////////////////////////////////////////////////////////////////
class CTestLifeAllocated : public CKillable
{
public:
	int32	miValue;

	void Init(CTestMallocArgument& cArgument, const CTestMallocArgument& cOther)
	{
		cArgument.miValue = 8;
		miValue = cOther.miValue;
	}

	void Kill(void) override
	{
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsLifeAllocArgumentsNotCopied(void)
{
	CTestMallocArgument			cArgument(1);
	CTestMallocArgument			cOther(9);
	CLifeInit<CKillable>		cInit;
	CLife<CKillable>			cLife;
	CKillable*					pcKillable;

	guiTestMallocArgumentCopies = 0;
	cInit = LifeAlloc<CTestLifeAllocated, CKillable>(cArgument, cOther);
	AssertInt((int32)8, cArgument.miValue);
	AssertSize(0, guiTestMallocArgumentCopies);

	cInit.ConfigureLife(&cLife, &pcKillable);
	AssertInt((int32)9, ((CTestLifeAllocated*)pcKillable)->miValue);
	cLife.Kill();
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
	TestObjectsWalkStampWrap();
	TestObjectsMallocArgumentsNotCopied();
	TestObjectsMallocArgumentsValues();
	TestObjectsLifeAllocArgumentsNotCopied();
	TestObjectsWriteAndReadFailuresNameTheObject();
	TestObjectsFileMagic();
	TestObjectsDependentWriteObjectCompare();
	TestObjectsCallbacks();
	TestObjectsValidateEmpty();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

