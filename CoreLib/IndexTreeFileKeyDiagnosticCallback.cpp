#include "BaseLib/StringHelper.h"
#include "IndexTreeFileKeyDiagnosticCallback.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Init(char* szKey)
{
	Init(szKey, strlen(szKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Init(void* pvKey, int iKeyLength)
{
	mpvKey = pvKey;
	miKeyLength = iKeyLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Kill(void)
{
	mpvKey = NULL;
	miKeyLength = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Get(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Remove(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Flush(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Read(void* pvKey, int iKeySize, void* pvData, int iDataSize, void* pvBuffer, int iBufferSize)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCallback::Write(void* pvKey, int iKeySize, void* pvData, int iDataSize, void* pvBuffer, int iBufferSize)
{
}

