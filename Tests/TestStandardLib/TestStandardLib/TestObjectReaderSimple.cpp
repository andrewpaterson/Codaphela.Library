#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ObjectReaderSimpleDisk.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/MultiFileObjectWriter.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "TestLib/Assert.h"
#include "ChunkFileObjectWriterTestClasses.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer SetupObjectReaderSimpleFile(void)
{
	Ptr<CTestNamedString>	cNS1;
	Ptr<CTestNamedString>	cNS2;

	cNS2 = ONMalloc<CTestNamedString>("Dog", Null(), Null(), "NS2");
	cNS1 = ONMalloc<CTestNamedString>("Waggy", Null(), cNS2, "NS1");


	return cNS1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WriteObjectReaderSimpleFile(void)
{
	ObjectsInit();
	{
		gcObjects.AddConstructor<CTestNamedString>();

		CPointer				cBase;
		CMultiFileObjectWriter		cWriter;
		CExternalObjectSerialiser	cGraphSerialiser;

		cBase = SetupObjectReaderSimpleFile();

		cWriter.Init("Output" _FS_ "ObjectReaderSimple" _FS_ "Test" _FS_, "");
		cGraphSerialiser.Init(&cWriter);
		AssertTrue(cGraphSerialiser.Write(cBase.BaseObject()));
		cGraphSerialiser.Kill();
		cWriter.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReaderSimpleDeserialised(void)
{
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "ObjectReaderSimple";

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	WriteObjectReaderSimpleFile();

	CCodabase*						pcDatabase;
	CSequence*						pcSequence;

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_Yes, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		CObjectReaderSimpleDisk			cReader;
		CExternalObjectDeserialiser		cDeserialiser;
		CPointer						cBase;

		Ptr<CTestNamedString>			cNS1;
		Ptr<CTestNamedString>			cNS2;
		CPointer						cTemp;

		gcObjects.AddConstructor<CTestNamedString>();

		AssertLong(0, pcDatabase->NumIndices());
		AssertLong(0, gcObjects.NumMemoryIndexes());

		cReader.Init("Output" _FS_ "ObjectReaderSimple" _FS_ "Test" _FS_);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		cBase = cDeserialiser.Read("Waggy");

		AssertTrue(cBase.IsNotNull());
		AssertString("CTestNamedString", cBase->ClassName());
		AssertString("Waggy", cBase.GetName());
		AssertString("NS1", ((CTestNamedString*)&cBase)->mszEmbedded.Text());

		AssertLong(0LL, pcDatabase->NumIndices());
		AssertLong(2LL, gcObjects.NumMemoryIndexes());

		cNS1 = gcObjects.Get("Waggy");
		AssertTrue(cNS1.IsNotNull());
		AssertString("NS1", cNS1->mszEmbedded.Text());

		cNS2 = gcObjects.Get("Dog");
		AssertTrue(cNS2.IsNotNull());
		AssertString("NS2", cNS2->mszEmbedded.Text());

		AssertPointer(&cNS1, &cBase);

		AssertPointer(&cNS2, &cNS1->mpAnother);
		AssertPointer(NULL, &cNS2->mpAnother);

		cDeserialiser.Kill();
		cReader.Kill();

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReaderSimple(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectReaderSimpleDeserialised();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

