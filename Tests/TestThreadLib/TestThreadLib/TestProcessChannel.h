#ifndef __TEST_PROCESS_CHANNEL_H__
#define __TEST_PROCESS_CHANNEL_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TestTimeout.h"


//How long the tests wait for a message before failing.
#define CHANNEL_TEST_TIMEOUT		TEST_TIMEOUT

//The memory filled by TestChannelResizeMultiProcess.
#define CHANNEL_FILL_SIZE			16000
#define CHANNEL_FILL_CHUNK_SIZE		16


enum ETestTransport
{
	TT_Network,
	TT_SharedMemory,
};


enum EChannelTestMessage
{
	CTM_Hello = 1,			//Child to parent: text.
	CTM_Memory,				//Both ways: the memory to reverse and the reversed memory.

	CTM_FillRequest,		//Child to parent: wants to write the next chunk.
	CTM_FillPosition,		//Parent to child: uint32 position to write the chunk to.
	CTM_FillChunk,			//Child to parent: the chunk.
	CTM_FillStop,			//Parent to child: the memory is full.

	CTM_QueueConsumer,		//Child to parent: int process number.
	CTM_QueuePop,			//Child to parent: wants the next element.
	CTM_QueueElement,		//Parent to child: SSharedMemoryQueueElement.
	CTM_QueueEmpty,			//Parent to child: nothing to take yet, ask again.
	CTM_QueueEnd,			//Parent to child: everything has been produced and taken.
	CTM_QueueResult,		//Child to parent: the SSharedMemoryQueueElement it took.

	CTM_Large,				//Both ways: CHANNEL_LARGE_SIZE bytes, more than the receiver's buffer.
	CTM_Small,				//Both ways: CHANNEL_SMALL_TEXT, sent after CTM_Large.
	CTM_TooLargeReport,		//Child to parent: STooLargeReport.
};


//The receiver's buffer is smaller than CTM_Large.
#define CHANNEL_LARGE_SIZE		1000
#define CHANNEL_RECEIVE_SIZE	16
#define CHANNEL_SMALL_TEXT		"after"


//What the child received when the parent sent CTM_Large then CTM_Small.
struct STooLargeReport
{
	int		iLargeResult;
	uint32	uiLargeType;
	uint32	uiLargeSize;
	int		iSmallResult;
	uint32	uiSmallType;
	uint32	uiSmallSize;
	char	szSmall[CHANNEL_RECEIVE_SIZE];
};


#endif // __TEST_PROCESS_CHANNEL_H__

