#include "IndexTreeCallbackTestObject.h"
#include "IndexTreeFileDataTestCallback.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size	CIndexTreeFileDataTestCallback::IndexTreeDataSize(size uiSourceSize)
{
	if (uiSourceSize != 0)
	{
		return sizeof(STestIndexTreeFileObject);
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileDataTestCallback::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileDataSize, size uiSourceDataSize)
{
	STestIndexTreeFileObject*		psFileObject;
	CTestIndexTreeMemoryObject*		pcMemoryObject;
	
	psFileObject = (STestIndexTreeFileObject*)pvDataBuffer;
	pcMemoryObject = (CTestIndexTreeMemoryObject*)pvSource;

	psFileObject->Init(pcMemoryObject->mi2, pcMemoryObject->mi4);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileDataTestCallback::IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestDataSize, size iFileDataSize)
{
	STestIndexTreeFileObject*		psFileObject;
	CTestIndexTreeMemoryObject*		pcMemoryObject;

	psFileObject = (STestIndexTreeFileObject*)pvDataBuffer;
	pcMemoryObject = (CTestIndexTreeMemoryObject*)pvDest;

	pcMemoryObject->Init(17, psFileObject->i2, 56, psFileObject->i4);

	return true;
}

