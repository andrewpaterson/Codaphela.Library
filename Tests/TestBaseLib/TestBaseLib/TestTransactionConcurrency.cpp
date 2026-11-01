#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/LockManager.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/WriteTransaction.h"
#include "BaseLib/ReadTransaction.h"
#include "TestLib/Assert.h"
#include "TestTransactionHelper.h"


#define CONCURRENCY_DIR	"Output" _FS_ "TransactionConcurrency"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void KillConcurrencyCodabase(CCodabase* pcDatabase)
{
	CFileUtil	cFileUtil;

	AssertSize(0, pcDatabase->GetLockManager()->NumLocked());
	SafeKill(pcDatabase);
	AssertTrue(cFileUtil.RemoveDir(CONCURRENCY_DIR));
}


struct SLockWaiter
{
	CLockManager*		pcLocks;
	CTestStage			cStage;
	ELockResult			eResult;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LockWaiter(void* pvData)
{
	SLockWaiter*	psWaiter;

	psWaiter = (SLockWaiter*)pvData;
	psWaiter->cStage.Set(1);
	psWaiter->eResult = psWaiter->pcLocks->Lock(1, 100LL, LM_Exclusive);
	psWaiter->cStage.Set(2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLockManagerRules(void)
{
	CLockManager	cLocks;
	CNativeThread	cThread;
	SLockWaiter		sWaiter;
	CArrayOIndex	aoiLocked;

	AssertTrue(CLockManager::IsCompatible(LM_Shared, LM_Shared));
	AssertTrue(CLockManager::IsCompatible(LM_Shared, LM_Update));
	AssertTrue(CLockManager::IsCompatible(LM_Update, LM_Shared));
	AssertFalse(CLockManager::IsCompatible(LM_Update, LM_Update));
	AssertFalse(CLockManager::IsCompatible(LM_Shared, LM_Exclusive));
	AssertFalse(CLockManager::IsCompatible(LM_Exclusive, LM_Shared));
	AssertFalse(CLockManager::IsCompatible(LM_Update, LM_Exclusive));

	cLocks.Init();

	//Shared locks are shared.  A younger transaction that conflicts dies.
	AssertInt(LR_Granted, cLocks.Lock(5, 100LL, LM_Shared));
	AssertInt(LR_Granted, cLocks.Lock(6, 100LL, LM_Shared));
	AssertInt(LR_Died, cLocks.Lock(7, 100LL, LM_Exclusive));
	AssertInt(LM_None, cLocks.GetMode(7, 100LL));

	//Only one update lock.  Holding a stronger lock grants a weaker one.
	AssertInt(LR_Granted, cLocks.Lock(5, 100LL, LM_Update));
	AssertInt(LM_Update, cLocks.GetMode(5, 100LL));
	AssertInt(LR_Died, cLocks.Lock(8, 100LL, LM_Update));
	AssertInt(LR_Granted, cLocks.Lock(5, 100LL, LM_Shared));
	AssertInt(LM_Update, cLocks.GetMode(5, 100LL));

	//An upgrade only conflicts with the other holders.
	cLocks.ReleaseAll(6);
	AssertInt(LR_Granted, cLocks.Lock(5, 100LL, LM_Exclusive));
	AssertInt(LM_Exclusive, cLocks.GetMode(5, 100LL));
	cLocks.ReleaseAll(5);
	AssertSize(0, cLocks.NumLocked());

	//An older transaction waits.  A younger one behind a waiting writer dies even though it could share with the holder.
	AssertInt(LR_Granted, cLocks.Lock(5, 100LL, LM_Shared));
	sWaiter.pcLocks = &cLocks;
	sWaiter.cStage.Init();
	cThread.Init();
	AssertTrue(cThread.Start(LockWaiter, &sWaiter));
	sWaiter.cStage.WaitFor(1);
	ThreadSleep(50);
	AssertInt(1, sWaiter.cStage.Get());
	AssertInt(LR_Died, cLocks.Lock(3, 100LL, LM_Shared));
	cLocks.ReleaseAll(5);
	cThread.Join();
	cThread.Kill();
	AssertInt(2, sWaiter.cStage.Get());
	AssertInt(LR_Granted, sWaiter.eResult);
	AssertInt(LM_Exclusive, cLocks.GetMode(1, 100LL));
	sWaiter.cStage.Kill();
	cLocks.ReleaseAll(1);
	AssertSize(0, cLocks.NumLocked());

	//Release only frees the objects it is given.
	AssertInt(LR_Granted, cLocks.Lock(9, 200LL, LM_Shared));
	AssertInt(LR_Granted, cLocks.Lock(9, 201LL, LM_Exclusive));
	AssertInt(LR_Granted, cLocks.Lock(9, 202LL, LM_Update));
	aoiLocked.Init();
	aoiLocked.Add(200LL);
	aoiLocked.Add(201LL);
	cLocks.Release(9, &aoiLocked);
	aoiLocked.Kill();
	AssertSize(1, cLocks.NumLocked());
	AssertInt(LM_None, cLocks.GetMode(9, 200LL));
	AssertInt(LM_Update, cLocks.GetMode(9, 202LL));
	cLocks.ReleaseAll(9);
	AssertSize(0, cLocks.NumLocked());

	cLocks.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTransactionConcurrencyDie(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcOlder;
	CWriteTransaction*	pcYounger;
	CWriteTransaction*	pcYoungest;
	char				szOlder[] = "Older";
	char				szYounger[] = "Younger";
	char				szResult[256];
	int64				iStartNumber;

	pcDatabase = CreateTransactionTestCodabase(CONCURRENCY_DIR, DS_No);

	//Transactions that do not conflict both commit.
	pcOlder = pcDatabase->BeginWriteTransaction();
	pcYounger = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcOlder->GetStartNumber() < pcYounger->GetStartNumber());
	AssertTrue(pcOlder->Set(1LL, szOlder, strlen(szOlder) + 1));
	AssertTrue(pcYounger->Set(2LL, szYounger, strlen(szYounger) + 1));
	AssertTrue(pcYounger->Commit());
	AssertTrue(pcOlder->Commit());
	SafeKill(pcOlder);
	SafeKill(pcYounger);
	AssertStoredString(pcDatabase, 1LL, szOlder);
	AssertStoredString(pcDatabase, 2LL, szYounger);

	//Readers share.  A younger transaction that conflicts with an older one dies and loses its changes.
	pcOlder = pcDatabase->BeginWriteTransaction();
	pcYounger = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcOlder->Get(1LL, szResult));
	AssertTrue(pcYounger->Get(1LL, szResult));
	AssertTrue(pcYounger->Set(2LL, szOlder, strlen(szOlder) + 1));
	AssertFalse(pcYounger->Set(1LL, szYounger, strlen(szYounger) + 1));
	AssertTrue(pcYounger->HasDied());
	AssertFalse(pcYounger->IsActive());
	AssertFalse(pcYounger->Get(1LL, szResult));
	AssertFalse(pcYounger->Commit());

	//Restarting keeps the start number.  Once the older one has committed the restarted one succeeds.
	iStartNumber = pcYounger->GetStartNumber();
	AssertTrue(pcOlder->Set(1LL, szOlder, strlen(szOlder) + 1));
	AssertTrue(pcOlder->Commit());
	AssertTrue(pcYounger->Restart());
	AssertLong(iStartNumber, pcYounger->GetStartNumber());
	pcYoungest = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcYoungest->Get(2LL, szResult));
	AssertTrue(pcYounger->Set(1LL, szYounger, strlen(szYounger) + 1));
	AssertTrue(pcYounger->Commit());
	SafeKill(pcYoungest);
	SafeKill(pcOlder);
	SafeKill(pcYounger);
	AssertStoredString(pcDatabase, 1LL, szYounger);
	AssertStoredString(pcDatabase, 2LL, szYounger);

	//An update lock lets readers in but not another updater.
	pcOlder = pcDatabase->BeginWriteTransaction();
	pcYounger = pcDatabase->BeginWriteTransaction();
	pcYoungest = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcOlder->Lock(1LL, LM_Update));
	AssertTrue(pcYounger->Get(1LL, szResult));
	AssertFalse(pcYoungest->Lock(1LL, LM_Update));
	AssertTrue(pcYoungest->HasDied());
	pcOlder->Abort();
	pcYounger->Abort();
	SafeKill(pcOlder);
	SafeKill(pcYounger);
	SafeKill(pcYoungest);

