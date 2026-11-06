#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/DataMemory.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/LayeredConnection.h"
#include "TestLib/Assert.h"


#define LAYERED_DIRECTORY	"Output" _FS_ "LayeredConnection"
#define LEVEL_DIRECTORY		LAYERED_DIRECTORY _FS_ "Level"
#define LEVEL_SEQUENCE		LAYERED_DIRECTORY _FS_ "LevelSequence"
#define WRITABLE_DIRECTORY	LAYERED_DIRECTORY _FS_ "Writable"
#define REMOVED_DIRECTORY	LAYERED_DIRECTORY _FS_ "Removed"
#define GAME_SEQUENCE		LAYERED_DIRECTORY _FS_ "GameSequence"


//////////////////////////////////////////////////////////////////////////
//
// The level: a Door, an unnamed Rock and a Tree, numbered 1 to 3 by the
// level's own sequence.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredConnectionWriteLevel(void)
{
	CCodabase*	pcLevel;
	CSequence*	pcSequence;

	pcSequence = CSequenceFactory::Create(LEVEL_SEQUENCE, DS_No);
	pcLevel = CCodabaseFactory::Create(LEVEL_DIRECTORY, IWT_No, DS_No);
	AssertTrue(pcLevel->Open());
	AssertTrue(pcLevel->Add(pcSequence->GetNext(), "Door", (void*)"closed", 7));
	AssertTrue(pcLevel->Add(pcSequence->GetNext(), (void*)"rock", 5));
	AssertTrue(pcLevel->Add(pcSequence->GetNext(), "Tree", (void*)"oak", 4));
	AssertTrue(pcLevel->Close());
	SafeKill(pcLevel);
	SafeKill(pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertLayeredString(const char* szExpected, CDataReader* pcReader, OIndex oi)
{
	char	szData[64];

	szData[0] = '\0';
	AssertTrue(pcReader->Get(oi, NULL, szData, 64));
	AssertString(szExpected, szData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 TestLayeredConnectionNumIndices(CDataReader* pcReader)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	int64					iCount;

	iCount = 0;
	oi = pcReader->StartIndexIteration(&sIter, NULL, NULL, 0);
	while (oi != INVALID_O_INDEX)
	{
		iCount++;
		oi = pcReader->IndexIterate(&sIter, NULL, NULL, 0);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The names the connection resolves, sorted, separated by commas.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredConnectionNames(CDataReader* pcReader, CChars* pszNames)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	char					szName[MAX_KEY_SIZE + 1];
	CArrayChars				aszNames;
	bool					bExists;
	size					i;

	aszNames.Init();
	bExists = pcReader->StartNameIteration(&sIter, szName, &oi);
	while (bExists)
	{
		AssertLong(oi, pcReader->GetIndex(szName));
		aszNames.Add(szName);
		bExists = pcReader->NameIterate(&sIter, szName, &oi);
	}
	aszNames.QuickSort(false);
	pszNames->Init();
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		if (i != 0)
		{
			pszNames->Append(',');
		}
		pszNames->Append(aszNames.Get(i));
	}
	aszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertLayeredNames(const char* szExpected, CDataReader* pcReader)
{
	CChars	szNames;

	TestLayeredConnectionNames(pcReader, &szNames);
	AssertString(szExpected, szNames.Text());
	szNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads fall through to the level, writes go to the writable layer, a
// changed level object keeps its OIndex and name, removed objects stay
// removed, and renamed objects lose their old names.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredConnectionReadsAndWrites(void)
{
	CFileUtil			cFileUtil;
	CCodabase*			pcLevel;
	CSequence*			pcLevelSequence;
	CCodabase*			pcWritable;
	CCodabase*			pcRemoved;
	CSequence*			pcGameSequence;
	CLayeredConnection	cLayered;
	SLogConfig			sLogConfig;
	OIndex				oiRabbit;
	lsize				uiSize;

	cFileUtil.RemoveDir(LAYERED_DIRECTORY);
	TestLayeredConnectionWriteLevel();

	pcLevelSequence = CSequenceFactory::Create(LEVEL_SEQUENCE, DS_No);
	pcLevel = CCodabaseFactory::Create(LEVEL_DIRECTORY, IWT_No, DS_No);
	pcWritable = CCodabaseFactory::Create(WRITABLE_DIRECTORY, IWT_No, DS_No);
	pcRemoved = CCodabaseFactory::CreateSmall(REMOVED_DIRECTORY, IWT_No, DS_No);
	pcGameSequence = CSequenceFactory::Create(GAME_SEQUENCE, DS_No);
	AssertTrue(pcWritable->Open());
	AssertTrue(pcRemoved->Open());

	cLayered.Init(pcWritable, pcRemoved, pcGameSequence);
	AssertTrue(cLayered.AddReadOnly(pcLevel, pcLevelSequence));
	AssertSize(2, cLayered.NumLayers());
	AssertLong(4LL, pcGameSequence->PeekNext());

	//Reads fall through to the level.
	AssertLayeredString("closed", &cLayered, 1);
	AssertLayeredString("rock", &cLayered, 2);
	AssertLong(1LL, cLayered.GetIndex("Door"));
	AssertLong(3LL, cLayered.GetIndex("Tree"));
	AssertTrue(cLayered.Contains(2));
	AssertFalse(cLayered.Contains(4));
	AssertLong(3, TestLayeredConnectionNumIndices(&cLayered));
	AssertLayeredNames("Door,Tree", &cLayered);

	//A changed level object is copied up with its name and keeps its OIndex.  The level is unchanged.
	AssertTrue(cLayered.Put(1, (void*)"opened", 7));
	AssertLayeredString("opened", &cLayered, 1);
	AssertLayeredString("opened", pcWritable, 1);
	AssertLayeredString("closed", pcLevel, 1);
	AssertLong(1LL, cLayered.GetIndex("Door"));
	AssertLong(1LL, pcWritable->GetIndex("Door"));

	//Set without a size keeps the level object's size.
	AssertTrue(cLayered.Set("Tree", (void*)"elm"));
	AssertLayeredString("elm", &cLayered, 3);
	AssertTrue(cLayered.Get(3, &uiSize, NULL, 0));
	AssertLSize(4, uiSize);

	//New objects number above the level.
	oiRabbit = pcGameSequence->GetNext();
	AssertLong(4LL, oiRabbit);
	AssertTrue(cLayered.Add(oiRabbit, "Rabbit", (void*)"hop", 4));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cLayered.Add(2, (void*)"pebble", 7));
	AssertFalse(cLayered.Add(pcGameSequence->GetNext(), "Door", (void*)"another", 8));
	gcLogger.SetConfig(&sLogConfig);
	AssertLong(4, TestLayeredConnectionNumIndices(&cLayered));
	AssertLayeredNames("Door,Rabbit,Tree", &cLayered);

	//Removing a level object leaves a tombstone.  The level still holds it.
	AssertTrue(cLayered.Remove(2));
	AssertFalse(cLayered.Contains(2));
	AssertFalse(cLayered.Get(2, NULL, NULL, 0));
	AssertTrue(pcRemoved->Contains(2));
	AssertTrue(pcLevel->Contains(2));
	AssertFalse(cLayered.Remove(2));

	//Removing a copied-up object by name removes it from both layers, and its name with it.
	AssertTrue(cLayered.Remove("Door"));
	AssertFalse(cLayered.Contains(1));
	AssertFalse(pcWritable->Contains(1));
	AssertFalse(cLayered.Contains("Door"));
	AssertLong(INVALID_O_INDEX, cLayered.GetIndex("Door"));

	//Removing an object only the writable layer holds leaves no tombstone.
	AssertTrue(cLayered.Remove(oiRabbit));
	AssertFalse(cLayered.Contains(oiRabbit));
	AssertFalse(pcRemoved->Contains(oiRabbit));

	//An object keeps its name: Put with another name is refused, as it is by a Codabase.
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cLayered.Put(3, "Birch", (void*)"birch", 6));
	AssertFalse(cLayered.Put(2, "Tree", (void*)"birch", 6));
	gcLogger.SetConfig(&sLogConfig);
	AssertTrue(cLayered.Put(3, "Tree", (void*)"birch", 6));
	AssertLayeredString("birch", &cLayered, 3);
	AssertLayeredNames("Tree", &cLayered);

	//Writing a removed object brings it back.
	AssertTrue(cLayered.Put(2, (void*)"boulder", 8));
	AssertTrue(cLayered.Contains(2));
	AssertFalse(pcRemoved->Contains(2));
	AssertLayeredString("boulder", &cLayered, 2);

	//Written back without its name, the level's name for it no longer finds it.
	AssertTrue(cLayered.Put(1, (void*)"gone", 5));
	AssertTrue(cLayered.Contains(1));
	AssertLong(INVALID_O_INDEX, cLayered.GetIndex("Door"));
	AssertLong(1LL, pcLevel->GetIndex("Door"));
	AssertLayeredNames("Tree", &cLayered);
	AssertLong(3, TestLayeredConnectionNumIndices(&cLayered));
	AssertTrue(cLayered.Remove(1));

	AssertTrue(cLayered.Remove(2));
	cLayered.Kill();
	AssertTrue(pcWritable->Close());
	AssertTrue(pcRemoved->Close());
	SafeKill(pcWritable);
	SafeKill(pcRemoved);
	SafeKill(pcGameSequence);

	//The layers are as they were left when they are opened again.
	pcWritable = CCodabaseFactory::Create(WRITABLE_DIRECTORY, IWT_No, DS_No);
	pcRemoved = CCodabaseFactory::CreateSmall(REMOVED_DIRECTORY, IWT_No, DS_No);
	pcGameSequence = CSequenceFactory::Create(GAME_SEQUENCE, DS_No);
	AssertTrue(pcWritable->Open());
	AssertTrue(pcRemoved->Open());
	cLayered.Init(pcWritable, pcRemoved, pcGameSequence);
	AssertTrue(cLayered.AddReadOnly(pcLevel, pcLevelSequence));
	AssertLong(6LL, pcGameSequence->PeekNext());
	AssertFalse(cLayered.Contains(1));
	AssertFalse(cLayered.Contains(2));
	AssertLayeredString("birch", &cLayered, 3);
	AssertLong(3LL, cLayered.GetIndex("Tree"));
	AssertLayeredNames("Tree", &cLayered);
	AssertLong(1, TestLayeredConnectionNumIndices(&cLayered));
	AssertLong(3, TestLayeredConnectionNumIndices(pcLevel));

	cLayered.Kill();
	AssertTrue(pcWritable->Close());
	AssertTrue(pcRemoved->Close());
	SafeKill(pcWritable);
	SafeKill(pcRemoved);
	SafeKill(pcGameSequence);
	SafeKill(pcLevel);
	SafeKill(pcLevelSequence);

	cFileUtil.RemoveDir(LAYERED_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredConnection(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestLayeredConnectionReadsAndWrites();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

