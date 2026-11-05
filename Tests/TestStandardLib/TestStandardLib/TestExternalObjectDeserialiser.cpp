#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/MapLongLong.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/InternalObjectDeserialiser.h"
#include "StandardLib/MultiFileObjectWriter.h"
#include "StandardLib/ObjectReaderSimpleDisk.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/ObjectFileGeneral.h"
#include "StandardLib/Root.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


//Objects in an external format (simple files, chunk files) are read into memory with new indices, and can then be
//rooted and saved into the Codabase like any others.


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


#define EXTERNAL_DIRECTORY			"Output" _FS_ "ExternalDeserialiser"
#define EXTERNAL_FILES_DIRECTORY	EXTERNAL_DIRECTORY _FS_ "Files"
#define EXTERNAL_WRITE_DATABASE		EXTERNAL_DIRECTORY _FS_ "WriteDatabase"
#define EXTERNAL_READ_DATABASE		EXTERNAL_DIRECTORY _FS_ "ReadDatabase"
#define EXTERNAL_CHUNK_FILE			"Graph"


//////////////////////////////////////////////////////////////////////////
//
// Counts Begins and Ends, so a Read can be checked to have ended what it
// began, however it went.  Begin can be made to fail.
//
//////////////////////////////////////////////////////////////////////////
class CTestCountingObjectReader : public CObjectReaderSimpleDisk
{
CONSTRUCTABLE(CTestCountingObjectReader)
public:
	size	muiBegins;
	size	muiEnds;
	bool	mbFailBegin;

	void Init(const char* szDirectory)
	{
		CObjectReaderSimpleDisk::Init(szDirectory);
		muiBegins = 0;
		muiEnds = 0;
		mbFailBegin = false;
	}

	bool Begin(void) override
	{
		muiBegins++;
		if (mbFailBegin)
		{
			return false;
		}
		return CObjectReaderSimpleDisk::Begin();
	}

