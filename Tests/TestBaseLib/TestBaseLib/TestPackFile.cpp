#include "BaseLib/PackFile.h"
#include "BaseLib/PackFiles.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"
#include "TestPackFile.h"
#ifdef REBUILD_TEST_PACK_FILES
#include "BaseLib/PackFilePacker.h"
#endif // REBUILD_TEST_PACK_FILES


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFileReadWriteSingleFile(void)
{
	CMemoryFile*	pcMemoryFile;
	CPackFiles		cPackFiles;
	CPackFile*		pcPackFile;
	CFileBasic		cBasic;
	bool			bResult;
	char			c;

	pcMemoryFile = MemoryFile();
	pcMemoryFile->mbBasicFileMustFree = false;

	bResult = cPackFiles.Init(pcMemoryFile, PFM_Write);
	AssertTrue(bResult);

	pcPackFile = cPackFiles.WriteOpen("Directory/File1.txt");
	AssertNotNull(pcPackFile);
	cBasic.Init(pcPackFile);

	bResult = cBasic.WriteChar('A');
	AssertTrue(bResult);

	bResult = cBasic.Close();
	AssertTrue(bResult);

	cPackFiles.Kill();
	cBasic.Kill();

	cPackFiles.Init(pcMemoryFile, PFM_Read);

	pcPackFile = cPackFiles.ReadOpen("Directory/File1.txt");
	AssertNotNull(pcPackFile);
	cBasic.Init(pcPackFile);

	bResult = cBasic.ReadChar(&c);
	AssertTrue(bResult);
	AssertChar('A', c);

	bResult = cBasic.Close();
	AssertTrue(bResult);

	cPackFiles.Kill();
	cBasic.Kill();

	pcMemoryFile->Kill();
	free(pcMemoryFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFileReadWriteMultipleFiles(void)
{
	CMemoryFile*	pcMemoryFile;
	CPackFiles		cPackFiles;
	CPackFile*		pcPackFile;
	CPackFile*		pcPackFile1;
	CPackFile*		pcPackFile2;
	CFileBasic		cBasic;
	CFileBasic		cBasic1;
	CFileBasic		cBasic2;
	bool			bResult;
	size			iLen;
	char			sz[20];
	int32			iNumber;

	pcMemoryFile = MemoryFile();
	pcMemoryFile->mbBasicFileMustFree = false;

	bResult = cPackFiles.Init(pcMemoryFile, PFM_Write);
	AssertTrue(bResult);

	pcPackFile = cPackFiles.WriteOpen("Directory/File1.txt");
	AssertNotNull(pcPackFile);
	cBasic.Init(pcPackFile);

	bResult = cBasic.WriteString("Yeeeaahh!");
	AssertTrue(bResult);

	bResult = cBasic.WriteInt32(1032);
	AssertTrue(bResult);

	bResult = cBasic.WriteInt32(-8);
	AssertTrue(bResult);

	bResult = cBasic.Close();
	AssertTrue(bResult);
	cBasic.Kill();

	pcPackFile = cPackFiles.WriteOpen("Directory/Sub/File2.txt");
	AssertNotNull(pcPackFile);
	cBasic.Init(pcPackFile);

	bResult = cBasic.WriteString("Aaaahh");
	AssertTrue(bResult);

	bResult = cBasic.WriteInt32(743);
	AssertTrue(bResult);

	bResult = cBasic.WriteInt32(380);
	AssertTrue(bResult);

	bResult = cBasic.Close();
	AssertTrue(bResult);
	cBasic.Kill();

	cPackFiles.Kill();

	cPackFiles.Init(pcMemoryFile, PFM_Read);

	pcPackFile1 = cPackFiles.ReadOpen("Directory/File2.txt");
	AssertNull(pcPackFile1);

	pcPackFile1 = cPackFiles.ReadOpen("Directory/File1.txt");
	AssertNotNull(pcPackFile1);
	cBasic1.Init(pcPackFile1);

	bResult = cBasic1.ReadStringLength(&iLen);
	AssertTrue(bResult);
	AssertSize(10, iLen);

	bResult = cBasic1.ReadStringChars(sz, iLen);
	AssertTrue(bResult);
	AssertString("Yeeeaahh!", sz);

	pcPackFile2 = cPackFiles.ReadOpen("Directory/Sub/File2.txt");
	AssertNotNull(pcPackFile2);
	cBasic2.Init(pcPackFile2);

	bResult = cBasic2.ReadStringLength(&iLen);
	AssertTrue(bResult);
	AssertSize(7, iLen);

	bResult = cBasic2.ReadStringChars(sz, iLen);
	AssertTrue(bResult);
	AssertString("Aaaahh", sz);

	bResult = cBasic1.ReadInt32(&iNumber);
	AssertTrue(bResult);
	AssertInt((int32)1032, iNumber);

	bResult = cBasic2.ReadInt32(&iNumber);
	AssertTrue(bResult);
	AssertInt((int32)743, iNumber);

	cBasic2.Close();

	bResult = cBasic1.ReadInt32(&iNumber);
	AssertTrue(bResult);
	AssertInt((int32)-8, iNumber);

	bResult = cBasic2.ReadInt32(&iNumber);
	AssertFalse(bResult);

	cBasic1.Close();
	cBasic1.Kill();
	cBasic2.Kill();
	cPackFiles.Kill();

	pcMemoryFile->Kill();
	free(pcMemoryFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFileAdd(void)
{
	CFileUtil				cFileUtil;
	CChars					szDir;
	CDiskFile*				pcDiskFile;
	bool					bResult;
	CFileNodePackFileNode*	pcNode;
	CPackFiles				cPak;
	CPackFileNode*			pcFile;

	szDir.Init("PackFile" FILE_SEPARATOR "Test.PAK");

	cFileUtil.RemoveDir("PackFile");
	cFileUtil.MakeDir("PackFile");

	pcDiskFile = DiskFile(szDir.Text());

	bResult = cPak.Init(pcDiskFile, PFM_Write);
	AssertTrue(bResult);

	bResult = cPak.AddDirectory("Finder");
	AssertTrue(bResult);

	pcNode = cPak.GetNode("Ants.rar");
	AssertTrue(pcNode->IsFile());
	pcFile = pcNode->File();
	AssertLong(72, pcFile->Size());
	AssertLong(1009, pcFile->FilePos());

	pcNode = cPak.GetNode("Ants.txt");
	AssertTrue(pcNode->IsFile());
	pcFile = pcNode->File();
	AssertLong(0, pcFile->Size());
	AssertLong(0, pcFile->FilePos());

	cPak.Kill();

	cFileUtil.RemoveDir("PackFile");
	cFileUtil.MakeDir("PackFile");

	pcDiskFile = DiskFile(szDir.Text());
	cPak.Init(pcDiskFile, PFM_Write);

	bResult = cPak.AddDirectory("InFile/Giana", "Giana");
	AssertTrue(bResult);

	pcNode = cPak.GetNode("Giana/Brothers.PAK");
	AssertNotNull(pcNode);
	AssertTrue(pcNode->IsFile());

	pcNode = cPak.GetNode("Giana/Sisters.PAK");
	AssertNotNull(pcNode);
	AssertTrue(pcNode->IsFile());

	pcNode = cPak.GetNode("Giana");
	AssertNotNull(pcNode);
	AssertTrue(pcNode->IsDirectory());

	pcNode = cPak.GetNode("Giana/Sisters");
	AssertNotNull(pcNode);
	AssertTrue(pcNode->IsDirectory());

	cPak.Kill();
	szDir.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFileIterate(void)
{
	CPackFiles			cPak;
	CDiskFile*			pcDiskFile;
	CPackFileIterator	cIter;
	CChars				sz;

	pcDiskFile = DiskFile("Input/Test.PAK");
	AssertNotNull(pcDiskFile);
	cPak.Init(pcDiskFile, PFM_Read);

	sz.Init(); AssertStringCase("Giana/Sisters/Document.txt", cPak.StartIteration(&cIter)->GetFullName(&sz), false);  sz.Kill();
	sz.Init(); AssertStringCase("Giana/Brothers.PAK", cPak.Iterate(&cIter)->GetFullName(&sz), false);  sz.Kill();
	sz.Init(); AssertStringCase("Giana/Sisters.PAK", cPak.Iterate(&cIter)->GetFullName(&sz), false);  sz.Kill();
	AssertNull(cPak.Iterate(&cIter));
	cPak.StopIteration(&cIter);

	cPak.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFileGetFiles(void)
{
#ifdef REBUILD_TEST_PACK_FILES
	PackDirectory("Input/Test.PAK", "InFile/Giana", "Giana");
#endif // REBUILD_TEST_PACK_FILES

	CPackFiles				cPak;
	CDiskFile*				pcDiskFile;
	CChars					sz;
	CArrayPackFileNodePtrs	cArrayFiles;

	pcDiskFile = DiskFile("Input/Test.PAK");
	AssertNotNull(pcDiskFile);
	cPak.Init(pcDiskFile, PFM_Read);
	cArrayFiles.Init();

	cPak.GetFiles(&cArrayFiles);

	AssertSize(3, cArrayFiles.NumElements());

	sz.Init(); AssertStringCase("Giana/Sisters/Document.txt", (*cArrayFiles.Get(0))->GetFullName(&sz), false);  sz.Kill();
	sz.Init(); AssertStringCase("Giana/Brothers.PAK", (*cArrayFiles.Get(1))->GetFullName(&sz), false);  sz.Kill();
	sz.Init(); AssertStringCase("Giana/Sisters.PAK", (*cArrayFiles.Get(2))->GetFullName(&sz), false);  sz.Kill();

	cArrayFiles.Kill();
	cPak.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFile(void)
{
	BeginTests();
	MemoryInit();

	TestPackFileReadWriteSingleFile();
	TestPackFileReadWriteMultipleFiles();

	TestPackFileAdd();
	TestPackFileGetFiles();
	TestPackFileIterate();

	MemoryKill();
	TestStatistics();
}

