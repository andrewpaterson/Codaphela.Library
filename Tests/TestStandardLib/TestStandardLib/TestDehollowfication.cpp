#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ObjectConverterText.h"
#include "StandardLib/ObjectConverterNative.h"
#include "StandardLib/String.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/FatHollowConfiguration.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetupDehollowficationConstructors(void)
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
Ptr<CTestDoubleNamedString> SetupDehollowficationScene(void)
{
	Ptr<CTestNamedString>			cNS1;
	Ptr<CTestNamedString>			cNS2;
	Ptr<CTestNamedString>			cDiamond;
	Ptr<CTestDoubleNamedString>		cDouble;
	Ptr<CString>					sz1;
	Ptr<CString>					sz3;
	Ptr<CString>					sz2;
	Ptr<CString>					sz4;
	Ptr<CRoot>						cRoot;
	Ptr<CTestNamedString>			cNS3;

	cRoot = ORoot();

	sz3 = OMalloc<CString>("End");

	cDiamond = ONMalloc<CTestNamedString>("Diamond End", sz3, Null(), "Before Swine");  //cDiamond->mpAnother = sz3
	sz1 = OMalloc<CString>("World");

	cNS1 = ONMalloc<CTestNamedString>("NamedString 1", sz1, cDiamond, "Hello");		//cNS1->mpAnother = cDiamond

	sz2 = OMalloc<CString>("6789");
	cNS2 = ONMalloc<CTestNamedString>("NamedString 2", sz2, cDiamond, "12345");		//cNS2->mpAnother = cDiamond

	sz4 = OMalloc<CString>("Start");
	cDouble = ONMalloc<CTestDoubleNamedString>("Double Start", sz4, cNS1, Null());  //cDouble->szString = sz4
																					//cDouble->mpSplit2 = cNS1
	cRoot->Add(cDouble);

	cDouble->mpSplit1 = cNS2;

	cNS3 = ONMalloc<CTestNamedString>("NamedString 3", sz1, Null(), "Random");

	cRoot->Add(cNS3);

//
//      sz3
//       |
//    cDiamond
//      / \
// sz2 /   \  sz1
//  | /     \  |
//  cNS2    cNS1
//    \     /
//     \   / sz4
//      \ /  /
//    cDouble   sz1 
//       \       |
//        \    cNS3
//         \   /
//          \ /
//          Set
//           |
//         cRoot
//       

	return cDouble;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WriteDehollowficationChunkedFile(void)
{
	Ptr<CTestDoubleNamedString>		cDouble;
	CChunkFileObjectWriter			cWriter;
	CExternalObjectSerialiser		cGraphSerialiser;

	cDouble = SetupDehollowficationScene();

	cWriter.Init("Output" _FS_ "Dehollowfication" _FS_ "ChunkFile", "", "Double");
	cGraphSerialiser.Init(&cWriter);
	AssertTrue(cGraphSerialiser.Write(&cDouble));
	cGraphSerialiser.Kill();
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDehollowficationFromDatabaseSimple(void)
{
	Ptr<CString>				pString1;
	Ptr<CString>				pString2;
	Ptr<CString>				pString3;
	Ptr<CTestEmbeddedStrings>	pContainer;
	OIndex						oiContainer;
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CFileUtil					cFileUtil;
	Ptr<CRoot>					pRoot;
	char						szDirectory[] = "Output" _FS_ "Dehollowfication" _FS_ "Database3";

	cFileUtil.RemoveDir(szDirectory);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.AddConstructor<CTestEmbeddedStrings>();

	pRoot = ORoot();
	pContainer = OMalloc<CTestEmbeddedStrings>();
	pRoot->Add(pContainer);

	oiContainer = pContainer->GetIndex();

	pContainer->mString1.Set("Goldberry is first");
	pContainer->mString2.Set("Jane is great");
	pContainer->mString3.Set("Baby Wooglers");

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pContainer = gcObjects.Get(oiContainer);
	AssertTrue(pContainer.IsNotNull());

	pString1 = &pContainer->mString1;
	pString2 = &pContainer->mString2;
	pString3 = &pContainer->mString3;

	AssertString("Goldberry is first", pString1->Text());
	AssertString("Jane is great", pString2->Text());
	AssertString("Baby Wooglers", pString3->Text());

	ObjectsFlush();
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
void TestDehollowficationFromDatabaseComplex(void)
{
	CFileUtil		cFileUtil;
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	size			uiSize;
	bool			bResult;
	CBaseObject*	pcObject;
	char			szDirectory[] = "Output" _FS_ "Dehollowfication" _FS_ "Database1";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	
	SetupDehollowficationScene();
	
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupDehollowficationConstructors();
	AssertLong(0, gcUnknowns.NumElements());
	AssertSize(0, gcObjects.GetStackPointers()->NumElements());

	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertLong(0, gcObjects.NumMemoryNames());
	AssertLong(0, pcDatabase->NumIndicesCached());
	AssertLong(11, pcDatabase->NumIndices());
	AssertLong(6, pcDatabase->NumNames());
	Pass();

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	Ptr<CRoot> pRoot = ORoot();

	AssertTrue(pRoot->IsSetHollow());
	Pass();

	AssertLong(2, gcUnknowns.NumElements());
	AssertLong(2, gcObjects.NumMemoryIndexes());
	AssertLong(1, gcObjects.NumMemoryNames());
	AssertLong(1, pcDatabase->NumIndicesCached());
	AssertLong(6, pcDatabase->NumNames());
	AssertSize(1, gcObjects.GetStackPointers()->NumElements());
	Pass();

	Ptr<CTestDoubleNamedString> pDouble = pRoot->Get<CTestDoubleNamedString>("Double Start");  //This will cause pRoot.Set to be dehollowed.
	AssertSize(2, gcObjects.GetStackPointers()->NumElements());
	AssertFalse(pRoot->IsSetHollow());
	AssertTrue(pDouble.IsNotNull());
	AssertTrue(pDouble.IsHollow());
	AssertLong(10, pDouble.GetIndex());
	AssertLong(4LL, gcUnknowns.NumElements());
	AssertLong(4LL, gcObjects.NumMemoryIndexes());
	Pass();

	AssertString("12345", pDouble->mpSplit1->mszEmbedded.Text()); //This will cause pDouble and pDouble.mpSplit1 to be dehollowed.
	AssertLong(9LL, gcUnknowns.NumElements());
	AssertLong(9LL, gcObjects.NumMemoryIndexes());
	AssertFalse(pDouble.IsHollow());
	AssertLong(10, pDouble.GetIndex());
	AssertSize(2, gcObjects.GetStackPointers()->NumElements());
	AssertFalse(pDouble->mpSplit1.IsHollow());
	AssertLong(8LL, pDouble->mpSplit1.GetIndex());
	AssertString("NamedString 2", pDouble->mpSplit1.GetName());
	pcObject = gcObjects.GetMemory()->GetObjects()->Get(pDouble->mpSplit1.GetIndex());  //Why Is 8LL not in the index?
	bResult = gcObjects.GetMemory()->GetNames()->Get((uint8*)pDouble->mpSplit1.GetName(), 13, NULL, &uiSize, 0);  //Why Is "NamedString 2" not in the index?
	AssertNotNull(pcObject);
	AssertTrue(bResult);
	AssertTrue(pDouble->mpSplit1->mpAnother.IsHollow());
	AssertTrue(pDouble->mpSplit2.IsHollow());
	AssertTrue(pDouble->mpSplit1->mszString.IsHollow());
	AssertString("Diamond End", pDouble->mpSplit1->mpAnother.GetName());
	AssertString("6789", pDouble->mpSplit1->mszString->Text());
	AssertFalse(pDouble->mpSplit1->mszString.IsHollow());
	Pass();

	AssertSize(2, gcObjects.GetStackPointers()->NumElements());
	Pass();

	Ptr<CTestNamedString> pDiamond = pDouble->mpSplit1->mpAnother;
	AssertLong(9, gcUnknowns.NumElements());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertSize(3, gcObjects.GetStackPointers()->NumElements());
	AssertTrue(pDouble->mpSplit1->mpAnother.IsHollow());
	AssertTrue(pDiamond.IsHollow());
	Pass();

	pDiamond->ClassName(); //This will cause pDiamond to be de-hollowed

	AssertFalse(pDiamond.IsHollow());
	AssertFalse(pDouble->mpSplit1->mpAnother.IsHollow());
	AssertLong(4LL, pDouble->mpSplit1->mpAnother.GetIndex());
	AssertString("Diamond End", pDouble->mpSplit1->mpAnother.GetName());
	Pass();

	AssertLong(10LL, gcUnknowns.NumElements());
	AssertLong(10LL, gcObjects.NumMemoryIndexes());
	AssertLong(6LL, gcObjects.NumMemoryNames());
	AssertLong(6LL, pcDatabase->NumNames());
	Pass();

	AssertTrue(pDouble->mpSplit2.IsHollow());
	pDouble->mpSplit2->ClassSize(); //This will cause mpSplit2 to be de-hollowed
	AssertFalse(pDouble->mpSplit2.IsHollow());
	AssertNotNull(&pDouble->mpSplit2->mpAnother);
	AssertLong(4LL, pDouble->mpSplit2->mpAnother.GetIndex());
	AssertString("Diamond End", pDouble->mpSplit2->mpAnother.GetName());
	AssertFalse(pDouble->mpSplit2->mpAnother.IsHollow());
	AssertPointer(pDiamond.Object(), pDouble->mpSplit2->mpAnother.Object());
	Pass();

	AssertLong(11, gcObjects.NumMemoryIndexes());
	AssertLong(6, gcObjects.NumMemoryNames());
	AssertLong(6, pcDatabase->NumNames());
	Pass();

	Ptr<CTestNamedString> pRandom = pRoot->Get<CTestNamedString>("NamedString 3");
	AssertTrue(pRandom.IsNotNull());
	AssertTrue(pRandom.IsHollow());
	Pass();

	AssertTrue(pRandom->mszString.IsHollow());
	AssertFalse(pRandom.IsHollow());
	AssertTrue(pDouble->mpSplit2->mszString.IsHollow());
	Pass();

	AssertString("World", pRandom->mszString->Text());
	AssertFalse(pRandom->mszString.IsHollow());
	AssertFalse(pDouble->mpSplit2->mszString.IsHollow());
	AssertPointer(pRandom->mszString.Object(), pDouble->mpSplit2->mszString.Object());
	Pass();

	AssertLong(11, pcDatabase->NumIndices());
	Pass();

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	AssertTrue(pRoot.IsNull());
	AssertTrue(pDouble.IsNull());
	AssertTrue(pDiamond.IsNull());
	AssertTrue(pRandom.IsNull());
	Pass();

	pDiamond.UnsafeClearObject();
	
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDehollowficationFromDatabaseOfTwoPointers(void)
{
	CFileUtil							cFileUtil;
	CCodabase*							pcDatabase;
	CSequence*							pcSequence;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	char								szDirectory[] = "Output" _FS_ "Dehollowfication" _FS_ "Database2";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	SetupDehollowficationScene();

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupDehollowficationConstructors();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CTestDoubleNamedString> pDouble = pRoot->Get<CTestDoubleNamedString>("Double Start");

	Ptr<CTestNamedString> pString1 = pDouble->mpSplit1;
	Ptr<CTestNamedString> pString2 = pDouble->mpSplit2;
	pString1->ClassName();
	pString2->ClassName();

	Ptr<CTestNamedString> pDiamond1 = pString1->mpAnother;
	Ptr<CTestNamedString> pDiamond2 = pString2->mpAnother;

	AssertTrue(pDiamond1.IsHollow());
	AssertTrue(pDiamond2.IsHollow());
	AssertPointer(pDiamond1.Object(), pDiamond2.Object());
	AssertPointer(pString1->mpAnother.Object(), pString2->mpAnother.Object());
	AssertLong(4LL, pDiamond1.GetIndex());
	AssertLong(4LL, pDiamond2.GetIndex());

	//Dehollofication of pDiamond1 happens here. pString1->mpAnother and pString2->mpAnother are remapped to the dehollowed diamond object.
	pDiamond1->ClassName();

	AssertFalse(pDiamond1.IsHollow());
	AssertFalse(pDiamond2.IsHollow());  //This should be false but it's not until I remap local pointers.
	AssertPointer(pDiamond1.Object(), pDiamond2.Object());  //These should be the same but they're not until I remap local pointers.
	AssertLong(4LL, pDiamond1.GetIndex());

	AssertFalse(pString1->mpAnother.IsHollow());
	AssertFalse(pString2->mpAnother.IsHollow());
	AssertPointer(pString1->mpAnother.Object(), pString2->mpAnother.Object());

	ObjectsFlush();
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
void TestDehollowficationFromChunkFileSource(void)
{
	CFileUtil		cFileUtil;
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	char			szDirectory[] = "Output" _FS_ "Dehollowfication" _FS_ "Temp";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	WriteDehollowficationChunkedFile();

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
	cFileUtil.RemoveDir(szDirectory);

	CObjectSourceChunked* pcObjectSourceChunked;


	CDiskFile*	pcDiskFile = DiskFile("Output" _FS_ "Dehollowfication" _FS_ "ChunkFile" _FS_ "Double.DRG");

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Dehollowfication" _FS_ "Temp", IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupDehollowficationConstructors();

	pcObjectSourceChunked = (CObjectSourceChunked*)gcObjects.AddSource<CObjectConverterNative>(pcDiskFile, "Double");  //Note the .DRG is intentionally dropped.
	AssertNotNull(pcObjectSourceChunked);

	AssertSize(4, pcObjectSourceChunked->NumNames());
	AssertString("Diamond End", pcObjectSourceChunked->GetName(0));
	AssertString("Double Start", pcObjectSourceChunked->GetName(1));
	AssertString("NamedString 1", pcObjectSourceChunked->GetName(2));
	AssertString("NamedString 2", pcObjectSourceChunked->GetName(3));

	Ptr<CTestDoubleNamedString> pDouble;

	pDouble = gcObjects.Get("Double Start");
	AssertTrue(pDouble.IsNotNull());
	AssertString("CTestDoubleNamedString", pDouble.ClassName());

	AssertTrue(pDouble->mpSplit1.IsHollow());
	AssertTrue(pDouble->mpSplit2.IsHollow());
	AssertFalse(pDouble->mszString.IsHollow());

	pDouble->mpSplit1->ClassName();
	AssertFalse(pDouble->mpSplit1.IsHollow());
	AssertString("CTestNamedString", pDouble->mpSplit1.ClassName());

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
void TestDehollowficationFatFromDatabaseSimple(void)
{
	ForceFatHollows();

	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "Dehollowfication" _FS_ "Database3";

	cFileUtil.RemoveDir(szDirectory);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.AddConstructor<CTestEmbeddedStrings>();
	{
		Ptr<CRoot>								pRoot;
		Ptr<CTestEmbeddedStrings>				pContainer;
		Ptr<CPointerContainer>					pPtr1;
		Ptr<CPointerContainer>					pPtr2;
		Ptr<CPointerContainer>					pPtr3;
		CArrayBaseObjectPtrE32		apcFromArray;

		pRoot = ORoot();
		pContainer = ONMalloc<CTestEmbeddedStrings>("Embedded");
		pPtr1 = ONMalloc<CPointerContainer>("1");
		pPtr2 = ONMalloc<CPointerContainer>("2");
		pPtr3 = ONMalloc<CPointerContainer>("3");
		pRoot->Add(pPtr1);
		pRoot->Add(pPtr2);
		pRoot->Add(pPtr3);

		pPtr1->mp = &pContainer;
		pPtr2->mp = &pContainer;
		pPtr3->mp = &pContainer;

		AssertPointer(&pPtr1, pPtr1->mp.Embedding());
		AssertString("CTestEmbeddedStrings", pPtr1->mp.ClassName());
		AssertPointer(&pPtr2, pPtr2->mp.Embedding());
		AssertString("CTestEmbeddedStrings", pPtr2->mp.ClassName());
		AssertPointer(&pPtr3, pPtr3->mp.Embedding());
		AssertString("CTestEmbeddedStrings", pPtr3->mp.ClassName());

		pContainer->mString1.Set("Goldberry is first");
		pContainer->mString2.Set("Jane is great");
		pContainer->mString3.Set("Baby Wooglers");

		apcFromArray.Init();
		pPtr1->GetHeapFroms(&apcFromArray);
		AssertSize(1, apcFromArray.NumElements());
		AssertPointer(pRoot->TestGetSet(), apcFromArray.GetPtr(0));
		apcFromArray.Kill();

		apcFromArray.Init();
		pPtr2->GetHeapFroms(&apcFromArray);
		AssertSize(1, apcFromArray.NumElements());
		AssertPointer(pRoot->TestGetSet(), apcFromArray.GetPtr(0));
		apcFromArray.Kill();

		apcFromArray.Init();
		pPtr3->GetHeapFroms(&apcFromArray);
		AssertSize(1, apcFromArray.NumElements());
		AssertPointer(pRoot->TestGetSet(), apcFromArray.GetPtr(0));
		apcFromArray.Kill();

		apcFromArray.Init();
		pContainer->GetHeapFroms(&apcFromArray);
		AssertSize(3, apcFromArray.NumElements());
		AssertPointer(&pPtr1, apcFromArray.GetPtr(0));
		AssertPointer(&pPtr2, apcFromArray.GetPtr(1));
		AssertPointer(&pPtr3, apcFromArray.GetPtr(2));
		apcFromArray.Kill();
	}
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CRoot>								pRoot;
		Ptr<CTestEmbeddedStrings>				pContainer;
		Ptr<CTestEmbeddedStrings>				pContainer2;
		Ptr<CPointerContainer>					pPtr1;
		Ptr<CPointerContainer>					pPtr2;
		Ptr<CPointerContainer>					pPtr3;
		Ptr<CString>							pString1;
		Ptr<CString>							pString2;
		Ptr<CString>							pString3;
		CArrayBaseObjectPtrE32		apcFromArray;
		Ptr<CSetObject>							pRootSet2;

		pPtr1 = gcObjects.Get("1");
		pPtr2 = gcObjects.Get("2");
		pPtr3 = gcObjects.Get("3");

		AssertString("CHollowObject", pPtr1->PointerObject()->ClassName());
		AssertTrue(((CHollowObject*)pPtr1->PointerObject())->IsFatHollow());
		AssertString("CHollowObject", pPtr2->PointerObject()->ClassName());
		AssertTrue(((CHollowObject*)pPtr2->PointerObject())->IsFatHollow());
		AssertString("CHollowObject", pPtr3->PointerObject()->ClassName());
		AssertTrue(((CHollowObject*)pPtr3->PointerObject())->IsFatHollow());

		AssertPointer(&pPtr1, pPtr1->mp.Embedding());
		&(pPtr1->mp);
		AssertPointer(&pPtr1, pPtr1->mp.Embedding());
		AssertString("CTestEmbeddedStrings", pPtr1->PointerObject()->ClassName());

		AssertPointer(&pPtr2, pPtr2->mp.Embedding());
		&(pPtr2->mp);
		AssertPointer(&pPtr2, pPtr2->mp.Embedding());
		AssertString("CTestEmbeddedStrings", pPtr2->PointerObject()->ClassName());

		AssertPointer(&pPtr3, pPtr3->mp.Embedding());
		&(pPtr3->mp);
		AssertPointer(&pPtr3, pPtr3->mp.Embedding());
		AssertString("CTestEmbeddedStrings", pPtr3->PointerObject()->ClassName());

		pContainer = pPtr1->mp;

		pString1 = &pContainer->mString1;
		pString2 = &pContainer->mString2;
		pString3 = &pContainer->mString3;

		AssertString("Goldberry is first", pString1->Text());
		AssertString("Jane is great", pString2->Text());
		AssertString("Baby Wooglers", pString3->Text());

		pContainer2 = gcObjects.Get("Embedded");
		AssertPointer(&pContainer2, &pContainer);

		pRoot = ORoot();
		pRootSet2 = pRoot->GetAll();
		AssertString("CSetObject", pRootSet2.ClassName());
	}
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);


	cFileUtil.RemoveDir(szDirectory);

	UnforceFatHollows();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDehollowficationFatFromDatabaseEmbeddedPointer(void)
{
	ForceFatHollows();

	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "Dehollowfication" _FS_ "Database4";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.AddConstructor<CTestEmbeddedStrings>();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestEmbeddedStrings>	pContainer;
		Ptr<CPointerContainer>		pPtr1;
		Ptr<CPointerContainer>		pPtr2;

		pRoot = ORoot();
		pContainer = ONMalloc<CTestEmbeddedStrings>("Embedded");
		pPtr1 = ONMalloc<CPointerContainer>("1");
		pPtr2 = ONMalloc<CPointerContainer>("2");
		pRoot->Add(pPtr1);
		pRoot->Add(pPtr2);

		pPtr1->mp = &pContainer;
		pPtr2->mp = &pContainer->mString2;

		pContainer->mString1.Set("Goldberry is first");
		pContainer->mString2.Set("Jane is great");
		pContainer->mString3.Set("Baby Wooglers");

		AssertSize(1, pContainer->CEmbeddedObject::NumHeapFroms());
		AssertSize(1, pContainer->mString2.NumHeapFroms());
	}
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CTestEmbeddedStrings>	pContainer;
		Ptr<CPointerContainer>		pPtr1;
		Ptr<CPointerContainer>		pPtr2;
		Ptr<CString>				pString2;
		CEmbeddedObject*			pcHollowEmbedded;

		//Loading "2" first makes the fat hollow with a heap from on its embedded hollow for mString2.
		pPtr2 = gcObjects.Get("2");
		pPtr1 = gcObjects.Get("1");

		AssertString("CHollowObject", pPtr1->PointerObject()->ClassName());
		AssertTrue(((CHollowObject*)pPtr1->PointerObject())->IsFatHollow());
		pcHollowEmbedded = pPtr2->mp.Object();
		AssertTrue(pcHollowEmbedded->IsHollow());
		AssertPointer(pPtr1->mp.Object(), pcHollowEmbedded->GetEmbeddingContainer());
		AssertSize(1, pcHollowEmbedded->CEmbeddedObject::NumHeapFroms());

		&(pPtr1->mp);
		AssertString("CTestEmbeddedStrings", pPtr1->PointerObject()->ClassName());
		pContainer = pPtr1->mp;

		//The pointer to the embedded hollow must now point to the real embedded object.
		AssertPointer(&pContainer->mString2, pPtr2->mp.Object());
		AssertSize(1, pContainer->mString2.NumHeapFroms());
		AssertPointer(&pPtr2, pContainer->mString2.GetHeapFrom(0));
		AssertSize(1, pContainer->CEmbeddedObject::NumHeapFroms());
		AssertSize(0, pContainer->mString1.NumHeapFroms());
		AssertSize(0, pContainer->mString3.NumHeapFroms());

		pString2 = pPtr2->mp;
		AssertString("Jane is great", pString2->Text());
	}
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	cFileUtil.RemoveDir(szDirectory);

	UnforceFatHollows();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDehollowfication(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestDehollowficationFromDatabaseSimple();
	TestDehollowficationFromDatabaseComplex();
	TestDehollowficationFromDatabaseOfTwoPointers();
	TestDehollowficationFromChunkFileSource();
	TestDehollowficationFatFromDatabaseSimple();
	TestDehollowficationFatFromDatabaseEmbeddedPointer();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

