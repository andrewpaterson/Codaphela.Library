#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/LayeredConnection.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/Root.h"
#include "TestLib/Assert.h"


#define LAYERED_OBJECTS_DIRECTORY	"Output" _FS_ "LayeredObjects"
#define LAYERED_LEVEL				LAYERED_OBJECTS_DIRECTORY _FS_ "Level"
#define LAYERED_LEVEL_SEQUENCE		LAYERED_OBJECTS_DIRECTORY _FS_ "LevelSequence"
#define LAYERED_WRITABLE			LAYERED_OBJECTS_DIRECTORY _FS_ "Writable"
#define LAYERED_REMOVED				LAYERED_OBJECTS_DIRECTORY _FS_ "Removed"
#define LAYERED_GAME_SEQUENCE		LAYERED_OBJECTS_DIRECTORY _FS_ "GameSequence"


//////////////////////////////////////////////////////////////////////////
//
// The artist's level: a Door and a Tree held by the root.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredObjectsWriteLevel(void)
{
	CCodabase*		pcLevel;
	CSequence*		pcSequence;
	Ptr<CRoot>		pRoot;
	Ptr<CString>	pDoor;
	Ptr<CString>	pTree;

	pcSequence = CSequenceFactory::Create(LAYERED_LEVEL_SEQUENCE, DS_No);
	pcLevel = CCodabaseFactory::Create(LAYERED_LEVEL, IWT_No, DS_No);
	AssertTrue(pcLevel->Open());
	ObjectsInit(pcLevel, pcSequence);

	pRoot = ORoot();
	pDoor = ONMalloc<CString>("Door", "closed");
	pTree = ONMalloc<CString>("Tree", "oak");
	pRoot->Add(pDoor);
	pRoot->Add(pTree);
	pDoor = NULL;
	pTree = NULL;
	pRoot = NULL;

	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(pcLevel->Close());
	SafeKill(pcLevel);
	SafeKill(pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// The game over the level: its writable and Removed Codabases and its
// sequence, opened and wound into CObjects.
//
//////////////////////////////////////////////////////////////////////////
struct STestLayeredGame
{
	CCodabase*			pcLevel;
	CSequence*			pcLevelSequence;
	CCodabase*			pcWritable;
	CCodabase*			pcRemoved;
	CSequence*			pcGameSequence;
	CLayeredConnection	cLayered;

	void Init(void)
	{
		pcLevelSequence = CSequenceFactory::Create(LAYERED_LEVEL_SEQUENCE, DS_No);
		pcLevel = CCodabaseFactory::Create(LAYERED_LEVEL, IWT_No, DS_No);
		pcWritable = CCodabaseFactory::Create(LAYERED_WRITABLE, IWT_No, DS_No);
		pcRemoved = CCodabaseFactory::CreateSmall(LAYERED_REMOVED, IWT_No, DS_No);
		pcGameSequence = CSequenceFactory::Create(LAYERED_GAME_SEQUENCE, DS_No);
		AssertTrue(pcWritable->Open());
		AssertTrue(pcRemoved->Open());
		cLayered.Init(pcWritable, pcRemoved, pcGameSequence);
		AssertTrue(cLayered.AddReadOnly(pcLevel, pcLevelSequence));
		ObjectsInit(&cLayered, cLayered.GetSequence());
	}

	void Kill(void)
	{
		AssertTrue(ObjectsFlush());
		ObjectsKill(false);
		cLayered.Kill();
		AssertTrue(pcWritable->Close());
		AssertTrue(pcRemoved->Close());
		SafeKill(pcWritable);
		SafeKill(pcRemoved);
		SafeKill(pcGameSequence);
		SafeKill(pcLevel);
		SafeKill(pcLevelSequence);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Objects read from the level, changed and added to by the game, are
// flushed into the writable layer only, and read back from the layers
// after a restart.  The level is unchanged.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredObjectsGame(void)
{
	CFileUtil			cFileUtil;
	STestLayeredGame	sGame;
	Ptr<CString>		pDoor;
	Ptr<CString>		pTree;
	Ptr<CString>		pRabbit;
	Ptr<CRoot>			pRoot;
	OIndex				oiDoor;
	OIndex				oiTree;
	OIndex				oiRabbit;
	OIndex				oiLevelNext;
	CCodabase*			pcLevel;
	CSequence*			pcLevelSequence;

	cFileUtil.RemoveDir(LAYERED_OBJECTS_DIRECTORY);
	TestLayeredObjectsWriteLevel();

	sGame.Init();
	oiLevelNext = sGame.pcLevelSequence->PeekNext();
	AssertLong(0, sGame.pcWritable->NumIndices());

	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNotNull());
	AssertString("closed", pDoor->Text());
	oiDoor = pDoor->GetIndex();
	pTree = gcObjects.Get<CString>("Tree");
	AssertString("oak", pTree->Text());
	oiTree = pTree->GetIndex();

	pDoor->Set("opened");
	pRabbit = ONMalloc<CString>("Rabbit", "hop");
	oiRabbit = pRabbit->GetIndex();
	AssertTrue(oiRabbit >= oiLevelNext);
	pRoot = ORoot();
	pRoot->Add(pRabbit);

	pDoor = NULL;
	pTree = NULL;
	pRabbit = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());

	//Only the changed and new objects are in the writable layer.  The level is unchanged.
	AssertTrue(sGame.pcWritable->Contains(oiDoor));
	AssertTrue(sGame.pcWritable->Contains(oiRabbit));
	AssertFalse(sGame.pcWritable->Contains(oiTree));
	AssertFalse(sGame.pcLevel->Contains(oiRabbit));
	AssertLong(oiDoor, sGame.pcWritable->GetIndex("Door"));
	sGame.Kill();

	//The game again: the changes come back from the writable layer, the rest from the level.
	sGame.Init();
	pDoor = gcObjects.Get<CString>("Door");
	AssertString("opened", pDoor->Text());
	AssertLong(oiDoor, pDoor->GetIndex());
	pRabbit = gcObjects.Get<CString>("Rabbit");
	AssertTrue(pRabbit.IsNotNull());
	AssertString("hop", pRabbit->Text());
	AssertLong(oiRabbit, pRabbit->GetIndex());
	pTree = gcObjects.Get<CString>("Tree");
	AssertString("oak", pTree->Text());
	pRoot = ORoot();
	AssertLong(3, pRoot->NumObjects());
	pDoor = NULL;
	pTree = NULL;
	pRabbit = NULL;
	pRoot = NULL;
	sGame.Kill();

	//The level by itself is as the artist left it.
	pcLevelSequence = CSequenceFactory::Create(LAYERED_LEVEL_SEQUENCE, DS_No);
	pcLevel = CCodabaseFactory::Create(LAYERED_LEVEL, IWT_No, DS_No);
	AssertTrue(pcLevel->Open());
	ObjectsInit(pcLevel, pcLevelSequence);
	pDoor = gcObjects.Get<CString>("Door");
	AssertString("closed", pDoor->Text());
	pRabbit = gcObjects.Get<CString>("Rabbit");
	AssertTrue(pRabbit.IsNull());
	pRoot = ORoot();
	AssertLong(2, pRoot->NumObjects());
	pDoor = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(pcLevel->Close());
	SafeKill(pcLevel);
	SafeKill(pcLevelSequence);

	cFileUtil.RemoveDir(LAYERED_OBJECTS_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredObjects(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestLayeredObjectsGame();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

