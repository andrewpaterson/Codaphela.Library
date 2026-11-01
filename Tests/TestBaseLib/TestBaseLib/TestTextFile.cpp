#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


#define TEXT_FILE_TEST_DIR	"Output" _FS_ "TextFile"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextFileWriteAndRead(void)
{
	CFileUtil	cFileUtil;
	CTextFile	cTextFile;

	cFileUtil.RemoveDir(TEXT_FILE_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(TEXT_FILE_TEST_DIR, false));

	cTextFile.Init("Written text");
	AssertTrue(cTextFile.Write(TEXT_FILE_TEST_DIR _FS_ "File.txt"));
	AssertFalse(cTextFile.Write(NULL));
	cTextFile.Kill();

	cTextFile.Init();
	AssertTrue(cTextFile.Read(TEXT_FILE_TEST_DIR _FS_ "File.txt"));
	AssertString("Written text", cTextFile.Text());
	cTextFile.Kill();

	AssertTrue(cFileUtil.RemoveDir(TEXT_FILE_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextFileWriteShorter(void)
{
	CFileUtil	cFileUtil;
	CTextFile	cTextFile;

	cFileUtil.RemoveDir(TEXT_FILE_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(TEXT_FILE_TEST_DIR, false));

	cTextFile.Init("A much longer text than the next one");
	AssertTrue(cTextFile.Write(TEXT_FILE_TEST_DIR _FS_ "File.txt"));
	cTextFile.Kill();

	//Writing shorter text over an existing file must not leave the end of the old text.
	cTextFile.Init("Short");
	AssertTrue(cTextFile.Write(TEXT_FILE_TEST_DIR _FS_ "File.txt"));
	cTextFile.Kill();
	AssertLong(5, cFileUtil.Size(TEXT_FILE_TEST_DIR _FS_ "File.txt"));

	cTextFile.Init();
	AssertTrue(cTextFile.Read(TEXT_FILE_TEST_DIR _FS_ "File.txt"));
	AssertString("Short", cTextFile.Text());
	cTextFile.Kill();

	AssertTrue(cFileUtil.RemoveDir(TEXT_FILE_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextFile(void)
{
	TypeConverterInit();
	MemoryInit();
	BeginTests();

	TestTextFileWriteAndRead();
	TestTextFileWriteShorter();

	TestStatistics();
	MemoryKill();
	TypeConverterKill();
}

