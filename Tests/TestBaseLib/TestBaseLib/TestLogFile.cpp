#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileOpen(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	SLogConfig		sLogConfig;

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	AssertNotNull(pcLogFile);

	cFile.Init(pcLogFile);

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cFile.Open(EFM_ReadWrite_Create));
	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(pcLogFile->Begin());

	AssertTrue(cFile.Open(EFM_ReadWrite_Create));

	AssertTrue(cFile.Close());

	pcLogFile->End();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileRead(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	bool			bResult;
	size			iLength;
	char			sz[200];
	filePos			iRead;
	SLogConfig		sLogConfig;

	pcMemoryFile = MemoryFile();
	cFile.Init(pcMemoryFile);
	cFile.Open(EFM_Write_Create);
	cFile.WriteString("The suspense is killing me!");
	cFile.Close();

	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	sLogConfig = gcLogger.SetSilent();;
	bResult = cFile.Open(EFM_Read);
	AssertFalse(bResult);
	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(pcLogFile->Begin());
	AssertTrue(cFile.Open(EFM_Read));

	bResult = cFile.ReadStringLength(&iLength);
	AssertTrue(bResult);
	AssertSize(28, iLength);
	bResult = cFile.ReadStringChars(sz, iLength);
	AssertString("The suspense is killing me!", sz);

	memset(sz, 0, 200);
	bResult = cFile.Seek(20);
	AssertTrue(bResult);
	bResult = cFile.ReadStringChars(sz, 8);
	AssertString("killing ", sz);
	AssertLong(28, cFile.GetFilePos());

	memset(sz, 0, 200);
	bResult = cFile.ReadStringChars(sz, 4);
	AssertString("me!", sz);
	AssertLong(32, cFile.GetFilePos());

	iRead = cFile.Read(sz, 1, 1);
	AssertLong(0, iRead);
	AssertLong(32, cFile.GetFilePos());

	bResult = cFile.Close();
	AssertTrue(bResult);

	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileWriteMemory(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	bool			bResult;
	size			iLength;
	char			sz[200];

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	pcLogFile->Begin();
	bResult = cFile.Open(EFM_ReadWrite_Create);
	AssertTrue(bResult);

	bResult = cFile.WriteString("The suspense is killing me!");
	AssertTrue(bResult);
	AssertNull((char*)pcMemoryFile->GetBufferPointer());
	
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(32, pcLogFile->GetWriteSize(0));

	bResult = cFile.Seek(8);
	AssertTrue(bResult);

	bResult = cFile.WriteData("camisole", 8);
	AssertTrue(bResult);
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(32, pcLogFile->GetWriteSize(0));
	
	AssertNull((char*)pcMemoryFile->GetBufferPointer());
	cFile.Seek(0);
	bResult = cFile.ReadStringLength(&iLength);
	AssertTrue(bResult);
	AssertSize(28, iLength);
	bResult = cFile.ReadStringChars(sz, iLength);
	AssertString("The camisole is killing me!", sz);

	bResult = pcLogFile->Commit();
	AssertTrue(bResult);
	AssertString("The camisole is killing me!", (char*)RemapSinglePointer(pcMemoryFile->GetBufferPointer(), sizeof(int32)));

	bResult = cFile.Close();  //This should go before Commit
	AssertTrue(bResult);

	pcLogFile->Commit();
	pcLogFile->End();
	pcLogFile->Begin();
	
	bResult = cFile.Open(EFM_ReadWrite_Create);
	AssertTrue(bResult);

	bResult = cFile.Seek(4);
	AssertTrue(bResult);
	bResult = cFile.WriteData("Dog", 3);

	bResult = cFile.Seek(20);
	AssertTrue(bResult);
	bResult = cFile.WriteData("plurgle", 7);

	AssertSize(2, pcLogFile->GetNumWrites());
	AssertLong(3, pcLogFile->GetWriteSize(0));
	AssertLong(7, pcLogFile->GetWriteSize(1));

	AssertString("The camisole is killing me!", (char*)RemapSinglePointer(pcMemoryFile->GetBufferPointer(), sizeof(int32)));
	cFile.Seek(0);
	bResult = cFile.ReadStringLength(&iLength);
	AssertTrue(bResult);
	AssertSize(28, iLength);
	bResult = cFile.ReadStringChars(sz, iLength);
	AssertTrue(bResult);
	AssertString("Dog camisole is plurgle me!", sz);

	bResult = pcLogFile->Commit();
	AssertTrue(bResult);
	AssertString("Dog camisole is plurgle me!", (char*)RemapSinglePointer(pcMemoryFile->GetBufferPointer(), sizeof(int32)));

	bResult = cFile.Close();
	AssertTrue(bResult);

	pcLogFile->Commit();
	pcLogFile->End();
	pcLogFile->Begin();

	bResult = cFile.Open(EFM_ReadWrite_Create);
	AssertTrue(bResult);

	cFile.Seek(4);
	cFile.WriteData("X", 1);
	cFile.Seek(6);
	bResult = cFile.WriteData("Z", 1);
	cFile.Seek(28);
	cFile.WriteData("A", 1);
	cFile.Seek(30);
	bResult = cFile.WriteData("C", 1);
	AssertSize(4, pcLogFile->GetNumWrites());

	cFile.Seek(5);
	cFile.WriteData("Y", 1);
	AssertSize(3, pcLogFile->GetNumWrites());
	
	cFile.Seek(29);
	cFile.WriteData("B", 1);
	AssertSize(2, pcLogFile->GetNumWrites());

	cFile.Seek(0);
	bResult = cFile.ReadStringLength(&iLength);
	AssertTrue(bResult);
	bResult = cFile.ReadStringChars(sz, iLength);
	AssertString("XYZ camisole is plurgle ABC", sz);

	bResult = pcLogFile->Commit();
	AssertTrue(bResult);
	AssertString("XYZ camisole is plurgle ABC", (char*)RemapSinglePointer(pcMemoryFile->GetBufferPointer(), sizeof(int32)));

	bResult = cFile.Close();
	AssertTrue(bResult);

	pcLogFile->Commit();
	pcLogFile->End();

	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertLogFileContents(CFileBasic* pcFile, const char* szExpected, size uiLength)
{
	char	sz[64];

	AssertTrue(uiLength < 64);
	AssertTrue(pcFile->Seek(0));
	AssertSize(1, pcFile->Read(sz, uiLength, 1));
	sz[uiLength] = '\0';
	AssertString(szExpected, sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileExtendLastWrite(void)
{
	CLogFile*				pcLogFile;
	CMemoryFile*			pcMemoryFile;
	CFileBasic				cFile;
	CLogFileCommandWrite*	psWrite;
	CLogFileCommandWrite*	psMoved;

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	pcLogFile->Begin();
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));

	//Appending just after the last write grows it.
	AssertTrue(cFile.WriteData("abcd", 4));
	AssertTrue(cFile.WriteData("efgh", 4));
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(8, pcLogFile->GetWriteSize(0));
	AssertLogFileContents(&cFile, "abcdefgh", 8);

	//The first growth doubled the allocation so the next append fits without moving.
	psWrite = pcLogFile->GetWriteData(0);
	AssertTrue(cFile.Seek(8));
	AssertTrue(cFile.WriteData("ij", 2));
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(10, pcLogFile->GetWriteSize(0));
	AssertPointer(psWrite, pcLogFile->GetWriteData(0));
	AssertLogFileContents(&cFile, "abcdefghij", 10);

	//Writing inside the last write overwrites it in place.
	AssertTrue(cFile.Seek(2));
	AssertTrue(cFile.WriteData("XY", 2));
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(10, pcLogFile->GetWriteSize(0));
	AssertPointer(psWrite, pcLogFile->GetWriteData(0));
	AssertLogFileContents(&cFile, "abXYefghij", 10);

	//Writing from inside the last write to past its end grows it.
	AssertTrue(cFile.Seek(8));
	AssertTrue(cFile.WriteData("IJKL", 4));
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(12, pcLogFile->GetWriteSize(0));
	AssertLogFileContents(&cFile, "abXYefghIJKL", 12);

	//Writing before the start of the last write is amalgamated.
	psWrite = pcLogFile->GetWriteData(0);
	AssertTrue(cFile.Seek(20));
	AssertTrue(cFile.WriteData("uvw", 3));
	AssertSize(2, pcLogFile->GetNumWrites());
	AssertTrue(cFile.Seek(18));
	AssertTrue(cFile.WriteData("st", 2));
	AssertSize(2, pcLogFile->GetNumWrites());
	AssertPointer(psWrite, pcLogFile->GetWriteData(0));
	AssertLong(18, pcLogFile->GetWriteData(1)->iPosition);
	AssertSize(5, pcLogFile->GetWriteSize(1));

	//Appending to a write that is not the last command is amalgamated and moves that write to the end.
	AssertTrue(cFile.Seek(12));
	AssertTrue(cFile.WriteData("mn", 2));
	AssertSize(2, pcLogFile->GetNumWrites());
	AssertLong(18, pcLogFile->GetWriteData(0)->iPosition);
	AssertLong(0, pcLogFile->GetWriteData(1)->iPosition);
	AssertSize(14, pcLogFile->GetWriteSize(1));

	//Bridging the two writes amalgamates all three into one.
	AssertTrue(cFile.Seek(14));
	AssertTrue(cFile.WriteData("opqr", 4));
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(23, pcLogFile->GetWriteSize(0));
	AssertLogFileContents(&cFile, "abXYefghIJKLmnopqrstuvw", 23);

	AssertTrue(pcLogFile->Commit());
	AssertTrue(cFile.Close());
	AssertTrue(pcLogFile->Commit());
	pcLogFile->End();
	AssertLong(23, pcMemoryFile->Size());
	AssertMemory("abXYefghIJKLmnopqrstuvw", pcMemoryFile->GetBufferPointer(), 23);

	//A write after another command (here an open) is not the last command, so it is not grown.
	pcLogFile->Begin();
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));
	AssertTrue(cFile.WriteData("ABCD", 4));
	AssertTrue(cFile.Close());
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));
	psWrite = pcLogFile->GetWriteData(0);
	AssertTrue(cFile.Seek(4));
	AssertTrue(cFile.WriteData("EF", 2));
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertSize(6, pcLogFile->GetWriteSize(0));
	psMoved = pcLogFile->GetWriteData(0);
	AssertFalse(psWrite == psMoved);
	AssertTrue(pcLogFile->Commit());
	AssertTrue(cFile.Close());
	AssertTrue(pcLogFile->Commit());
	pcLogFile->End();
	AssertLong(23, pcMemoryFile->Size());
	AssertMemory("ABCDEFghIJKLmnopqrstuvw", pcMemoryFile->GetBufferPointer(), 23);

	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileExtendLastWriteLarge(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	size			i;
	size			j;
	uint8			auiChunk[1000];
	uint8*			puiData;
	bool			bMatch;

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	pcLogFile->Begin();
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));

	//Grow one write from a free list sized allocation to a large one, well past the largest free list element.
	for (i = 0; i < 300; i++)
	{
		for (j = 0; j < 1000; j++)
		{
			auiChunk[j] = (uint8)(i + j * 7);
		}
		AssertSize(1, cFile.Write(auiChunk, 1000, 1));
		AssertSize(1, pcLogFile->GetNumWrites());
		AssertLSize((lsize)(i + 1) * 1000, pcLogFile->GetWriteSize(0));
	}

	AssertTrue(pcLogFile->Commit());
	AssertTrue(cFile.Close());
	AssertTrue(pcLogFile->Commit());
	pcLogFile->End();

	AssertLong(300000, pcMemoryFile->Size());
	puiData = (uint8*)pcMemoryFile->GetBufferPointer();
	bMatch = true;
	for (i = 0; i < 300; i++)
	{
		for (j = 0; j < 1000; j++)
		{
			if (puiData[(lsize)i * 1000 + j] != (uint8)(i + j * 7))
			{
				bMatch = false;
			}
		}
	}
	AssertTrue(bMatch);

	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileWriteDiskSingle(void)
{
	CLogFile*		pcLogFile;
	CDiskFile*		pcDiskFile;
	CFileBasic		cFile;
	bool			bResult;
	CFileUtil		cFileUtil;
	char			szDirectory[] = "Output" _FS_ "LogFile4";
	char			szFileName[] = "Output" _FS_ "LogFile4" _FS_ "File.txt";
	CTextFile		cTextFile;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	pcDiskFile = DiskFile(szFileName);

	pcLogFile = LogFile(pcDiskFile);
	cFile.Init(pcLogFile);

	AssertTrue(pcLogFile->Begin());
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));

	bResult = cFile.WriteString("The suspense is killing me!");
	AssertTrue(bResult);

	bResult = cFile.Close();  
	AssertTrue(bResult);
	AssertSize(3, pcLogFile->GetNumCommands());

	AssertTrue(pcLogFile->Commit());
	AssertTrue(cFileUtil.Exists(szFileName));

	AssertTrue(pcLogFile->End());

	cTextFile.Init();
	cTextFile.Read(DiskFile(szFileName));
	AssertString("The suspense is killing me!", cTextFile.Text(sizeof(int32)));
	cTextFile.Kill();

	cFile.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileWriteDisk(void)
{
	CLogFile*		pcLogFile;
	CDiskFile*		pcDiskFile;
	CFileBasic		cFile;
	bool			bResult;
	size			iLength;
	char			sz[200];
	CFileUtil		cFileUtil;
	char			szDirectory[] = "Output" _FS_ "LogFile3";
	char			szFileName[] = "Output" _FS_ "LogFile3" _FS_ "File.txt";
	CTextFile		cTextFile;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	pcDiskFile = DiskFile(szFileName);

	pcLogFile = LogFile(pcDiskFile);
	cFile.Init(pcLogFile);

	AssertTrue(pcLogFile->Begin());
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));

	bResult = cFile.WriteString("The suspense is killing me!");
	AssertTrue(bResult);
	AssertFalse(cFileUtil.Exists(szFileName));

	AssertSize(1, pcLogFile->GetNumWrites());
	AssertLong(32, pcLogFile->GetWriteSize(0));

	bResult = cFile.Seek(8);
	AssertTrue(bResult);

	bResult = cFile.WriteData("camisole", 8);
	AssertTrue(bResult);
	AssertSize(1, pcLogFile->GetNumWrites());
	AssertLong(32, pcLogFile->GetWriteSize(0));

	AssertFalse(cFileUtil.Exists(szFileName));
	cFile.Seek(0);
	bResult = cFile.ReadStringLength(&iLength);
	AssertTrue(bResult);
	AssertSize(28, iLength);
	bResult = cFile.ReadStringChars(sz, iLength);
	AssertString("The camisole is killing me!", sz);

	bResult = cFile.Close();  //This should go before Commit
	AssertTrue(bResult);
	AssertSize(3, pcLogFile->GetNumCommands());

	AssertTrue(pcLogFile->Commit());
	AssertTrue(cFileUtil.Exists(szFileName));
	AssertTrue(pcLogFile->End());

	cTextFile.Init();
	cTextFile.Read(DiskFile(szFileName));
	AssertString("The camisole is killing me!", cTextFile.Text(sizeof(int32)));
	cTextFile.Kill();

	cFile.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileFindHoles(void)
{
	CLogFile*				pcLogFile;
	CFileBasic				cFile;
	bool					bResult;
	int32					iInt1;
	int32					iInt2;
	int32					iInt3;
	CArrayIntAndPointer		apvOverlapping;
	size					i;
	CMemoryFile*			pcMemoryFile;

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	pcLogFile->Begin();

	bResult = cFile.Open(EFM_ReadWrite_Create);
	AssertTrue(bResult);
	
	iInt1 = 872349342;
	iInt2 = 763421934;
	iInt3 = 158723346;

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 1);
	AssertTrue(bResult);
	AssertSize(0, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	cFile.Write(&iInt1, 4, 1);
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 4);
	AssertFalse(bResult);
	apvOverlapping.Kill();

	apvOverlapping.Init();
	cFile.Write(&iInt2, 4, 1);
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 8);
	AssertFalse(bResult);
	apvOverlapping.Kill();

	for (i = 0; i < 8; i++)
	{
		apvOverlapping.Init();
		bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, i);
		AssertFalse(bResult);
		apvOverlapping.Kill();
	}
	for (i = 0; i < 8; i++)
	{
		apvOverlapping.Init();
		bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, i, 8 -i);
		AssertFalse(bResult);
		apvOverlapping.Kill();
	}
	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 7, 2);
	AssertTrue(bResult);
	AssertSize(1, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	cFile.Seek(4, EFSO_CURRENT);
	cFile.Write(&iInt3, 4, 1);
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 16);
	AssertTrue(bResult);
	AssertSize(2, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 12, 4);
	AssertFalse(bResult);
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 11, 5);
	AssertTrue(bResult);
	AssertSize(1, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 8);
	AssertFalse(bResult);
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 9);
	AssertTrue(bResult);
	AssertSize(1, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 7, 2);
	AssertTrue(bResult);
	AssertSize(1, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 8, 1);
	AssertTrue(bResult);
	AssertSize(0, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 8, 4);
	AssertTrue(bResult);
	AssertSize(0, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 8, 5);
	AssertTrue(bResult);
	AssertSize(1, apvOverlapping.NumElements());
	apvOverlapping.Kill();

	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 0, 0);
	AssertFalse(bResult);
	apvOverlapping.Kill();
	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 7, 0);
	AssertFalse(bResult);
	apvOverlapping.Kill();
	apvOverlapping.Init();
	bResult = pcLogFile->TestFindHoles(1, &apvOverlapping, 8, 0);
	AssertFalse(bResult);
	apvOverlapping.Kill();

	cFile.Close();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileCommandsSimple(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	bool			bResult;
	int32			iInt;
	size			iWritten;
	int32			iResult;
	size			iRead;

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	pcLogFile->Begin();

	bResult = cFile.Open(EFM_ReadWrite_Create);
	AssertTrue(bResult);
	AssertTrue(cFile.IsOpen());

	iInt = 872349342;
	iWritten = cFile.Write(&iInt, sizeof(int32), 1);
	AssertSize(1, iWritten);
	AssertSize(0, pcMemoryFile->GetBufferSize());

	bResult = cFile.Close();
	AssertTrue(bResult);
	AssertSize(3, pcLogFile->GetNumCommands());

	bResult = cFile.Open(EFM_ReadWrite_Create);
	AssertTrue(bResult);
	AssertTrue(cFile.IsOpen());
	AssertLong(sizeof(int32), cFile.GetFileLength());
	iRead = cFile.Read(&iResult, sizeof(int32), 1);
	AssertSize(1, iRead);
	AssertSize(iInt, iResult);
	bResult = cFile.Close();
	AssertTrue(bResult);
	AssertSize(5, pcLogFile->GetNumCommands());

	bResult = pcLogFile->Commit();
	AssertTrue(bResult);
	AssertFalse(pcMemoryFile->IsOpen());
	AssertSize(sizeof(int32), pcMemoryFile->GetBufferSize());
	AssertInt(iInt, *((int32*)pcMemoryFile->GetBufferPointer()));

	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileCommandsComplex(void)
{
	CLogFile* pcLogFile;
	CMemoryFile* pcMemoryFile;
	CFileBasic		cFile;
	bool			bResult;
	size			iWritten;
	char			szABC[] = { "ABCDEFGHIJK" };
	char			sz123[] = { "123" };
	char			sz4[] = { "4" };
	char			szExclamation[] = { "!?" };
	char			szQWE[] = { "_QWE_" };
	char			szResult[12];

	pcMemoryFile = MemoryFile();
	pcMemoryFile->Open(EFM_ReadWrite_Create);
	pcMemoryFile->Write(szABC, 1, 12);
	pcMemoryFile->Close();

	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);

	pcLogFile->Begin();

	cFile.Open(EFM_ReadWrite_Create);
	AssertLong(12, cFile.GetFileLength());

	iWritten = cFile.Write(sz123, 1, 3);
	AssertSize(3, iWritten);
	iWritten = cFile.Write(sz4, 1, 1);
	AssertSize(1, iWritten);
	cFile.Close();

	cFile.Open(EFM_ReadWrite_Create);
	cFile.Write(szExclamation, 1, 2);
	cFile.Seek(3, EFSO_CURRENT);
	cFile.Write(szQWE, 5, 1);
	cFile.Close();

	cFile.Open(EFM_Read);
	AssertLong(12, cFile.GetFileLength());
	memset(szResult, 0, 12);
	cFile.Read(szResult, 12, 1);
	AssertString("!?34E_QWE_K", szResult);
	cFile.Close();

	bResult = pcLogFile->Commit();
	AssertTrue(bResult);
	AssertFalse(pcMemoryFile->IsOpen());
	AssertSize(12, pcMemoryFile->GetBufferSize());
	AssertString("!?34E_QWE_K", (char*)pcMemoryFile->GetBufferPointer());

	cFile.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileDelete(void)
{
	CLogFile*		pcLogFile;
	CDiskFile*		pcDiskFile;
	CFileBasic		cFile;
	CFileUtil		cFileUtil;
	char			szSource[] = {"The Name of the Wise Man"};
	size			iSourcelen;
	char			szResult[50];
	char			szWrite[] = {"Cat Catt ct... "};
	size			iWriteLen;
	char			szA[] = {"A"};
	char			szDirectory[] = "Output" _FS_ "LogFile";
	char			szFileName[] = "Output" _FS_ "LogFile" _FS_ "File.txt";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	pcDiskFile = DiskFile(szFileName);
	pcDiskFile->Open(EFM_ReadWrite_Create);
	iSourcelen = strlen(szSource);
	pcDiskFile->Write(szSource, iSourcelen + 1, 1);
	pcDiskFile->Close();
	AssertTrue(cFileUtil.Exists(szFileName));

	pcLogFile = LogFile(pcDiskFile);
	cFile.Init(pcLogFile);
	pcLogFile->Begin();

	AssertTrue(cFile.Open(EFM_Read));
	AssertLong(iSourcelen + 1, cFile.Size());
	cFile.ReadData(szResult, iSourcelen + 1);
	AssertString(szSource, szResult);
	cFile.Close();

	cFile.Open(EFM_ReadWrite_Create);
	iWriteLen = strlen(szWrite);
	cFile.WriteData(szWrite, iWriteLen);
	AssertLong(iSourcelen + 1, cFile.Size());
	
	pcLogFile->Close();
	AssertSize(5, pcLogFile->GetNumCommands());
	AssertLong(25, cFile.Size());

	cFile.Delete();
	AssertTrue(cFileUtil.Exists(szFileName));
	AssertSize(1, pcLogFile->GetNumCommands());
	AssertLong(0, cFile.Size());

	cFile.Open(EFM_ReadWrite_Create);
	cFile.Write(szA, 2, 1);
	AssertLong(2, cFile.Size());
	cFile.Close();

	pcLogFile->Commit();
	cFile.Kill();

	AssertTrue(cFileUtil.Exists(szFileName));
	AssertLong(2, cFileUtil.Size(szFileName));

	pcDiskFile = DiskFile(szFileName);
	pcLogFile = LogFile(pcDiskFile);
	cFile.Init(pcLogFile);
	pcLogFile->Begin();

	AssertTrue(cFile.Open(EFM_Read));
	cFile.ReadData(szResult, 2);
	AssertString("A", szResult);
	cFile.Close();

	cFile.Delete();
	pcLogFile->Commit();
	cFile.Kill();

	AssertFalse(cFileUtil.Exists(szFileName));
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileMultipleReadsAfterOpens(void)
{
	CLogFile*		pcLogFile;
	CDiskFile*		pcDiskFile;
	CFileBasic		cFile;
	CFileUtil		cFileUtil;
	char			szSource[] = {"The Name of the Wise Man"};
	size			iSourcelen;
	char			szResult[50];
	char			szWrite[] = {"Cat Catt ct... "};
	size			iWriteLen;
	char			szA[] = {"A"};
	char			szDirectory[] = "Output" _FS_ "LogFile2";
	char			szFileName[] = "Output" _FS_ "LogFile2" _FS_ "OpenClose.txt";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	pcDiskFile = DiskFile(szFileName);
	pcDiskFile->Open(EFM_ReadWrite_Create);
	iSourcelen = strlen(szSource);
	pcDiskFile->Write(szSource, iSourcelen + 1, 1);
	pcDiskFile->Close();
	AssertTrue(cFileUtil.Exists(szFileName));

	pcLogFile = LogFile(pcDiskFile);
	cFile.Init(pcLogFile);
	pcLogFile->Begin();

	AssertTrue(cFile.Open(EFM_Read));
	AssertLong(iSourcelen + 1, cFile.Size());
	cFile.ReadData(szResult, iSourcelen + 1);
	AssertString(szSource, szResult);
	cFile.Close();

	cFile.Open(EFM_ReadWrite_Create);
	iWriteLen = strlen(szWrite);
	cFile.WriteData(szWrite, iWriteLen);
	AssertLong(iSourcelen + 1, cFile.Size());
	pcLogFile->Close();

	cFile.Delete();
	AssertTrue(cFileUtil.Exists(szFileName));
	AssertLong(0, cFile.Size());

	cFile.Open(EFM_ReadWrite_Create);
	cFile.Write(szA, 2, 1);
	AssertLong(2, cFile.Size());

	cFile.Close();

	pcLogFile->Commit();
	cFile.Kill();

	AssertTrue(cFileUtil.Exists(szFileName));
	AssertLong(2, cFileUtil.Size(szFileName));

	pcDiskFile = DiskFile(szFileName);
	pcLogFile = LogFile(pcDiskFile);
	cFile.Init(pcLogFile);
	pcLogFile->Begin();

	AssertTrue(cFile.Open(EFM_Read));
	cFile.ReadData(szResult, 2);
	AssertString("A", szResult);

	cFile.Seek(0);
	cFile.ReadData(szResult, 2);
	AssertString("A", szResult);

	cFile.Close();

	cFile.Delete();
	pcLogFile->Commit();
	cFile.Kill();

	AssertFalse(cFileUtil.Exists(szFileName));

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileTruncateMemory(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	char			sz[32];

	pcMemoryFile = MemoryFile();
	cFile.Init(pcMemoryFile);
	cFile.Open(EFM_Write_Create);
	cFile.WriteData("0123456789ABCDEF", 16);
	cFile.Close();

	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);
	AssertTrue(pcLogFile->Begin());

	//A file that is not open for writing cannot be truncated.
	AssertFalse(cFile.Truncate(4));
	AssertTrue(cFile.Open(EFM_Read));
	AssertFalse(cFile.Truncate(4));
	AssertTrue(cFile.Close());

	AssertTrue(cFile.Open(EFM_ReadWrite));
	AssertFalse(cFile.Truncate(-1));

	//Shrinking hides the end of the backing file.
	AssertTrue(cFile.Truncate(10));
	AssertLong(10, cFile.GetFileLength());
	memset(sz, 0, 32);
	AssertTrue(cFile.Seek(0));
	AssertTrue(cFile.ReadData(sz, 10));
	AssertString("0123456789", sz);
	AssertFalse(cFile.ReadData(sz, 1));

	//Writing past the end leaves a gap of zeros, not the old backing file.
	AssertTrue(cFile.Seek(12));
	AssertTrue(cFile.WriteData("XY", 2));
	AssertLong(14, cFile.GetFileLength());
	memset(sz, 'z', 32);
	AssertTrue(cFile.Seek(10));
	AssertTrue(cFile.ReadData(sz, 4));
	AssertMemory("\0\0XY", sz, 4);

	//Shrinking cuts writes back and removes those past the end.
	AssertTrue(cFile.Seek(2));
	AssertTrue(cFile.WriteData("ab", 2));
	AssertTrue(cFile.Truncate(3));
	AssertLong(3, cFile.GetFileLength());
	memset(sz, 0, 32);
	AssertTrue(cFile.Seek(0));
	AssertTrue(cFile.ReadData(sz, 3));
	AssertString("01a", sz);

	//Growing fills with zeros.
	AssertTrue(cFile.Truncate(6));
	AssertLong(6, cFile.GetFileLength());
	memset(sz, 'z', 32);
	AssertTrue(cFile.Seek(0));
	AssertTrue(cFile.ReadData(sz, 6));
	AssertMemory("01a\0\0\0", sz, 6);

	//Truncating to the same size changes nothing.
	AssertTrue(cFile.Truncate(6));
	AssertLong(6, cFile.GetFileLength());
	AssertTrue(cFile.Close());

	//Nothing reaches the backing file until the commit.
	AssertSize(16, pcMemoryFile->GetBufferSize());
	AssertTrue(pcLogFile->Commit());
	AssertSize(6, pcMemoryFile->GetBufferSize());
	AssertMemory("01a\0\0\0", pcMemoryFile->GetBufferPointer(), 6);
	AssertTrue(pcLogFile->End());

	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFileTruncateDisk(void)
{
	CLogFile*		pcLogFile;
	CFileBasic		cFile;
	CFileUtil		cFileUtil;
	char			szDirectory[] = "Output" _FS_ "LogFile8";
	char			szFileName[] = "Output" _FS_ "LogFile8" _FS_ "File.txt";
	char			sz[32];

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	cFile.Init(DiskFile(szFileName));
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertTrue(cFile.WriteData("The suspense is killing me!", 27));
	AssertTrue(cFile.Close());
	cFile.Kill();

	pcLogFile = LogFile(DiskFile(szFileName));
	cFile.Init(pcLogFile);
	AssertTrue(pcLogFile->Begin());
	AssertTrue(cFile.Open(EFM_ReadWrite));
	AssertTrue(cFile.Truncate(12));
	AssertTrue(cFile.Seek(12));
	AssertTrue(cFile.WriteData(" over", 5));
	AssertTrue(cFile.Close());

	AssertLong(27, cFileUtil.Size(szFileName));
	AssertTrue(pcLogFile->Commit());
	AssertTrue(pcLogFile->End());
	cFile.Kill();

	AssertLong(17, cFileUtil.Size(szFileName));
	memset(sz, 0, 32);
	cFile.Init(DiskFile(szFileName));
	AssertTrue(cFile.Open(EFM_Read));
	AssertTrue(cFile.ReadData(sz, 17));
	AssertTrue(cFile.Close());
	cFile.Kill();
	AssertString("The suspense over", sz);

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFilePrint(void)
{
	CLogFile*		pcLogFile;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	CChars			sz;
	CChars			szExpected;

	pcMemoryFile = MemoryFile();
	pcLogFile = LogFile(pcMemoryFile);
	cFile.Init(pcLogFile);
	AssertTrue(pcLogFile->Begin());

	AssertTrue(cFile.Open(EFM_ReadWrite_Create));
	AssertTrue(cFile.WriteData("Hello", 5));
	AssertTrue(cFile.Truncate(3));
	AssertTrue(cFile.Close());

	//Every kind of command is printed as itself.
	szExpected.Init("Log File [Without Name] Commands: 4\n   Open (");
	szExpected.Append((int)EFM_ReadWrite_Create);
	szExpected.Append(")\n   Write (0, 3) Hel\n   Truncate (3)\n   Close\n");
	sz.Init();
	pcLogFile->Print(&sz);
	AssertString(szExpected.Text(), sz.Text());
	sz.Kill();
	szExpected.Kill();

	pcLogFile->End();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogFile(void)
{
	TypeConverterInit();
	DataMemoryInit();
	BeginTests();

	TestLogFileOpen();
	TestLogFileRead();
	TestLogFileWriteMemory();
	TestLogFileExtendLastWrite();
	TestLogFileExtendLastWriteLarge();
	TestLogFileWriteDiskSingle();
	TestLogFileWriteDisk();
	TestLogFileFindHoles();
	TestLogFileCommandsSimple();
	TestLogFileCommandsComplex();
	TestLogFileDelete();
	TestLogFileMultipleReadsAfterOpens();
	TestLogFileTruncateMemory();
	TestLogFileTruncateDisk();
	TestLogFilePrint();

	TestStatistics();
	DataMemoryKill();
	TypeConverterKill();
}