	KillConcurrencyCodabase(pcDatabase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTransactionConcurrencyNames(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcFirst;
	CWriteTransaction*	pcSecond;
	SLogConfig			sLogConfig;
	char				szFirst[] = "First";
	char				szSecond[] = "Second";

	pcDatabase = CreateTransactionTestCodabase(CONCURRENCY_DIR, DS_No);

	//The second transaction to add a name fails straight away.  It does not die and does not wait.
	pcFirst = pcDatabase->BeginWriteTransaction();
	pcSecond = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcFirst->Add(5LL, "Epsilon", szFirst, strlen(szFirst) + 1));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcSecond->Add(6LL, "Epsilon", szSecond, strlen(szSecond) + 1));
	AssertFalse(pcSecond->Add(7LL, "Alpha", szSecond, strlen(szSecond) + 1));
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(pcSecond->HasDied());
	AssertTrue(pcSecond->IsActive());

	//Once the first aborts the name is free.
	pcFirst->Abort();
	AssertTrue(pcSecond->Add(6LL, "Epsilon", szSecond, strlen(szSecond) + 1));
	AssertTrue(pcSecond->Commit());
	SafeKill(pcFirst);
	SafeKill(pcSecond);
	AssertLong(6LL, pcDatabase->GetIndex("Epsilon"));
	AssertFalse(pcDatabase->Contains(5LL));

