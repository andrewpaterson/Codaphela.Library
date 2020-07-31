#include "BaseLib/StringHelper.h"
#include "IndexTreeFileKeyDiagnosticCountingCallback.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCountingCallback::Init(char* szKey)
{
	Init(szKey, strlen(szKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCountingCallback::Init(void* pvKey, int iKeyLength)
{
	CIndexTreeFileDiagnosticCallback::Init();
	miCount = 0;
	mpvKey = pvKey;
	miKeyLength = iKeyLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCountingCallback::Kill(void)
{
	miCount = 0;
	mpvKey = NULL;
	miKeyLength = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticCountingCallback::Evict(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	if (Matches(pvKey, iKeySize))
	{
		miCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFileKeyDiagnosticCountingCallback::Matches(void* pvKey, int iKeySize)
{
	if (miKeyLength == iKeySize)
	{
		if (memcmp(mpvKey, pvKey, iKeySize) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

