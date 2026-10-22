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
	bResult = mcFileController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	mcFileController.SetSync(mpcConfig->GetSync());
	cHelper.Kill(false);

	mcFile.Init(&mcFileController, "Sequence.DAT", "_Sequence.DAT");

	//A crash while the sequence was being written must be recovered, otherwise indices already handed out would be handed out again.
	bResult = bResult && mcFileController.Recover();
	if (bResult)
	{
		bResult = mcFileController.Begin();
		moiNext = ReadIndex();
		bResult &= mcFileController.End();
	}

	if (!bResult || (moiNext == INVALID_O_INDEX))
	{
		//Not FIRST_O_INDEX, that could hand out indices that are already used.
		moiNext = INVALID_O_INDEX;
		moiReserved = INVALID_O_INDEX;
		return gcLogger.Error2(__METHOD__, " Could not read Sequence file.", NULL);
	}

	moiReserved = moiNext;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSequence::Kill(void)
{
	//Give back the unused part of the block so a clean close leaves no gap in the indices.
	if ((moiNext != INVALID_O_INDEX) && (moiNext < moiReserved))
	{
		if (!Reserve(moiNext))
		{
			gcLogger.Error2(__METHOD__, " Could not write Sequence file.", NULL);
		}
	}

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

	if (moiNext > moiReserved)
	{
		bResult = Reserve(uiNext + mpcConfig->GetBlockSize());
		if (!bResult)
		{
			//Keep counting in memory.  Going back to the first index would hand out indices that are already used.
			gcLogger.Error2(__METHOD__, " Could not write Sequence file.", NULL);
		}
	}

	return uiNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSequence::Reserve(OIndex oiReserved)
{
	bool	bResult;

	if (oiReserved < moiNext)
	{
		oiReserved = moiNext;
	}

	bResult = mcFileController.Begin();
	bResult &= WriteIndex(oiReserved);
	bResult &= mcFileController.End();

	if (bResult)
	{
		moiReserved = oiReserved;
	}
	return bResult;
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
	return mcFile.WriteInt64(oi);
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


