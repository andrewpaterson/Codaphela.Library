#include "IndexPtr.h"



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void* CIndexPtr::GetPtr(uint8* pvKey, size iKeySize)
{
	void**	ppvValue;

	ppvValue = CIndexTemplate<void*>::Get(pvKey, iKeySize);
	if (ppvValue)
	{
		return *ppvValue;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void** CIndexPtr::Put(uint8* pvKey, size iKeySize)
{
	void** ppvValue;

	ppvValue = CIndexTemplate<void*>::Put(pvKey, iKeySize);
	return ppvValue;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexPtr::Put(uint8* pvKey, size iKeySize, void* pvData)
{
	return CIndexTemplate<void*>::Put(pvKey, iKeySize, &pvData);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexPtr::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData)
{
	return CIndexBlock::StartIteration(psIterator, ppvData, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexPtr::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData)
{
	return CIndexBlock::Iterate(psIterator, ppvData, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexPtr::StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData)
{
	return CIndexBlock::StartIteration(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, ppvData, NULL, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexPtr::Iterate(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData)
{
	return CIndexBlock::Iterate(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, ppvData, NULL, sizeof(void*));
}

