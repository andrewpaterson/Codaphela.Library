//The parts of CArrayBlock the CArrayTemplate tests call, on a fixed buffer.  The real ones are in ArrayBlock.cpp,
//which needs the rest of BaseLib and the C library to run.  Include it in one translation unit: it instantiates them
//(ArrayBlock.h declares them extern).
#include "BaseLib/ArrayBlock.h"


char	gacArrayBuffer[1024];


template<class SIZE>
void CArrayBlockBase<SIZE>::Init(typename SIZE::Count iElementSize)
{
	mpcMalloc = NULL;
	miElementSize = iElementSize;
	miUsedElements = 0;
	miChunkSize = 1;
	miNumElements = 0;
	mpvArray = gacArrayBuffer;
}


template<class SIZE>
void CArrayBlockBase<SIZE>::Kill(void)
{
	miNumElements = 0;
	mpvArray = NULL;
}


template<class SIZE>
void* CArrayBlockBase<SIZE>::Add(void)
{
	void*	pv;

	pv = (char*)mpvArray + miNumElements * miElementSize;
	miNumElements++;
	return pv;
}


template<class SIZE>
void* CArrayBlockBase<SIZE>::Get(typename SIZE::Count iIndex)
{
	return (char*)mpvArray + iIndex * miElementSize;
}


template<class SIZE>
void* CArrayBlockBase<SIZE>::Tail(void)
{
	return Get(miNumElements - 1);
}


template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::NumElements(void)
{
	return miNumElements;
}


template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::ElementSize(void)
{
	return miElementSize;
}


template class CArrayBlockBase<SArrayNaturalSize>;

