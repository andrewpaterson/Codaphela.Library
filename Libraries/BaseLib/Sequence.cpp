#include "IndexTreeHelper.h"
#include "Logger.h"
#include "LogString.h"
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
		moiEnd = INVALID_O_INDEX;
		return gcLogger.Error2(__METHOD__, " Could not read Sequence file.", NULL);
	}

	moiEnd = mpcConfig->GetEndIndex();
	if ((moiNext < mpcConfig->GetFirstIndex()) || (moiNext > moiEnd))
	{
		gcLogger.Error2(__METHOD__, " Sequence next index [", LongToString(moiNext), "] is outside its range [", LongToString(mpcConfig->GetFirstIndex()), "] to [", LongToString(moiEnd), "].", NULL);
		moiNext = INVALID_O_INDEX;
		moiReserved = INVALID_O_INDEX;
		moiEnd = INVALID_O_INDEX;
		return false;
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
	OIndex	oiReserve;

	if (moiNext >= moiEnd)
	{
		gcLogger.Error2(__METHOD__, " Sequence has no indices left before [", LongToString(moiEnd), "].", NULL);
		return INVALID_O_INDEX;
	}

	uiNext = moiNext;

	moiNext++;

	if (moiNext > moiReserved)
	{
		oiReserve = uiNext + mpcConfig->GetBlockSize();
		if ((oiReserve > moiEnd) || (oiReserve < uiNext))
		{
			oiReserve = moiEnd;
		}
		bResult = Reserve(oiReserve);
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
// One past the last index this sequence may hand out.
//
//////////////////////////////////////////////////////////////////////////
OIndex CSequence::GetEnd(void)
{
	return moiEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes iCount indices out of the sequence: GetNext never hands them out.
// Returns the first; the range is first up to (not including) first +
// iCount.  The range is written to the Sequence file before it is
// returned, so it is never handed out again after a crash.
// INVALID_O_INDEX if there is not room or the file could not be written.
//
//////////////////////////////////////////////////////////////////////////
OIndex CSequence::ReserveRange(OIndex iCount)
{
	OIndex	oiFirst;

	if ((iCount <= 0) || (moiNext == INVALID_O_INDEX) || (iCount > moiEnd - moiNext))
	{
		gcLogger.Error2(__METHOD__, " Cannot reserve [", LongToString(iCount), "] indices from [", LongToString(moiNext), "] before [", LongToString(moiEnd), "].", NULL);
		return INVALID_O_INDEX;
	}

	oiFirst = moiNext;
	if (!MoveNext(oiFirst + iCount))
	{
		return INVALID_O_INDEX;
	}
	return oiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the next index handed out at least oiNext (a sequence opened over
// layers starts above their indices).  Never moves the sequence back.
//
//////////////////////////////////////////////////////////////////////////
bool CSequence::AdvanceTo(OIndex oiNext)
{
	if ((moiNext == INVALID_O_INDEX) || (oiNext > moiEnd))
	{
		return gcLogger.Error2(__METHOD__, " Cannot advance sequence to [", LongToString(oiNext), "], its end is [", LongToString(moiEnd), "].", NULL);
	}
	if (oiNext <= moiNext)
	{
		return true;
	}
	return MoveNext(oiNext);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the next index forward to oiNext, writing it to the Sequence file
// first if it passes what is reserved there.
//
//////////////////////////////////////////////////////////////////////////
bool CSequence::MoveNext(OIndex oiNext)
{
	OIndex	oiOldNext;

	oiOldNext = moiNext;
	moiNext = oiNext;
	if (moiNext > moiReserved)
	{
		if (!Reserve(moiNext))
		{
			moiNext = oiOldNext;
			return gcLogger.Error2(__METHOD__, " Could not write Sequence file.", NULL);
		}
	}
	return true;
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
		return mpcConfig->GetFirstIndex();
	}
}