	KillConcurrencyCodabase(pcDatabase);
}


struct SConcurrencyThread
{
	CCodabase*			pcDatabase;
	CTestStage			cStage;
	bool				bResult;
	bool				bDied;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void OlderTransactionThread(void* pvData)
{
	SConcurrencyThread*	psData;
	CWriteTransaction*	pcTransaction;
	char				szOlder[] = "Older";

	//Begun before the main thread's transaction, so it is older and waits for it.
	psData = (SConcurrencyThread*)pvData;
	pcTransaction = psData->pcDatabase->BeginWriteTransaction();
	psData->cStage.Set(1);
	psData->cStage.WaitFor(2);
	psData->bResult = pcTransaction->Set(1LL, szOlder, strlen(szOlder) + 1);
	psData->bResult = psData->bResult && pcTransaction->Commit();
	SafeKill(pcTransaction);
	psData->cStage.Set(3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTransactionConcurrencyOlderWaits(void)
{
	CCodabase*			pcDatabase;
	CWriteTransaction*	pcYounger;
	CNativeThread		cThread;
	SConcurrencyThread	sData;
	char				szYounger[] = "Younger";

	pcDatabase = CreateTransactionTestCodabase(CONCURRENCY_DIR, DS_No);

	sData.pcDatabase = pcDatabase;
	sData.cStage.Init();
	cThread.Init();
	AssertTrue(cThread.Start(OlderTransactionThread, &sData));
	sData.cStage.WaitFor(1);

	pcYounger = pcDatabase->BeginWriteTransaction();
	AssertTrue(pcYounger->Set(1LL, szYounger, strlen(szYounger) + 1));
	sData.cStage.Set(2);
	ThreadSleep(50);
	AssertInt(2, sData.cStage.Get());

	//The older transaction was waiting for this one.  It goes on once this one commits.
	AssertTrue(pcYounger->Commit());
	SafeKill(pcYounger);
	cThread.Join();
	cThread.Kill();
	AssertInt(3, sData.cStage.Get());
	AssertTrue(sData.bResult);
	AssertStoredString(pcDatabase, 1LL, "Older");
	sData.cStage.Kill();

	KillConcurrencyCodabase(pcDatabase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void HoldingTransactionThread(void* pvData)
{
	SConcurrencyThread*	psData;
	CWriteTransaction*	pcTransaction;
	char				szHeld[] = "Held";

	psData = (SConcurrencyThread*)pvData;
	pcTransaction = psData->pcDatabase->BeginWriteTransaction();
	psData->bResult = pcTransaction->Set(1LL, szHeld, strlen(szHeld) + 1);
	psData->cStage.Set(1);
	ThreadSleep(100);
	psData->cStage.Set(2);
	psData->bResult = psData->bResult && pcTransaction->Commit();
	SafeKill(pcTransaction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void YoungerThanSessionThread(void* pvData)
{
	SConcurrencyThread*	psData;
	CWriteTransaction*	pcTransaction;
	char				szResult[256];

	psData = (SConcurrencyThread*)pvData;
	pcTransaction = psData->pcDatabase->BeginWriteTransaction();
	psData->bResult = pcTransaction->Get(1LL, szResult);
	psData->bDied = pcTransaction->HasDied();
	SafeKill(pcTransaction);
	psData->cStage.Set(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTransactionConcurrencySession(void)
{
	CCodabase*			pcDatabase;
	CNativeThread		cThread;
	SConcurrencyThread	sData;
	char				szSession[] = "Session";

	pcDatabase = CreateTransactionTestCodabase(CONCURRENCY_DIR, DS_No);

	//The session is always the oldest.  It waits for a transaction that has the object it wants.
	sData.pcDatabase = pcDatabase;
	sData.cStage.Init();
	cThread.Init();
	AssertTrue(cThread.Start(HoldingTransactionThread, &sData));
	sData.cStage.WaitFor(1);
	AssertTrue(pcDatabase->Open());
	AssertTrue(pcDatabase->Set(1LL, szSession, strlen(szSession) + 1));
	AssertInt(2, sData.cStage.Get());
	AssertTrue(pcDatabase->Close());
	cThread.Join();
	cThread.Kill();
	AssertTrue(sData.bResult);
	AssertStoredString(pcDatabase, 1LL, szSession);
	sData.cStage.Kill();

	//A transaction that wants an object the session has dies.
	AssertTrue(pcDatabase->Open());
	AssertTrue(pcDatabase->Set(1LL, szSession, strlen(szSession) + 1));
	sData.cStage.Init();
	sData.bDied = false;
	cThread.Init();
	AssertTrue(cThread.Start(YoungerThanSessionThread, &sData));
	cThread.Join();
	cThread.Kill();
	AssertFalse(sData.bResult);
	AssertTrue(sData.bDied);
	AssertTrue(pcDatabase->Close());
	sData.cStage.Kill();

	KillConcurrencyCodabase(pcDatabase);
}


#define BANK_ACCOUNTS			8
#define BANK_START_BALANCE		1000
#define BANK_TELLERS			4
#define BANK_TRANSFERS			150
#define BANK_AUDITORS			2


struct SBankThread
{
	CCodabase*			pcDatabase;
	CTestStage*			pcTellersFinished;
	int					iSeed;
	int					iCommitted;
	int					iDeaths;
	int					iWrongTotals;
	int					iAudits;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BankTeller(void* pvData)
{
	SBankThread*		psData;
	CWriteTransaction*	pcTransaction;
	int					i;
	int					iFrom;
	int					iTo;
	int					iAmount;
	int					iFromBalance;
	int					iToBalance;
	bool				bResult;
	uint32				uiRandom;

	psData = (SBankThread*)pvData;
	uiRandom = (uint32)psData->iSeed;
	for (i = 0; i < BANK_TRANSFERS; i++)
	{
		uiRandom = uiRandom * 1103515245u + 12345u;
		iFrom = (int)((uiRandom >> 16) % BANK_ACCOUNTS);
		uiRandom = uiRandom * 1103515245u + 12345u;
		iTo = (int)((uiRandom >> 16) % BANK_ACCOUNTS);
		uiRandom = uiRandom * 1103515245u + 12345u;
		iAmount = (int)((uiRandom >> 16) % 50);
		if (iFrom == iTo)
		{
			continue;
		}

		//A transaction that dies is restarted until it commits.
		pcTransaction = psData->pcDatabase->BeginWriteTransaction();
		for (;;)
		{
			bResult = pcTransaction->Get((OIndex)(iFrom + 1), &iFromBalance);
			bResult = bResult && pcTransaction->Get((OIndex)(iTo + 1), &iToBalance);
			if (bResult)
			{
				iFromBalance -= iAmount;
				iToBalance += iAmount;
				bResult = pcTransaction->Set((OIndex)(iFrom + 1), &iFromBalance, sizeof(int));
				bResult = bResult && pcTransaction->Set((OIndex)(iTo + 1), &iToBalance, sizeof(int));
				bResult = bResult && pcTransaction->Commit();
			}
			if (bResult)
			{
				psData->iCommitted++;
				break;
			}
			if (!pcTransaction->HasDied())
			{
				break;
			}
			psData->iDeaths++;
			pcTransaction->Restart();
			ThreadYield();
		}
		SafeKill(pcTransaction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BankAuditor(void* pvData)
{
	SBankThread*		psData;
	CReadTransaction*	pcRead;
	int					i;
	int					iBalance;
	int					iTotal;

	//Every snapshot must hold all the money, however the transfers interleave.
	psData = (SBankThread*)pvData;
	do
	{
		pcRead = psData->pcDatabase->BeginReadTransaction();
		iTotal = 0;
		for (i = 0; i < BANK_ACCOUNTS; i++)
		{
			iBalance = 0;
			pcRead->Get((OIndex)(i + 1), &iBalance);
			iTotal += iBalance;
			ThreadYield();
		}
		SafeKill(pcRead);
		if (iTotal != BANK_ACCOUNTS * BANK_START_BALANCE)
		{
			psData->iWrongTotals++;
		}
		psData->iAudits++;
	}
	while (psData->pcTellersFinished->Get() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTransactionConcurrencyBank(void)
{
	CCodabase*			pcDatabase;
	CFileUtil			cFileUtil;
	CWriteTransaction*	pcTransaction;
	CNativeThread		acTellers[BANK_TELLERS];
	CNativeThread		acAuditors[BANK_AUDITORS];
	SBankThread			asTellers[BANK_TELLERS];
	SBankThread			asAuditors[BANK_AUDITORS];
	CTestStage			cTellersFinished;
	int					i;
	int					iBalance;
	int					iTotal;
	int					iCommitted;

	cFileUtil.RemoveDir(CONCURRENCY_DIR);
	pcDatabase = CCodabaseFactory::Create(CONCURRENCY_DIR, IWT_No, DS_No);
	pcTransaction = pcDatabase->BeginWriteTransaction();
	iBalance = BANK_START_BALANCE;
	for (i = 0; i < BANK_ACCOUNTS; i++)
	{
		AssertTrue(pcTransaction->Add((OIndex)(i + 1), &iBalance, sizeof(int)));
	}
	AssertTrue(pcTransaction->Commit());
	SafeKill(pcTransaction);

	cTellersFinished.Init();
	for (i = 0; i < BANK_AUDITORS; i++)
	{
		memset(&asAuditors[i], 0, sizeof(SBankThread));
		asAuditors[i].pcDatabase = pcDatabase;
		asAuditors[i].pcTellersFinished = &cTellersFinished;
		acAuditors[i].Init();
		AssertTrue(acAuditors[i].Start(BankAuditor, &asAuditors[i]));
	}
	for (i = 0; i < BANK_TELLERS; i++)
	{
		memset(&asTellers[i], 0, sizeof(SBankThread));
		asTellers[i].pcDatabase = pcDatabase;
		asTellers[i].iSeed = i * 7919 + 1;
		acTellers[i].Init();
		AssertTrue(acTellers[i].Start(BankTeller, &asTellers[i]));
	}

	iCommitted = 0;
	for (i = 0; i < BANK_TELLERS; i++)
	{
		acTellers[i].Join();
		acTellers[i].Kill();
		iCommitted += asTellers[i].iCommitted;
	}
	cTellersFinished.Set(1);
	for (i = 0; i < BANK_AUDITORS; i++)
	{
		acAuditors[i].Join();
		acAuditors[i].Kill();
		AssertInt(0, asAuditors[i].iWrongTotals);
		AssertTrue(asAuditors[i].iAudits > 0);
	}
	cTellersFinished.Kill();
	AssertTrue(iCommitted > 0);

	//No money was made or lost.
	iTotal = 0;
	for (i = 0; i < BANK_ACCOUNTS; i++)
	{
		iBalance = 0;
		AssertTrue(pcDatabase->Get((OIndex)(i + 1), &iBalance));
		iTotal += iBalance;
	}
	AssertInt(BANK_ACCOUNTS * BANK_START_BALANCE, iTotal);
	AssertLSize(0, pcDatabase->NumVersions());

	KillConcurrencyCodabase(pcDatabase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTransactionConcurrency(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestLockManagerRules();
	TestTransactionConcurrencyDie();
	TestTransactionConcurrencyNames();
	TestTransactionConcurrencyOlderWaits();
	TestTransactionConcurrencySession();
	TestTransactionConcurrencyBank();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

