#include "BaseLib/TypeNames.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/ChannelPadder.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOptimalPadding(void)
{
	CChannels		cChannels;
	CChannelPadder	cPadder;
	CChannel*		cChannel;

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(0, PT_char8);
	cChannels.AddChannel(1, PT_int32);
	cChannels.AddChannel(2, PT_int64);
	cChannels.EndChange();

	cPadder.Init(&cChannels);
	cPadder.OptimalPadding();

	cChannel = cChannels.GetChannelAtIndex(0);
	AssertSize(0, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(1);
	AssertSize(4, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(2);
	AssertSize(8, cChannel->miByteOffset);
	AssertSize(16, cChannels.GetByteStride());

	cChannels.Kill();

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(0, PT_int64);
	cChannels.AddChannel(1, PT_int32);
	cChannels.AddChannel(2, PT_char8);
	cChannels.EndChange();

	cPadder.Init(&cChannels);
	cPadder.OptimalPadding();

	cChannel = cChannels.GetChannelAtIndex(0);
	AssertSize(0, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(1);
	AssertSize(8, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(2);
	AssertSize(12, cChannel->miByteOffset);
	AssertSize(16, cChannels.GetByteStride());

	cChannels.Kill();

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(0, PT_char8);
	cChannels.AddChannel(1, PT_char8);
	cChannels.AddChannel(2, PT_char8);
	cChannels.AddChannel(3, PT_char8);
								   
	cChannels.AddChannel(4, PT_char8);
	cChannels.AddChannel(5, PT_char8);
	cChannels.AddChannel(6, PT_char8);
	cChannels.AddChannel(7, PT_char8);

	cChannels.AddChannel(8, PT_int64);
	cChannels.EndChange();

	cPadder.Init(&cChannels);
	cPadder.OptimalPadding();

	cChannel = cChannels.GetChannelAtIndex(0);
	AssertSize(0, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(1);
	AssertSize(1, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(2);
	AssertSize(2, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(3);
	AssertSize(3, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(4);
	AssertSize(4, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(5);
	AssertSize(5, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(6);
	AssertSize(6, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(7);
	AssertSize(7, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(8);
	AssertSize(8, cChannel->miByteOffset);
	AssertSize(16, cChannels.GetByteStride());

	cChannels.Kill();

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(0, PT_char8);
	cChannels.AddChannel(1, PT_int64);
	cChannels.AddChannel(2, PT_char8);
	cChannels.AddChannel(3, PT_int64);
	cChannels.EndChange();

	cPadder.Init(&cChannels);
	cPadder.OptimalPadding();

	cChannel = cChannels.GetChannelAtIndex(0);
	AssertSize(0, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(1);
	AssertSize(8, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(2);
	AssertSize(16, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(3);
	AssertSize(24, cChannel->miByteOffset);
	AssertSize(32, cChannels.GetByteStride());

	cChannels.Kill();

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(0, PT_char8);
	cChannels.AddChannel(1, PT_int16);
	cChannels.AddChannel(2, PT_char8);
	cChannels.AddChannel(3, PT_int16);
	cChannels.AddChannel(4, PT_int64);
	cChannels.AddChannel(5, PT_int16);
	cChannels.AddChannel(6, PT_char8);
	cChannels.AddChannel(7, PT_int32);
	cChannels.AddChannel(8, PT_int64);
	cChannels.EndChange();

	cPadder.Init(&cChannels);
	cPadder.OptimalPadding();

	cChannel = cChannels.GetChannelAtIndex(0);
	AssertSize(0, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(1);
	AssertSize(2, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(2);
	AssertSize(4, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(3);
	AssertSize(6, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(4);
	AssertSize(8, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(5);
	AssertSize(16, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(6);
	AssertSize(18, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(7);
	AssertSize(20, cChannel->miByteOffset);
	cChannel = cChannels.GetChannelAtIndex(8);
	AssertSize(24, cChannel->miByteOffset);
	AssertSize(32, cChannels.GetByteStride());

	cChannels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalChannelPadder(void)
{
	BeginTests();

	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();

	TestOptimalPadding();

	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();

	TestStatistics();
}

