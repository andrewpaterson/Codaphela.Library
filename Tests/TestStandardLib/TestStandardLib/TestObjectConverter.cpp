#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ObjectConverterText.h"
#include "StandardLib/ObjectConverterNative.h"
#include "StandardLib/String.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "TestLib/Assert.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetupObjectConverterConstructors(void)
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
Ptr<CTestDoubleNamedString> SetupObjectConverterChunkFile(void)
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

	cRoot = ORoot();

	sz3 = OMalloc<CString>("End");

	cDiamond = ONMalloc<CTestNamedString>("Diamond End", sz3, Null(), "Before Swine");
	AssertLong(4LL, cDiamond->GetIndex());

	sz1 = OMalloc<CString>("World");
	cNS1 = ONMalloc<CTestNamedString>("NamedString 1", sz1, cDiamond, "Hello");
	AssertLong(6LL, cNS1->GetIndex());

	sz2 = OMalloc<CString>("6789");
	cNS2 = ONMalloc<CTestNamedString>("NamedString 2", sz2, cDiamond, "12345");
	AssertLong(8LL, cNS2->GetIndex());

	sz4 = OMalloc<CString>("Start");
	cDouble = ONMalloc<CTestDoubleNamedString>("Double Start", sz4, cNS1, Null());

	cRoot->Add(cDouble);

	AssertInt(2, cDouble->GetDistToRoot());
	AssertInt(3, cNS1->GetDistToRoot());
	AssertInt(4, cDiamond->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, cNS2->GetDistToRoot());

	cDouble->mpSplit1 = cNS2;

	AssertInt(3, cNS2->GetDistToRoot());
	AssertInt(4, cDiamond->GetDistToRoot());

	return cDouble;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WriteObjectConverterChunkedFile(void)
{
	Ptr<CTestDoubleNamedString>		cDouble;
	CChunkFileObjectWriter			cWriter;
	CExternalObjectSerialiser		cSerialiser;

	cDouble = SetupObjectConverterChunkFile();

	cWriter.Init("Output" _FS_ "ObjectConverter" _FS_, "", "Double");
	cSerialiser.Init(&cWriter);
	AssertTrue(cSerialiser.Write(&cDouble));
	cSerialiser.Kill();
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverterDragonExistingHollows(void)
{
	CFileUtil						cFileUtil;

	cFileUtil.RemoveDir("Output" _FS_ "ObjectConverter" _FS_);

	ObjectsInit();
	WriteObjectConverterChunkedFile();
	ObjectsFlush();
	ObjectsKill();

	ObjectsInit();
	{
		CObjectConverterNative			cChunkedConverter;
		CObjectSource* pcObjectSource;
		CObjectSourceChunked* pcObjectMultipleSource;
		CPointer						pcObject;
		CDiskFile* pcDiskFile;
		Ptr<CTestDoubleNamedString>		pcDoubleNamedString;
		Ptr<CRoot>						pcRoot;
		Ptr<CTestNamedString>			pcObject2;
		CTestNamedString* pcEnd;
		Ptr<CTestNamedString>			pcObject3;
		CTestNamedString* pcTestNamedString3;
		CTestNamedString* pcTestNamedString2;
		OIndex							oiNew;

		SetupObjectConverterConstructors();

		pcDiskFile = DiskFile("Output" _FS_ "ObjectConverter" _FS_ "Double.DRG");
		cChunkedConverter.Init();
		pcObjectSource = cChunkedConverter.CreateSource(pcDiskFile, "Double");
		AssertNotNull(pcObjectSource);
		AssertTrue(pcObjectSource->IsNative());
		AssertTrue(pcObjectSource->IsMultiSource());

		pcObjectMultipleSource = (CObjectSourceChunked*)pcObjectSource;
		AssertSize(4, pcObjectMultipleSource->NumNames());
		AssertString("Diamond End", pcObjectMultipleSource->GetName(0));
		AssertString("Double Start", pcObjectMultipleSource->GetName(1));
		AssertString("NamedString 1", pcObjectMultipleSource->GetName(2));
		AssertString("NamedString 2", pcObjectMultipleSource->GetName(3));

		AssertTrue(pcObjectSource->Contains("Diamond End"));
		AssertTrue(pcObjectSource->Contains("Double Start"));
		AssertTrue(pcObjectSource->Contains("NamedString 1"));
		AssertTrue(pcObjectSource->Contains("NamedString 2"));
		AssertFalse(pcObjectSource->Contains("Unnamed/0000000000000006"));

		pcObject = pcObjectSource->Convert("Double Start");
		AssertNotNull(&pcObject);
		AssertString("CTestDoubleNamedString", pcObject.ClassName());

		pcDoubleNamedString = pcObject;
		AssertNotNull(&pcDoubleNamedString->mszString);
		AssertFalse(pcDoubleNamedString->mszString->IsHollow());
		AssertString("Start", pcDoubleNamedString->mszString->Text());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcDoubleNamedString->mszString->GetDistToRoot());

		pcObject = pcDoubleNamedString->mpSplit1;
		AssertTrue(pcObject.IsNotNull());
		AssertTrue(pcObject.IsHollow());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcObject.GetDistToRoot());
		AssertString("NamedString 2", pcObject.GetName());

		pcRoot = ORoot();

		pcRoot->Add(pcDoubleNamedString);
		AssertInt(2, pcDoubleNamedString->GetDistToRoot());
		AssertInt(3, pcDoubleNamedString->mszString->GetDistToRoot());
		AssertTrue(pcObject.IsNotNull());
		AssertTrue(pcObject.IsHollow());
		AssertInt(3, pcObject.GetDistToRoot());
		AssertString("NamedString 2", pcObject.GetName());

		oiNew = pcObject.GetIndex();
		AssertString("NamedString 1", pcDoubleNamedString->mpSplit2.GetName());
		AssertTrue(pcDoubleNamedString->mpSplit2.IsNotNull());
		AssertTrue(pcDoubleNamedString->mpSplit2.IsHollow());
		AssertInt(3, pcDoubleNamedString->mpSplit2.GetDistToRoot());

		pcTestNamedString2 = (CTestNamedString*)gcObjects.TestGetFromMemory(oiNew).Object();
		AssertNotNull(pcTestNamedString2);
		AssertString("NamedString 2", pcTestNamedString2->GetName());

		oiNew = cChunkedConverter.TestGetNewIndexFromOld(3LL);
		AssertLong(INVALID_O_INDEX, oiNew);

		pcObject2 = pcObjectSource->Convert("NamedString 2");
		AssertString("CTestNamedString", pcObject2.ClassName());
		AssertLong(7, pcObject2.GetIndex());
		AssertTrue(pcObject2.IsNotNull());
		pcObject = pcDoubleNamedString->mpSplit1;
		AssertPointer(pcObject.Object(), pcObject2.Object());
		AssertFalse(pcObject2.IsHollow());
		AssertInt(3, pcObject2.GetDistToRoot());
		AssertString("NamedString 2", pcObject2.GetName());
		pcTestNamedString2 = (CTestNamedString*)pcObject2.Object();
		AssertString("6789", pcTestNamedString2->mszString->Text());
		AssertTrue(pcTestNamedString2->mpAnother.IsHollow());
		AssertString("Diamond End", pcTestNamedString2->mpAnother.GetName());
		AssertLong(9, pcTestNamedString2->mpAnother.GetIndex());

		oiNew = pcTestNamedString2->mpAnother.GetIndex();
		pcEnd = (CTestNamedString*)gcObjects.TestGetFromMemory(oiNew).Object();
		AssertString("Diamond End", pcEnd->GetName());

		pcObject3 = pcObjectSource->Convert("NamedString 1");
		AssertFalse(pcObject3.IsNull());
		AssertString("CTestNamedString", pcObject3.ClassName());
		pcTestNamedString3 = (CTestNamedString*)pcObject3.Object();
		AssertString("World", pcTestNamedString3->mszString->Text());
		AssertTrue(pcTestNamedString3->mpAnother.IsHollow());
		AssertString("Diamond End", pcTestNamedString3->mpAnother.GetName());
		AssertLong(9, pcTestNamedString3->mpAnother.GetIndex());
		AssertPointer(pcTestNamedString2->mpAnother.Object(), pcTestNamedString3->mpAnother.Object());
		AssertTrue(pcTestNamedString2->mpAnother.IsHollow());

		pcObjectSource->Kill();
		cChunkedConverter.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverterText(void)
{
	ObjectsInit();
	{
		CObjectConverterText	cTextConverter;
		char					szTextInFile[] = { "This is text\nin a file.\n" };
		size					iTextLen;
		CMemoryFile				cMemoryFile;
		CObjectSource* pcObjectSource;
		CPointer				pcObject;
		Ptr<CString>			pcString;

		iTextLen = strlen(szTextInFile);
		cMemoryFile.Init(szTextInFile, iTextLen);

		cTextConverter.Init();
		pcObjectSource = cTextConverter.CreateSource(&cMemoryFile, "TextFile");
		AssertTrue(pcObjectSource->Contains("TextFile"));
		AssertFalse(pcObjectSource->Contains("AnythingElse"));
		AssertFalse(pcObjectSource->Contains(""));
		AssertFalse(pcObjectSource->Contains(NULL));
		pcString = pcObjectSource->Convert("TextFile");
		AssertTrue(pcString.IsNotNull());
		AssertString(szTextInFile, pcString->Text());
		pcString->CBaseObject::Kill();

		pcString = pcObjectSource->Convert("");
		AssertNull(&pcString);

		cMemoryFile.Kill();
		pcObjectSource->Kill();
		cTextConverter.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTestDoubleNamedString> SetupObjectConverterChunkFile2(void)
{
	Ptr<CTestNamedString>			cNS1;
	Ptr<CTestNamedString>			cNS2;
	Ptr<CTestNamedString>			cNS3;
	Ptr<CTestNamedString>			cDiamond;
	Ptr<CTestDoubleNamedString>		cDouble;
	Ptr<CRoot>						cRoot;

	cRoot = ORoot();

	cDiamond = ONMalloc<CTestNamedString>("Diamond", Null(), Null(), "Diamond");

	cNS1 = ONMalloc<CTestNamedString>("NS1", Null(), cDiamond, "NS1");

	cNS2 = ONMalloc<CTestNamedString>("NS2", Null(), cDiamond, "NS2");

	cNS3 = ONMalloc<CTestNamedString>("NS3", Null(), cNS1, "NS3");

	cDouble = ONMalloc<CTestDoubleNamedString>("Double", Null(), cNS2, cNS3);

	cRoot->Add(cDouble);

	//  
	//      Diamond
	//        / \
	//       /   \
	//      /     \
	//    NS1     NS2
	//     |      /
	//    NS3    /
	//      \   /
	//       \ /
	//      Double
	//        |
	//       ...
	//     GraphRoot
	//

	return cDouble;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WriteObjectConverterChunkedFile2(void)
{
	Ptr<CTestDoubleNamedString>		cDouble;
	CChunkFileObjectWriter			cWriter;
	CExternalObjectSerialiser			cGraphSerialiser;

	cDouble = SetupObjectConverterChunkFile2();

	cWriter.Init("Output" _FS_ "ObjectConverter" _FS_ , "", "Trouble");
	cGraphSerialiser.Init(&cWriter);
	AssertTrue(cGraphSerialiser.Write(&cDouble));
	cGraphSerialiser.Kill();
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverterDragonRootDistance(void)
{
	ObjectsInit();
	WriteObjectConverterChunkedFile2();
	ObjectsFlush();
	ObjectsKill();

	ObjectsInit();
	{
		CObjectConverterNative			cChunkedConverter;
		CObjectSource* pcObjectSource;
		Ptr<CTestDoubleNamedString>		pDouble;
		Ptr<CTestNamedString>			pNS1;
		Ptr<CTestNamedString>			pNS2;
		Ptr<CTestNamedString>			pNS3;
		Ptr<CTestNamedString>			pDiamond;
		CDiskFile* pcDiskFile;
		Ptr<CRoot>						pRoot;
		Ptr<CTestNamedString>			pNS1InMemory;
		Ptr<CTestNamedString>			pNS2InMemory;
		Ptr<CTestNamedString>			pNS3InMemory;
		Ptr<CTestNamedString>			pDiamondInMemory;

		SetupObjectConverterConstructors();

		pcDiskFile = DiskFile("Output" _FS_ "ObjectConverter" _FS_ "Trouble.DRG");
		cChunkedConverter.Init();
		pcObjectSource = cChunkedConverter.CreateSource(pcDiskFile, "Trouble");

		pDouble = pcObjectSource->Convert("Double");
		pRoot = ORoot();
		pRoot->Add(pDouble);

		pNS1 = pcObjectSource->Convert("NS1");
		pNS1InMemory = gcObjects.TestGetFromMemory("NS1");
		AssertPointer(&pNS1, &pNS1InMemory);

		pNS3 = pcObjectSource->Convert("NS3");
		AssertNotNull(pNS1.Object());
		AssertNotNull(pNS3.Object());
		AssertPointer(&pNS1, &pNS3->mpAnother);
		AssertPointer(&pNS3, &pDouble->mpSplit1);
		pNS3InMemory = gcObjects.TestGetFromMemory("NS3");
		AssertPointer(&pNS3, &pNS3InMemory);

		AssertInt(2, pDouble->GetDistToRoot());
		AssertInt(3, pNS3->GetDistToRoot());
		AssertInt(4, pNS1->GetDistToRoot());

		pDiamond = pcObjectSource->Convert("Diamond");
		AssertPointer(&pDiamond, &pNS1->mpAnother);
		AssertInt(5, pDiamond->GetDistToRoot());
		pDiamondInMemory = gcObjects.TestGetFromMemory("Diamond");
		AssertPointer(&pDiamond, &pDiamondInMemory);

		pNS2 = pcObjectSource->Convert("NS2");
		AssertPointer(&pNS2, &pDouble->mpSplit2);
		AssertInt(3, pNS2->GetDistToRoot());
		pNS2InMemory = gcObjects.TestGetFromMemory("NS2");
		AssertPointer(&pNS2, &pNS2InMemory);

		AssertPointer(&pDiamond, &pNS2->mpAnother);
		AssertInt(4, pDiamond->GetDistToRoot());

		pcObjectSource->Kill();
		cChunkedConverter.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverter(void)
{
	CFileUtil	cFileUtil;
	char		szDirectory[] = "Output" _FS_ "ObjectConverter";

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectConverterText();
	TestObjectConverterDragonExistingHollows();
	TestObjectConverterDragonRootDistance();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();

	cFileUtil.RemoveDir("Output" _FS_ "ObjectConverter");
}

