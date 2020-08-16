#include "IndexTreeHelper.h"
#include "Codabase.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodabase::Init(char* szDirectory, CNamedIndexedDataConfig* pcConfig)
{
	CIndexTreeHelper	cHelper;

	cHelper.Init(szDirectory, "Primary", "Backup", FALSE);

	mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	mcFileController.Begin();
	mcNamedIndexedData.Init(&mcFileController, pcConfig);
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

