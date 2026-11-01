#include <stdio.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/WriteTransaction.h"
#include "BaseLib/ReadTransaction.h"
#include "TestLib/Assert.h"
#include "TestTransactionHelper.h"


#define READ_TRANSACTION_DIR	"Output" _FS_ "ReadTransaction"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertReadIndices(CReadTransaction* pcRead, OIndex oi1, OIndex oi2)
{
	SIndexTreeFileIterator	sIter;
	char					szData[256];
	lsize					iDataSize;

	AssertLong(oi1, pcRead->StartIndexIteration(&sIter, szData, &iDataSize, 256));
	AssertLong(oi2, pcRead->IndexIterate(&sIter, szData, &iDataSize, 256));
	AssertLong(INVALID_O_INDEX, pcRead->IndexIterate(&sIter, szData, &iDataSize, 256));
	sIter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadTransactionWriteTransaction(void)
{
	CCodabase*				pcDatabase;
	CReadTransaction*		pcBefore;
	CReadTransaction*		pcAfter;
	CWriteTransaction*		pcWrite;
	CFileUtil				cFileUtil;
	SIndexTreeFileIterator	sIter;
	char					szKey[MAX_KEY_SIZE + 1];
	char					szName[64];
	size					uiNameLength;
	OIndex					oi;
	char					szUno[] = "Uno";
	char					szThree[] = "Three";

	pcDatabase = CreateTransactionTestCodabase(READ_TRANSACTION_DIR, DS_No);
	pcBefore = pcDatabase->BeginReadTransaction();
	AssertNotNull(pcBefore);

	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Set(1LL, szUno, strlen(szUno) + 1));
	AssertTrue(pcWrite->Add(3LL, "Gamma", szThree, strlen(szThree) + 1));
	AssertTrue(pcWrite->Remove(2LL));
	AssertTrue(pcWrite->Commit());
	SafeKill(pcWrite);
	AssertTrue(pcDatabase->NumVersions() > 0);

	//The reader that began before the commit still sees everything as it was.
	AssertReadString(pcBefore, 1LL, "One");
	AssertReadString(pcBefore, 2LL, "Two");
	AssertFalse(pcBefore->Contains(3LL));
	AssertLong(1LL, pcBefore->GetIndex("Alpha"));
	AssertLong(INVALID_O_INDEX, pcBefore->GetIndex("Gamma"));
	AssertTrue(pcBefore->GetName(1LL, szName, &uiNameLength, 64));
	AssertString("Alpha", szName);
	AssertReadIndices(pcBefore, 1LL, 2LL);
	AssertTrue(pcBefore->StartNameIteration(&sIter, szKey, &oi));
	AssertString("Alpha", szKey);
	AssertFalse(pcBefore->NameIterate(&sIter, szKey, &oi));

	//A reader that begins after the commit sees it.
	pcAfter = pcDatabase->BeginReadTransaction();
	AssertTrue(pcAfter->GetSnapshot() > pcBefore->GetSnapshot());
	AssertReadString(pcAfter, 1LL, szUno);
	AssertFalse(pcAfter->Contains(2LL));
	AssertReadString(pcAfter, 3LL, szThree);
	AssertLong(3LL, pcAfter->GetIndex("Gamma"));
	AssertReadIndices(pcAfter, 1LL, 3LL);
	AssertTrue(pcAfter->StartNameIteration(&sIter, szKey, &oi));
	AssertString("Alpha", szKey);
	AssertTrue(pcAfter->NameIterate(&sIter, szKey, &oi));
	AssertString("Gamma", szKey);
	AssertFalse(pcAfter->NameIterate(&sIter, szKey, &oi));
	sIter.Kill();

	//The old versions go when the last reader that needs them ends.
	SafeKill(pcBefore);
	AssertLSize(0, pcDatabase->NumVersions());
	SafeKill(pcAfter);
	SafeKill(pcDatabase);
	AssertTrue(cFileUtil.RemoveDir(READ_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadTransactionDirectSession(void)
{
	CCodabase*			pcDatabase;
	CReadTransaction*	pcBefore;
	CReadTransaction*	pcDuring;
	CReadTransaction*	pcAfter;
	CFileUtil			cFileUtil;
	char				szUno[] = "Uno";
	char				szFour[] = "Four";

	pcDatabase = CreateTransactionTestCodabase(READ_TRANSACTION_DIR, DS_No);
	pcBefore = pcDatabase->BeginReadTransaction();

	//A session changes the store before it commits.  Readers, even one that begins during the session, see the old state.
	AssertTrue(pcDatabase->Open());
	AssertTrue(pcDatabase->Set(1LL, szUno, strlen(szUno) + 1));
	AssertTrue(pcDatabase->Add(4LL, "Delta", szFour, strlen(szFour) + 1));
	AssertTrue(pcDatabase->Remove(2LL));
	pcDuring = pcDatabase->BeginReadTransaction();
	AssertReadString(pcBefore, 1LL, "One");
	AssertReadString(pcDuring, 1LL, "One");
	AssertReadString(pcDuring, 2LL, "Two");
	AssertFalse(pcDuring->Contains(4LL));
	AssertLong(INVALID_O_INDEX, pcDuring->GetIndex("Delta"));
	AssertReadIndices(pcDuring, 1LL, 2LL);
	AssertTrue(pcDatabase->Close());

	AssertReadString(pcBefore, 1LL, "One");
	AssertReadString(pcDuring, 2LL, "Two");
	pcAfter = pcDatabase->BeginReadTransaction();
	AssertReadString(pcAfter, 1LL, szUno);
	AssertReadString(pcAfter, 4LL, szFour);
	AssertLong(4LL, pcAfter->GetIndex("Delta"));
	AssertFalse(pcAfter->Contains(2LL));
	AssertReadIndices(pcAfter, 1LL, 4LL);

	SafeKill(pcBefore);
	SafeKill(pcDuring);
	SafeKill(pcAfter);
	AssertLSize(0, pcDatabase->NumVersions());
	SafeKill(pcDatabase);
	AssertTrue(cFileUtil.RemoveDir(READ_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadTransactionVersions(void)
{
	CCodabase*			pcDatabase;
	CReadTransaction*	pcFirst;
	CReadTransaction*	pcSecond;
	CReadTransaction*	pcThird;
	CWriteTransaction*	pcWrite;
	CFileUtil			cFileUtil;
	char				szA[] = "A";
	char				szB[] = "B";
	char				szFive[] = "Five";

	pcDatabase = CreateTransactionTestCodabase(READ_TRANSACTION_DIR, DS_No);

	//Without readers no versions are kept.  A session changes nothing in the store before it closes, so it needs none.
	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Set(2LL, szA, strlen(szA) + 1));
	AssertTrue(pcWrite->Commit());
	SafeKill(pcWrite);
	AssertLSize(0, pcDatabase->NumVersions());
	AssertTrue(pcDatabase->Open());
	AssertTrue(pcDatabase->Set(2LL, szB, strlen(szB) + 1));
	AssertLSize(0, pcDatabase->NumVersions());
	AssertTrue(pcDatabase->Close());
	AssertLSize(0, pcDatabase->NumVersions());

	//Each reader sees the commit before it began.
	pcFirst = pcDatabase->BeginReadTransaction();
	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Set(1LL, szA, strlen(szA) + 1));
	AssertTrue(pcWrite->Commit());
	SafeKill(pcWrite);
	pcSecond = pcDatabase->BeginReadTransaction();
	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Set(1LL, szB, strlen(szB) + 1));
	AssertTrue(pcWrite->Commit());
	SafeKill(pcWrite);
	pcThird = pcDatabase->BeginReadTransaction();

	AssertReadString(pcFirst, 1LL, "One");
	AssertReadString(pcSecond, 1LL, szA);
	AssertReadString(pcThird, 1LL, szB);

	//A name removed from one object and given to another.
	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Remove("Alpha"));
	AssertTrue(pcWrite->Add(5LL, "Alpha", szFive, strlen(szFive) + 1));
	AssertTrue(pcWrite->Commit());
	SafeKill(pcWrite);
	AssertLong(1LL, pcThird->GetIndex("Alpha"));
	AssertReadString(pcThird, 1LL, szB);

	//Ending a newer reader keeps the versions an older one still needs.
	SafeKill(pcSecond);
	AssertReadString(pcFirst, 1LL, "One");
	SafeKill(pcThird);
	AssertReadString(pcFirst, 1LL, "One");
	AssertTrue(pcDatabase->NumVersions() > 0);
	SafeKill(pcFirst);
	AssertLSize(0, pcDatabase->NumVersions());

	//Reading never waits for a writer, even on the writer's own thread.
	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Remove(5LL));
	pcFirst = pcDatabase->BeginReadTransaction();
	AssertNotNull(pcFirst);
	AssertLong(5LL, pcFirst->GetIndex("Alpha"));
	SafeKill(pcFirst);
	pcWrite->Abort();
	SafeKill(pcWrite);

	SafeKill(pcDatabase);
	AssertTrue(cFileUtil.RemoveDir(READ_TRANSACTION_DIR));
}


#define READ_TRANSACTION_COMMITS	200
#define READ_TRANSACTION_READERS	3


struct SReadTransactionThread
{
	CCodabase*	pcDatabase;
	CMutex*		pcMutex;
	bool*		pbWriting;
	int			iMismatches;
	int			iReads;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ReadTransactionWriter(void* pvData)
{
	SReadTransactionThread*	psData;
	CWriteTransaction*		pcWrite;
	int						i;
	int						iValue;

	//Every commit changes both objects to the same value.
	psData = (SReadTransactionThread*)pvData;
	for (i = 1; i <= READ_TRANSACTION_COMMITS; i++)
	{
		iValue = i;
		pcWrite = psData->pcDatabase->BeginWriteTransaction();
		pcWrite->Put(10LL, &iValue, sizeof(int));
		pcWrite->Put(11LL, &iValue, sizeof(int));
		pcWrite->Commit();
		SafeKill(pcWrite);
	}
	psData->pcMutex->Lock();
	*psData->pbWriting = false;
	psData->pcMutex->Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ReadTransactionReader(void* pvData)
{
	SReadTransactionThread*	psData;
	CReadTransaction*		pcRead;
	int						iFirst;
	int						iSecond;
	bool					bWriting;

	//Both objects are read from one snapshot while commits happen, so they must always match.
	psData = (SReadTransactionThread*)pvData;
	do
	{
		pcRead = psData->pcDatabase->BeginReadTransaction();
		iFirst = -1;
		iSecond = -2;
		pcRead->Get(10LL, &iFirst);
		ThreadYield();
		pcRead->Get(11LL, &iSecond);
		if (iFirst != iSecond)
		{
			psData->iMismatches++;
		}
		psData->iReads++;
		SafeKill(pcRead);

		psData->pcMutex->Lock();
		bWriting = *psData->pbWriting;
		psData->pcMutex->Unlock();
	}
	while (bWriting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadTransactionThreads(void)
{
	CCodabase*				pcDatabase;
	CFileUtil				cFileUtil;
	CNativeThread			cWriter;
	CNativeThread			acReaders[READ_TRANSACTION_READERS];
	SReadTransactionThread	sWriter;
	SReadTransactionThread	asReaders[READ_TRANSACTION_READERS];
	CMutex					cMutex;
	bool					bWriting;
	int						i;
	int						iValue;
	CWriteTransaction*		pcWrite;

	pcDatabase = CreateTransactionTestCodabase(READ_TRANSACTION_DIR, DS_No);
	iValue = 0;
	pcWrite = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcWrite->Add(10LL, &iValue, sizeof(int)));
	AssertTrue(pcWrite->Add(11LL, &iValue, sizeof(int)));
	AssertTrue(pcWrite->Commit());
	SafeKill(pcWrite);

	cMutex.Init();
	bWriting = true;
	for (i = 0; i < READ_TRANSACTION_READERS; i++)
	{
		asReaders[i].pcDatabase = pcDatabase;
		asReaders[i].pcMutex = &cMutex;
		asReaders[i].pbWriting = &bWriting;
		asReaders[i].iMismatches = 0;
		asReaders[i].iReads = 0;
		acReaders[i].Init();
		AssertTrue(acReaders[i].Start(ReadTransactionReader, &asReaders[i]));
	}
	sWriter.pcDatabase = pcDatabase;
	sWriter.pcMutex = &cMutex;
	sWriter.pbWriting = &bWriting;
	cWriter.Init();
	AssertTrue(cWriter.Start(ReadTransactionWriter, &sWriter));

	cWriter.Join();
	cWriter.Kill();
	for (i = 0; i < READ_TRANSACTION_READERS; i++)
	{
		acReaders[i].Join();
		acReaders[i].Kill();
		AssertInt(0, asReaders[i].iMismatches);
		AssertTrue(asReaders[i].iReads > 0);
	}
	cMutex.Kill();

	AssertLSize(0, pcDatabase->NumVersions());
	SafeKill(pcDatabase);
	AssertTrue(cFileUtil.RemoveDir(READ_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReadTransaction(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestReadTransactionWriteTransaction();
	TestReadTransactionDirectSession();
	TestReadTransactionVersions();
	TestReadTransactionThreads();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

