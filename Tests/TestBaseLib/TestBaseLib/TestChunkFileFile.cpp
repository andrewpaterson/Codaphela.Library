#include "BaseLib/ChunkFile.h"
#include "BaseLib/ChunkFileFile.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFileRead(void)
{
	CChunkFile		cChunkFile;
	size				iChunkNum;
	char			szZ[] = {"Z"};
	CChunkFileFile	cChunkFileFile;
	char			cz;

	cChunkFile.Init(MemoryFile());
	AssertTrue(cChunkFile.WriteOpen());
	AssertTrue(cChunkFile.WriteChunkBegin());
	AssertLong(1, cChunkFile.Write(szZ, 1, 1));
	AssertTrue(cChunkFile.WriteChunkEnd("Zynaps"));
	AssertTrue(cChunkFile.WriteClose());

	AssertTrue(cChunkFile.ReadOpen());
	iChunkNum = cChunkFile.FindFirstChunkWithName("Zynaps");
	AssertSize(0, iChunkNum);
	AssertTrue(cChunkFile.ReadChunkBegin(iChunkNum));

	cChunkFileFile.Init(&cChunkFile);
	AssertTrue(cChunkFileFile.Open(EFM_Read));
	AssertLong(1, cChunkFileFile.Size());
	AssertLong(0, cChunkFileFile.Tell());
	AssertLong(1, cChunkFileFile.Read(&cz, 1, 1));
	AssertChar('Z', cz); cz = 'Q';

	AssertTrue(cChunkFileFile.Seek(0, EFSO_SET));
	AssertLong(0, cChunkFileFile.Tell());
	AssertLong(1, cChunkFileFile.Read(&cz, 1, 1));
	AssertChar('Z', cz); cz = 'Q';
	AssertLong(1, cChunkFileFile.Tell());
	AssertLong(0, cChunkFileFile.Read(&cz, 1, 1));
	AssertLong(1, cChunkFileFile.Tell());

	AssertTrue(cChunkFileFile.Seek(0, EFSO_SET));
	AssertLong(1, cChunkFileFile.Read(&cz, 1, 2));
	AssertChar('Z', cz); cz = 'Q';
	AssertLong(1, cChunkFileFile.Tell());

	AssertTrue(cChunkFileFile.Close());

	AssertTrue(cChunkFile.ReadChunkEnd());
	AssertTrue(cChunkFile.ReadClose());

	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFile(void)
{
	BeginTests();
	MemoryInit();

	TestChunkFileFileRead();

	MemoryKill();
	TestStatistics();
}

