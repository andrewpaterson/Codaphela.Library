#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "TestLib/Assert.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer SetupObjectReaderChunkedChunkFile(void)
{
	Ptr<CTestNamedString>	cNS1;
	Ptr<CTestNamedString>	cNS2;
	Ptr<CTestNamedString>	cNS3;
	Ptr<CTestNamedString>	cNS4;
	Ptr<CTestInteger>		cI1;
	Ptr<CTestInteger>		cI2;
	Ptr<CTestInteger>		cI3;
	Ptr<CTestWithArray>		cA1;
	Ptr<CTestWithArray>		cA2;
	Ptr<CString>			sz1;
	Ptr<CString>			sz2;
	Ptr<CString>			sz3;

	cNS1 = ONMalloc<CTestNamedString>("NamedString 1");
	cNS2 = ONMalloc<CTestNamedString>("NamedString 2");
	cNS3 = ONMalloc<CTestNamedString>("NamedString 3");
	cNS4 = ONMalloc<CTestNamedString>("NamedString 4");

	cI1 = OMalloc<CTestInteger>(3, 2, 1);
	AssertLong(5LL, cI1->GetIndex());
	cI2 = OMalloc<CTestInteger>(543, 3, 4);
	AssertLong(6LL, cI2->GetIndex());
	cI3 = OMalloc<CTestInteger>(10, 8192, 7);
	AssertLong(7LL, cI3->GetIndex());

	cA1 = ONMalloc<CTestWithArray>("Array 1", "Something with One", 1);
	cA2 = ONMalloc<CTestWithArray>("Array X", "An with 2", 2);

	sz1 = OMalloc<CString>("String 1");
	sz2 = OMalloc<CString>("Ye!");
	sz3 = OMalloc<CString>("3");

	cNS1->Set(sz1, cNS2, "In Named 1");
	cNS2->Set(sz1, cNS3, "Another in 2");
	cNS3->Set(sz3, cNS1, "Three");
	cNS4->Set(Null(), Null(), "Nulloid!");

	cA1->Add(sz2);
	cA1->Add(cNS1);
	cA1->Add(cI1);
	cA1->Add(cNS3);
	cA1->Add(cI2);
	cA1->Add(cA2);

	cA2->Add(cI3);
	cA2->Add(cNS2);
	cA2->Add(cI1);
	cA2->Add(cNS4);

	return cA1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int WriteObjectReaderChunkedFile(void)
{
	CPointer				cBase;
	CChunkFileObjectWriter	cWriter;
	CExternalObjectSerialiser	cGraphSerialiser;

	cBase = SetupObjectReaderChunkedChunkFile();

	cWriter.Init("Output" _FS_ "ObjectReaderChunked" _FS_ "Test" _FS_, "", "Reader");
	cGraphSerialiser.Init(&cWriter);
	AssertTrue(cGraphSerialiser.Write(cBase.BaseObject()));
	cGraphSerialiser.Kill();
	cWriter.Kill();

	return (int)gcObjects.NumMemoryIndexes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReaderChunkedDeserialised(void)
{
	CFileUtil	cFileUtil;
	char		szDirectory[] = "Output" _FS_ "ObjectReaderChunked";

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	CChunkFileSystemObjectReader	cReader;
	CExternalObjectDeserialiser		cDeserialiser;
	CPointer						cBase;
	Ptr<CTestWithArray>				cA1;
	Ptr<CTestWithArray>				cA2;
	Ptr<CTestNamedString>			cNS1;
	Ptr<CTestNamedString>			cNS2;
	Ptr<CTestNamedString>			cNS3;
	Ptr<CTestNamedString>			cNS4;
	Ptr<CString>					sz1;
	Ptr<CString>					sz2;
	Ptr<CString>					sz3;
	CPointer						cTemp;
	Ptr<CTestInteger>				cI1;
	Ptr<CTestInteger>				cI2;
	Ptr<CTestInteger>				cI3;
	OIndex							oiI1;
	OIndex							oiI2;
	OIndex							oiI3;
	int								iNumMemoryIndexes;
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	gcObjects.AddConstructor<CTestWithArray>();
	gcObjects.AddConstructor<CTestInteger>();
	gcObjects.AddConstructor<CTestNamedString>();

	iNumMemoryIndexes = WriteObjectReaderChunkedFile();

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(14, iNumMemoryIndexes);

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill();

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	gcObjects.AddConstructor<CTestWithArray>();
	gcObjects.AddConstructor<CTestInteger>();
	gcObjects.AddConstructor<CTestNamedString>();

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(0, gcObjects.NumMemoryIndexes());

	cReader.Init("Output" _FS_ "ObjectReaderChunked" _FS_ "Test" _FS_ , "Reader");
	cDeserialiser.Init(&cReader, false, &gcObjects);
	cBase = cDeserialiser.Read("Array 1");
	AssertTrue(cBase.IsNotNull());
	AssertString("CTestWithArray", cBase.ClassName());

	AssertLong(0, pcDatabase->NumIndices());
	AssertLong(14, gcObjects.NumMemoryIndexes());

	cA1 = gcObjects.Get("Array 1");
	AssertTrue(cA1.IsNotNull());
	AssertString("CTestWithArray", cA1->ClassName());
	AssertString("Something with One", cA1->mszString.Text());
	AssertInt((int32)1, cA1->mx);

	AssertPointer(cBase.Object(), cA1.Object());

	cA2 = gcObjects.Get("Array X");
	AssertTrue(cA2.IsNotNull());
	AssertString("CTestWithArray", cA2.ClassName());
	AssertString("An with 2", cA2->mszString.Text());
	AssertInt((int32)2, cA2->mx);

	cNS1 = gcObjects.Get("NamedString 1");
	AssertTrue(cNS1.IsNotNull());
	AssertString("CTestNamedString", cNS1->ClassName());
	AssertString("In Named 1", cNS1->mszEmbedded.Text());

	cNS2 = gcObjects.Get("NamedString 2");
	AssertTrue(cNS2.IsNotNull());
	AssertString("CTestNamedString", cNS2->ClassName());
	AssertString("Another in 2", cNS2->mszEmbedded.Text());

	cNS3 = gcObjects.Get("NamedString 3");
	AssertTrue(cNS3.IsNotNull());
	AssertString("CTestNamedString", cNS3->ClassName());
	AssertString("Three", cNS3->mszEmbedded.Text());

	cNS4 = gcObjects.Get("NamedString 4");
	AssertTrue(cNS4.IsNotNull());

	AssertTrue(cBase.IsNotNull());
	AssertString("CTestWithArray", cBase->ClassName());
	AssertPointer(&cA1, &cBase);
	
	AssertNotNull(&cA1->mcArray);
	AssertSize(6, cA1->mcArray->NumElements())
	sz2 = cA1->mcArray->Get(0);
	AssertString("Ye!", sz2->Text());
	cTemp = cA1->mcArray->Get(1);
	AssertPointer(&cNS1, &cTemp);

	oiI1 = cDeserialiser.GetNewIndexFromOld(5LL);
	oiI2 = cDeserialiser.GetNewIndexFromOld(6LL);
	oiI3 = cDeserialiser.GetNewIndexFromOld(7LL);

	cI1 = gcObjects.Get(oiI1);
	AssertString("CTestInteger", cI1->ClassName());
	cI2 = gcObjects.Get(oiI2);
	AssertString("CTestInteger", cI2->ClassName());
	cI3 = gcObjects.Get(oiI3);
	AssertString("CTestInteger", cI3->ClassName());

	AssertInt((int32)3, cI1->mx);
	AssertInt(2, cI1->my);
	AssertInt(1, cI1->mz);

	AssertPointer(&cNS2, &cNS1->mpAnother);
	AssertPointer(&cI1, &cA1->mcArray->Get(2));
	AssertPointer(&cNS3, &cA1->mcArray->Get(3));
	AssertPointer(&cNS1, &cNS3->mpAnother);
	AssertPointer(&cI2, &cA1->mcArray->Get(4));
	AssertPointer(&cA2, &cA1->mcArray->Get(5));

	AssertNotNull(&cA2->mcArray);
	AssertSize(4, cA2->mcArray->NumElements());
	AssertPointer(&cI3, &cA2->mcArray->Get(0));
	AssertPointer(&cNS2, &cA2->mcArray->Get(1));
	AssertPointer(&cNS3, &cNS2->mpAnother);
	AssertPointer(&cI1, &cA2->mcArray->Get(2));
	AssertPointer(&cNS4, &cA2->mcArray->Get(3));
	AssertPointer(NULL, &cNS4->mpAnother);
	AssertPointer(NULL, &cNS4->mszString);

	cDeserialiser.Kill();
	cReader.Kill();

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
void TestObjectReaderChunked(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectReaderChunkedDeserialised();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

