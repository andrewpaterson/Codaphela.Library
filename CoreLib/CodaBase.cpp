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

