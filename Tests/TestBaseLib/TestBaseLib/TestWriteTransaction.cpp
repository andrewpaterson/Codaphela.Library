#include <stdio.h>
#ifdef LINUX_GNU
#include <sys/stat.h>
#endif // LINUX_GNU
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/WriteTransaction.h"
#include "TestLib/Assert.h"
#include "TestTransactionHelper.h"


#define WRITE_TRANSACTION_DIR	"Output" _FS_ "WriteTransaction"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransactionCommit(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcTransaction;
	CFileUtil			cFileUtil;
	char				szUno[] = "Uno";
	char				szThree[] = "Three";
	char				szName[64];
	size				uiNameLength;

	cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR);
	pcDatabase = CreateTransactionTestCodabase(WRITE_TRANSACTION_DIR, DS_No);

	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertNotNull(pcTransaction);
	AssertTrue(pcTransaction->IsActive());
	AssertTrue(pcTransaction->Set(1LL, szUno, strlen(szUno) + 1));
	AssertTrue(pcTransaction->Add(3LL, "Gamma", szThree, strlen(szThree) + 1));
	AssertTrue(pcTransaction->Remove(2LL));

	//The transaction sees its own changes.
	AssertTransactionString(pcTransaction, 1LL, szUno);
	AssertTransactionString(pcTransaction, 3LL, szThree);
	AssertFalse(pcTransaction->Contains(2LL));
	AssertLong(3LL, pcTransaction->GetIndex("Gamma"));
	AssertTrue(pcTransaction->GetName(1LL, szName, &uiNameLength, 64));
	AssertString("Alpha", szName);

	//Nobody else sees them before the commit.
	AssertStoredString(pcDatabase, 1LL, "One");
	AssertFalse(pcDatabase->Contains(3LL));
	AssertTrue(pcDatabase->Contains(2LL));

	AssertTrue(pcTransaction->Commit());
	AssertFalse(pcTransaction->IsActive());
	SafeKill(pcTransaction);

	AssertStoredString(pcDatabase, 1LL, szUno);
	AssertStoredString(pcDatabase, 3LL, szThree);
	AssertFalse(pcDatabase->Contains(2LL));
	AssertLong(3LL, pcDatabase->GetIndex("Gamma"));
	SafeKill(pcDatabase);

	//The commit is durable.
	pcDatabase = CCodabaseFactory::Create(WRITE_TRANSACTION_DIR, IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	AssertStoredString(pcDatabase, 1LL, szUno);
	AssertStoredString(pcDatabase, 3LL, szThree);
	AssertFalse(pcDatabase->Contains(2LL));
	AssertLong(2, pcDatabase->NumIndices());
	AssertLong(2, pcDatabase->NumNames());
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransactionAbort(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcTransaction;
	CFileUtil			cFileUtil;
	char				szChanged[] = "Changed";
	SLogConfig			sLogConfig;

	cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR);
	pcDatabase = CreateTransactionTestCodabase(WRITE_TRANSACTION_DIR, DS_No);

	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcTransaction->Set(1LL, szChanged, strlen(szChanged) + 1));
	AssertTrue(pcTransaction->Remove(2LL));
	pcTransaction->Abort();
	AssertFalse(pcTransaction->IsActive());

	//A finished transaction cannot be changed or committed.
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcTransaction->Set(1LL, szChanged, strlen(szChanged) + 1));
	AssertFalse(pcTransaction->Commit());
	gcLogger.SetConfig(&sLogConfig);
	SafeKill(pcTransaction);
	AssertStoredString(pcDatabase, 1LL, "One");
	AssertTrue(pcDatabase->Contains(2LL));

	//Killing a transaction before it commits aborts it and lets the next writer in.
	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcTransaction->Remove(1LL));
	SafeKill(pcTransaction);
	AssertTrue(pcDatabase->Contains(1LL));

	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertNotNull(pcTransaction);
	pcTransaction->Abort();
	SafeKill(pcTransaction);
	SafeKill(pcDatabase);

	pcDatabase = CCodabaseFactory::Create(WRITE_TRANSACTION_DIR, IWT_No, DS_No);
	AssertStoredString(pcDatabase, 1LL, "One");
	AssertTrue(pcDatabase->Contains(2LL));
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransactionRules(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcTransaction;
	CFileUtil			cFileUtil;
	SLogConfig			sLogConfig;
	char				szBad[] = "Bad";
	char				szNew[] = "New";
	char				szLonger[] = "Longer data";
	char				szReplaced[] = "Replaced";
	char				szResult[256];
	lsize				uiDataSize;

	cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR);
	pcDatabase = CreateTransactionTestCodabase(WRITE_TRANSACTION_DIR, DS_No);
	pcTransaction = pcDatabase->BeginWriteTransaction();

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcTransaction->Add(1LL, szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Add(5LL, "Alpha", szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Add(INVALID_O_INDEX, szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Add(5LL, NULL, 0));
	AssertFalse(pcTransaction->Set(9LL, szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Set("Missing", szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Put(1LL, "Beta", szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Put(2LL, "Alpha", szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Put(1LL, (char*)NULL, szBad, strlen(szBad) + 1));
	AssertFalse(pcTransaction->Remove(9LL));
	gcLogger.SetConfig(&sLogConfig);

	//A name added in the transaction cannot be added again.
	AssertTrue(pcTransaction->Add(5LL, "Epsilon", szNew, strlen(szNew) + 1));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcTransaction->Add(6LL, "Epsilon", szNew, strlen(szNew) + 1));
	gcLogger.SetConfig(&sLogConfig);

	//Set with no size keeps the size, Put changes an existing object.
	AssertTrue(pcTransaction->Set(5LL, szBad));
	AssertTrue(pcTransaction->Get(5LL, &uiDataSize, NULL, 0));
	AssertLSize(4, uiDataSize);
	AssertTrue(pcTransaction->Put(5LL, "Epsilon", szLonger, strlen(szLonger) + 1));
	AssertTrue(pcTransaction->Get("Epsilon", &uiDataSize, szResult, 256));
	AssertLSize(12, uiDataSize);
	AssertString(szLonger, szResult);

	//An object added and removed in the same transaction never reaches the Codabase.
	AssertTrue(pcTransaction->Add(7LL, szNew, strlen(szNew) + 1));
	AssertTrue(pcTransaction->Remove(7LL));
	AssertFalse(pcTransaction->Contains(7LL));

	//A stored object removed and added again with another name replaces it.  Its old name is free to use.
	AssertTrue(pcTransaction->Remove("Alpha"));
	AssertFalse(pcTransaction->Contains("Alpha"));
	AssertTrue(pcTransaction->Add(1LL, "Omega", szReplaced, strlen(szReplaced) + 1));
	AssertTrue(pcTransaction->Add(8LL, "Alpha", szNew, strlen(szNew) + 1));
	AssertTrue(pcTransaction->Add(9LL, szNew, strlen(szNew) + 1));

	AssertTrue(pcTransaction->Commit());
	SafeKill(pcTransaction);

	AssertStoredString(pcDatabase, 1LL, szReplaced);
	AssertLong(1LL, pcDatabase->GetIndex("Omega"));
	AssertLong(8LL, pcDatabase->GetIndex("Alpha"));
	AssertLong(5LL, pcDatabase->GetIndex("Epsilon"));
	AssertFalse(pcDatabase->Contains(7LL));
	AssertStoredString(pcDatabase, 9LL, szNew);
	AssertLong(5, pcDatabase->NumIndices());
	AssertLong(3, pcDatabase->NumNames());
	AssertTrue(pcDatabase->ValidateIdentifiers());
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransactionIteration(void)
{
	CCodabase*				pcDatabase;
	CWriteTransaction*		pcTransaction;
	CFileUtil				cFileUtil;
	SIndexTreeFileIterator	sIter;
	char					szData[256];
	char					szKey[MAX_KEY_SIZE + 1];
	lsize					iDataSize;
	OIndex					oi;
	char					szUno[] = "Uno";
	char					szThree[] = "Three";

	cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR);
	pcDatabase = CreateTransactionTestCodabase(WRITE_TRANSACTION_DIR, DS_No);
	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcTransaction->Set(1LL, szUno, strlen(szUno) + 1));
	AssertTrue(pcTransaction->Add(3LL, "Gamma", szThree, strlen(szThree) + 1));
	AssertTrue(pcTransaction->Remove(2LL));

	//Iteration shows the transaction's own changes, in index order.
	memset(szData, 0, 256);
	oi = pcTransaction->StartIndexIteration(&sIter, szData, &iDataSize, 256);
	AssertLong(1LL, oi);
	AssertString(szUno, szData);
	AssertLSize(4, iDataSize);
	memset(szData, 0, 256);
	oi = pcTransaction->IndexIterate(&sIter, szData, &iDataSize, 256);
	AssertLong(3LL, oi);
	AssertString(szThree, szData);
	AssertLong(INVALID_O_INDEX, pcTransaction->IndexIterate(&sIter, szData, &iDataSize, 256));

	//Names are in name order.  A removed name is not shown.
	AssertTrue(pcTransaction->StartNameIteration(&sIter, szKey, &oi));
	AssertString("Alpha", szKey);
	AssertLong(1LL, oi);
	AssertTrue(pcTransaction->NameIterate(&sIter, szKey, &oi));
	AssertString("Gamma", szKey);
	AssertLong(3LL, oi);
	AssertFalse(pcTransaction->NameIterate(&sIter, szKey, &oi));

	AssertTrue(pcTransaction->Remove("Alpha"));
	AssertTrue(pcTransaction->StartNameIteration(&sIter, szKey, &oi));
	AssertString("Gamma", szKey);
	AssertFalse(pcTransaction->NameIterate(&sIter, szKey, &oi));
	sIter.Kill();

	pcTransaction->Abort();
	SafeKill(pcTransaction);
	SafeKill(pcDatabase);
	AssertTrue(cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransactionDirectSession(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcTransaction;
	CFileUtil			cFileUtil;
	SLogConfig			sLogConfig;
	char				szDirect[] = "Direct";

	cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR);
	pcDatabase = CreateTransactionTestCodabase(WRITE_TRANSACTION_DIR, DS_No);

	//Writing directly needs a session.  Reading does not.
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcDatabase->Add(9LL, szDirect, strlen(szDirect) + 1));
	AssertFalse(pcDatabase->Close());
	gcLogger.SetConfig(&sLogConfig);
	AssertStoredString(pcDatabase, 1LL, "One");

	//The thread that has the session cannot begin a transaction, or open a second session: it would wait for itself.
	AssertTrue(pcDatabase->Open());
	sLogConfig = gcLogger.SetSilent();
	AssertNull(pcDatabase->BeginWriteTransaction());
	AssertFalse(pcDatabase->Open());
	gcLogger.SetConfig(&sLogConfig);
	AssertTrue(pcDatabase->Add(9LL, szDirect, strlen(szDirect) + 1));
	AssertTrue(pcDatabase->Close());

	//Nor can the thread with a transaction open a session.
	pcTransaction = pcDatabase->BeginWriteTransaction();
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcDatabase->Open());
	AssertFalse(pcDatabase->Add(10LL, szDirect, strlen(szDirect) + 1));
	gcLogger.SetConfig(&sLogConfig);
	pcTransaction->Abort();
	SafeKill(pcTransaction);
	SafeKill(pcDatabase);

	pcDatabase = CCodabaseFactory::Create(WRITE_TRANSACTION_DIR, IWT_No, DS_No);
	AssertStoredString(pcDatabase, 9LL, szDirect);
	AssertFalse(pcDatabase->Contains(10LL));
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransactionCommitFails(void)
{
#ifdef LINUX_GNU
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcTransaction;
	CFileUtil			cFileUtil;
	SLogConfig			sLogConfig;
	char				szLost[] = "Lost";

	cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR);
	pcDatabase = CreateTransactionTestCodabase(WRITE_TRANSACTION_DIR, DS_No);

	//The commit cannot create its start mark, so it fails part way.
	chmod(WRITE_TRANSACTION_DIR _FS_ "Primary", 0555);
	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcTransaction->Set(1LL, szLost, strlen(szLost) + 1));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcTransaction->Commit());
	gcLogger.SetConfig(&sLogConfig);
	SafeKill(pcTransaction);
	chmod(WRITE_TRANSACTION_DIR _FS_ "Primary", 0755);

	//The Codabase is read only until it is restarted.
	AssertTrue(pcDatabase->IsFailed());
	sLogConfig = gcLogger.SetSilent();
	AssertNull(pcDatabase->BeginWriteTransaction());
	AssertFalse(pcDatabase->Open());
	gcLogger.SetConfig(&sLogConfig);
	AssertTrue(pcDatabase->Contains(2LL));
	SafeKill(pcDatabase);

	//Starting again recovers the state before the failed commit.
	pcDatabase = CCodabaseFactory::Create(WRITE_TRANSACTION_DIR, IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	AssertFalse(pcDatabase->IsFailed());
	AssertStoredString(pcDatabase, 1LL, "One");
	pcTransaction = pcDatabase->BeginWriteTransaction();
	AssertNotNull(pcTransaction);
	AssertTrue(pcTransaction->Set(1LL, szLost, strlen(szLost) + 1));
	AssertTrue(pcTransaction->Commit());
	SafeKill(pcTransaction);
	AssertStoredString(pcDatabase, 1LL, szLost);
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir(WRITE_TRANSACTION_DIR));
#endif // LINUX_GNU
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWriteTransaction(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestWriteTransactionCommit();
	TestWriteTransactionAbort();
	TestWriteTransactionRules();
	TestWriteTransactionIteration();
	TestWriteTransactionDirectSession();
	TestWriteTransactionCommitFails();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

