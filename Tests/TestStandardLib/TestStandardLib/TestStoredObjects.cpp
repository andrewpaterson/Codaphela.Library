#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/Root.h"
#include "TestLib/Assert.h"


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
//
//////////////////////////////////////////////////////////////////////////
void TestStoredObjects(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestStoredObjectsWithoutPathToRoot();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

