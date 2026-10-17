/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Logger.h"
#include "LogString.h"
#include "DurableFile.h"
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Init(const char* szDirectory)
{
	return Init(szDirectory, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Init(const char* szDirectory, const char* szRewriteDirectory, const char* szMarkWrite, const char* szMarkRewrite)
{
	CChars		szStart;
	CChars		szRewrite;
	CFileUtil	cFileUtil;

	//Init only fails on bad arguments.  Whether a crashed commit needs recovering is found by Check and fixed by Recover.
	mbDurable = !StrEmpty(szRewriteDirectory);
	mbSync = true;
	mszDirectory.Init(szDirectory);
	if (mbDurable)
	{
		mszRewriteDirectory.Init(szRewriteDirectory);
	}
	else
	{
		mszRewriteDirectory.Init();
	}

	if (!szMarkWrite)
	{
		szMarkWrite = "Mark1.Write";
	}
	if (!szMarkRewrite)
	{
		szMarkRewrite = "Mark2.Rewrite";
	}
	szStart.Init(szDirectory);
	cFileUtil.AppendToPath(&szStart, szMarkWrite);
	if (mbDurable)
	{
		szRewrite.Init(szRewriteDirectory);
	}
	else
	{
		szRewrite.Init(szDirectory);
	}
	cFileUtil.AppendToPath(&szRewrite, szMarkRewrite);
	mcDurableSet.Init(szStart.Text(), szRewrite.Text());
	szRewrite.Kill();
	szStart.Kill();

	if (StrEmpty(szDirectory))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory must be supplied.", NULL);
	}
	if (mbDurable && (StringInsensitiveCompare(szDirectory, szRewriteDirectory) == 0))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory and rewrite directory must be different.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::Kill(void)
{
	mcDurableSet.Kill();

	mszRewriteDirectory.Kill();
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Begin(void)
{
	bool			bResult;

	bResult = mcDurableSet.Begin();
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Begin(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile*	pc;
	size			iCount;

	iCount = 0;
	pc = pcFirst;

	va_start(vaMarker, pcFirst);
	while (pc != NULL)
	{
		pc->AddFile();
		iCount++;
		pc = va_arg(vaMarker, CDurableFile*);
	}
	va_end(vaMarker);

	return Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::End(void)
{
	return mcDurableSet.End();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Recover(void)
{
	return mcDurableSet.Recover();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Recover(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile*	pc;
	size			iCount;

	iCount = 0;
	pc = pcFirst;

	va_start(vaMarker, pcFirst);
	while (pc != NULL)
	{
		pc->AddFile();
		iCount++;
		pc = va_arg(vaMarker, CDurableFile*);
	}
	va_end(vaMarker);

	return Recover();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Check(bool bThorough, bool bLogError)
{
	return mcDurableSet.Check(bThorough, bLogError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::AddFile(CDurableFile* pcFile)
{
	return mcDurableSet.Add(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDurableFileController::NumFiles(void)
{
	return mcDurableSet.Num();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFile* CDurableFileController::GetFile(size iIndex)
{
	return mcDurableSet.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::IsBegun(void) { return mcDurableSet.HasBegun(); }

char* CDurableFileController::GetDirectory(void) { return mszDirectory.Text(); }

char* CDurableFileController::GetRewriteDirectory(void) { return mszRewriteDirectory.Text(); }

bool CDurableFileController::IsDurable(void) { return mbDurable; }

CDurableSet* CDurableFileController::GetDurableSet(void) { return &mcDurableSet; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::SetSync(EDurableSync eSync)
{
	mbSync = (eSync != DS_No);
	mcDurableSet.SetSync(mbSync);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::IsSync(void)
{
	return mbSync;
}

