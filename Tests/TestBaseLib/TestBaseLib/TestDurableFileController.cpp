#ifdef LINUX_GNU
#include <dirent.h>
#endif // LINUX_GNU
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/DurableFile.h"
#include "BaseLib/DurableSet.h"
#include "BaseLib/DurableFileController.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerDirectoryCreation(void)
{
	CDurableFileController	cController;
	CDurableFile			cDurableFile;
	char					szDirectory[] = "Output" _FS_ "Durable2" _FS_ "Write";
	char					szRewrite[] = "Output" _FS_ "Durable2" _FS_ "Rewrite";
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir("Output" _FS_ "Durable2");

	cController.Init(szDirectory, szRewrite);

	cDurableFile.Init(&cController, "Sub" _FS_ "Test.txt", "Sub" _FS_ "Test.txt");

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable2" _FS_ "Write" _FS_ "Sub" _FS_ "Test.txt"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable2" _FS_ "Rewrite" _FS_ "Sub" _FS_ "Test.txt"));

	cController.Begin();
	cController.End();
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable2" _FS_ "Write" _FS_ "Sub" _FS_ "Test.txt"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable2" _FS_ "Rewrite" _FS_ "Sub" _FS_ "Test.txt"));


	cController.Begin();
	cDurableFile.Write("Conan", 5, 1);
	cController.End();
	AssertBool(true, cFileUtil.Exists("Output" _FS_ "Durable2" _FS_ "Write" _FS_ "Sub" _FS_ "Test.txt"));
	AssertBool(true, cFileUtil.Exists("Output" _FS_ "Durable2" _FS_ "Rewrite" _FS_ "Sub" _FS_ "Test.txt"));

	cDurableFile.Kill();
	cController.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Durable2");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerRecovery(void)
{
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	CDurableFile			cDurableFile1;
	CDurableFile			cDurableFile2;
	CDurableFile			cDurableFile3;
	CDurableFile			cDurableFile4;
	bool					bResult;
	char					szDirectory[] = "Output" _FS_ "Durable1" _FS_ "Write";
	char					szRewrite[] = "Output" _FS_ "Durable1" _FS_ "Rewrite";

	cFileUtil.RemoveDir("Output" _FS_ "Durable1");

	cController.Init(szDirectory, szRewrite);

	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	cDurableFile2.Init(&cController, "2.txt", "2.txt");
	cDurableFile3.Init(&cController, "3.txt", "3.txt");
	cDurableFile4.Init(&cController, "4.txt", "4.txt");

	cController.Begin();
	cDurableFile1.Write("Dulcedo ", 8, 1);
	cDurableFile1.Write("Cogitationis", 12, 1);

	//Write some date to the files.
	cDurableFile4.Write("Age ", 4, 1);
	cDurableFile4.Write("Chaos ", 6, 1);
	cDurableFile4.Write("Battle ", 7, 1);
	cDurableFile4.Write("Orcs ", 5, 1);
	cDurableFile4.Write("Humans", 6, 1);

	cDurableFile3.Write(4, "BBBB", 4, 1);
	cDurableFile3.Write(0, "AAAA", 4, 1);
	cDurableFile3.Write(12, "DDDD", 4, 1);
	cDurableFile3.Write(8, "CCCC", 4, 1);

	cController.End();
	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();
	cController.Kill();

	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "4.txt", "Age Chaos Battle Orcs Humans");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "4.txt", "Age Chaos Battle Orcs Humans");

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "2.txt"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "2.txt"));

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "Mark1.Write"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "Mark2.Rewrite"));

	//////////////////////////////////////////////////////////////////////////

	cController.Init(szDirectory, szRewrite);

	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	cDurableFile2.Init(&cController, "2.txt", "2.txt");
	cDurableFile3.Init(&cController, "3.txt", "3.txt");
	cDurableFile4.Init(&cController, "4.txt", "4.txt");

	//Recovery should do nothing as the files are consistent.
	bResult = cController.Recover(&cDurableFile1, &cDurableFile2, &cDurableFile3, &cDurableFile4, NULL);
	AssertTrue(bResult);

	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();
	cController.Kill();

	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "4.txt", "Age Chaos Battle Orcs Humans");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "4.txt", "Age Chaos Battle Orcs Humans");

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "2.txt"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "2.txt"));

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write" _FS_ "Mark1.Write"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "Mark2.Rewrite"));

	//////////////////////////////////////////////////////////////////////////

	cFileUtil.Touch("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "Mark1.Write");
	cFileUtil.Delete("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "3.txt");
	cFileUtil.Delete("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "4.txt");

	//Init only checks its arguments.  Check finds the interrupted commit.
	bResult = cController.Init(szDirectory, szRewrite);
	AssertTrue(bResult);
	AssertFalse(cController.Check(false, false));

	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	cDurableFile2.Init(&cController, "2.txt", "2.txt");
	cDurableFile3.Init(&cController, "3.txt", "3.txt");
	cDurableFile4.Init(&cController, "4.txt", "4.txt");

	//Recovery should copy 3.txt and 4.txt back again.
	bResult = cController.Recover(&cDurableFile1, &cDurableFile2, &cDurableFile3, &cDurableFile4, NULL);
	AssertTrue(bResult);

	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();
	cController.Kill();

	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "4.txt", "Age Chaos Battle Orcs Humans");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "4.txt", "Age Chaos Battle Orcs Humans");

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "2.txt"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "2.txt"));

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "Mark1.Write"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "Mark2.Rewrite"));

	//////////////////////////////////////////////////////////////////////////

	cFileUtil.Touch("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "Mark1.Write");
	cFileUtil.Touch("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "Mark2.Rewrite");
	cFileUtil.Delete("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "1.txt");
	cFileUtil.Delete("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "4.txt");

	cController.Init(szDirectory, szRewrite);

	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	cDurableFile2.Init(&cController, "2.txt", "2.txt");
	cDurableFile3.Init(&cController, "3.txt", "3.txt");
	cDurableFile4.Init(&cController, "4.txt", "4.txt");

	//Recovery should copy _1.txt and _4.txt back again.
	bResult = cController.Recover(&cDurableFile1, &cDurableFile2, &cDurableFile3, &cDurableFile4, NULL);
	AssertTrue(bResult);

	cController.Kill();

	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "1.txt", "Dulcedo Cogitationis");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "3.txt", "AAAABBBBCCCCDDDD");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "4.txt", "Age Chaos Battle Orcs Humans");
	AssertFileString("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "4.txt", "Age Chaos Battle Orcs Humans");

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "2.txt"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "2.txt"));

	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Write"  _FS_ "Mark1.Write"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable1" _FS_ "Rewrite"  _FS_ "Mark2.Rewrite"));

	//////////////////////////////////////////////////////////////////////////
	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Durable1");
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerTwoInOneDirectory(void)
{
	CFileUtil				cFileUtil;
	CDurableFileController	cController1;
	CDurableFileController	cController2;
	CDurableFile			cDurableFile1;
	CDurableFile			cDurableFile2;
	CDurableFile			cDurableFile3;
	CDurableFile			cDurableFile4;
	bool					bResult;
	char					szDirectory[] = "Output" _FS_ "Durable3" _FS_ "Write";
	char					szRewrite[] = "Output" _FS_ "Durable3" _FS_ "Rewrite";

	cFileUtil.RemoveDir("Output" _FS_ "Durable3");

	bResult = cController1.Init(szDirectory, szRewrite, "C1_Mark1.Write", "C1_Mark2.Rewrite");
	AssertTrue(bResult); 
	bResult = cController2.Init(szDirectory, szRewrite, "C2_Mark1.Write", "C2_Mark2.Rewrite");
	AssertTrue(bResult);

	cController1.Begin();
	cController2.Begin();

	cDurableFile1.Init(&cController1, "1.txt", "1.txt");
	cDurableFile2.Init(&cController1, "2.txt", "2.txt");
	cDurableFile3.Init(&cController2, "3.txt", "3.txt");
	cDurableFile4.Init(&cController2, "4.txt", "4.txt");

	cDurableFile1.Write("Dulcedo ", 8, 1);
	cDurableFile1.Write("Cogitationis", 12, 1);

	cDurableFile2.Write("Alpha ", 6, 1);
	cDurableFile2.Write("Centauri", 8, 1);

	cDurableFile3.Write(4, "BBBB", 4, 1);
	cDurableFile3.Write(0, "AAAA", 4, 1);
	cDurableFile3.Write(12, "DDDD", 4, 1);
	cDurableFile3.Write(8, "CCCC", 4, 1);

	cDurableFile4.Write("Age ", 4, 1);
	cDurableFile4.Write("Chaos ", 6, 1);
	cDurableFile4.Write("Battle ", 7, 1);
	cDurableFile4.Write("Orcs ", 5, 1);
	cDurableFile4.Write("Humans", 6, 1);

	cController1.End();
	cController2.End();
	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();
	cController1.Kill();
	cController2.Kill();

	AssertTrue(cFileUtil.Touch("Output" _FS_ "Durable3" _FS_ "Write"  _FS_ "C2_Mark1.Write"));
	AssertTrue(cFileUtil.Delete("Output" _FS_ "Durable3" _FS_ "Write"  _FS_ "3.txt"));
	AssertTrue(cFileUtil.Delete("Output" _FS_ "Durable3" _FS_ "Write"  _FS_ "4.txt"));

	bResult = cController1.Init(szDirectory, szRewrite, "C1_Mark1.Write", "C1_Mark2.Rewrite");
	AssertTrue(bResult);
	bResult = cController2.Init(szDirectory, szRewrite, "C2_Mark1.Write", "C2_Mark2.Rewrite");
	AssertTrue(bResult);
	AssertTrue(cController1.Check(false, false));
	AssertFalse(cController2.Check(false, false));

	cDurableFile1.Init(&cController1, "1.txt", "1.txt");
	cDurableFile2.Init(&cController1, "2.txt", "2.txt");
	cDurableFile3.Init(&cController2, "3.txt", "3.txt");
	cDurableFile4.Init(&cController2, "4.txt", "4.txt");

	AssertSize(0, cController1.NumFiles());
	AssertSize(0, cController2.NumFiles());
	bResult = cController1.Recover(&cDurableFile1, &cDurableFile2, NULL);
	AssertTrue(bResult);

	bResult = cController2.Recover(&cDurableFile3, &cDurableFile4, NULL);
	AssertTrue(bResult);

	AssertTrue(cFileUtil.Exists("Output" _FS_ "Durable3" _FS_ "Write"  _FS_ "3.txt"));
	AssertTrue(cFileUtil.Exists("Output" _FS_ "Durable3" _FS_ "Write"  _FS_ "4.txt"));

	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();
	cController1.Kill();
	cController2.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Durable3");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountOpenFileDescriptors(void)
{
#ifdef LINUX_GNU
	DIR*			pDIR;
	struct dirent*	pDirEnt;
	int				iCount;

	iCount = 0;
	pDIR = opendir("/proc/self/fd");
	if (pDIR != NULL)
	{
		pDirEnt = readdir(pDIR);
		while (pDirEnt != NULL)
		{
			iCount++;
			pDirEnt = readdir(pDIR);
		}
		closedir(pDIR);
	}
	return iCount;
#else
	return 0;
#endif // LINUX_GNU
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDurableFileControllerCrashState(bool bNew)
{
	CFileUtil	cFileUtil;

	if (bNew)
	{
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "1.txt", "New One Longer");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "1.txt", "New One Longer");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "2.txt", "New2");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "2.txt", "New2");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "3.txt", "Three");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "3.txt", "Three");
	}
	else
	{
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "1.txt", "Old One");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "1.txt", "Old One");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "2.txt", "Old Two Is Longer");
		AssertFileString("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "2.txt", "Old Two Is Longer");
		AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "3.txt"));
		AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "3.txt"));
	}
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable4" _FS_ "Write" _FS_ "Mark1.Write"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "Mark2.Rewrite"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerCrash(int iCrashPoint, bool bPassFiles)
{
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	CDurableFile			cDurableFile1;
	CDurableFile			cDurableFile2;
	CDurableFile			cDurableFile3;
	CDurableFile*			apcFiles[3];
	int						i;
	int						iStep;
	SLogConfig				sLogConfig;
	char					szDirectory[] = "Output" _FS_ "Durable4" _FS_ "Write";
	char					szRewrite[] = "Output" _FS_ "Durable4" _FS_ "Rewrite";
	int						iOpenFiles;

	cFileUtil.RemoveDir("Output" _FS_ "Durable4");
	iOpenFiles = CountOpenFileDescriptors();

	cController.Init(szDirectory, szRewrite);
	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	cDurableFile2.Init(&cController, "2.txt", "2.txt");
	cDurableFile3.Init(&cController, "3.txt", "3.txt");

	AssertTrue(cController.Begin());
	cDurableFile1.Write("Old One", 7, 1);
	cDurableFile2.Write("Old Two Is Longer", 17, 1);
	AssertTrue(cController.End());

	//Change the files, then do the steps of CDurableSet::End one at a time and stop at iCrashPoint as if the process died.
	AssertTrue(cController.Begin());
	cDurableFile1.Write(0, "New One Longer", 14, 1);
	AssertTrue(cDurableFile2.Truncate(0));
	cDurableFile2.Write(0, "New2", 4, 1);
	cDurableFile3.Write("Three", 5, 1);

	apcFiles[0] = &cDurableFile1;
	apcFiles[1] = &cDurableFile2;
	apcFiles[2] = &cDurableFile3;

	iStep = 0;
	if (iStep++ < iCrashPoint)
	{
		AssertTrue(cController.GetDurableSet()->MarkStart());
	}
	for (i = 0; i < 3; i++)
	{
		if (iStep++ < iCrashPoint)
		{
			AssertTrue(apcFiles[i]->Commit());
		}
	}
	if (iStep++ < iCrashPoint)
	{
		AssertTrue(cController.GetDurableSet()->MarkRewrite());
	}
	for (i = 0; i < 3; i++)
	{
		if (iStep++ < iCrashPoint)
		{
			AssertTrue(apcFiles[i]->Recommit());
		}
	}
	if (iStep++ < iCrashPoint)
	{
		//First half of CDurableSet::MarkFinish.
		AssertTrue(cFileUtil.Delete("Output" _FS_ "Durable4" _FS_ "Rewrite" _FS_ "Mark2.Rewrite"));
	}

	//Files left open by the 'crash' are reported as errors when killed.
	sLogConfig = gcLogger.SetSilent();
	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cController.Kill();
	gcLogger.SetConfig(&sLogConfig);

	//A crash releases every open file.  Killing the abandoned files must do the same or Windows cannot open them again.
	AssertInt(iOpenFiles, CountOpenFileDescriptors());

	//Recover finds the files to copy from the list in the start mark.  Files passed in as well are only copied once.
	cController.Init(szDirectory, szRewrite);
	if (bPassFiles)
	{
		cDurableFile1.Init(&cController, "1.txt", "1.txt");
		cDurableFile2.Init(&cController, "2.txt", "2.txt");
		cDurableFile3.Init(&cController, "3.txt", "3.txt");
		AssertTrue(cController.Recover(&cDurableFile1, &cDurableFile2, &cDurableFile3, NULL));
		cDurableFile1.Kill();
		cDurableFile2.Kill();
		cDurableFile3.Kill();
	}
	else
	{
		AssertTrue(cController.Recover());
	}
	cController.Kill();

	//Until the rewrite mark exists the old files must come back.  After it the new files must.
	AssertDurableFileControllerCrashState(iCrashPoint > 4);

	cFileUtil.RemoveDir("Output" _FS_ "Durable4");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerIncompleteMark(void)
{
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	CDurableFile			cDurableFile1;
	CTextFile				cMark;
	SLogConfig				sLogConfig;
	char					szDirectory[] = "Output" _FS_ "Durable5" _FS_ "Write";
	char					szRewrite[] = "Output" _FS_ "Durable5" _FS_ "Rewrite";

	cFileUtil.RemoveDir("Output" _FS_ "Durable5");

	cController.Init(szDirectory, szRewrite);
	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	AssertTrue(cController.Begin());
	cDurableFile1.Write("Kept", 4, 1);
	AssertTrue(cController.End());
	cDurableFile1.Kill();
	cController.Kill();

	//A start mark cut short by a crash, without the "#End" line.  The second path is cut off too.
	cMark.Init();
	cMark.mcText.Append("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "1.txt");
	cMark.mcText.AppendNewLine();
	cMark.mcText.Append("Output" _FS_ "Durable5" _FS_ "Rew");
	AssertTrue(cMark.Write("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "Mark1.Write"));
	cMark.Kill();

	//Only the start mark: no primary file was written yet, so nothing is copied and the files are kept.
	cController.Init(szDirectory, szRewrite);
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(cController.Recover());
	gcLogger.SetConfig(&sLogConfig);
	cController.Kill();
	AssertFileString("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "1.txt", "Kept");
	AssertFileString("Output" _FS_ "Durable5" _FS_ "Rewrite" _FS_ "1.txt", "Kept");
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "Mark1.Write"));

	//With the rewrite mark the start mark must have been complete, so an incomplete one is an error.
	cMark.Init();
	cMark.mcText.Append("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "1.txt");
	AssertTrue(cMark.Write("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "Mark1.Write"));
	cMark.Kill();
	AssertTrue(cFileUtil.Touch("Output" _FS_ "Durable5" _FS_ "Rewrite" _FS_ "Mark2.Rewrite"));

	cController.Init(szDirectory, szRewrite);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cController.Recover());
	gcLogger.SetConfig(&sLogConfig);
	cController.Kill();
	AssertFileString("Output" _FS_ "Durable5" _FS_ "Write" _FS_ "1.txt", "Kept");
	AssertFileString("Output" _FS_ "Durable5" _FS_ "Rewrite" _FS_ "1.txt", "Kept");

	cFileUtil.RemoveDir("Output" _FS_ "Durable5");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerNoSync(void)
{
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	CDurableFile			cDurableFile1;
	char					szDirectory[] = "Output" _FS_ "Durable6" _FS_ "Write";
	char					szRewrite[] = "Output" _FS_ "Durable6" _FS_ "Rewrite";

	cFileUtil.RemoveDir("Output" _FS_ "Durable6");

	cController.Init(szDirectory, szRewrite);
	AssertTrue(cController.IsSync());
	cController.SetSync(DS_No);
	AssertFalse(cController.IsSync());

	//Without syncing the files are still written to both directories and the marks are removed.
	cDurableFile1.Init(&cController, "1.txt", "1.txt");
	AssertTrue(cController.Begin());
	cDurableFile1.Write("Unsynced", 8, 1);
	AssertTrue(cController.End());
	AssertFileString("Output" _FS_ "Durable6" _FS_ "Write" _FS_ "1.txt", "Unsynced");
	AssertFileString("Output" _FS_ "Durable6" _FS_ "Rewrite" _FS_ "1.txt", "Unsynced");
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable6" _FS_ "Write" _FS_ "Mark1.Write"));
	AssertBool(false, cFileUtil.Exists("Output" _FS_ "Durable6" _FS_ "Rewrite" _FS_ "Mark2.Rewrite"));

	cController.SetSync(DS_Yes);
	AssertTrue(cController.IsSync());

	cDurableFile1.Kill();
	cController.Kill();
	cFileUtil.RemoveDir("Output" _FS_ "Durable6");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileControllerInit(void)
{
	CDurableFileController	cController;
	SLogConfig				sLogConfig;

	//Init only fails on bad arguments, and the controller can still be killed.
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cController.Init("", "Output" _FS_ "Durable7" _FS_ "Rewrite"));
	cController.Kill();
	AssertFalse(cController.Init("Output" _FS_ "Durable7" _FS_ "Same", "Output" _FS_ "Durable7" _FS_ "Same"));
	cController.Kill();
	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(cController.Init("Output" _FS_ "Durable7" _FS_ "Write", "Output" _FS_ "Durable7" _FS_ "Rewrite"));
	AssertTrue(cController.IsDurable());
	cController.Kill();

	AssertTrue(cController.Init("Output" _FS_ "Durable7" _FS_ "Write"));
	AssertFalse(cController.IsDurable());
	cController.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileController(void)
{
	TypeConverterInit();
	DataMemoryInit();
	BeginTests();

	TestDurableFileControllerDirectoryCreation();
	TestDurableFileControllerRecovery();
	TestDurableFileControllerTwoInOneDirectory();
	TestDurableFileControllerCrash(0, false);
	TestDurableFileControllerCrash(1, false);
	TestDurableFileControllerCrash(2, false);
	TestDurableFileControllerCrash(3, false);
	TestDurableFileControllerCrash(4, false);
	TestDurableFileControllerCrash(5, false);
	TestDurableFileControllerCrash(6, false);
	TestDurableFileControllerCrash(7, false);
	TestDurableFileControllerCrash(8, false);
	TestDurableFileControllerCrash(9, false);
	TestDurableFileControllerCrash(3, true);
	TestDurableFileControllerCrash(7, true);
	TestDurableFileControllerIncompleteMark();
	TestDurableFileControllerNoSync();
	TestDurableFileControllerInit();

	TestStatistics();
	DataMemoryKill();
	TypeConverterKill();
}

