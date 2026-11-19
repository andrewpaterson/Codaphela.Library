#include <stdlib.h>
#include <string.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/IndexedFile.h"
#include "TestLib/Assert.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFileWrite(void)
{
	CIndexedFile				cIndexedFile;
	CDurableFileController		cController;
	CFileUtil					cFileUtil;
	CChars						szWrite;
	CChars						szRewrite;
	bool						bResult;
	char						sz1[] = "12345678";
	char						sz2[] = "ABCDEFGH";
	char						sz3[] = "iopfghjk";
	filePos						iFileIndex;
	CTextFile					cTextFile;
	char						szTemp[16];
	char						szDirectory[] = "Output" _FS_ "IndexedFile";
	char						szRewriteDirectory[] = "Output" _FS_ "_IndexedFile";

	bResult = cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);
	AssertTrue(bResult);

	szWrite.Init("Output" _FS_ "IndexedFile" _FS_ "File.DAT");
	szRewrite.Init("Output" _FS_ "_IndexedFile" _FS_ "_File.DAT");

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cController.Begin();

	cIndexedFile.Init(&cController, 0, "File.DAT", "_File.DAT", 9, 0);

	iFileIndex = cIndexedFile.Write(sz3);
	AssertLong(0, iFileIndex);
	AssertLong(9, cIndexedFile.GetFileSize());

	iFileIndex = cIndexedFile.Write(sz2);
	AssertLong(1, iFileIndex);
	AssertLong(18, cIndexedFile.GetFileSize());

	bResult = cIndexedFile.Write(0, sz1);
	AssertTrue(bResult);
	AssertLong(18, cIndexedFile.GetFileSize());

	bResult = cController.End();
	AssertTrue(bResult);
	cIndexedFile.Kill();

	cController.Kill();

	cTextFile.Init();
	cTextFile.Read(szWrite.Text());
	AssertString(sz1, cTextFile.mcText.Text(0));
	AssertString(sz2, cTextFile.mcText.Text(9));
	cTextFile.Kill();

	cTextFile.Init();
	cTextFile.Read(szRewrite.Text());
	AssertString(sz1, cTextFile.mcText.Text(0));
	AssertString(sz2, cTextFile.mcText.Text(9));
	cTextFile.Kill();


	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cController.Begin();

	cIndexedFile.Init(&cController, 0, "File.DAT", "_File.DAT", 9, 0);

	bResult = cIndexedFile.Read(1, szTemp);
	AssertTrue(bResult);
	AssertString(sz2, szTemp);

	bResult = cIndexedFile.Read(0, szTemp);
	AssertTrue(bResult);
	AssertString(sz1, szTemp);

	bResult = cIndexedFile.Read(1, szTemp);
	AssertTrue(bResult);
	AssertString(sz2, szTemp);
	AssertInt(18, (int)cIndexedFile.GetFileSize());

	bResult = cController.End();
	AssertTrue(bResult);
	cIndexedFile.Kill();

	cController.Kill();

	szWrite.Kill();
	szRewrite.Kill();

	bResult = cFileUtil.RemoveDirs(szDirectory, szRewriteDirectory, NULL);
	AssertTrue(bResult);
}


//////////////////////////////////////////////////////////////////////////
//
// More datas than a size counts, written, read and deleted (to past the
// end) with one call each; Delete blanks them a 2KB buffer at a time.
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFileManyDatas(void)
{
	CIndexedFile				cIndexedFile;
	CDurableFileController		cController;
	CFileUtil					cFileUtil;
	bool						bResult;
	uint8*						puiData;
	lsize						uiCount;
	lsize						i;
	char						szDirectory[] = "Output" _FS_ "IndexedFileMany";
	char						szRewriteDirectory[] = "Output" _FS_ "_IndexedFileMany";

	bResult = cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);
	AssertTrue(bResult);

	uiCount = 70000;
	puiData = (uint8*)malloc(uiCount);
	for (i = 0; i < uiCount; i++)
	{
		puiData[i] = (uint8)(i % 251);
	}

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexedFile.Init(&cController, 0, "File.DAT", "_File.DAT", 1, 0);

	AssertTrue(cIndexedFile.Write(0, puiData, uiCount));
	AssertLong(70000, cIndexedFile.NumDatas());

	memset(puiData, 0, uiCount);
	AssertTrue(cIndexedFile.Read(0, puiData, uiCount));
	AssertInt((int)(69999 % 251), puiData[69999]);
	AssertInt((int)(65536 % 251), puiData[65536]);

	AssertTrue(cIndexedFile.Delete(10, 80000));
	memset(puiData, 0, uiCount);
	AssertTrue(cIndexedFile.Read(0, puiData, uiCount));
	AssertInt(9, puiData[9]);
	AssertInt(INDEX_FILE_EMPTY_CHAR, puiData[10]);
	AssertInt(INDEX_FILE_EMPTY_CHAR, puiData[2058]);
	AssertInt(INDEX_FILE_EMPTY_CHAR, puiData[65536]);
	AssertInt(INDEX_FILE_EMPTY_CHAR, puiData[69999]);
	AssertLong(70000, cIndexedFile.NumDatas());

	AssertTrue(cController.End());
	cIndexedFile.Kill();
	cController.Kill();
	free(puiData);

	bResult = cFileUtil.RemoveDirs(szDirectory, szRewriteDirectory, NULL);
	AssertTrue(bResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFile(void)
{
	BeginTests();
	DataMemoryInit();

	TestIndexedFileWrite();
	TestIndexedFileManyDatas();

	DataMemoryKill();
	TestStatistics();
}

