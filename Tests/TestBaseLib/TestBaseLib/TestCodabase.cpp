#include "BaseLib/TypeConverter.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/CharsHelper.h"
#include "BaseLib/LogFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/Codabase.h"
#include "TestLib/Words.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabaseFactoryCreate(void)
{
	CCodabase*	pcDatabase;
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir("Output" _FS_ "CodabaseFactory1");

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "CodabaseFactory1", IWT_No, DS_No);

	AssertTrue(cFileUtil.Exists("Output" _FS_ "CodabaseFactory1" _FS_ "Primary" _FS_ "Index"));
	AssertTrue(cFileUtil.Exists("Output" _FS_ "CodabaseFactory1" _FS_ "Primary" _FS_ "Names"));
	AssertTrue(pcDatabase->ValidateConfigInitialised());
	Pass();

	pcDatabase->Kill();
	AssertTrue(pcDatabase->ValidateConfigKilled()); 
	Pass();
	
	SafeFree(pcDatabase);
	Pass();

	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "CodabaseFactory1"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabaseOperations(void)
{
	CCodabase*	pcDatabase;
	CFileUtil	cFileUtil;

	OIndex		oiKey1 = 0x4902385234347LL;
	char		szData1[] = "Village did removed enjoyed explain nor ham saw calling talking. Securing as informed declared or margaret. Joy horrible moreover man feelings own shy. Request norland neither mistake for yet. Between the for morning assured country believe. On even feet time have an no at. Relation so in confined smallest children unpacked delicate. Why sir end believe uncivil respect. Always get adieus nature day course for common. My little garret repair to desire he esteem.";

	OIndex		oiKey2 = 0x4902383278911LL;
	char		szData2[] = "Folly words widow one downs few age every seven. If miss part by fact he park just shew. Discovered had get considered projection who favourable. Necessary up knowledge it tolerably. Unwilling departure education is be dashwoods or an. Use off agreeable law unwilling sir deficient curiosity instantly. Easy mind life fact with see has bore ten. Parish any chatty can elinor direct for former. Up as meant widow equal an share least.";

	OIndex		oiKey3 = 0x4902394579823LL;
	char		szName3[] = "Advanced";
	char		szData3[] = "One advanced diverted domestic sex repeated bringing you old.Possible procured her trifling laughter thoughts property she met way.Companions shy had solicitude favourable own.Which could saw guest man now heard but.Lasted my coming uneasy marked so should.Gravity letters it amongst herself dearest an windows by.Wooded ladies she basket season age her uneasy saw.Discourse unwilling am no described dejection incommode no listening of.Before nature his parish boy.";

	OIndex		oiKey4 = 0x4902393749238LL;
	char		szName4[] = "Article";
	char		szData4[] = "Article nor prepare chicken you him now. Shy merits say advice ten before lovers innate add. She cordially behaviour can attempted estimable. Trees delay fancy noise manor do as an small. Felicity now law securing breeding likewise extended and. Roused either who favour why ham.";

	char		szResult[2 KB];

	cFileUtil.RemoveDir("Output" _FS_ "Codabase1");

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase1", IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	Pass();
	pcDatabase->Open();

	AssertTrue(pcDatabase->Add(oiKey1, szData1, strlen(szData1) + 1));
	AssertTrue(pcDatabase->Add(oiKey2, szData2, strlen(szData2) + 1));

	AssertLong(2, pcDatabase->NumIndicesCached());
	AssertLong(0, pcDatabase->NumNamesCached());
	AssertLong(2, pcDatabase->NumIndices());
	AssertLong(0, pcDatabase->NumNames());
	Pass();

	pcDatabase->Close();
	SafeKill(pcDatabase);
	Pass();


	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase1", IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	Pass();
	pcDatabase->Open();

	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(oiKey1, szResult));
	AssertString(szData1, szResult);

	AssertTrue(pcDatabase->Add(oiKey3, szName3, szData3, strlen(szData3) + 1));

	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(oiKey2, szResult));
	AssertString(szData2, szResult);

	AssertTrue(pcDatabase->Add(oiKey4, szName4, szData4, strlen(szData4) + 1));

	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(oiKey3, szResult));
	AssertString(szData3, szResult);

	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(oiKey4, szResult));
	AssertString(szData4, szResult);

	AssertLong(4, pcDatabase->NumIndicesCached());
	AssertLong(2, pcDatabase->NumNamesCached());
	AssertLong(4, pcDatabase->NumIndices());
	AssertLong(2, pcDatabase->NumNames());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	Pass();


	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase1", IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	Pass();
	pcDatabase->Open();

	AssertLong(0, pcDatabase->NumIndicesCached());
	AssertLong(0, pcDatabase->NumNamesCached());
	AssertLong(4, pcDatabase->NumIndices());
	AssertLong(2, pcDatabase->NumNames());
	AssertLong(4, pcDatabase->NumIndicesCached());
	AssertLong(2, pcDatabase->NumNamesCached());

	pcDatabase->Close();
	SafeKill(pcDatabase);
	Pass();


	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "Codabase1"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabaseSetPutRemove(EIndexWriteThrough eWriteThrough)
{
	CCodabase*	pcDatabase;
	CFileUtil	cFileUtil;
	SLogConfig	sLogConfig;
	char		szResult[2 KB];
	char		szName[MAX_KEY_SIZE];
	lsize		uiDataSize;
	size		uiNameLength;
	char		szOne[] = "One";
	char		szTwo[] = "Two";
	char		szUno[] = "Uno";
	char		szEins[] = "Eins";
	char		szVier[] = "Vier";
	char		szZwei[] = "Zwei";
	char		szFour[] = "Four";
	char		szFunf[] = "Funf";
	char		szBad[] = "Bad";

	cFileUtil.RemoveDir("Output" _FS_ "Codabase2");

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase2", eWriteThrough, DS_No);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());

	AssertTrue(pcDatabase->Add(1LL, "Alpha", szOne, strlen(szOne) + 1));
	AssertTrue(pcDatabase->Add(2LL, szTwo, strlen(szTwo) + 1));
	AssertLong(2, pcDatabase->NumIndices());
	AssertLong(1, pcDatabase->NumNames());

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcDatabase->Add(1LL, "Beta", szBad, strlen(szBad) + 1));
	AssertFalse(pcDatabase->Add(3LL, "Alpha", szBad, strlen(szBad) + 1));
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(pcDatabase->Contains("Beta"));
	AssertFalse(pcDatabase->Contains(3LL));
	AssertLong(1LL, pcDatabase->GetIndex("Alpha"));
	AssertLong(2, pcDatabase->NumIndices());
	AssertLong(1, pcDatabase->NumNames());

	AssertTrue(pcDatabase->Set(1LL, szUno, strlen(szUno) + 1));
	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get("Alpha", szResult));
	AssertString(szUno, szResult);
	AssertTrue(pcDatabase->GetName(1LL, szName, &uiNameLength, MAX_KEY_SIZE));
	AssertString("Alpha", szName);

	AssertTrue(pcDatabase->Set("Alpha", szEins, strlen(szEins) + 1));
	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(1LL, &uiDataSize, szResult, 2 KB));
	AssertLSize(5, uiDataSize);
	AssertString(szEins, szResult);

	AssertTrue(pcDatabase->Set(1LL, szVier));
	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(1LL, &uiDataSize, szResult, 2 KB));
	AssertLSize(5, uiDataSize);
	AssertString(szVier, szResult);

	AssertTrue(pcDatabase->Put(2LL, szZwei, strlen(szZwei) + 1));
	AssertTrue(pcDatabase->Put(4LL, "Delta", szVier, strlen(szVier) + 1));
	AssertTrue(pcDatabase->Put(4LL, "Delta", szFour, strlen(szFour) + 1));
	AssertTrue(pcDatabase->Put(5LL, szFunf, strlen(szFunf) + 1));
	AssertLong(4, pcDatabase->NumIndices());
	AssertLong(2, pcDatabase->NumNames());

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcDatabase->Put(6LL, "Alpha", szBad, strlen(szBad) + 1));
	AssertFalse(pcDatabase->Put(1LL, "Delta", szBad, strlen(szBad) + 1));
	AssertFalse(pcDatabase->Put(2LL, "Gamma", szBad, strlen(szBad) + 1));
	AssertFalse(pcDatabase->Put(2LL, (char*)NULL, szBad, strlen(szBad) + 1));
	AssertFalse(pcDatabase->Put(2LL, "", szBad, strlen(szBad) + 1));
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(pcDatabase->Contains(6LL));
	AssertFalse(pcDatabase->Contains("Gamma"));
	AssertLong(4, pcDatabase->NumIndices());
	AssertLong(2, pcDatabase->NumNames());

	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(2LL, szResult));
	AssertString(szZwei, szResult);
	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get("Delta", szResult));
	AssertString(szFour, szResult);
	AssertLong(4LL, pcDatabase->GetIndex("Delta"));

	AssertTrue(pcDatabase->Remove(5LL));
	AssertTrue(pcDatabase->Remove("Delta"));
	AssertFalse(pcDatabase->Contains(5LL));
	AssertFalse(pcDatabase->Contains(4LL));
	AssertFalse(pcDatabase->Contains("Delta"));
	AssertLong(2, pcDatabase->NumIndices());
	AssertLong(1, pcDatabase->NumNames());

	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	Pass();


	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase2", eWriteThrough, DS_No);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());

	AssertLong(2, pcDatabase->NumIndices());
	AssertLong(1, pcDatabase->NumNames());
	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get("Alpha", szResult));
	AssertString(szVier, szResult);
	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get(2LL, szResult));
	AssertString(szZwei, szResult);
	AssertFalse(pcDatabase->Contains(4LL));
	AssertFalse(pcDatabase->Contains(5LL));
	AssertFalse(pcDatabase->Contains("Delta"));
	AssertFalse(pcDatabase->Contains("Beta"));

	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	Pass();

	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "Codabase2"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabaseIterate(EIndexWriteThrough eWriteThrough)
{
	CCodabase*				pcDatabase;
	CFileUtil				cFileUtil;
	SIndexTreeFileIterator	sIter;
	char					szResult[2 KB];
	char					szKey[MAX_KEY_SIZE];
	lsize					iDataSize;
	OIndex					oi;
	bool					bResult;
	int						iCount;
	char					szRutherford[] = "Rutherford";
	char					szBohr[] = "Bohr";
	char					szPlanck[] = "Planck";

	cFileUtil.RemoveDir("Output" _FS_ "Codabase3");

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase3", eWriteThrough, DS_No);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());

	AssertTrue(pcDatabase->Add(0x0102LL, "Ernest", szRutherford, strlen(szRutherford) + 1));
	AssertTrue(pcDatabase->Add(0x0203LL, "Niels", szBohr, strlen(szBohr) + 1));
	AssertTrue(pcDatabase->Add(0x0304LL, szPlanck, strlen(szPlanck) + 1));
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "Codabase3", eWriteThrough, DS_No);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());

	iCount = 0;
	memset(szResult, 0, 2 KB);
	oi = pcDatabase->StartIndexIteration(&sIter, szResult, &iDataSize, 2 KB);
	while (oi != INVALID_O_INDEX)
	{
		if (oi == 0x0102LL)
		{
			AssertString(szRutherford, szResult);
			AssertLSize(strlen(szRutherford) + 1, iDataSize);
		}
		else if (oi == 0x0203LL)
		{
			AssertString(szBohr, szResult);
			AssertLSize(strlen(szBohr) + 1, iDataSize);
		}
		else if (oi == 0x0304LL)
		{
			AssertString(szPlanck, szResult);
			AssertLSize(strlen(szPlanck) + 1, iDataSize);
		}
		else
		{
			Fail();
		}
		iCount++;
		memset(szResult, 0, 2 KB);
		oi = pcDatabase->IndexIterate(&sIter, szResult, &iDataSize, 2 KB);
	}
	AssertInt(3, iCount);

	iCount = 0;
	memset(szKey, 0, MAX_KEY_SIZE);
	bResult = pcDatabase->StartNameIteration(&sIter, szKey, &oi);
	while (bResult)
	{
		AssertLong(pcDatabase->GetIndex(szKey), oi);
		iCount++;
		memset(szKey, 0, MAX_KEY_SIZE);
		bResult = pcDatabase->NameIterate(&sIter, szKey, &oi);
	}
	AssertInt(2, iCount);

	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	Pass();

	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "Codabase3"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddCodabaseCrashFiles(CArrayChars* paszRelative, const char* szDirectory, bool bBackup)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszFiles;
	size		i;
	size		uiPrefix;
	CChars		szRelative;
	size		iSeparator;

	//FindAllFiles returns full paths.  Backup files are named with a '_' in front of the primary name.
	aszFiles.Init();
	cFileUtil.FindAllFiles(szDirectory, &aszFiles, true);
	uiPrefix = strlen(szDirectory) + 1;
	for (i = 0; i < aszFiles.NumElements(); i++)
	{
		szRelative.Init(strstr(aszFiles.GetText(i), szDirectory) + uiPrefix);
		if (bBackup)
		{
			iSeparator = szRelative.FindFromEnd(_FS_);
			szRelative.RemoveCharacter(iSeparator + 1);
		}
		if (paszRelative->GetIndex(&szRelative) == ARRAY_ELEMENT_NOT_FOUND)
		{
			paszRelative->Add(szRelative.Text());
		}
		szRelative.Kill();
	}
	aszFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WriteCodabaseCrashMarks(bool bRewriteMark)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszRelative;
	CTextFile	cMark;
	CChars		szBackup;
	size		i;
	size		iSeparator;

	//Write the marks CDurableSet::End would have written: the primary and backup name of every file, then "#End".
	aszRelative.Init();
	AddCodabaseCrashFiles(&aszRelative, "Output" _FS_ "CodabaseCrash" _FS_ "Primary", false);
	AddCodabaseCrashFiles(&aszRelative, "Output" _FS_ "CodabaseCrash" _FS_ "Backup", true);

	cMark.Init();
	for (i = 0; i < aszRelative.NumElements(); i++)
	{
		szBackup.Init(aszRelative.GetText(i));
		iSeparator = szBackup.FindFromEnd(_FS_);
		szBackup.Insert(iSeparator + 1, "_");

		cMark.mcText.Append("Output" _FS_ "CodabaseCrash" _FS_ "Primary" _FS_);
		cMark.mcText.Append(aszRelative.GetText(i));
		cMark.mcText.AppendNewLine();
		cMark.mcText.Append("Output" _FS_ "CodabaseCrash" _FS_ "Backup" _FS_);
		cMark.mcText.Append(szBackup.Text());
		cMark.mcText.AppendNewLine();
		szBackup.Kill();
	}
	cMark.mcText.Append("#End");
	cMark.mcText.AppendNewLine();
	AssertTrue(cMark.Write("Output" _FS_ "CodabaseCrash" _FS_ "Primary" _FS_ "Mark1.Write"));
	cMark.Kill();
	aszRelative.Kill();

	if (bRewriteMark)
	{
		AssertTrue(cFileUtil.Touch("Output" _FS_ "CodabaseCrash" _FS_ "Backup" _FS_ "Mark2.Rewrite"));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabaseCrashRecovery(bool bRewriteMark)
{
	CCodabase*	pcDatabase;
	CFileUtil	cFileUtil;
	SLogConfig	sLogConfig;
	char		szResult[2 KB];
	char		szOne[] = "Version One";
	char		szTwo[] = "Version Two Is Longer";
	char		szNew[] = "Added in version two";

	cFileUtil.RemoveDir("Output" _FS_ "CodabaseCrash");
	cFileUtil.RemoveDir("Output" _FS_ "CodabaseCrashSaved");

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "CodabaseCrash", IWT_No, DS_Yes);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());
	AssertTrue(pcDatabase->Add(1LL, "Alpha", szOne, strlen(szOne) + 1));
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "CodabaseCrashSaved" _FS_ "Backup"));
	AssertTrue(cFileUtil.CopyDir("Output" _FS_ "CodabaseCrash" _FS_ "Backup", "Output" _FS_ "CodabaseCrashSaved" _FS_ "Backup"));

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "CodabaseCrash", IWT_No, DS_Yes);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());
	AssertTrue(pcDatabase->Set(1LL, szTwo, strlen(szTwo) + 1));
	AssertTrue(pcDatabase->Add(2LL, szNew, strlen(szNew) + 1));
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);

	//Put the backup back to version one as if the process died during the commit, with the primary already at version two.
	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "CodabaseCrash" _FS_ "Backup"));
	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "CodabaseCrash" _FS_ "Backup"));
	AssertTrue(cFileUtil.CopyDir("Output" _FS_ "CodabaseCrashSaved" _FS_ "Backup", "Output" _FS_ "CodabaseCrash" _FS_ "Backup"));
	WriteCodabaseCrashMarks(bRewriteMark);

	sLogConfig = gcLogger.SetSilent();
	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "CodabaseCrash", IWT_No, DS_Yes);
	gcLogger.SetConfig(&sLogConfig);
	AssertNotNull(pcDatabase);
	AssertFalse(cFileUtil.Exists("Output" _FS_ "CodabaseCrash" _FS_ "Primary" _FS_ "Mark1.Write"));
	AssertFalse(cFileUtil.Exists("Output" _FS_ "CodabaseCrash" _FS_ "Backup" _FS_ "Mark2.Rewrite"));
	AssertTrue(pcDatabase->Open());

	memset(szResult, 0, 2 KB);
	AssertTrue(pcDatabase->Get("Alpha", szResult));
	if (bRewriteMark)
	{
		//The rewrite mark means the primary was complete, so version two is kept.
		AssertString(szTwo, szResult);
		AssertTrue(pcDatabase->Contains(2LL));
		AssertLong(2, pcDatabase->NumIndices());
	}
	else
	{
		//Without the rewrite mark the primary may be incomplete, so version one comes back.
		AssertString(szOne, szResult);
		AssertFalse(pcDatabase->Contains(2LL));
		AssertLong(1, pcDatabase->NumIndices());
	}
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "CodabaseCrash"));
	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "CodabaseCrashSaved"));
}


