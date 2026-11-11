/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/Timer.h"
#include "CPPCompileStatistics.h"


static const char*	gaszCPPCompilePhases[CPPCP_NumPhases] =
{
	"preprocess",
	"tokenise",
	"parse",
	"declarations",
	"types",
	"front end",
	"inline",
	"optimise",
	"back end",
	"code passes",
	"assemble",
	"link",
};


static const char*	gaszCPPCompileCounts[CPPCC_NumCounts] =
{
	"functions",
	"TypeOf",
	"TypeOf worked out",
	"quiet trials",
	"quiet trials failed",
	"FindFunction",
	"FindNamedFunction",
	"FindAddressedFunction",
	"FindAssociatedFunction",
	"FindOperator",
	"FindFreeOperator",
	"FindStaticFunction",
	"FindConversion",
	"FindMemberTemplate",
	"ChooseMemberTemplate",
	"ChooseTemplate",
	"GetCallTemplate",
	"pack expansions",
	"pack expansion nodes",
	"unsupported exits",
	"65C64 code loaded",
	"65C64 code refused",
	"dominators",
	"optimised functions",
	"optimiser rounds",
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Init(void)
{
	mbTimes = false;
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The times and counts are zero and no phase is begun.
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Clear(void)
{
	memset(malliMicroseconds, 0, sizeof(malliMicroseconds));
	memset(mauiCounts, 0, sizeof(mauiCounts));
	memset(masPasses, 0, sizeof(masPasses));
	miDepth = 0;
	mlliSince = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::SetTimes(bool bTimes)
{
	mbTimes = bTimes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPCompileStatistics::IsTimed(void)
{
	return mbTimes;
}


//////////////////////////////////////////////////////////////////////////
//
// The time from now until End(ePhase) is ePhase's and not the phase's it
// began in (unless that is ePhase too).
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Begin(ECPPCompilePhase ePhase)
{
	int64	lliNow;

	if (!mbTimes || (miDepth >= (int)(sizeof(maePhases) / sizeof(maePhases[0]))))
	{
		return;
	}
	if ((miDepth == 0) || (maePhases[miDepth - 1] != ePhase))
	{
		lliNow = GetDebugMicroseconds();
		if (miDepth > 0)
		{
			malliMicroseconds[maePhases[miDepth - 1]] += lliNow - mlliSince;
		}
		mlliSince = lliNow;
	}
	maePhases[miDepth] = ePhase;
	miDepth++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::End(ECPPCompilePhase ePhase)
{
	int64	lliNow;

	if (!mbTimes || (miDepth == 0) || (maePhases[miDepth - 1] != ePhase))
	{
		return;
	}
	miDepth--;
	if ((miDepth == 0) || (maePhases[miDepth - 1] != ePhase))
	{
		lliNow = GetDebugMicroseconds();
		malliMicroseconds[ePhase] += lliNow - mlliSince;
		mlliSince = lliNow;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Count(ECPPCompileCount eCount, uint64 uiCount)
{
	mauiCounts[eCount] += uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The clock, in microseconds, when the times are kept; 0 when they aren't
// (so a pass's time is 0 without the clock being read).
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPCompileStatistics::Now(void)
{
	return mbTimes ? GetDebugMicroseconds() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The pass numbered iPass (below CPP_MAX_PASSES) and named szName ran:
// whether it changed its function, the round of passes it ran in (from 0, or
// CPP_NO_ROUND) and how long it took.
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::CountPass(int iPass, const char* szName, bool bChanged, int iRound, int64 lliMicroseconds)
{
	SCPPPassStatistics*	psPass;

	psPass = &masPasses[iPass];
	psPass->szName = szName;
	psPass->uiRuns++;
	psPass->lliMicroseconds += lliMicroseconds;
	if (bChanged)
	{
		psPass->uiChanged++;
		if (iRound == 0)
		{
			psPass->uiChangedFirst++;
		}
		else if (iRound >= 2)
		{
			psPass->uiChangedLate++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPPassStatistics* CCPPCompileStatistics::GetPass(int iPass)
{
	return &masPasses[iPass];
}


//////////////////////////////////////////////////////////////////////////
//
// pcOther's times and counts are added to these.
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Add(CCPPCompileStatistics* pcOther)
{
	int		i;

	for (i = 0; i < CPPCP_NumPhases; i++)
	{
		malliMicroseconds[i] += pcOther->malliMicroseconds[i];
	}
	for (i = 0; i < CPPCC_NumCounts; i++)
	{
		mauiCounts[i] += pcOther->mauiCounts[i];
	}
	for (i = 0; i < CPP_MAX_PASSES; i++)
	{
		if (pcOther->masPasses[i].szName)
		{
			masPasses[i].szName = pcOther->masPasses[i].szName;
			masPasses[i].uiRuns += pcOther->masPasses[i].uiRuns;
			masPasses[i].uiChanged += pcOther->masPasses[i].uiChanged;
			masPasses[i].uiChangedFirst += pcOther->masPasses[i].uiChangedFirst;
			masPasses[i].uiChangedLate += pcOther->masPasses[i].uiChangedLate;
			masPasses[i].lliMicroseconds += pcOther->masPasses[i].lliMicroseconds;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPCompileStatistics::GetMicroseconds(ECPPCompilePhase ePhase)
{
	return malliMicroseconds[ePhase];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CCPPCompileStatistics::GetCount(ECPPCompileCount eCount)
{
	return mauiCounts[eCount];
}


//////////////////////////////////////////////////////////////////////////
//
// A line for each phase's milliseconds (when they were timed), each count
// and each pass that ran: "TIME parse 1234", "COUNT TypeOf 5678" and "PASS
// FoldConstants runs changed changed-first changed-late milliseconds".
//
//////////////////////////////////////////////////////////////////////////
void CCPPCompileStatistics::Print(CChars* pszDest)
{
	int		i;

	for (i = 0; mbTimes && (i < CPPCP_NumPhases); i++)
	{
		pszDest->Append("TIME ");
		pszDest->Append(gaszCPPCompilePhases[i]);
		pszDest->Append(" ");
		pszDest->Append(malliMicroseconds[i] / 1000);
		pszDest->AppendNewLine();
	}
	for (i = 0; i < CPPCC_NumCounts; i++)
	{
		pszDest->Append("COUNT ");
		pszDest->Append(gaszCPPCompileCounts[i]);
		pszDest->Append(" ");
		pszDest->Append((int64)mauiCounts[i]);
		pszDest->AppendNewLine();
	}
	for (i = 0; i < CPP_MAX_PASSES; i++)
	{
		if (masPasses[i].szName)
		{
			pszDest->Append("PASS ");
			pszDest->Append(masPasses[i].szName);
			pszDest->Append(" ");
			pszDest->Append((int64)masPasses[i].uiRuns);
			pszDest->Append(" ");
			pszDest->Append((int64)masPasses[i].uiChanged);
			pszDest->Append(" ");
			pszDest->Append((int64)masPasses[i].uiChangedFirst);
			pszDest->Append(" ");
			pszDest->Append((int64)masPasses[i].uiChangedLate);
			pszDest->Append(" ");
			pszDest->Append(masPasses[i].lliMicroseconds / 1000);
			pszDest->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPCompileStatistics::GetPhaseName(ECPPCompilePhase ePhase)
{
	return gaszCPPCompilePhases[ePhase];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPCompileStatistics::GetCountName(ECPPCompileCount eCount)
{
	return gaszCPPCompileCounts[eCount];
}

