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
#include "ChunkFileObjectWriterTestClasses.h"


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
// A level whose root holds a Door, a Holder of the Door, a Window and a
// WindowHolder of the Window.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredObjectsWriteDestroyLevel(void)
{
	CCodabase*				pcLevel;
	CSequence*				pcSequence;
	Ptr<CRoot>				pRoot;
	Ptr<CString>			pDoor;
	Ptr<CString>			pWindow;
	Ptr<CTestNamedString>	pHolder;
	Ptr<CTestNamedString>	pWindowHolder;

	pcSequence = CSequenceFactory::Create(LAYERED_LEVEL_SEQUENCE, DS_No);
	pcLevel = CCodabaseFactory::Create(LAYERED_LEVEL, IWT_No, DS_No);
	AssertTrue(pcLevel->Open());
	ObjectsInit(pcLevel, pcSequence);
	gcObjects.AddConstructor<CTestNamedString>();

	pRoot = ORoot();
	pDoor = ONMalloc<CString>("Door", "closed");
	pWindow = ONMalloc<CString>("Window", "open");
	pHolder = ONMalloc<CTestNamedString>("Holder", pDoor, Null(), "holder");
	pWindowHolder = ONMalloc<CTestNamedString>("WindowHolder", pWindow, Null(), "window holder");
	pRoot->Add(pDoor);
	pRoot->Add(pHolder);
	pRoot->Add(pWindow);
	pRoot->Add(pWindowHolder);
	pDoor = NULL;
	pWindow = NULL;
	pHolder = NULL;
	pWindowHolder = NULL;
	pRoot = NULL;

	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(pcLevel->Close());
	SafeKill(pcLevel);
	SafeKill(pcSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// Destroying a level object through the layered connection leaves a
// tombstone in the Removed Codabase; the level is unchanged.  Its holders
// are copied up without it, so they load with a NULL pointer afterwards.
//
//////////////////////////////////////////////////////////////////////////
void TestLayeredObjectsDestroy(void)
{
	CFileUtil				cFileUtil;
	STestLayeredGame		sGame;
	Ptr<CString>			pDoor;
	Ptr<CString>			pWindow;
	Ptr<CTestNamedString>	pHolder;
	Ptr<CTestNamedString>	pWindowHolder;
	Ptr<CRoot>				pRoot;
	OIndex					oiDoor;
	OIndex					oiHolder;
	OIndex					oiWindow;

	cFileUtil.RemoveDir(LAYERED_OBJECTS_DIRECTORY);
	TestLayeredObjectsWriteDestroyLevel();

	//Door is loaded by name alone (none of its holders is in memory) and destroyed.
	sGame.Init();
	gcObjects.AddConstructor<CTestNamedString>();
	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNotNull());
	oiDoor = pDoor->GetIndex();
	oiHolder = sGame.pcLevel->GetIndex("Holder");
	AssertTrue(pDoor->Destroy());
	pDoor = NULL;

	//The tombstone: the layers no longer have Door; the level, read-only, still does.
	AssertTrue(sGame.pcRemoved->Contains(oiDoor));
	AssertFalse(sGame.cLayered.Contains(oiDoor));
	AssertFalse(sGame.cLayered.Contains("Door"));
	AssertTrue(sGame.pcLevel->Contains(oiDoor));
	AssertTrue(sGame.pcLevel->Contains("Door"));
	AssertFalse(sGame.pcWritable->Contains(oiDoor));

	//Its holders were copied up into the writable layer without it.
	AssertTrue(sGame.pcWritable->Contains(oiHolder));
	pHolder = gcObjects.Get<CTestNamedString>("Holder");
	AssertTrue(pHolder->mszString.IsNull());
	pRoot = ORoot();
	AssertLong(3, pRoot->NumObjects());
	pHolder = NULL;
	pRoot = NULL;
	sGame.Kill();

	//Loaded again, the pointers to it are NULL and it cannot be got.
	sGame.Init();
	gcObjects.AddConstructor<CTestNamedString>();
	pRoot = ORoot();
	AssertLong(3, pRoot->NumObjects());
	pHolder = gcObjects.Get<CTestNamedString>("Holder");
	AssertTrue(pHolder.IsNotNull());
	AssertTrue(pHolder->mszString.IsNull());
	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNull());
	pDoor = gcObjects.Get<CString>(oiDoor);
	AssertTrue(pDoor.IsNull());

	//Window, reached through its holder's pointer, is destroyed with the holder in memory.
	pWindowHolder = gcObjects.Get<CTestNamedString>("WindowHolder");
	pWindow = pWindowHolder->mszString;
	AssertString("open", pWindow->Text());
	oiWindow = pWindow->GetIndex();
	AssertTrue(pWindow->Destroy());
	pWindow = NULL;
	AssertTrue(pWindowHolder->mszString.IsNull());
	AssertTrue(sGame.pcRemoved->Contains(oiWindow));
	AssertTrue(sGame.pcLevel->Contains(oiWindow));
	AssertLong(2, pRoot->NumObjects());
	pHolder = NULL;
	pWindowHolder = NULL;
	pRoot = NULL;
	sGame.Kill();

	sGame.Init();
	gcObjects.AddConstructor<CTestNamedString>();
	pRoot = ORoot();
	AssertLong(2, pRoot->NumObjects());
	pWindowHolder = gcObjects.Get<CTestNamedString>("WindowHolder");
	AssertTrue(pWindowHolder->mszString.IsNull());
	AssertTrue(gcObjects.Get<CString>("Window").IsNull());
	pWindowHolder = NULL;
	pRoot = NULL;
	sGame.Kill();

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
	TestLayeredObjectsDestroy();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

