#include "IndexTreeHelper.h"
#include "BaseLib/Logger.h"
#include "Sequence.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSequence::Init(CLifeInit<CSequenceConfig> cConfig)
{
	CIndexTreeHelper	cHelper;
	BOOL				bResult;

	cConfig.ConfigureLife(&mcConfig, &mpcConfig);

	cHelper.Init(mpcConfig->Getdirectory(), "Primary", "Backup", FALSE);
	mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());

	mcFile.Init(&mcFileController, "Sequence.DAT", "_Sequence.DAT");

	bResult = mcFileController.Begin();
	muiNext = ReadIndex();
	bResult &= mcFileController.End();

	if (muiNext == INVALID_O_INDEX || !bResult)
	{
		muiNext = FIRST_O_INDEX;
		return gcLogger.Error2(__METHOD__, " Could not read Sequence file.", NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSequence::Kill(void)
{
	mcFile.Kill();
	mcFileController.Kill();
	mcConfig.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CSequence::GetNext(void)
{
	OIndex	uiNext;
	BOOL	bResult;

	uiNext = muiNext;

	muiNext++;

	bResult = mcFileController.Begin();
	bResult &= WriteIndex(muiNext);
	bResult &= mcFileController.End();

	if (!bResult)
	{
		muiNext = FIRST_O_INDEX;
		gcLogger.Error2(__METHOD__, " Could not write Sequence file.", NULL);
	}

	return uiNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSequence::WriteIndex(OIndex oi)
{
	return mcFile.WriteLong(muiNext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CSequence::ReadIndex(void)
{
	OIndex	oi;
	BOOL	bResult;

	if (mcFile.Exists())
	{
		bResult = mcFile.ReadLong(&oi);
		if (bResult)
		{
			return oi;
		}
		else
		{
			return INVALID_O_INDEX;
		}
	}
	else
	{
		return FIRST_O_INDEX;
	}
}