	bool End(void) override
	{
		muiEnds++;
		return CObjectReaderSimpleDisk::End();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Gives the tests the protected AddDependent.
//
//////////////////////////////////////////////////////////////////////////
class CTestInternalObjectDeserialiser : public CInternalObjectDeserialiser
{
public:
	bool TestAddDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
	{
		return AddDependent(pcObjectIdentifier, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, iNumEmbedded, iEmbeddedIndex);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserAddConstructors(void)
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
void TestExternalObjectDeserialiserOpen(const char* szDirectory, CCodabase** ppcDatabase, CSequence** ppcSequence)
{
	*ppcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	*ppcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	(*ppcDatabase)->Open();
	ObjectsInit(*ppcDatabase, *ppcSequence);
	TestExternalObjectDeserialiserAddConstructors();
}


//////////////////////////////////////////////////////////////////////////
//
// The objects must have been flushed (or not wanted) already.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserClose(CCodabase** ppcDatabase, CSequence** ppcSequence)
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
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserClean(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(EXTERNAL_DIRECTORY);
	AssertTrue(cFileUtil.TouchDir(EXTERNAL_FILES_DIRECTORY));
	AssertTrue(cFileUtil.TouchDir(EXTERNAL_WRITE_DATABASE));
	AssertTrue(cFileUtil.TouchDir(EXTERNAL_READ_DATABASE));
}


//////////////////////////////////////////////////////////////////////////
//
// Start (named) -> mp1 Shared (named, points to itself)
//               -> mp2 "Black" (unnamed)
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTestSaveableObject2> TestExternalObjectDeserialiserBuildGraph(void)
{
	Ptr<CTestSaveableObject2>	pStart;
	Ptr<CTestSaveableObject1>	pShared;
	Ptr<CString>				pBlack;

	pStart = ONMalloc<CTestSaveableObject2>("Ext/Start", "Battery");
	pShared = ONMalloc<CTestSaveableObject1>("Ext/Shared");
	pBlack = OMalloc<CString>("Black");

	pShared->miInt = 89;
	pShared->mszString.Set("Shared");
	pShared->mpObject = pShared;

	pStart->mp1 = pShared;
	pStart->mp2 = pBlack;

	ORoot()->Add(pStart);
	return pStart;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the graph as simple files (Ext/Start.DRG, Ext/Shared.DRG and
// Unnamed/<index>.DRG) or as one chunk file, and leaves it saved in the
// write database too.  Returns the unnamed string's index.
//
//////////////////////////////////////////////////////////////////////////
OIndex TestExternalObjectDeserialiserWriteGraph(bool bChunked)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CExternalObjectSerialiser	cSerialiser;
	CMultiFileObjectWriter		cSimpleWriter;
	CChunkFileObjectWriter		cChunkWriter;
	CExternalObjectWriter*		pcWriter;
	OIndex						oiBlack;

	TestExternalObjectDeserialiserOpen(EXTERNAL_WRITE_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;

		pStart = TestExternalObjectDeserialiserBuildGraph();
		oiBlack = pStart->mp2->GetIndex();

		if (bChunked)
		{
			cChunkWriter.Init(EXTERNAL_FILES_DIRECTORY, "", EXTERNAL_CHUNK_FILE);
			pcWriter = &cChunkWriter;
		}
		else
		{
			cSimpleWriter.Init(EXTERNAL_FILES_DIRECTORY, "");
			pcWriter = &cSimpleWriter;
		}
		cSerialiser.Init(pcWriter);
		AssertTrue(cSerialiser.Write(&pStart));
		cSerialiser.Kill();
		pcWriter->Kill();

		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
	return oiBlack;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserDeleteFile(const char* szObjectName)
{
	CFileUtil	cFileUtil;
	CChars		szFilename;

	szFilename.Init(EXTERNAL_FILES_DIRECTORY);
	cFileUtil.AppendToPath(&szFilename, szObjectName);
	szFilename.Append("." OBJECT_FILE_EXTENSION);
	AssertTrue(cFileUtil.Exists(szFilename.Text()));
	AssertTrue(cFileUtil.Delete(szFilename.Text()));
	AssertFalse(cFileUtil.Exists(szFilename.Text()));
	szFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserDeleteUnnamedFile(OIndex oi)
{
	CChars	szName;

	szName.Init(OBJECT_UNNAMED_FILE _FS_);
	szName.AppendHexHiLo(&oi, sizeof(OIndex));
	TestExternalObjectDeserialiserDeleteFile(szName.Text());
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserAssertGraph(Ptr<CTestSaveableObject2> pStart)
{
	Ptr<CTestSaveableObject1>	pShared;
	Ptr<CString>				pBlack;

	AssertTrue(pStart.IsNotNull());
	AssertString("Ext/Start", pStart.GetName());
	AssertString("Battery", pStart->msz.Text());

	pShared = pStart->mp1;
	AssertTrue(pShared.IsNotNull());
	AssertString("Ext/Shared", pShared.GetName());
	AssertInt(89, pShared->miInt);
	AssertString("Shared", pShared->mszString.Text());
	AssertPointer(&pShared, &pShared->mpObject);

	AssertTrue(pStart->mp2.IsNotNull());
	AssertString("CString", pStart->mp2->ClassName());
	pBlack = pStart->mp2;
	AssertString("Black", pBlack->Text());
	AssertFalse(pBlack->IsNamed());
}


//////////////////////////////////////////////////////////////////////////
//
// Reading what was written gives the same graph, and Begin and End are
// called once each.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserSimpleRead(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CTestCountingObjectReader	cReader;
	CExternalObjectDeserialiser	cDeserialiser;

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(false);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pStart = cDeserialiser.Read("Ext/Start");
		cDeserialiser.Kill();
		AssertSize(1, cReader.muiBegins);
		AssertSize(1, cReader.muiEnds);
		cReader.Kill();

		TestExternalObjectDeserialiserAssertGraph(pStart);
		AssertSize(3, gcObjects.NumMemoryIndexes());
		AssertSize(2, gcObjects.NumMemoryNames());

		//Read objects aren't rooted: that's up to whoever reads them.
		AssertInt(UNATTACHED_DIST_TO_ROOT, pStart->GetDistToRoot());
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// An object that isn't there: nothing is read, and End is still called.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserMissingObject(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CTestCountingObjectReader	cReader;
	CExternalObjectDeserialiser	cDeserialiser;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLog[1024];

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(false);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		CPointer	pObject;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		StartCapturingLog(&cLog, &sLogConfig);
		pObject = cDeserialiser.Read("Ext/Nothing");
		StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
		cDeserialiser.Kill();

		AssertTrue(pObject.IsNull());
		AssertSize(1, cReader.muiBegins);
		AssertSize(1, cReader.muiEnds);
		AssertTrue(strstr(szLog, "Nothing") != NULL);
		AssertSize(0, gcObjects.NumMemoryIndexes());

		//The reader still works.
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pObject = cDeserialiser.Read("Ext/Start");
		cDeserialiser.Kill();
		AssertTrue(pObject.IsNotNull());
		AssertSize(2, cReader.muiBegins);
		AssertSize(2, cReader.muiEnds);
		TestExternalObjectDeserialiserAssertGraph(pObject);
		cReader.Kill();

		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// When Begin fails nothing is read and there is nothing to End.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserBeginFails(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CTestCountingObjectReader	cReader;
	CExternalObjectDeserialiser	cDeserialiser;

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(false);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		CPointer	pObject;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cReader.mbFailBegin = true;
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pObject = cDeserialiser.Read("Ext/Start");
		cDeserialiser.Kill();
		cReader.Kill();

		AssertTrue(pObject.IsNull());
		AssertSize(1, cReader.muiBegins);
		AssertSize(0, cReader.muiEnds);
		AssertSize(0, gcObjects.NumMemoryIndexes());
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A named object pointed to has no file.  The read fails, says which
// object is missing, Ends, and leaves nothing broken behind.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserMissingNamedDependency(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CTestCountingObjectReader	cReader;
	CExternalObjectDeserialiser	cDeserialiser;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLog[1024];

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(false);
	TestExternalObjectDeserialiserDeleteFile("Ext" _FS_ "Shared");

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		CPointer	pObject;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		StartCapturingLog(&cLog, &sLogConfig);
		pObject = cDeserialiser.Read("Ext/Start");
		StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
		cDeserialiser.Kill();

		AssertTrue(pObject.IsNull());
		AssertSize(1, cReader.muiBegins);
		AssertSize(1, cReader.muiEnds);
		AssertTrue(strstr(szLog, "Could not read object named [Ext/Shared] that another object points to.") != NULL);

		//Nothing read is rooted, so a flush saves none of it.
		cReader.Kill();
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		CPointer	pObject;

		pObject = gcObjects.Get("Ext/Start");
		AssertTrue(pObject.IsNull());
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// An unnamed object pointed to has no file.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserMissingUnnamedDependency(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CTestCountingObjectReader	cReader;
	CExternalObjectDeserialiser	cDeserialiser;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLog[1024];
	OIndex						oiBlack;
	CChars						szExpected;

	TestExternalObjectDeserialiserClean();
	oiBlack = TestExternalObjectDeserialiserWriteGraph(false);
	TestExternalObjectDeserialiserDeleteUnnamedFile(oiBlack);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		CPointer	pObject;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		StartCapturingLog(&cLog, &sLogConfig);
		pObject = cDeserialiser.Read("Ext/Start");
		StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
		cDeserialiser.Kill();
		cReader.Kill();

		AssertTrue(pObject.IsNull());
		AssertSize(1, cReader.muiBegins);
		AssertSize(1, cReader.muiEnds);
		szExpected.Init("Could not read object with index [");
		szExpected.Append(IndexToString(oiBlack));
		szExpected.Append("] that another object points to.");
		AssertTrue(strstr(szLog, szExpected.Text()) != NULL);
		szExpected.Kill();

		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// With named hollows, named objects pointed to aren't read from the files
// but left hollow, to be loaded from the database by name.  So a missing
// file doesn't matter.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserNamedHollows(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CTestCountingObjectReader	cReader;
	CExternalObjectDeserialiser	cDeserialiser;

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(false);
	TestExternalObjectDeserialiserDeleteFile("Ext" _FS_ "Shared");

	//Read into the database the graph was written from, so "Ext/Shared" is in it.
	TestExternalObjectDeserialiserOpen(EXTERNAL_WRITE_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;
		Ptr<CTestSaveableObject1>	pShared;
		CBaseObject*				pcShared;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, true, &gcObjects);
		pStart = cDeserialiser.Read("Ext/Start");
		cDeserialiser.Kill();
		cReader.Kill();
		AssertSize(1, cReader.muiBegins);
		AssertSize(1, cReader.muiEnds);

		AssertTrue(pStart.IsNotNull());
		pcShared = pStart->mp1.BaseObject();
		AssertTrue(pcShared->IsHollow());
		AssertString("Ext/Shared", pcShared->GetName());

		//The unnamed string was read.
		AssertString("Black", ((CString*)pStart->mp2.BaseObject())->Text());

		//Using it loads it.
		pShared = pStart->mp1;
		AssertInt(89, pShared->miInt);
		AssertFalse(pShared->IsHollow());
		AssertString("Shared", pShared->mszString.Text());
		AssertPointer(&pShared, &pShared->mpObject);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A chunk file is opened by Begin and closed by End: a read that fails
// must still close it, or the next read can't open it.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserChunkedFailThenRead(void)
{
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	CChunkFileSystemObjectReader	cReader;
	CExternalObjectDeserialiser		cDeserialiser;
	CMemoryFile						cLog;
	SLogConfig						sLogConfig;
	char							szLog[1024];
	size							i;

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(true);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		CPointer	pObject;

		cReader.Init(EXTERNAL_FILES_DIRECTORY, EXTERNAL_CHUNK_FILE);
		for (i = 0; i < 3; i++)
		{
			cDeserialiser.Init(&cReader, false, &gcObjects);
			StartCapturingLog(&cLog, &sLogConfig);
			pObject = cDeserialiser.Read("Ext/Nothing");
			StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
			cDeserialiser.Kill();
			AssertTrue(pObject.IsNull());
		}

		cDeserialiser.Init(&cReader, false, &gcObjects);
		pObject = cDeserialiser.Read("Ext/Start");
		cDeserialiser.Kill();
		cReader.Kill();

		TestExternalObjectDeserialiserAssertGraph(pObject);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// The point of it all: read from an external format, root, save into the
// Codabase, and get it back from the Codabase next time.  The database it
// is read into already has objects, so the read objects get new indices.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserIntoCodabase(bool bChunked)
{
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	CObjectReaderSimpleDisk			cSimpleReader;
	CChunkFileSystemObjectReader	cChunkReader;
	CExternalObjectReader*			pcReader;
	CExternalObjectDeserialiser		cDeserialiser;
	OIndex							oiOldBlack;
	OIndex							oiNewBlack;
	OIndex							oiNewStart;
	OIndex							oiNewShared;
	OIndex							aoiAlreadyHere[10];
	size							i;

	TestExternalObjectDeserialiserClean();
	oiOldBlack = TestExternalObjectDeserialiserWriteGraph(bChunked);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;
		Ptr<CRoot>					pRoot;
		Ptr<CString>				pString;

		pRoot = ORoot();
		for (i = 0; i < 10; i++)
		{
			pString = OMalloc<CString>("Already here ");
			pString->Append((int)i);
			pRoot->Add(pString);
			aoiAlreadyHere[i] = pString->GetIndex();
		}

		if (bChunked)
		{
			cChunkReader.Init(EXTERNAL_FILES_DIRECTORY, EXTERNAL_CHUNK_FILE);
			pcReader = &cChunkReader;
		}
		else
		{
			cSimpleReader.Init(EXTERNAL_FILES_DIRECTORY);
			pcReader = &cSimpleReader;
		}
		cDeserialiser.Init(pcReader, false, &gcObjects);
		pStart = cDeserialiser.Read("Ext/Start");

		oiNewStart = pStart->GetIndex();
		oiNewShared = pStart->mp1->GetIndex();
		oiNewBlack = pStart->mp2->GetIndex();
		AssertLong(oiNewBlack, cDeserialiser.GetNewIndexFromOld(oiOldBlack));
		AssertSize(3, cDeserialiser.GetArrayIndexNewOld()->NumElements());
		cDeserialiser.Kill();
		pcReader->Kill();

		//New indices, after those already in the database.
		AssertTrue(oiNewStart != oiNewShared);
		AssertTrue(oiNewStart != oiNewBlack);
		AssertTrue(oiNewShared != oiNewBlack);
		AssertTrue(oiNewStart > pString->GetIndex());
		AssertTrue(oiNewShared > pString->GetIndex());
		AssertTrue(oiNewBlack > pString->GetIndex());

		pRoot->Add(pStart);
		AssertInt(2, pStart->GetDistToRoot());
		ObjectsFlush();
		pcDatabase->ValidateIdentifiers();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;
		Ptr<CRoot>					pRoot;
		Ptr<CString>				pString;
		CPointer					pObject;

		pStart = gcObjects.Get("Ext/Start");
		TestExternalObjectDeserialiserAssertGraph(pStart);
		AssertLong(oiNewStart, pStart->GetIndex());
		AssertLong(oiNewShared, pStart->mp1->GetIndex());
		AssertLong(oiNewBlack, pStart->mp2->GetIndex());

		pObject = gcObjects.Get(oiNewBlack);
		AssertPointer(&pStart->mp2, &pObject);
		pObject = gcObjects.Get("Ext/Shared");
		AssertPointer(&pStart->mp1, &pObject);

		//What was already there is untouched.
		pRoot = ORoot();
		AssertSize(11, pRoot->NumObjects());
		for (i = 0; i < 10; i++)
		{
			pString = gcObjects.Get(aoiAlreadyHere[i]);
			AssertTrue(pString->StartsWith("Already here "));
		}
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// The database already has an object with a name that's read: the read
// one replaces it, keeping its index, so it saves over it and what points
// to it in the database points to the read one.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserReplacesInCodabase(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CObjectReaderSimpleDisk		cReader;
	CExternalObjectDeserialiser	cDeserialiser;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLog[1024];
	OIndex						oiOldShared;

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserWriteGraph(false);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CTestSaveableObject1>	pShared;

		//Made before the root, so the holder has the first index (1): loading Shared makes it hollow.
		pHolder = ONMalloc<CTestSaveableObject2>("Ext/Holder", "Holder");
		pShared = ONMalloc<CTestSaveableObject1>("Ext/Shared");
		pShared->miInt = 1;
		pShared->mszString.Set("Old");
		pHolder->mp1 = pShared;
		ORoot()->Add(pHolder);
		oiOldShared = pShared->GetIndex();
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pStart = cDeserialiser.Read("Ext/Start");
		cDeserialiser.Kill();
		cReader.Kill();

		TestExternalObjectDeserialiserAssertGraph(pStart);
		AssertLong(oiOldShared, pStart->mp1->GetIndex());

		ORoot()->Add(pStart);
		StartCapturingLog(&cLog, &sLogConfig);
		ObjectsFlush();
		StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
		AssertString("", szLog);
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject2>	pStart;
		Ptr<CTestSaveableObject2>	pHolder;
		Ptr<CTestSaveableObject1>	pShared;

		pHolder = gcObjects.Get("Ext/Holder");
		AssertString("Holder", pHolder->msz.Text());
		pShared = pHolder->mp1;
		AssertLong(oiOldShared, pShared->GetIndex());
		AssertInt(89, pShared->miInt);
		AssertString("Shared", pShared->mszString.Text());

		pStart = gcObjects.Get("Ext/Start");
		TestExternalObjectDeserialiserAssertGraph(pStart);
		AssertPointer(&pShared, &pStart->mp1);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// Embedded objects can be pointed to, from inside the same object and
// from other objects, through the external format.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserEmbedded(void)
{
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CMultiFileObjectWriter		cWriter;
	CObjectReaderSimpleDisk		cReader;
	CExternalObjectSerialiser	cSerialiser;
	CExternalObjectDeserialiser	cDeserialiser;

	TestExternalObjectDeserialiserClean();

	TestExternalObjectDeserialiserOpen(EXTERNAL_WRITE_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CEmbeddedContainer>	pContainer;
		Ptr<CEmbeddedContainer>	pOther;

		pContainer = ONMalloc<CEmbeddedContainer>("Ext/Container");
		pOther = OMalloc<CEmbeddedContainer>();
		pContainer->mi = 1;
		pOther->mi = 2;
		pContainer->mcOne.miAmANumber = 11;
		pContainer->mcTwo.miAmANumber = 12;
		pOther->mcOne.miAmANumber = 21;
		pOther->mcTwo.miAmANumber = 22;

		pContainer->mcOne.mpTest = &pContainer->mcTwo;
		pContainer->mpTest = &pOther->mcTwo;
		pOther->mcOne.mpTest = pContainer;
		pOther->mpTest = &pContainer->mcOne;
		ORoot()->Add(pContainer);

		cWriter.Init(EXTERNAL_FILES_DIRECTORY, "");
		cSerialiser.Init(&cWriter);
		AssertTrue(cSerialiser.Write(&pContainer));
		cSerialiser.Kill();
		cWriter.Kill();
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CEmbeddedContainer>	pContainer;
		Ptr<CEmbeddedContainer>	pOther;

		cReader.Init(EXTERNAL_FILES_DIRECTORY);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pContainer = cDeserialiser.Read("Ext/Container");
		cDeserialiser.Kill();
		cReader.Kill();

		AssertTrue(pContainer.IsNotNull());
		AssertInt(1, pContainer->mi);
		AssertInt(11, pContainer->mcOne.miAmANumber);
		AssertInt(12, pContainer->mcTwo.miAmANumber);
		AssertPointer(&pContainer->mcTwo, &pContainer->mcOne.mpTest);
		AssertTrue(pContainer->mpTest.IsNotNull());
		AssertTrue(pContainer->mpTest.Object()->IsEmbedded());

		pOther = pContainer->mpTest.Object()->GetEmbeddingContainer();
		AssertInt(2, pOther->mi);
		AssertInt(21, pOther->mcOne.miAmANumber);
		AssertInt(22, pOther->mcTwo.miAmANumber);
		AssertPointer(&pOther->mcTwo, &pContainer->mpTest);
		AssertPointer(&pContainer, &pOther->mcOne.mpTest);
		AssertPointer(&pContainer->mcOne, &pOther->mpTest);

		ORoot()->Add(pContainer);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	//And they're saved into the Codabase like that.
	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CEmbeddedContainer>	pContainer;
		Ptr<CEmbeddedContainer>	pOther;

		pContainer = gcObjects.Get("Ext/Container");
		AssertTrue(pContainer.IsNotNull());
		AssertInt(1, pContainer->mi);
		AssertPointer(&pContainer->mcTwo, &pContainer->mcOne.mpTest);
		pOther = pContainer->mpTest.Object()->GetEmbeddingContainer();
		AssertInt(2, pOther->mi);
		AssertInt(22, pOther->mcTwo.miAmANumber);
		AssertPointer(&pOther->mcTwo, &pContainer->mpTest);
		AssertPointer(&pContainer, &pOther->mcOne.mpTest);
		AssertPointer(&pContainer->mcOne, &pOther->mpTest);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// A permutation, as 7 shares no factor with the numbers of objects used.
//
//////////////////////////////////////////////////////////////////////////
size TestExternalObjectDeserialiserBigPointsTo(size i, size uiNumObjects)
{
	return (i * 7 + 3) % uiNumObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserAssertBig(Ptr<CArrayObject> pArray, size uiNumObjects)
{
	Ptr<CTestSaveableObject1>	pObject;
	Ptr<CTestSaveableObject1>	pPointedTo;
	size						i;
	size						uiWrongInts;
	size						uiWrongStrings;
	size						uiWrongPointers;
	CChars						szExpected;

	AssertTrue(pArray.IsNotNull());
	AssertSize(uiNumObjects, pArray->NumElements());

	uiWrongInts = 0;
	uiWrongStrings = 0;
	uiWrongPointers = 0;
	for (i = 0; i < uiNumObjects; i++)
	{
		pObject = pArray->Get(i);
		if (pObject->miInt != (int32)(i * 3))
		{
			uiWrongInts++;
		}

		szExpected.Init("Object ");
		szExpected.Append((int)i);
		if (strcmp(szExpected.Text(), pObject->mszString.Text()) != 0)
		{
			uiWrongStrings++;
		}
		szExpected.Kill();

		pPointedTo = pArray->Get(TestExternalObjectDeserialiserBigPointsTo(i, uiNumObjects));
		if (&pPointedTo != &pObject->mpObject)
		{
			uiWrongPointers++;
		}
	}
	AssertSize(0, uiWrongInts);
	AssertSize(0, uiWrongStrings);
	AssertSize(0, uiWrongPointers);
}


//////////////////////////////////////////////////////////////////////////
//
// Checking every object on every pointer change (in debug builds) makes
// thousands of objects take minutes.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserBigValidation(size uiNumObjects)
{
	if (uiNumObjects > 1000)
	{
		gcObjects.DisableValidation();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Thousands of objects, each pointed to by the array and by another
// object: many pointers to remap and many objects to look up by their old
// index.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserBig(bool bChunked, size uiNumObjects)
{
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	CExternalObjectSerialiser		cSerialiser;
	CMultiFileObjectWriter			cSimpleWriter;
	CChunkFileObjectWriter			cChunkWriter;
	CExternalObjectWriter*			pcWriter;
	CObjectReaderSimpleDisk			cSimpleReader;
	CChunkFileSystemObjectReader	cChunkReader;
	CExternalObjectReader*			pcReader;
	CExternalObjectDeserialiser		cDeserialiser;
	CArrayIndexNewOld*				pacRemap;
	CIndexNewOld*					pcNewOld;
	size							i;
	size							uiWrongRemaps;
	size							uiOutOfOrder;
	CMapLongLong					cNewIndices;
	OIndex							oiFirstNew;

	TestExternalObjectDeserialiserClean();

	TestExternalObjectDeserialiserOpen(EXTERNAL_WRITE_DATABASE, &pcDatabase, &pcSequence);
	TestExternalObjectDeserialiserBigValidation(uiNumObjects);
	{
		Ptr<CArrayObject>			pArray;
		Ptr<CArrayObject>			pCreated;
		Ptr<CTestSaveableObject1>	pObject;
		Ptr<CTestSaveableObject1>	pPointedTo;

		//The objects are in the array in another order than they were made in (their indices' order), so they're
		//found in an order that isn't smallest index first.
		pCreated = OMalloc<CArrayObject>();
		ORoot()->Add(pCreated);
		for (i = 0; i < uiNumObjects; i++)
		{
			pObject = OMalloc<CTestSaveableObject1>();
			pCreated->Add(pObject);
		}

		pArray = ONMalloc<CArrayObject>("Ext/Big");
		ORoot()->Add(pArray);
		for (i = 0; i < uiNumObjects; i++)
		{
			pObject = pCreated->Get(TestExternalObjectDeserialiserBigPointsTo(i, uiNumObjects));
			pObject->miInt = (int32)(i * 3);
			pObject->mszString.Append("Object ");
			pObject->mszString.Append((int)i);
			pArray->Add(pObject);
		}
		ORoot()->Remove(pCreated);
		pCreated = NULL;
		for (i = 0; i < uiNumObjects; i++)
		{
			pObject = pArray->Get(i);
			pPointedTo = pArray->Get(TestExternalObjectDeserialiserBigPointsTo(i, uiNumObjects));
			pObject->mpObject = pPointedTo;
		}
		TestExternalObjectDeserialiserAssertBig(pArray, uiNumObjects);

		if (bChunked)
		{
			cChunkWriter.Init(EXTERNAL_FILES_DIRECTORY, "", EXTERNAL_CHUNK_FILE);
			pcWriter = &cChunkWriter;
		}
		else
		{
			cSimpleWriter.Init(EXTERNAL_FILES_DIRECTORY, "");
			pcWriter = &cSimpleWriter;
		}
		cSerialiser.Init(pcWriter);
		AssertTrue(cSerialiser.Write(&pArray));
		cSerialiser.Kill();
		pcWriter->Kill();
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	TestExternalObjectDeserialiserBigValidation(uiNumObjects);
	{
		Ptr<CArrayObject>	pArray;
		Ptr<CString>		pString;

		//Something already in the database, so the new indices aren't the old ones.
		pString = OMalloc<CString>("First");
		ORoot()->Add(pString);
		oiFirstNew = pString->GetIndex() + 1;

		if (bChunked)
		{
			cChunkReader.Init(EXTERNAL_FILES_DIRECTORY, EXTERNAL_CHUNK_FILE);
			pcReader = &cChunkReader;
		}
		else
		{
			cSimpleReader.Init(EXTERNAL_FILES_DIRECTORY);
			pcReader = &cSimpleReader;
		}
		cDeserialiser.Init(pcReader, false, &gcObjects);
		pArray = cDeserialiser.Read("Ext/Big");
		TestExternalObjectDeserialiserAssertBig(pArray, uiNumObjects);

		//Every object read was given a new index of its own, and its old one maps to it.
		pacRemap = cDeserialiser.GetArrayIndexNewOld();
		AssertSize(uiNumObjects + 1, pacRemap->NumElements());
		uiWrongRemaps = 0;
		cNewIndices.Init(false);
		for (i = 0; i < pacRemap->NumElements(); i++)
		{
			pcNewOld = pacRemap->Get(i);
			if (cDeserialiser.GetNewIndexFromOld(pcNewOld->moiOld) != pcNewOld->moiNew)
			{
				uiWrongRemaps++;
			}
			if ((pcNewOld->moiNew < oiFirstNew) || cNewIndices.HasKey(pcNewOld->moiNew) || (gcObjects.GetMemory()->Get(pcNewOld->moiNew) == NULL))
			{
				uiWrongRemaps++;
			}
			cNewIndices.Put(pcNewOld->moiNew, pcNewOld->moiOld);
		}
		cNewIndices.Kill();
		AssertSize(0, uiWrongRemaps);

		//The array is read first.  What it points to is read smallest old index first (not in the order found), so
		//the new indices are in the order of the old ones.
		uiOutOfOrder = 0;
		for (i = 2; i < pacRemap->NumElements(); i++)
		{
			if ((pacRemap->Get(i - 1)->moiOld >= pacRemap->Get(i)->moiOld) || (pacRemap->Get(i - 1)->moiNew >= pacRemap->Get(i)->moiNew))
			{
				uiOutOfOrder++;
			}
		}
		AssertSize(0, uiOutOfOrder);
		AssertLong(INVALID_O_INDEX, cDeserialiser.GetNewIndexFromOld(0x7FFFFFFF));
		cDeserialiser.Kill();
		pcReader->Kill();

		ORoot()->Add(pArray);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);

	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	TestExternalObjectDeserialiserBigValidation(uiNumObjects);
	{
		Ptr<CArrayObject>	pArray;

		pArray = gcObjects.Get("Ext/Big");
		TestExternalObjectDeserialiserAssertBig(pArray, uiNumObjects);
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// The internal deserialiser points a pointer read from the database at
// an embedded object of the object it points to, making that object
// hollow if it isn't in memory.  An embedded index it doesn't have is an
// error, whether the object was in memory or just made hollow.
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiserInternalEmbeddedIndex(void)
{
	CCodabase*						pcDatabase;
	CSequence*						pcSequence;
	CTestInternalObjectDeserialiser	cDeserialiser;
	CObjectIdentifier				cIdentifier;
	CMemoryFile						cLog;
	SLogConfig						sLogConfig;
	char							szLog[1024];
	bool							bResult;

	TestExternalObjectDeserialiserClean();
	TestExternalObjectDeserialiserOpen(EXTERNAL_READ_DATABASE, &pcDatabase, &pcSequence);
	{
		Ptr<CTestSaveableObject1>	pFrom;
		Ptr<CTestSaveableObject1>	pExisting;
		CBaseObject*				pcHollow;

		pFrom = OMalloc<CTestSaveableObject1>();
		pExisting = OMalloc<CTestSaveableObject1>();
		cDeserialiser.Init(&gcObjects, NULL, gcObjects.GetMemory());

		//Not in memory, and a hollow with one embedded object has no embedded object 1.
		cIdentifier.Init(1000LL, OBJECT_IDENTIFIER_SIZE_NOT_SET);
		StartCapturingLog(&cLog, &sLogConfig);
		bResult = cDeserialiser.TestAddDependent(&cIdentifier, pFrom->mpObject.ObjectPtr(), &pFrom, 1, 1);
		StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
		cIdentifier.Kill();
		AssertFalse(bResult);
		AssertTrue(pFrom->mpObject.IsNull());
		AssertTrue(strstr(szLog, "Hollow object {") != NULL);
		AssertTrue(strstr(szLog, "} has no embedded object [1].") != NULL);

		//In memory, but it has no embedded object 3.
		cIdentifier.Init(pExisting->GetIndex(), OBJECT_IDENTIFIER_SIZE_NOT_SET);
		StartCapturingLog(&cLog, &sLogConfig);
		bResult = cDeserialiser.TestAddDependent(&cIdentifier, pFrom->mpObject.ObjectPtr(), &pFrom, 1, 3);
		StopCapturingLog(&cLog, &sLogConfig, szLog, 1024);
		cIdentifier.Kill();
		AssertFalse(bResult);
		AssertTrue(pFrom->mpObject.IsNull());
		AssertTrue(strstr(szLog, "} has no embedded object [3].") != NULL);
		AssertSize(0, pExisting->NumHeapFroms());

		//In memory and pointing at the object itself.
		cIdentifier.Init(pExisting->GetIndex(), OBJECT_IDENTIFIER_SIZE_NOT_SET);
		bResult = cDeserialiser.TestAddDependent(&cIdentifier, pFrom->mpObject.ObjectPtr(), &pFrom, 1, 0);
		cIdentifier.Kill();
		AssertTrue(bResult);
		AssertPointer(&pExisting, &pFrom->mpObject);
		AssertSize(1, pExisting->NumHeapFroms());
		AssertPointer(&pFrom, pExisting->GetHeapFrom(0));

		//Not in memory: a hollow with three embedded objects is made and its third pointed to.
		pFrom->mpObject.UnsafeClearObject();
		pExisting->TestRemoveHeapFrom(&pFrom);
		cIdentifier.Init(1001LL, OBJECT_IDENTIFIER_SIZE_NOT_SET);
		bResult = cDeserialiser.TestAddDependent(&cIdentifier, pFrom->mpObject.ObjectPtr(), &pFrom, 3, 2);
		cIdentifier.Kill();
		AssertTrue(bResult);
		AssertTrue(pFrom->mpObject.IsNotNull());
		AssertTrue(pFrom->mpObject.Object()->IsEmbedded());
		pcHollow = pFrom->mpObject.Object()->GetEmbeddingContainer();
		AssertTrue(pcHollow->IsHollow());
		AssertLong(1001LL, pcHollow->GetIndex());
		AssertPointer(pcHollow->GetEmbeddedObject(2), pFrom->mpObject.Object());

		cDeserialiser.Kill();
		ObjectsFlush();
	}
	TestExternalObjectDeserialiserClose(&pcDatabase, &pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalObjectDeserialiser(void)
{
	CFileUtil	cFileUtil;

	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestExternalObjectDeserialiserSimpleRead();
	TestExternalObjectDeserialiserMissingObject();
	TestExternalObjectDeserialiserBeginFails();
	TestExternalObjectDeserialiserMissingNamedDependency();
	TestExternalObjectDeserialiserMissingUnnamedDependency();
	TestExternalObjectDeserialiserNamedHollows();
	TestExternalObjectDeserialiserChunkedFailThenRead();
	TestExternalObjectDeserialiserIntoCodabase(false);
	TestExternalObjectDeserialiserIntoCodabase(true);
	TestExternalObjectDeserialiserReplacesInCodabase();
	TestExternalObjectDeserialiserEmbedded();
	TestExternalObjectDeserialiserBig(false, 300);
	TestExternalObjectDeserialiserBig(true, 300);
	TestExternalObjectDeserialiserBig(true, 5000);
	TestExternalObjectDeserialiserInternalEmbeddedIndex();

	cFileUtil.RemoveDir(EXTERNAL_DIRECTORY);

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}
