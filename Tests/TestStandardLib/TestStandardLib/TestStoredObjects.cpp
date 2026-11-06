#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/Root.h"
#include "TestLib/Assert.h"
#include "ChunkFileObjectWriterTestClasses.h"


#define STORED_OBJECTS_DIRECTORY	"Output" _FS_ "StoredObjects"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
struct STestStoredObjects
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;

	void Init(void)
	{
		pcSequence = CSequenceFactory::Create(STORED_OBJECTS_DIRECTORY, DS_No);
		pcDatabase = CCodabaseFactory::Create(STORED_OBJECTS_DIRECTORY, IWT_No, DS_No);
		AssertTrue(pcDatabase->Open());
		ObjectsInit(pcDatabase, pcSequence);
	}

	void Kill(void)
	{
		ObjectsKill(false);
		AssertTrue(pcDatabase->Close());
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The root holds a Door, saved to the database.
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjectsWriteDoor(void)
{
	STestStoredObjects	sStore;
	Ptr<CRoot>			pRoot;
	Ptr<CString>		pDoor;

	sStore.Init();
	pRoot = ORoot();
	pDoor = ONMalloc<CString>("Door", "closed");
	AssertFalse(pDoor->IsStored());
	pRoot->Add(pDoor);
	pDoor = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());
	sStore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertStoredDoor(const char* szExpected)
{
	STestStoredObjects	sStore;
	Ptr<CString>		pDoor;

	sStore.Init();
	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNotNull());
	AssertString(szExpected, pDoor->Text());
	pDoor = NULL;
	AssertTrue(ObjectsFlush());
	sStore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object loaded by name alone (its holder, the root, is only in the
// database) has no path to the root.  It is stored, so Flush saves it and
// freeing it saves it first.  An object that never reached the database
// is freed without being saved.
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjectsWithoutPathToRoot(void)
{
	CFileUtil			cFileUtil;
	STestStoredObjects	sStore;
	Ptr<CString>		pDoor;
	Ptr<CString>		pTransient;
	int64				iNumIndices;

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
	TestStoredObjectsWriteDoor();

	//Flush saves a changed stored object without a path to the root.
	sStore.Init();
	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNotNull());
	AssertTrue(pDoor->IsStored());
	AssertFalse(pDoor->IsDirty());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pDoor->GetDistToRoot());
	pDoor->Set("opened");
	AssertTrue(pDoor->IsDirty());
	AssertTrue(ObjectsFlush());
	AssertFalse(pDoor->IsDirty());
	pDoor = NULL;
	sStore.Kill();
	AssertStoredDoor("opened");

	//Freeing a changed stored object saves it first.
	sStore.Init();
	pDoor = gcObjects.Get<CString>("Door");
	pDoor->Set("shut");
	pDoor = NULL;
	AssertLong(0, gcObjects.NumMemoryIndexes());
	sStore.Kill();
	AssertStoredDoor("shut");

	//An object that never reached the database is not saved when it is freed.
	sStore.Init();
	iNumIndices = sStore.pcDatabase->NumIndices();
	pTransient = OMalloc<CString>("transient");
	AssertFalse(pTransient->IsStored());
	AssertTrue(pTransient->IsDirty());
	pTransient = NULL;
	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertLong(iNumIndices, sStore.pcDatabase->NumIndices());

	//Saving an object makes it stored.
	pTransient = OMalloc<CString>("saved");
	AssertTrue(gcObjects.ForceSave((CBaseObject*)pTransient.Object()));
	AssertTrue(pTransient->IsStored());
	pTransient = NULL;
	AssertLong(iNumIndices + 1, sStore.pcDatabase->NumIndices());
	sStore.Kill();

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
// Evicting saves the dirty objects that are stored or can find the root
// first, and drops unreachable objects that never reached the database.
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjectsEvict(void)
{
	CFileUtil			cFileUtil;
	STestStoredObjects	sStore;
	Ptr<CString>		pDoor;
	Ptr<CString>		pWindow;
	Ptr<CString>		pTransient;
	Ptr<CRoot>			pRoot;
	int64				iNumIndices;

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
	TestStoredObjectsWriteDoor();

	sStore.Init();
	iNumIndices = sStore.pcDatabase->NumIndices();

	//Stored, with no path to the root.
	pDoor = gcObjects.Get<CString>("Door");
	pDoor->Set("ajar");

	//Rooted, never saved.
	pRoot = ORoot();
	pWindow = ONMalloc<CString>("Window", "open");
	pRoot->Add(pWindow);

	//Unreachable, never saved.
	pTransient = OMalloc<CString>("transient");

	pDoor = NULL;
	pWindow = NULL;
	pRoot = NULL;
	AssertTrue(gcObjects.EvictInMemory());
	pTransient = NULL;
	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertLong(iNumIndices + 1, sStore.pcDatabase->NumIndices());

	pWindow = gcObjects.Get<CString>("Window");
	AssertTrue(pWindow.IsNotNull());
	AssertString("open", pWindow->Text());
	pRoot = ORoot();
	AssertLong(2, pRoot->NumObjects());
	pWindow = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());
	sStore.Kill();
	AssertStoredDoor("ajar");

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
// ObjectsFlush or ObjectsDiscard comes before ObjectsKill: a dirty object
// that Flush would save (stored, or able to find the root) is reported.
// Discarding throws the changes away.
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjectsKillChecksDirty(void)
{
	CFileUtil			cFileUtil;
	STestStoredObjects	sStore;
	Ptr<CString>		pDoor;
	Ptr<CString>		pLamp;
	Ptr<CString>		pTransient;
	Ptr<CRoot>			pRoot;
	SLogConfig			sLogConfig;

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
	TestStoredObjectsWriteDoor();

	//A dirty stored object with no path to the root, discarded.
	sStore.Init();
	AssertTrue(gcObjects.ValidateNoDirty());
	pDoor = gcObjects.Get<CString>("Door");
	pDoor->Set("discarded");
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(gcObjects.ValidateNoDirty());
	gcLogger.SetConfig(&sLogConfig);
	ObjectsDiscard();
	AssertFalse(pDoor->IsDirty());
	AssertTrue(gcObjects.ValidateNoDirty());
	pDoor = NULL;

	//A dirty object held only on the stack, that never reached the database, is not one Flush would save.
	pTransient = OMalloc<CString>("transient");
	AssertTrue(pTransient->IsDirty());
	AssertTrue(gcObjects.ValidateNoDirty());
	pTransient = NULL;

	//A dirty rooted object, flushed.
	pRoot = ORoot();
	pLamp = ONMalloc<CString>("Lamp", "on");
	pRoot->Add(pLamp);
	pLamp = NULL;
	pRoot = NULL;
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(gcObjects.ValidateNoDirty());
	gcLogger.SetConfig(&sLogConfig);
	AssertTrue(ObjectsFlush());
	AssertTrue(gcObjects.ValidateNoDirty());
	sStore.Kill();

	AssertStoredDoor("closed");

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
// Destroy deletes an object from the database.  Its holders are loaded
// (here none is in memory), their pointers to it cleared and saved.  The
// objects it held are left in the database.  A pointer to an object that
// is not in the database is a programming error.
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjectsDestroy(void)
{
	CFileUtil				cFileUtil;
	STestStoredObjects		sStore;
	Ptr<CString>			pDoor;
	Ptr<CString>			pLamp;
	Ptr<CString>			pGhost;
	Ptr<CTestNamedString>	pHolder;
	Ptr<CTestNamedString>	pLampHolder;
	Ptr<CTestNamedString>	pGhostHolder;
	Ptr<CRoot>				pRoot;
	OIndex					oiDoor;
	OIndex					oiGhost;
	SLogConfig				sLogConfig;

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);

	//The root holds Door, a Holder of Door, a LampHolder of Lamp and a GhostHolder of Ghost.
	sStore.Init();
	gcObjects.AddConstructor<CTestNamedString>();
	pRoot = ORoot();
	pDoor = ONMalloc<CString>("Door", "closed");
	pLamp = ONMalloc<CString>("Lamp", "on");
	pGhost = ONMalloc<CString>("Ghost", "boo");
	pHolder = ONMalloc<CTestNamedString>("Holder", pDoor, Null(), "holder");
	pLampHolder = ONMalloc<CTestNamedString>("LampHolder", pLamp, Null(), "lamp holder");
	pGhostHolder = ONMalloc<CTestNamedString>("GhostHolder", pGhost, Null(), "ghost holder");
	pRoot->Add(pDoor);
	pRoot->Add(pHolder);
	pRoot->Add(pLampHolder);
	pRoot->Add(pGhostHolder);
	oiDoor = pDoor->GetIndex();
	oiGhost = pGhost->GetIndex();
	pDoor = NULL;
	pLamp = NULL;
	pGhost = NULL;
	pHolder = NULL;
	pLampHolder = NULL;
	pGhostHolder = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());
	sStore.Kill();

	//Door is loaded by name alone: none of its holders is in memory.
	sStore.Init();
	gcObjects.AddConstructor<CTestNamedString>();
	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNotNull());
	AssertTrue(pDoor->Destroy());
	AssertFalse(sStore.pcDatabase->Contains(oiDoor));
	AssertFalse(sStore.pcDatabase->Contains("Door"));
	pHolder = gcObjects.Get<CTestNamedString>("Holder");
	AssertTrue(pHolder->mszString.IsNull());
	AssertFalse(pHolder->IsDirty());
	pRoot = ORoot();
	AssertLong(3, pRoot->NumObjects());

	//Destroying LampHolder leaves Lamp in the database.
	pLampHolder = gcObjects.Get<CTestNamedString>("LampHolder");
	AssertTrue(pLampHolder->Destroy());
	AssertLong(2, pRoot->NumObjects());
	AssertTrue(sStore.pcDatabase->Contains("Lamp"));
	AssertFalse(sStore.pcDatabase->Contains("LampHolder"));
	pDoor = NULL;
	pHolder = NULL;
	pLampHolder = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());
	sStore.Kill();

	sStore.Init();
	gcObjects.AddConstructor<CTestNamedString>();
	pRoot = ORoot();
	AssertLong(2, pRoot->NumObjects());
	pDoor = gcObjects.Get<CString>("Door");
	AssertTrue(pDoor.IsNull());
	pHolder = gcObjects.Get<CTestNamedString>("Holder");
	AssertTrue(pHolder->mszString.IsNull());
	pLamp = gcObjects.Get<CString>("Lamp");
	AssertString("on", pLamp->Text());

	//Ghost removed from the database by hand: GhostHolder's pointer to it is an error.
	AssertTrue(sStore.pcDatabase->Remove(oiGhost));
	pGhostHolder = gcObjects.Get<CTestNamedString>("GhostHolder");
	sLogConfig = gcLogger.SetSilent();
	AssertNull(pGhostHolder->mszString.Dereference());
	gcLogger.SetConfig(&sLogConfig);

	pHolder = NULL;
	pLamp = NULL;
	pGhostHolder = NULL;
	pRoot = NULL;
	ObjectsDiscard();
	sStore.Kill();

	cFileUtil.RemoveDir(STORED_OBJECTS_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjects(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestStoredObjectsWithoutPathToRoot();
	TestStoredObjectsEvict();
	TestStoredObjectsKillChecksDirty();
	TestStoredObjectsDestroy();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

