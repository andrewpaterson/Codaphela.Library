#include "BaseLib/Logger.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Sequence.h"
#include "BaseLib/ValueSequenceConfig.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSequenceNext(void)
{
	CSequence					cSequence;
	CLifeInit<CSequenceConfig>	cConfig;
	char						szDirectory[] = "Output" _FS_ "Sequence" ;
	OIndex						oi;
	CFileUtil					cFileUtil;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	AssertLong(1LL, FIRST_O_INDEX);

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_Yes);
	cSequence.Init(cConfig);

	oi = cSequence.GetNext();
	AssertLong(1LL, oi);

	oi = cSequence.GetNext();
	AssertLong(2LL, oi);

	cSequence.Kill();

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_Yes);
	cSequence.Init(cConfig);

	oi = cSequence.GetNext();
	AssertLong(3LL, oi);

	oi = cSequence.GetNext();
	AssertLong(4LL, oi);

	cSequence.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSequenceCrashRecovery(bool bRewriteMark)
{
	CSequence					cSequence;
	CLifeInit<CSequenceConfig>	cConfig;
	char						szDirectory[] = "Output" _FS_ "SequenceCrash";
	CFileUtil					cFileUtil;
	CTextFile					cMark;
	SLogConfig					sLogConfig;

	cFileUtil.RemoveDir(szDirectory);

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_Yes);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(1LL, cSequence.GetNext());
	AssertLong(2LL, cSequence.GetNext());
	AssertLong(3LL, cSequence.GetNext());
	cSequence.Kill();
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceCrash" _FS_ "Backup" _FS_ "_Sequence.DAT", "Output" _FS_ "SequenceCrash" _FS_ "Saved.DAT"));

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_Yes);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(4LL, cSequence.GetNext());
	AssertLong(5LL, cSequence.GetNext());
	cSequence.Kill();

	//Put the backup back to 4 as if the process died while writing 6, with the primary already written.
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceCrash" _FS_ "Saved.DAT", "Output" _FS_ "SequenceCrash" _FS_ "Backup" _FS_ "_Sequence.DAT"));
	cMark.Init();
	cMark.mcText.Append("Output" _FS_ "SequenceCrash" _FS_ "Primary" _FS_ "Sequence.DAT");
	cMark.mcText.AppendNewLine();
	cMark.mcText.Append("Output" _FS_ "SequenceCrash" _FS_ "Backup" _FS_ "_Sequence.DAT");
	cMark.mcText.AppendNewLine();
	cMark.mcText.Append("#End");
	cMark.mcText.AppendNewLine();
	AssertTrue(cMark.Write("Output" _FS_ "SequenceCrash" _FS_ "Primary" _FS_ "Mark1.Write"));
	cMark.Kill();
	if (bRewriteMark)
	{
		AssertTrue(cFileUtil.Touch("Output" _FS_ "SequenceCrash" _FS_ "Backup" _FS_ "Mark2.Rewrite"));
	}

	//The sequence continues from the recovered value.  It must never start again from the first index.
	cConfig = CValueSequenceConfig::Create(szDirectory, DS_Yes);
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(cSequence.Init(cConfig));
	gcLogger.SetConfig(&sLogConfig);
	if (bRewriteMark)
	{
		AssertLong(6LL, cSequence.GetNext());
	}
	else
	{
		AssertLong(4LL, cSequence.GetNext());
	}
	cSequence.Kill();
	AssertFalse(cFileUtil.Exists("Output" _FS_ "SequenceCrash" _FS_ "Primary" _FS_ "Mark1.Write"));
	AssertFalse(cFileUtil.Exists("Output" _FS_ "SequenceCrash" _FS_ "Backup" _FS_ "Mark2.Rewrite"));

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSequenceBlocks(void)
{
	CSequence					cSequence;
	CLifeInit<CSequenceConfig>	cConfig;
	char						szDirectory[] = "Output" _FS_ "SequenceBlocks";
	CFileUtil					cFileUtil;

	cFileUtil.RemoveDir(szDirectory);

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(1LL, cSequence.GetNext());
	AssertLong(2LL, cSequence.GetNext());
	AssertLong(3LL, cSequence.PeekNext());

	//Keep the files as they are while the block 1..4 is reserved.
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Primary" _FS_ "Sequence.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Primary.DAT"));
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Backup" _FS_ "_Sequence.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Backup.DAT"));

	AssertLong(3LL, cSequence.GetNext());
	AssertLong(4LL, cSequence.GetNext());
	AssertLong(5LL, cSequence.GetNext());
	AssertLong(6LL, cSequence.GetNext());
	cSequence.Kill();

	//A clean close gives back the rest of the block.
	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(7LL, cSequence.GetNext());
	cSequence.Kill();

	//Dying while the block 1..4 was reserved skips the rest of the block.
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Primary.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Primary" _FS_ "Sequence.DAT"));
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Backup.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Backup" _FS_ "_Sequence.DAT"));
	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(5LL, cSequence.GetNext());
	cSequence.Kill();

	//A block size of zero writes every index, the same as a block size of one.
	cFileUtil.RemoveDir(szDirectory);
	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 0);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(1LL, cSequence.GetNext());
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Primary" _FS_ "Sequence.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Primary.DAT"));
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Backup" _FS_ "_Sequence.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Backup.DAT"));
	AssertLong(2LL, cSequence.GetNext());
	cSequence.Kill();
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Primary.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Primary" _FS_ "Sequence.DAT"));
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceBlocks" _FS_ "Backup.DAT", "Output" _FS_ "SequenceBlocks" _FS_ "Backup" _FS_ "_Sequence.DAT"));
	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 0);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(2LL, cSequence.GetNext());
	cSequence.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// A reserved range is never handed out by GetNext, before or after a
