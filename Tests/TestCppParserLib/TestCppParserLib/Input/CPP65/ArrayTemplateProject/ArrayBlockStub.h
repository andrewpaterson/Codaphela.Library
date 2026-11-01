//The parts of CArrayBlock the CArrayTemplate tests call, on a fixed buffer.  The real ones are in ArrayBlock.cpp,
//which needs the rest of BaseLib and the C library to run.  Include it in one translation unit.
#include "BaseLib/ArrayBlock.h"


char	gacArrayBuffer[1024];


void CArrayBlock::Init(size iElementSize)
{
	mpcMalloc = NULL;
	miElementSize = iElementSize;
	miUsedElements = 0;
	miChunkSize = 1;
	miNumElements = 0;
	mpvArray = gacArrayBuffer;
}


void CArrayBlock::Kill(void)
{
	miNumElements = 0;
	mpvArray = NULL;
}


void* CArrayBlock::Add(void)
{
	void*	pv;

	pv = (char*)mpvArray + miNumElements * miElementSize;
	miNumElements++;
	return pv;
}


void* CArrayBlock::Get(size iIndex)
{
	return (char*)mpvArray + iIndex * miElementSize;
}


void* CArrayBlock::Tail(void)
{
	return Get(miNumElements - 1);
}


size CArrayBlock::NumElements(void)
{
	return miNumElements;
}


size CArrayBlock::ElementSize(void)
{
	return miElementSize;
}
