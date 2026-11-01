#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBufferedFileRead(void)
{
	CFileUtil	cFileUtil;
	CFileBasic	cFile;
	CTextFile	cText;
	char		sz[20];
	char		c;
	int			iCount;
	int			i;
	char		szExpected[20];

	cFileUtil.TouchDir("Output");
	AssertTrue(cFileUtil.Delete("Output" _FS_ "Test.txt"));

	cText.Init();
	cText.mcText.Append("abcdefghijk");
	cText.Write("Output" _FS_ "Test.txt");
	cText.Kill();

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 3));

	cFile.Open(EFM_Read);

	for (c = 'a'; c <= 'k'; c++)
	{
		memset(sz, 0, 20);
		iCount = (int)cFile.Read(sz, 1, 1);
		AssertChar(c, sz[0]);
		AssertChar(0, sz[1]);
		AssertInt(1, iCount);
	}

	cFile.Seek(0);

	szExpected[2] = 0;
	for (i = 0; i < 5; i++)
	{
		memset(sz, 0, 20);
		iCount = (int)cFile.Read(sz, 1, 2);
		szExpected[0] = 'a' + (char)i * 2;
		szExpected[1] = 'b' + (char)i * 2;
		AssertString(szExpected, sz);
		AssertInt(2, iCount);
	}
	memset(sz, 0, 20);
	iCount = (int)cFile.Read(sz, 1, 2);
	AssertString("k", sz);
	AssertInt(1, iCount);

	cFile.Seek(0);

	szExpected[3] = 0;
	for (i = 0; i < 3; i++)
	{
		memset(sz, 0, 20);
		iCount = (int)cFile.Read(sz, 1, 3);
		szExpected[0] = 'a' + (char)i * 3;
		szExpected[1] = 'b' + (char)i * 3;
		szExpected[2] = 'c' + (char)i * 3;
		AssertString(szExpected, sz);
		AssertInt(3, iCount);
	}
	memset(sz, 0, 20);
	iCount = (int)cFile.Read(sz, 1, 3);
	AssertString("jk", sz);
	AssertInt(2, iCount);

	cFile.Close();
	cFile.Kill();

	AssertTrue(cFileUtil.Delete("Output" _FS_ "Test.txt"));
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBufferedFileWrite(void)
{
	CFileUtil	cFileUtil;
	CFileBasic	cFile;
	CTextFile	cText;

	cFileUtil.TouchDir("Output");
	AssertTrue(cFileUtil.Delete("Output" _FS_ "Test.txt"));

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 3));
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertLong(2, cFile.Write("ab", 1, 2));
	AssertLong(2, cFile.Write("cd", 1, 2));
	AssertTrue(cFile.Close());
	cFile.Kill();

	cText.Init();
	AssertTrue(cText.Read("Output" _FS_ "Test.txt"));
	AssertString("abcd", cText.mcText.Text());
	cText.Kill();

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 5));
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertLong(9, cFile.Write("abcdefghi", 1, 9));
	AssertLong(5, cFile.Write("jklmn", 1, 5));
	AssertTrue(cFile.Close());
	cFile.Kill();

	cText.Init();
	AssertTrue(cText.Read("Output" _FS_ "Test.txt"));
	AssertString("abcdefghijklmn", cText.mcText.Text());
	cText.Kill();

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 5));
	AssertTrue(cFile.Open(EFM_ReadWrite));
	AssertTrue(cFile.Seek(4));
	AssertLong(1, cFile.Write("xyz", 3, 1));
	AssertTrue(cFile.Close());
	cFile.Kill();

	cText.Init();
	AssertTrue(cText.Read("Output" _FS_ "Test.txt"));
	AssertString("abcdxyzhijklmn", cText.mcText.Text());
	cText.Kill();

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 8));
	AssertTrue(cFile.Open(EFM_ReadWrite));
	AssertTrue(cFile.Seek(0, EFSO_END));
	AssertLong(1, cFile.Write("opqrst", 6, 1));
	AssertTrue(cFile.Seek(0));
	AssertLong(1, cFile.Write("123456", 6, 1));
	AssertLong(1, cFile.Write("78", 2, 1));
	AssertTrue(cFile.Close());
	cFile.Kill();

	cText.Init();
	AssertTrue(cText.Read("Output" _FS_ "Test.txt"));
	AssertString("12345678ijklmnopqrst", cText.mcText.Text());
	cText.Kill();

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 3));
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertLong(2, cFile.Write("ab", 1, 2));
	AssertLong(4, cFile.Write("cdef", 1, 4));
	AssertLong(2, cFile.Write("gh", 1, 2));
	AssertLong(20, cFile.GetFileLength());
	AssertTrue(cFile.Close());
	cFile.Kill();

	cText.Init();
	AssertTrue(cText.Read("Output" _FS_ "Test.txt"));
	AssertString("abcdefghijklmnopqrst", cText.mcText.Text());
	cText.Kill();

	AssertTrue(cFileUtil.Delete("Output" _FS_ "Test.txt"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBufferedFileReadWrite(void)
{
	CFileUtil	cFileUtil;
	CFileBasic	cFile;
	CTextFile	cText;
	char		sz[20];

	cFileUtil.TouchDir("Output");
	AssertTrue(cFileUtil.Delete("Output" _FS_ "Test.txt"));

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "Test.txt"), 3));
	cFile.Open(EFM_ReadWrite_Create);
	cFile.Write("abcdefghijklmn", 1, 14);
	cFile.Write("op", 1, 2);
	cFile.Seek(3);
	memset(sz, 0, 20);
	cFile.Read(sz, 2, 1);
	AssertString("de", sz);
	cFile.Read(sz, 2, 1);
	AssertString("fg", sz);
	cFile.Write("12", 2, 1);
	cFile.Read(sz, 2, 1);
	AssertString("jk", sz);
	cFile.Write("34", 2, 1);
	cFile.Read(sz, 2, 1);
	AssertString("no", sz);
	cFile.Seek(2);
	cFile.Write("XY", 2, 1);
	cFile.Read(sz, 1, 3);
	AssertString("efg", sz);
	cFile.Close();
	cFile.Kill();

	cText.Init();
	cText.Read("Output" _FS_ "Test.txt");
	AssertString("abXYefg12jk34nop", cText.mcText.Text());
	cText.Kill();

	AssertTrue(cFileUtil.Delete("Output" _FS_ "Test.txt"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBufferedFile(void)
{
	BeginTests();

	TestBufferedFileRead();
	TestBufferedFileWrite();
	TestBufferedFileReadWrite();

	TestStatistics();
}