#define CODABASE_THREADS		4
#define CODABASE_THREAD_ITEMS	200


struct SCodabaseThreadData
{
	CCodabase*	pcDatabase;
	int			iThread;
	int			iFailures;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CodabaseThreadData(char* szDest, int iThread, int iItem, bool bUpdated)
{
	snprintf(szDest, 64, "%s from thread %d item %d", bUpdated ? "Updated" : "Added", iThread, iItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CodabaseThreadName(char* szDest, int iThread, int iItem)
{
	snprintf(szDest, 64, "Thread%dItem%d", iThread, iItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CodabaseThread(void* pvData)
{
	SCodabaseThreadData*	psData;
	int						i;
	OIndex					oi;
	char					szData[64];
	char					szName[64];
	char					szResult[64];
	bool					bResult;

	//Assert is not thread safe, so failures are counted and checked after the threads end.
	psData = (SCodabaseThreadData*)pvData;
	for (i = 0; i < CODABASE_THREAD_ITEMS; i++)
	{
		oi = (OIndex)psData->iThread * 1000 + i + 1;
		CodabaseThreadData(szData, psData->iThread, i, false);
		if ((i % 2) == 0)
		{
			CodabaseThreadName(szName, psData->iThread, i);
			bResult = psData->pcDatabase->Add(oi, szName, szData, strlen(szData) + 1);
		}
		else
		{
			bResult = psData->pcDatabase->Add(oi, szData, strlen(szData) + 1);
		}

		memset(szResult, 0, 64);
		bResult = bResult && psData->pcDatabase->Get(oi, szResult);
		bResult = bResult && (strcmp(szData, szResult) == 0);

		CodabaseThreadData(szData, psData->iThread, i, true);
		bResult = bResult && psData->pcDatabase->Put(oi, szData, strlen(szData) + 1);
		bResult = bResult && psData->pcDatabase->Contains(oi);
		if (!bResult)
		{
			psData->iFailures++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCodabaseThreadItems(CCodabase* pcDatabase)
{
	int		iThread;
	int		i;
	char	szData[64];
	char	szName[64];
	char	szResult[64];
	int		iWrong;

	iWrong = 0;
	for (iThread = 0; iThread < CODABASE_THREADS; iThread++)
	{
		for (i = 0; i < CODABASE_THREAD_ITEMS; i++)
		{
			CodabaseThreadData(szData, iThread, i, true);
			memset(szResult, 0, 64);
			if ((i % 2) == 0)
			{
				CodabaseThreadName(szName, iThread, i);
				pcDatabase->Get(szName, szResult);
			}
			else
			{
				pcDatabase->Get((OIndex)iThread * 1000 + i + 1, szResult);
			}
			if (strcmp(szData, szResult) != 0)
			{
				iWrong++;
			}
		}
	}
	AssertInt(0, iWrong);
	AssertLong(CODABASE_THREADS * CODABASE_THREAD_ITEMS, pcDatabase->NumIndices());
	AssertLong(CODABASE_THREADS * CODABASE_THREAD_ITEMS / 2, pcDatabase->NumNames());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabaseThreads(void)
{
	CCodabase*			pcDatabase;
	CFileUtil			cFileUtil;
	CNativeThread		acThreads[CODABASE_THREADS];
	SCodabaseThreadData	asData[CODABASE_THREADS];
	int					i;

	cFileUtil.RemoveDir("Output" _FS_ "CodabaseThreads");

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "CodabaseThreads", IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());

	//Several threads add, read and change their own objects at the same time.
	for (i = 0; i < CODABASE_THREADS; i++)
	{
		asData[i].pcDatabase = pcDatabase;
		asData[i].iThread = i;
		asData[i].iFailures = 0;
		acThreads[i].Init();
		AssertTrue(acThreads[i].Start(CodabaseThread, &asData[i]));
	}
	for (i = 0; i < CODABASE_THREADS; i++)
	{
		acThreads[i].Join();
		acThreads[i].Kill();
		AssertInt(0, asData[i].iFailures);
	}

	AssertCodabaseThreadItems(pcDatabase);
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);

	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "CodabaseThreads", IWT_No, DS_No);
	AssertNotNull(pcDatabase);
	AssertTrue(pcDatabase->Open());
	AssertCodabaseThreadItems(pcDatabase);
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);

	AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "CodabaseThreads"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCodabase(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	WordsInit();
	BeginTests();

	TestCodabaseFactoryCreate();
	TestCodabaseOperations();
	TestCodabaseSetPutRemove(IWT_No);
	TestCodabaseSetPutRemove(IWT_Yes);
	TestCodabaseIterate(IWT_No);
	TestCodabaseIterate(IWT_Yes);
	TestCodabaseCrashRecovery(false);
	TestCodabaseCrashRecovery(true);
	TestCodabaseThreads();

	TestStatistics();
	WordsKill();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