// crash.
//
//////////////////////////////////////////////////////////////////////////
void TestSequenceReserveRange(void)
{
	CSequence					cSequence;
	CLifeInit<CSequenceConfig>	cConfig;
	char						szDirectory[] = "Output" _FS_ "SequenceReserveRange";
	CFileUtil					cFileUtil;
	SLogConfig					sLogConfig;

	cFileUtil.RemoveDir(szDirectory);

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(MAX_O_INDEX, cSequence.GetEnd());
	AssertLong(1LL, cSequence.GetNext());
	AssertLong(2LL, cSequence.GetNext());
	AssertLong(3LL, cSequence.ReserveRange(10));
	AssertLong(13LL, cSequence.PeekNext());

	//Keep the files as they are just after the range 3..12 was reserved.
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceReserveRange" _FS_ "Primary" _FS_ "Sequence.DAT", "Output" _FS_ "SequenceReserveRange" _FS_ "Primary.DAT"));
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceReserveRange" _FS_ "Backup" _FS_ "_Sequence.DAT", "Output" _FS_ "SequenceReserveRange" _FS_ "Backup.DAT"));

	AssertLong(13LL, cSequence.GetNext());
	AssertLong(14LL, cSequence.ReserveRange(1));
	AssertLong(15LL, cSequence.GetNext());

	sLogConfig = gcLogger.SetSilent();
	AssertLong(INVALID_O_INDEX, cSequence.ReserveRange(0));
	AssertLong(INVALID_O_INDEX, cSequence.ReserveRange(-1));
	gcLogger.SetConfig(&sLogConfig);
	AssertLong(16LL, cSequence.PeekNext());
	cSequence.Kill();

	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(16LL, cSequence.GetNext());
	cSequence.Kill();

	//Dying just after the range was reserved still never hands it out.
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceReserveRange" _FS_ "Primary.DAT", "Output" _FS_ "SequenceReserveRange" _FS_ "Primary" _FS_ "Sequence.DAT"));
	AssertTrue(cFileUtil.Copy("Output" _FS_ "SequenceReserveRange" _FS_ "Backup.DAT", "Output" _FS_ "SequenceReserveRange" _FS_ "Backup" _FS_ "_Sequence.DAT"));
	cConfig = CValueSequenceConfig::Create(szDirectory, DS_No, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(13LL, cSequence.GetNext());
	cSequence.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// A range-limited sequence starts at its first index, keeps going after
// it is reopened and stops at its end.
//
//////////////////////////////////////////////////////////////////////////
void TestSequenceRange(void)
{
	CSequence					cSequence;
	CLifeInit<CSequenceConfig>	cConfig;
	char						szDirectory[] = "Output" _FS_ "SequenceRange";
	CFileUtil					cFileUtil;
	SLogConfig					sLogConfig;
	OIndex						i;

	cFileUtil.RemoveDir(szDirectory);

	cConfig = CValueSequenceConfig::CreateRange(szDirectory, DS_No, 100, 110, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(110LL, cSequence.GetEnd());
	AssertLong(100LL, cSequence.GetNext());
	AssertLong(101LL, cSequence.GetNext());
	cSequence.Kill();

	cConfig = CValueSequenceConfig::CreateRange(szDirectory, DS_No, 100, 110, 4);
	AssertTrue(cSequence.Init(cConfig));
	AssertLong(102LL, cSequence.ReserveRange(3));
	for (i = 105; i < 110; i++)
	{
		AssertLong(i, cSequence.GetNext());
	}

	sLogConfig = gcLogger.SetSilent();
	AssertLong(INVALID_O_INDEX, cSequence.GetNext());
	AssertLong(INVALID_O_INDEX, cSequence.ReserveRange(1));
	gcLogger.SetConfig(&sLogConfig);
	AssertLong(110LL, cSequence.PeekNext());
	cSequence.Kill();

	//Still used up after it is reopened.
	cConfig = CValueSequenceConfig::CreateRange(szDirectory, DS_No, 100, 110, 4);
	AssertTrue(cSequence.Init(cConfig));
	sLogConfig = gcLogger.SetSilent();
	AssertLong(INVALID_O_INDEX, cSequence.GetNext());
	gcLogger.SetConfig(&sLogConfig);
	cSequence.Kill();

	//A sequence file whose next index is outside the range is refused.
	cConfig = CValueSequenceConfig::CreateRange(szDirectory, DS_No, 200, 210, 4);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cSequence.Init(cConfig));
	gcLogger.SetConfig(&sLogConfig);
	cSequence.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// A released level reserves a range for the game.  The game numbers its
// objects (and its save games' objects) inside the range, and a later
// update to the level numbers above it, so the two never collide.
//
//////////////////////////////////////////////////////////////////////////
void TestSequenceLevelAndGame(void)
{
	CSequence*	pcLevel;
	CSequence*	pcGame;
	char		szLevel[] = "Output" _FS_ "SequenceLevel";
	char		szGame[] = "Output" _FS_ "SequenceGame";
	CFileUtil	cFileUtil;
	OIndex		oiFirst;

	cFileUtil.RemoveDir(szLevel);
	cFileUtil.RemoveDir(szGame);

	pcLevel = CSequenceFactory::Create(szLevel, DS_No);
	AssertNotNull(pcLevel);
	AssertLong(1LL, pcLevel->GetNext());
	AssertLong(2LL, pcLevel->GetNext());
	oiFirst = pcLevel->ReserveRange(1000);
	AssertLong(3LL, oiFirst);
	SafeKill(pcLevel);

	pcGame = CSequenceFactory::CreateRange(szGame, DS_No, oiFirst, oiFirst + 1000);
	AssertNotNull(pcGame);
	AssertLong(3LL, pcGame->GetNext());
	AssertLong(4LL, pcGame->GetNext());
	SafeKill(pcGame);

	pcLevel = CSequenceFactory::Create(szLevel, DS_No);
	AssertLong(1003LL, pcLevel->GetNext());
	SafeKill(pcLevel);

	pcGame = CSequenceFactory::CreateRange(szGame, DS_No, oiFirst, oiFirst + 1000);
	AssertLong(5LL, pcGame->GetNext());
	SafeKill(pcGame);

	cFileUtil.RemoveDir(szLevel);
	cFileUtil.RemoveDir(szGame);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSequence(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestSequenceNext();
	TestSequenceCrashRecovery(false);
	TestSequenceCrashRecovery(true);
	TestSequenceBlocks();
	TestSequenceReserveRange();
	TestSequenceRange();
	TestSequenceLevelAndGame();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

