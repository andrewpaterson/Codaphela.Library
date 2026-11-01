#ifndef __SHARED_MEMORY_QUEUE_THREAD_ELEMENT_H__
#define __SHARED_MEMORY_QUEUE_THREAD_ELEMENT_H__


struct SSharedMemoryQueueElementHeader
{
	unsigned int	muiSize;
	int				miIndex;
};


struct SSharedMemoryQueueElement
{
	SSharedMemoryQueueElementHeader		sHeader;
	char								sz[256 + 1];
};


#endif // !__SHARED_MEMORY_QUEUE_THREAD_ELEMENT_H__

