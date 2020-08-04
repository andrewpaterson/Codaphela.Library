#include "IndexTreeHelper.h"
#include "Database.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDatabase::Init(char* szDirectory, CNamedIndexedDataConfig* pcConfig)
{
	CIndexTreeHelper	cHelper;

	cHelper.Init(szDirectory, "Primary", "Backup", FALSE);

	mpcConfig = pcConfig;
	mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	mcNamedIndexedData.Init(&mcFileController, pcConfig);

	cHelper.Kill(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDatabase::Kill(void)
{
	mcNamedIndexedData.Kill();
	mcFileController.Kill();
	SafeKill(mpcConfig);
}
