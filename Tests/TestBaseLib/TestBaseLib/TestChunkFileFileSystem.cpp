#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ChunkFileFileSystem.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFileSystemWriting(void)
{
	CChunkFile				cChunkFile;
	CChunkFileFileSystem	cChunkFileFileSystem;
	CDiskFile*				pcDiskFile;
	CChunkFile				cTestFile;
	size						iIndex;
	CChars					szText;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir("Output" _FS_ "ChunkFileFileSystem");
	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "ChunkFileFileSystem"));

	pcDiskFile = DiskFile("Output" _FS_ "ChunkFileFileSystem" _FS_ "FrenchToast.DRG");
	cChunkFile.Init(pcDiskFile);
	cChunkFileFileSystem.Init(&cChunkFile);

	AssertTrue(cChunkFileFileSystem.WriteOpen());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("new/age/violet/Tempest"));
	AssertTrue(cChunkFileFileSystem.WriteString("No man, Mr. President, thinks more highly than I do of the patriotism"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("new/age/shrinking/daisy/Sunlight"));
	AssertTrue(cChunkFileFileSystem.WriteString("The question before the House is one of awful moment to this country."));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("new/age/shrinking/daisy/Pusher"));
	AssertTrue(cChunkFileFileSystem.WriteString("We are apt to shut our eyes against a painful truth"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("new/age/violet/Forensics"));
	AssertTrue(cChunkFileFileSystem.WriteString("Shall we try argument?"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("Unknown"));
	AssertTrue(cChunkFileFileSystem.WriteString("Sir, we have been trying that for the last ten years."));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("last/time/in/Paris"));
	AssertTrue(cChunkFileFileSystem.WriteString("There is no longer any room for hope."));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("last/time/in/world/on/See"));
	AssertTrue(cChunkFileFileSystem.WriteString("But when shall we be stronger?"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("last/Schism"));
	AssertTrue(cChunkFileFileSystem.WriteString("Finally! Something with Doom!"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("last/Tragic"));
	AssertTrue(cChunkFileFileSystem.WriteString("If you give me a one word text back, I'm gonna give you the same"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("Final"));
	AssertTrue(cChunkFileFileSystem.WriteString("Countdown"));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteChunkBegin("new/really/Final"));
	AssertTrue(cChunkFileFileSystem.WriteString("Seriously, how is it not possible to find text on the internet."));
	AssertTrue(cChunkFileFileSystem.WriteChunkEnd());

	AssertTrue(cChunkFileFileSystem.WriteClose());
	cChunkFileFileSystem.Kill();
	cChunkFile.Kill();

	//What's expected:
	//
	// 1:   new/age/violet/Tempest
	// 1:   new/age/shrinking/daisy/Sunlight
	// 1:   new/age/shrinking/daisy/Pusher
	// 1:   new/age/violet/Forensics
	// 2:   Unknown
	// 3:   last/time/in/world/on/See
	// 3:   last/Schism
	// 3:   last/Tragic
	// 4:   Final
	// 5:   new/really/Final

	pcDiskFile = DiskFile("Output" _FS_ "ChunkFileFileSystem" _FS_ "FrenchToast.DRG");
	cTestFile.Init(pcDiskFile);
	AssertTrue(cTestFile.ReadOpen());

	AssertSize(5, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("new");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(1, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("age");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(3, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("violet");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(1, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("Tempest");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("No man, Mr. President, thinks more highly than I do of the patriotism", szText.Text());
	szText.Kill();

	AssertTrue(cTestFile.ReadChunkEnd(2));

	iIndex = cTestFile.FindFirstChunkWithName("shrinking");
	AssertSize(1, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(1, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("daisy");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(2, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("Sunlight");
	AssertSize(0, iIndex);
	cTestFile.ReadChunkBegin(iIndex);
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("The question before the House is one of awful moment to this country.", szText.Text());
	szText.Kill();
	AssertTrue(cTestFile.ReadChunkEnd());

	iIndex = cTestFile.FindFirstChunkWithName("Pusher");
	AssertSize(1, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("We are apt to shut our eyes against a painful truth", szText.Text());
	szText.Kill();
	AssertTrue(cTestFile.ReadChunkEnd(3));

	iIndex = cTestFile.FindFirstChunkWithName("violet");
	AssertSize(0, iIndex);
	iIndex = cTestFile.FindNextChunkWithName();
	AssertSize(2, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(1, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("Forensics");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("Shall we try argument?", szText.Text());
	szText.Kill();
	AssertTrue(cTestFile.ReadChunkEnd(4));

	iIndex = cTestFile.FindFirstChunkWithName("Unknown");
	AssertSize(1, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("Sir, we have been trying that for the last ten years.", szText.Text());
	szText.Kill();
	AssertTrue(cTestFile.ReadChunkEnd());
	AssertSize(5, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("Final");
	AssertSize(3, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("Countdown", szText.Text());
	szText.Kill();
	AssertTrue(cTestFile.ReadChunkEnd());
	AssertSize(5, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("new");
	AssertSize(0, iIndex);
	iIndex = cTestFile.FindNextChunkWithName();
	AssertSize(4, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(1, cTestFile.GetNumChunks());

	iIndex = cTestFile.FindFirstChunkWithName("really");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(1, cTestFile.GetNumChunks());
	
	iIndex = cTestFile.FindFirstChunkWithName("Final");
	AssertSize(0, iIndex);
	AssertTrue(cTestFile.ReadChunkBegin(iIndex));
	AssertSize(0, cTestFile.GetNumChunks());
	AssertTrue(szText.ReadChars(&cTestFile));
	AssertString("Seriously, how is it not possible to find text on the internet.", szText.Text());
	szText.Kill();
	AssertTrue(cTestFile.ReadChunkEnd(3));
	AssertSize(5, cTestFile.GetNumChunks());

	cTestFile.ReadClose();
	cTestFile.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "ChunkFileFileSystem");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFileSystemReading(void)
{
	CChunkFile				cChunkFile;
	CChunkFileFileSystem	cChunkFileFileSystem;
	CDiskFile*				pcDiskFile;
	CChars					szText;

	pcDiskFile = DiskFile("Input" _FS_ "FrenchToast.DRG");
	cChunkFile.Init(pcDiskFile);
	cChunkFileFileSystem.Init(&cChunkFile);

	AssertTrue(cChunkFileFileSystem.ReadOpen());

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/violet/Tempest"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("No man, Mr. President, thinks more highly than I do of the patriotism", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertFalse(cChunkFileFileSystem.ReadChunkBegin("new/age/Duke"));

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/shrinking/daisy/Sunlight"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("The question before the House is one of awful moment to this country.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertFalse(cChunkFileFileSystem.ReadChunkBegin("new/age/Duke"));

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/shrinking/daisy/Pusher"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("We are apt to shut our eyes against a painful truth", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/violet/Forensics"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Shall we try argument?", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("Unknown"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Sir, we have been trying that for the last ten years.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/time/in/Paris"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("There is no longer any room for hope.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/time/in/world/on/See"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("But when shall we be stronger?", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/Schism"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Finally! Something with Doom!", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/Tragic"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("If you give me a one word text back, I'm gonna give you the same", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("Final"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Countdown", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/really/Final"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Seriously, how is it not possible to find text on the internet.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertFalse(cChunkFileFileSystem.ReadChunkBegin(""));
	AssertFalse(cChunkFileFileSystem.ReadChunkBegin("CCC//XX/xx"));
	AssertFalse(cChunkFileFileSystem.ReadChunkBegin(NULL));
	AssertFalse(cChunkFileFileSystem.ReadChunkBegin("/"));

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/shrinking/daisy/Sunlight"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("The question before the House is one of awful moment to this country.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/violet/Tempest"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("No man, Mr. President, thinks more highly than I do of the patriotism", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/shrinking/daisy/Pusher"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("We are apt to shut our eyes against a painful truth", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/Schism"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Finally! Something with Doom!", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/age/violet/Forensics"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Shall we try argument?", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/Tragic"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("If you give me a one word text back, I'm gonna give you the same", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("new/really/Final"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Seriously, how is it not possible to find text on the internet.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("Final"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Countdown", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/time/in/world/on/See"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("But when shall we be stronger?", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("last/time/in/Paris"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("There is no longer any room for hope.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	AssertTrue(cChunkFileFileSystem.ReadChunkBegin("Unknown"));
	AssertTrue(szText.ReadChars(&cChunkFileFileSystem));
	AssertString("Sir, we have been trying that for the last ten years.", szText.Text());
	AssertTrue(cChunkFileFileSystem.ReadChunkEnd());
	szText.Kill();

	cChunkFileFileSystem.ReadClose();
	cChunkFileFileSystem.Kill();
	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFileSystemNameIteration(void)
{
	CChunkFile					cChunkFile;
	CChunkFileFileSystem		cChunkFileFileSystem;
	CDiskFile*					pcDiskFile;
	SChunkFilenameIterator		sIter;
	char*						szName;

	pcDiskFile = DiskFile("Input" _FS_ "FrenchToast.DRG");
	cChunkFile.Init(pcDiskFile);
	cChunkFileFileSystem.Init(&cChunkFile);

	cChunkFileFileSystem.ReadOpen();


	szName = cChunkFileFileSystem.StartNameIteration(&sIter);
	AssertString("new/age/violet/Tempest", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("new/age/shrinking/daisy/Sunlight", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("new/age/shrinking/daisy/Pusher", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("new/age/violet/Forensics", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("Unknown", szName);
	
	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("last/time/in/Paris", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("last/time/in/world/on/See", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("last/Schism", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("last/Tragic", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("Final", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertString("new/really/Final", szName);

	szName = cChunkFileFileSystem.IterateName(&sIter);
	AssertNull(szName);

	cChunkFileFileSystem.StopIteration(&sIter);

	cChunkFileFileSystem.ReadClose();
	cChunkFileFileSystem.Kill();
	cChunkFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChunkFileFileSystem(void)
{
	BeginTests();
	MemoryInit();

	TestChunkFileFileSystemWriting();
	TestChunkFileFileSystemReading();
	TestChunkFileFileSystemNameIteration();

	MemoryKill();
	TestStatistics();
}
