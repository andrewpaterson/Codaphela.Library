#include "IndexTreeHelper.h"
#include "Codabase.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::Init(char* szDirectory, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig)
{
	CIndexTreeHelper	cHelper;

	cHelper.Init(szDirectory, "Primary", "Backup", FALSE);

	mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	mcFileController.Begin();
	mcNamedIndexedData.Init(&mcFileController, cIndexConfig, cNamedConfig);
	mcFileController.End();

	cHelper.Kill(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::Kill(void)
{
	mcNamedIndexedData.Kill();
	mcFileController.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Open(void)
{
	return mcFileController.Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Close(void)
{
	BOOL	bResult;

	if (!mcNamedIndexedData.IsWriteThrough())
	{
		bResult = Flush(FALSE);
	}
	else
	{
		bResult = TRUE;
	}
	bResult |= mcFileController.End();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::ValidateConfigInitialised(void)
{
	return mcNamedIndexedData.ValidateConfigInitialised();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::ValidateConfigKilled(void)
{
	return mcNamedIndexedData.ValidateConfigKilled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Add(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Add(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Add(oi, szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Add(oi, szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Set(OIndex oi, void* pvData)
{
	return mcNamedIndexedData.Set(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Set(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Set(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Set(char* szName, void* pvData)
{
	return mcNamedIndexedData.Set(szName, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Set(char* szName, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Set(szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Set(CChars* szName, void* pvData)
{
	return mcNamedIndexedData.Set(szName, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Set(CChars* szName, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Set(szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Put(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Put(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Put(oi, szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize)
{
	return mcNamedIndexedData.Put(oi, szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Get(OIndex oi, void* pvData)
{
	return mcNamedIndexedData.Get(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	return mcNamedIndexedData.Get(oi, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Get(char* szName, void* pvData)
{
	return mcNamedIndexedData.Get(szName, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	return mcNamedIndexedData.Get(szName, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Get(CChars* szName, void* pvData)
{
	return mcNamedIndexedData.Get(szName, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	return mcNamedIndexedData.Get(szName, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::GetName(OIndex oi, CChars* szName)
{
	return mcNamedIndexedData.GetName(oi, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength)
{
	return mcNamedIndexedData.GetName(oi, szName, puiNameLength, uiMaxNameLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::GetIndex(char* szName)
{
	return mcNamedIndexedData.GetIndex(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CCodabase::GetIndex(CChars* szName)
{
	return mcNamedIndexedData.GetIndex(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Contains(OIndex oi)
{
	return mcNamedIndexedData.Contains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Contains(char* szName)
{
	return mcNamedIndexedData.Contains(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Contains(CChars* szName)
{
	return mcNamedIndexedData.Contains(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Remove(OIndex oi)
{
	return mcNamedIndexedData.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Remove(char* szName)
{
	return mcNamedIndexedData.Remove(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Remove(CChars* szName)
{
	return mcNamedIndexedData.Remove(szName);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Flush(void)
{
	return mcNamedIndexedData.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCodabase::Flush(BOOL bClearCache)
{
	return mcNamedIndexedData.Flush(bClearCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndices(void)
{
	return mcNamedIndexedData.NumIndices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumIndicesCached(void)
{
	return mcNamedIndexedData.NumIndicesCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumNames(void)
{
	return mcNamedIndexedData.NumNames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCodabase::NumNamesCached(void)
{
	return mcNamedIndexedData.NumNamesCached();
}

