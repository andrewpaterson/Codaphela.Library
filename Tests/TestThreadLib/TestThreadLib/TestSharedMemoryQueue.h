#ifndef __TEST_SHARED_MEMORY_QUEUE_H__
#define __TEST_SHARED_MEMORY_QUEUE_H__
#include "BaseLib/Chars.h"
#include "SharedMemoryQueueTheadElement.h"


#define PROCESS_SHARED_QUEUE_PRODUCE_SIZE 20000
#define THREADED_SHARED_QUEUE_PRODUCE_SIZE 45000


struct SProcessSharedQueueResult
{
	int		aiTaken[4];
	char	aszData[PROCESS_SHARED_QUEUE_PRODUCE_SIZE][257];
};


struct SThreadedSharedQueueResult
{
	int		aiTaken[4];
	char	aszData[THREADED_SHARED_QUEUE_PRODUCE_SIZE][257];
};


//The elements the queue tests produce: sizes 100 to 255 and characters 33 to
//121, each one more than the one before.
struct SQueueTestSequence
{
	int		iSize;
	char	c;

	void	Init(void);
	void	Next(void);
};


void	FillQueueElement(SSharedMemoryQueueElement* psElement, int iIndex, SQueueTestSequence* psSequence);
void	CreateExpectedString(CChars* psz, int iProduce);
void	AssertQueueResult(int* aiTaken, char (*aszData)[257], int iProduce);


#endif // !__TEST_SHARED_MEMORY_QUEUE_H__

