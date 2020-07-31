#include "IndexTreeFileKeyDiagnosticLoggingCallback.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Init(void)
{
	CIndexTreeFileKeyDiagnosticCallback::Init();
	szLog.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Kill(void)
{
	szLog.Kill();
	CIndexTreeFileKeyDiagnosticCallback::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	Log("Put", pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Get(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	Log("Get", pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Remove(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	Log("Remove", pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Flush(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	Log("Flush", pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Evict(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	Log("Evict", pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Log(char* szOperation, void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	szLog.Append(szOperation);
	szLog.Append(":");
	szLog.Append(' ' , 7 - (int)strlen(szOperation));
	szLog.AppendData2((char*)pvKey, iKeySize);
	szLog.Append(" [");
	szLog.Append(iDataSize);
	szLog.Append("]");
	szLog.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Print(CChars* psz)
{
	psz->Append(szLog);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileKeyDiagnosticLoggingCallback::Dump(void)
{
	szLog.Dump();
}

