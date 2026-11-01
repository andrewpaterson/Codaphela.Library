#include "BaseLib/LeakDetector.h"
#include "TestLib/Assert.h"


void TestStackPointers(void);
void TestArrayType(void);
void TestArrayUnknown(void);
void TestUnknowns(void);
void TestMapStringUnknown(void);
void TestMapUnknownUnknown(void);
void TestIndexUnknown(void);
void TestSetType(void);
void TestSetUnknown(void);
void TestChannelsAccessor(void);
void TestExternalChannelPadder(void);
void TestChannels(void);
void TestExternalChannels(void);
void TestSingleChannelAccessor(void);
void TestCompressionZLib(void);
void TestActionEvent(void);
void TestEvent(void);
void TestObjectGraphSerialiser(void);
void TestPointer(void);
void TestObjectConverter(void);
void TestObject(void);
void TestObjectStack(void);
void TestObjectStackPointers(void);
void TestReplaceAndRemap(void);
void TestHollowObject(void);
void TestNamedObject(void);
void TestObjectsSimple(void);
void TestRoot(void);
void TestDistToRoot(void);
void TestDistToStack(void);
void TestObjectGraphWriting(void);
void TestArrayCommonObject(void);
void TestIndexObject(void);
void TestMapObject(void);
void TestParameters(void);
void TestObjectDirty(void);
void TestObjectAllocator(void);
void TestString(void);
void TestStringOverride(void);
void TestClass(void);
void TestChunkFileObjectWriter(void);
void TestObjectReaderChunked(void);
void TestObjectGraphDeserialiser(void);
void TestObjectReaderSimple(void);
void TestObjects(void);
void TestObjectsEvictVsKill(void);
void TestDehollowfication(void);
void TestKill(void);
void TestUsage(void);
void TestArrayObject(void);
void TestSetObject(void);
void TestEmbedded(void);
void TestEmbeddedObjectRemapTos(void);
void TestEmbeddedStackPointers(void);
void TestFreeEmbedded(void);
void TestIndexTreeMemory(void);
void TestStackEmbeddedReInit(void);
void TestObjectFree(void);
void TestObjectFreeOrdering(void);
void TestFreeObjects(void);
void TestOrderer(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(void)
{
	int iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();
	InitAllocatorDebug(2 MB);
	
	NumberInit();

	TestIndexTreeMemory();
	TestUnknowns();
	TestArrayUnknown();
	TestSetUnknown();
	TestArrayType();
	TestSetType();
	TestMapStringUnknown();
	TestMapUnknownUnknown();
	TestIndexUnknown();
	TestEvent();
	TestActionEvent();
	TestParameters();
	TestClass();
	TestDistToRoot();
	TestDistToStack();
	TestObjectsSimple();
	TestRoot();
	TestObjectGraphWriting();
	TestObject();
	TestObjectStack();
	TestPointer();
	TestFreeEmbedded();
	TestStringOverride();
	TestReplaceAndRemap();
	TestHollowObject();
	TestNamedObject();
	TestObjectDirty();
	TestObjectAllocator();
	TestChunkFileObjectWriter();
	TestObjectGraphSerialiser();
	TestObjectReaderSimple();
	TestObjectReaderChunked();
	TestObjectStackPointers();
	TestObjectGraphDeserialiser();
	TestObjectConverter();
	TestObjects();
	TestStackPointers();
	TestDehollowfication();
	TestString();
	TestObjectsEvictVsKill();
	TestUsage();
	TestArrayCommonObject();
	TestArrayObject();
	TestSetObject();
	TestIndexObject();
	TestMapObject();
	TestKill();
	TestEmbedded();
	TestEmbeddedObjectRemapTos();
	TestEmbeddedStackPointers();
	TestStackEmbeddedReInit();
	TestObjectFree();
	TestObjectFreeOrdering();
	TestFreeObjects();
	TestOrderer();
	TestExternalChannels();
	TestExternalChannelPadder();
	TestChannels();
	TestChannelsAccessor();
	TestSingleChannelAccessor();
	TestCompressionZLib();

	NumberKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

