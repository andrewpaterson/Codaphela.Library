#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/ChunkFileFile.h"
#include "BaseLib/ArrayChar.h"
#include "BaseLib/PointerFunctions.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileSimple(void)
{
	CChunkFile	cChunkFile;
	CFileUtil	cFileUtil;
	size		iChunkNum;
	
	cFileUtil.MakeDir("Output");
	cFileUtil.Delete("Output/ChunkFile.bin");
	cChunkFile.Init(DiskFile("Output/ChunkFile.bin"));
	AssertFalse(cChunkFile.IsOpen());
	AssertTrue(cChunkFile.WriteOpen());
	AssertTrue(cChunkFile.IsOpen());
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertTrue(cChunkFile.WriteChunkEnd("ThisChunk"));
	AssertTrue(cChunkFile.WriteClose());
	AssertFalse(cChunkFile.IsOpen());
	cChunkFile.Kill();

	cChunkFile.Init(DiskFile("Output/ChunkFile.bin"));
	AssertFalse(cChunkFile.IsOpen());
	AssertTrue(cChunkFile.ReadOpen());
	Pass();
	AssertTrue(cChunkFile.IsOpen());
	iChunkNum = cChunkFile.FindFirstChunkWithName("ThisChunk");
	AssertSize(0, iChunkNum);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunkNum));
	Pass();
	AssertTrue(cChunkFile.ReadClose());
	Pass();
	AssertFalse(cChunkFile.IsOpen());
	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileMD5ing(void)
{
	char		szFox[] = "The quick brown fox jumps over the lazy dog";
	uint8		ucFoxMD5[] = {0x9e, 0x10, 0x7d, 0x9d, 0x37, 0x2b, 0xb6, 0x82, 0x6b, 0xd8, 0x1d, 0x35, 0x42, 0xa4, 0x19, 0xd6};
	size		iFoxLen;
	char		szLorem[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
	uint8		ucLoremMD5[] = {0xfa, 0x5c, 0x89, 0xf3, 0xc8, 0x8b, 0x81, 0xbf, 0xd5, 0xe8, 0x21, 0xb0, 0x31, 0x65, 0x69, 0xaf};
	size		iLoremLen;
	CChunkFile	cChunkFile;

	iFoxLen = strlen(szFox);
	iLoremLen = strlen(szLorem);

	cChunkFile.Init(MemoryFile());
	AssertTrue(cChunkFile.WriteOpen());
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertTrue(cChunkFile.Write(szFox, 1, iFoxLen) > 0);
	AssertTrue(cChunkFile.WriteChunkEnd("ChunkName"));
	AssertTrue(cChunkFile.Write(szLorem, 1, iLoremLen) > 0);
	AssertTrue(cChunkFile.WriteClose());

	AssertTrue(cChunkFile.ReadOpen());
	AssertMD5(ucLoremMD5, (uint8 *)cChunkFile.GetMD5Hash());
	AssertTrue(cChunkFile.ReadChunkBegin((size)0));
	AssertMD5(ucFoxMD5, (uint8 *)cChunkFile.GetMD5Hash());
	AssertTrue(cChunkFile.ReadChunkEnd());
	AssertTrue(cChunkFile.ReadClose());

	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFilenameing(void)
{
	CChunkFile		cChunkFile;
	size			iChunkNum;
	char			acTwentyOne[] =   {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
	char			acTwentyTwo[] =   {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
	char			acTwentyThree[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29};
	char			acActual[10];

	cChunkFile.Init(MemoryFile());
	AssertTrue(cChunkFile.WriteOpen());
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertLong(1, cChunkFile.Write(acTwentyOne, 10, 1));
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertLong(1, cChunkFile.Write(acTwentyTwo, 10, 1));
	AssertTrue(cChunkFile.WriteChunkEnd("Floopy"));
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertLong(1, cChunkFile.Write(acTwentyThree, 10, 1));
	AssertTrue(cChunkFile.WriteChunkEnd("Floopy"));
	AssertTrue(cChunkFile.WriteChunkEnd("Rise"));
	AssertTrue(cChunkFile.WriteClose());

	AssertTrue(cChunkFile.ReadOpen());
	iChunkNum = cChunkFile.FindFirstChunkWithName("Rise");
	AssertSize(0, iChunkNum);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunkNum));
	AssertLong(1, cChunkFile.Read(acActual, 10, 1));
	AssertMemory(acTwentyOne, acActual, 10);

	iChunkNum = cChunkFile.FindFirstChunkWithName("Floopy");
	AssertSize(0, iChunkNum);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunkNum));
	AssertLong(1, cChunkFile.Read(acActual, 10, 1));
	AssertMemory(acTwentyTwo, acActual, 10);
	AssertTrue(cChunkFile.ReadChunkEnd());

	iChunkNum = cChunkFile.FindNextChunkWithName();
	AssertSize(1, iChunkNum);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunkNum));
	AssertLong(1, cChunkFile.Read(acActual, 10, 1));
	AssertMemory(acTwentyThree, acActual, 10);
	AssertTrue(cChunkFile.ReadChunkEnd());

	AssertTrue(cChunkFile.ReadChunkEnd());
	AssertTrue(cChunkFile.ReadClose());

	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFileBasic(void)
{
	CChunkFile		cChunkFile;
	CChunkFileFile	cChunkFileFile1;
	CChunkFileFile	cChunkFileFile2;
	CFileBasic		cFileBasic1;
	CFileBasic		cFileBasic2;
	int32			iResult;
	char			szResult[256];

	cChunkFile.Init(MemoryFile());
	AssertTrue(cChunkFile.WriteOpen());

	AssertTrue(cChunkFile.WriteChunkBegin());
	cChunkFileFile1.Init(&cChunkFile);
	cFileBasic1.Init(&cChunkFileFile1);
	AssertTrue(cFileBasic1.Open(EFM_Write_Create));
	AssertTrue(cFileBasic1.WriteInt32(12345678));
	AssertTrue(cFileBasic1.WriteInt32(0x7fffffff));
	AssertTrue(cFileBasic1.Close());
	AssertTrue(cChunkFile.WriteChunkEnd("2 Ints"));
	cFileBasic1.Kill();

	AssertTrue(cChunkFile.WriteChunkBegin());
	cChunkFileFile1.Init(&cChunkFile);
	cFileBasic2.Init(&cChunkFileFile1);
	AssertTrue(cFileBasic2.Open(EFM_Write_Create));
	AssertTrue(cFileBasic2.WriteString("Hello"));
	AssertTrue(cFileBasic2.WriteString("World"));
	AssertTrue(cFileBasic2.Close());
	AssertTrue(cChunkFile.WriteChunkEnd("The Usual"));
	cFileBasic2.Kill();

	AssertTrue(cChunkFile.WriteClose());

	AssertTrue(cChunkFile.ReadOpen());
	AssertTrue(cChunkFile.ReadChunkBegin("2 Ints"));
	cChunkFileFile1.Init(&cChunkFile);
	cFileBasic1.Init(&cChunkFileFile1);
	AssertTrue(cFileBasic1.Open(EFM_Read));
	AssertTrue(cFileBasic1.ReadInt32(&iResult));
	AssertInt(12345678, iResult);
	AssertTrue(cFileBasic1.ReadInt32(&iResult));
	AssertInt(0x7fffffff, iResult);
	AssertTrue(cFileBasic1.Close());
	AssertTrue(cChunkFile.ReadChunkEnd());
	cFileBasic1.Kill();

	AssertTrue(cChunkFile.ReadChunkBegin("The Usual"));
	cChunkFileFile1.Init(&cChunkFile);
	cFileBasic2.Init(&cChunkFileFile1);
	AssertTrue(cFileBasic2.Open(EFM_Read));
	AssertTrue(cFileBasic2.ReadString(szResult, 256));
	AssertString("Hello", szResult);
	AssertTrue(cFileBasic2.ReadString(szResult, 256));
	AssertString("World", szResult);
	AssertTrue(cFileBasic2.Close());
	AssertTrue(cChunkFile.ReadChunkEnd());
	cFileBasic2.Kill();

	cChunkFile.ReadClose();
	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a chunk file to memory, hashed (Init) or not (InitUnhashed): an
// "Outer" chunk holding some text and an "Inner" chunk.  The file's bytes
// are copied to pacFile.
//
//////////////////////////////////////////////////////////////////////////
static void TestChunkFileWriteHashing(bool bHashed, CArrayChar* pacFile)
{
	CChunkFile		cChunkFile;
	CMemoryFile*	pcMemoryFile;
	char			szOuter[] = "The quick brown fox";
	char			szInner[] = "jumps over the lazy dog";

	pcMemoryFile = MemoryFile();
	pcMemoryFile->mbBasicFileMustFree = false;
	if (bHashed)
	{
		cChunkFile.Init(pcMemoryFile);
	}
	else
	{
		cChunkFile.InitUnhashed(pcMemoryFile);
	}
	AssertTrue(cChunkFile.WriteOpen());
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertTrue(cChunkFile.Write(szOuter, 1, (size)strlen(szOuter)) > 0);
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertTrue(cChunkFile.Write(szInner, 1, (size)strlen(szInner)) > 0);
	AssertTrue(cChunkFile.WriteChunkEnd("Inner"));
	AssertTrue(cChunkFile.WriteChunkEnd("Outer"));
	AssertTrue(cChunkFile.WriteClose());
	cChunkFile.Kill();

	pacFile->Init();
	pacFile->Resize((size)pcMemoryFile->GetBufferSize());
	memcpy(pacFile->GetData(), pcMemoryFile->GetBufferPointer(), (size)pcMemoryFile->GetBufferSize());
	SafeKill(pcMemoryFile);
}


//////////////////////////////////////////////////////////////////////////
//
// True if every byte of a hash is zero (written without hashes).
//
//////////////////////////////////////////////////////////////////////////
static bool TestChunkFileHashIsZero(void* pvHash)
{
	size	i;

	for (i = 0; i < 16; i++)
	{
		if (((uint8*)pvHash)[i] != 0)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads TestChunkFileWriteHashing's file, hashed or not, and checks every
// chunk's text.  bZeroHashes says whether its hashes must be zeros.
//
//////////////////////////////////////////////////////////////////////////
static void TestChunkFileReadHashing(bool bHashed, CArrayChar* pacFile, bool bZeroHashes)
{
	CChunkFile	cChunkFile;
	char		szText[32];
	size		iChunk;

	if (bHashed)
	{
		cChunkFile.Init(MemoryFile(pacFile->GetData(), pacFile->NumElements()));
	}
	else
	{
		cChunkFile.InitUnhashed(MemoryFile(pacFile->GetData(), pacFile->NumElements()));
	}
	AssertTrue(cChunkFile.ReadOpen());
	AssertBool(bZeroHashes, TestChunkFileHashIsZero(cChunkFile.GetMD5Hash()));

	iChunk = cChunkFile.FindFirstChunkWithName("Outer");
	AssertSize(0, iChunk);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunk));
	AssertBool(bZeroHashes, TestChunkFileHashIsZero(cChunkFile.GetMD5Hash()));
	memset(szText, 0, sizeof(szText));
	AssertTrue(cChunkFile.Read(szText, 19, 1) == 1);
	AssertString("The quick brown fox", szText);

	iChunk = cChunkFile.FindFirstChunkWithName("Inner");
	AssertSize(0, iChunk);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunk));
	memset(szText, 0, sizeof(szText));
	AssertTrue(cChunkFile.Read(szText, 23, 1) == 1);
	AssertString("jumps over the lazy dog", szText);
	AssertTrue(cChunkFile.ReadChunkEnd());
	AssertTrue(cChunkFile.ReadChunkEnd());
	AssertTrue(cChunkFile.ReadClose());
	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Chunk files with and without hashes, each read with and without
