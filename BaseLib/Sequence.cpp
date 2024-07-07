#include "IndexTreeHelper.h"
#include "Logger.h"
#include "Sequence.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSequence::Init(CLifeInit<CSequenceConfig> cConfig)
{
	CIndexTreeHelper	cHelper;
	bool				bResult;

	cConfig.ConfigureLife(&mcConfig, &mpcConfig);

	cHelper.Init(mpcConfig->Getdirectory(), "Primary", "Backup", false);
	mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());

	mcFile.Init(&mcFileController, "Sequence.DAT", "_Sequence.DAT");

	bResult = mcFileController.Begin();
	moiNext = ReadIndex();
	bResult &= mcFileController.End();

	cHelper.Kill(false);

	if (moiNext == INVALID_O_INDEX || !bResult)
	{
		moiNext = FIRST_O_INDEX;
		return gcLogger.Error2(__METHOD__, " Could not read Sequence file.", NULL);
	}

	return true;
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
	bool	bResult;

	uiNext = moiNext;

	moiNext++;

	bResult = mcFileController.Begin();
	bResult &= WriteIndex(moiNext);
	bResult &= mcFileController.End();

	if (!bResult)
	{
		moiNext = FIRST_O_INDEX;
		gcLogger.Error2(__METHOD__, " Could not write Sequence file.", NULL);
	}

	return uiNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CSequence::PeekNext(void)
{
	return moiNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSequence::WriteIndex(OIndex oi)
{
	return mcFile.WriteInt64(moiNext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CSequence::ReadIndex(void)
{
	OIndex	oi;
	bool	bResult;

	if (mcFile.Exists())
	{
		bResult = mcFile.ReadInt64(&oi);
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