// checking them: a file written without hashes has zeros for them and a
// hashed reader accepts it; an unhashed reader reads a hashed file without
// checking it, even when its hash is wrong (which a hashed reader refuses).
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileHashing(void)
{
	CArrayChar	acHashed;
	CArrayChar	acUnhashed;
	CChunkFile	cChunkFile;

	TestChunkFileWriteHashing(true, &acHashed);
	TestChunkFileWriteHashing(false, &acUnhashed);

	//Only the hashes differ.
	AssertSize(acHashed.NumElements(), acUnhashed.NumElements());

	TestChunkFileReadHashing(true, &acHashed, false);
	TestChunkFileReadHashing(false, &acHashed, false);
	TestChunkFileReadHashing(true, &acUnhashed, true);
	TestChunkFileReadHashing(false, &acUnhashed, true);

	//The file's hash (in its header, after the user ID and the chunk names' position) made wrong.
	acHashed.GetData()[16] ^= 0x5A;
	cChunkFile.Init(MemoryFile(acHashed.GetData(), acHashed.NumElements()));
	AssertFalse(cChunkFile.ReadOpen());
	cChunkFile.ReadClose();
	cChunkFile.Kill();
	TestChunkFileReadHashing(false, &acHashed, false);

	acUnhashed.Kill();
	acHashed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFile(void)
{
	BeginTests();
	MemoryInit();
	
	TestChunkFileSimple();
	TestChunkFileMD5ing();
	TestChunkFilenameing();
	TestChunkFileFileBasic();
	TestChunkFileHashing();

	MemoryKill();
	TestStatistics();
}

