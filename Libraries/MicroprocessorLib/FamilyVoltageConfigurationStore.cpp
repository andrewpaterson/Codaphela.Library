/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/TextParser.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "LongTime.h"
#include "Voltage.h"
#include "FamilyVoltageConfigurationStore.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFamilyVoltageConfigurationStore::Init(void)
{
	mapcFamilies.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFamilyVoltageConfigurationStore::Kill(void)
{
	size							i;
	CFamilyVoltageConfiguration*	pcFamily;

	for (i = 0; i < mapcFamilies.NumElements(); i++)
	{
		pcFamily = mapcFamilies.GetPtr(i);
		SafeKill(pcFamily);
	}
	mapcFamilies.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds every family at every VCC in the propagation times file, see the
// class comment.  Families and configurations loaded before an error
// are kept.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfigurationStore::Load(const char* szFileName)
{
	CArrayVoltagePropagationTimeRow	asRows;
	bool							bResult;

	asRows.Init();
	bResult = ReadRows(szFileName, &asRows);
	if (bResult)
	{
		bResult = LoadRows(&asRows);
	}
	KillRows(&asRows);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Each family at each VCC, in the order they first appear, becomes a
// configuration.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfigurationStore::LoadRows(CArrayVoltagePropagationTimeRow* pasRows)
{
	size								i;
	size								j;
	SVoltagePropagationTimeRow*			psRow;
	SVoltagePropagationTimeRow*			psOther;
	CArrayVoltagePropagationTimeRowPtr	apsGroup;
	bool								bDone;
	CChars								szLogicLevel;
	CVoltageConfiguration				cConfiguration;
	bool								bResult;

	bResult = true;
	apsGroup.Init();
	for (i = 0; i < pasRows->NumElements(); i++)
	{
		psRow = pasRows->Get(i);

		bDone = false;
		for (j = 0; j < i; j++)
		{
			psOther = pasRows->Get(j);
			if (psOther->szFamily.Equals(&psRow->szFamily) && (psOther->fVcc == psRow->fVcc))
			{
				bDone = true;
				break;
			}
		}
		if (bDone)
		{
			continue;
		}

		apsGroup.ReInit();
		szLogicLevel.Init();
		for (j = i; j < pasRows->NumElements(); j++)
		{
			psOther = pasRows->Get(j);
			if (psOther->szFamily.Equals(&psRow->szFamily) && (psOther->fVcc == psRow->fVcc))
			{
				apsGroup.Add(psOther);
				if (!psOther->szLogicLevel.Empty())
				{
					szLogicLevel.Set(&psOther->szLogicLevel);
				}
			}
		}

		if (CreateVoltageConfiguration(&cConfiguration, psRow->fVcc, &apsGroup))
		{
			if (!Add(&cConfiguration, szLogicLevel.Text(), psRow->szFamily.Text()))
			{
				bResult = false;
			}
		}
		else
		{
			bResult = false;
		}
		szLogicLevel.Kill();
	}
	apsGroup.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The configuration of a family at fVcc from its rows, see the class
// comment.  The voltages and delays can be on any of the rows; a later
// row replaces an earlier one.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfigurationStore::CreateVoltageConfiguration(CVoltageConfiguration* pcConfiguration, float32 fVcc, CArrayVoltagePropagationTimeRowPtr* papsRows)
{
	SVoltagePropagationTimeRow*	psPHL;
	SVoltagePropagationTimeRow*	psPLH;
	SVoltagePropagationTimeRow*	psPZH;
	SVoltagePropagationTimeRow*	psPZL;
	SVoltagePropagationTimeRow*	psPHZ;
	SVoltagePropagationTimeRow*	psPLZ;
	SVoltagePropagationTimeRow*	psRow;
	float32						fVIH;
	float32						fVIL;
	float32						fVOH;
	float32						fVOL;
	size						i;
	char*						szDelayType;

	psPHL = NULL;
	psPLH = NULL;
	psPZH = NULL;
	psPZL = NULL;
	psPHZ = NULL;
	psPLZ = NULL;
	fVIH = NoVoltage();
	fVIL = NoVoltage();
	fVOH = NoVoltage();
	fVOL = NoVoltage();

	for (i = 0; i < papsRows->NumElements(); i++)
	{
		psRow = papsRows->GetPtr(i);
		szDelayType = psRow->szDelayType.Text();
		if (StringCompare(szDelayType, "tPD") == 0)
		{
			psPHL = psRow;
			psPLH = psRow;
		}
		else if (StringCompare(szDelayType, "tPHL") == 0)
		{
			psPHL = psRow;
		}
		else if (StringCompare(szDelayType, "tPLH") == 0)
		{
			psPLH = psRow;
		}
		else if (StringCompare(szDelayType, "tEN") == 0)
		{
			psPZH = psRow;
			psPZL = psRow;
		}
		else if (StringCompare(szDelayType, "tPZH") == 0)
		{
			psPZH = psRow;
		}
		else if (StringCompare(szDelayType, "tPZL") == 0)
		{
			psPZL = psRow;
		}
		else if (StringCompare(szDelayType, "tDIS") == 0)
		{
			psPHZ = psRow;
			psPLZ = psRow;
		}
		else if (StringCompare(szDelayType, "tPHZ") == 0)
		{
			psPHZ = psRow;
		}
		else if (StringCompare(szDelayType, "tPLZ") == 0)
		{
			psPLZ = psRow;
		}

		if (IsVoltage(psRow->fVIH))
		{
			fVIH = psRow->fVIH;
		}
		if (IsVoltage(psRow->fVIL))
		{
			fVIL = psRow->fVIL;
		}
		if (IsVoltage(psRow->fVOH))
		{
			fVOH = psRow->fVOH;
		}
		if (IsVoltage(psRow->fVOL))
		{
			fVOL = psRow->fVOL;
		}
	}

	if ((psPHL == NULL) || (psPLH == NULL) ||
		(psPZH == NULL) || (psPZL == NULL) ||
		(psPHZ == NULL) || (psPLZ == NULL))
	{
		gcLogger.Error2(__METHOD__, " tPHL, tPLH, tPZH, tPZL, tPHZ and tPLZ must all be provided.", NULL);
		return false;
	}

	if (!IsVoltage(psPHL->fTypicalDelay) || !IsVoltage(psPLH->fTypicalDelay) ||
		!IsVoltage(psPZH->fTypicalDelay) || !IsVoltage(psPZL->fTypicalDelay) ||
		!IsVoltage(psPHZ->fTypicalDelay) || !IsVoltage(psPLZ->fTypicalDelay))
	{
		gcLogger.Error2(__METHOD__, " Typical delays must all be provided.", NULL);
		return false;
	}

	if (!IsVoltage(fVIH) || !IsVoltage(fVIL) ||
		!IsVoltage(fVOH) || !IsVoltage(fVOL))
	{
		gcLogger.Error2(__METHOD__, " VIH, VIL, VOH and VOL must all be provided.", NULL);
		return false;
	}

	return pcConfiguration->Init(fVcc,
								 fVIL, fVIH,
								 fVOL, fVOH,
								 NanosecondsToTime((float64)(psPHL->fTypicalDelay / 2)),
								 NanosecondsToTime((float64)psPHL->fTypicalDelay),
								 NanosecondsToTime((float64)(psPLH->fTypicalDelay / 2)),
								 NanosecondsToTime((float64)psPLH->fTypicalDelay),
								 NanosecondsToTime((float64)(psPHZ->fTypicalDelay / 2)),
								 NanosecondsToTime((float64)psPHZ->fTypicalDelay),
								 NanosecondsToTime((float64)(psPLZ->fTypicalDelay / 2)),
								 NanosecondsToTime((float64)psPLZ->fTypicalDelay),
								 NanosecondsToTime((float64)(psPZH->fTypicalDelay / 2)),
								 NanosecondsToTime((float64)psPZH->fTypicalDelay),
								 NanosecondsToTime((float64)(psPZL->fTypicalDelay / 2)),
								 NanosecondsToTime((float64)psPZL->fTypicalDelay));
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the cell in column iColumn, without its quotes, into sz.  A
// column past the end of the row, or -1 for a column missing from the
// header, is empty.
//
//////////////////////////////////////////////////////////////////////////
static void GetCell(SCSVRowImmutable* psCSVRow, size iColumn, CChars* sz)
{
	sz->Init();
	if (iColumn < psCSVRow->NumElements())
	{
		sz->Append(psCSVRow->Get(iColumn));
		sz->StripWhitespace();
		if ((sz->Length() >= 2) && sz->StartsWith("\"") && sz->EndsWith("\""))
		{
			sz->RemoveFromStart(1);
			sz->RemoveFromEnd(1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The column named szName in the header, or -1.
//
//////////////////////////////////////////////////////////////////////////
static size FindColumn(SCSVRowImmutable* psHeader, const char* szName)
{
	size	i;
	CChars	sz;

	for (i = 0; i < psHeader->NumElements(); i++)
	{
		GetCell(psHeader, i, &sz);
		if (sz.Equals(szName))
		{
			sz.Kill();
			return i;
		}
		sz.Kill();
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The number in column szName, NaN when it is empty.  False if it isn't
// a number.
//
//////////////////////////////////////////////////////////////////////////
static bool GetNumber(SCSVRowImmutable* psCSVRow, SCSVRowImmutable* psHeader, const char* szName, float32* pf)
{
	CChars		sz;
	CTextParser	cParser;
	TRISTATE	tResult;

	GetCell(psCSVRow, FindColumn(psHeader, szName), &sz);
	if (sz.Empty())
	{
		*pf = NoVoltage();
		sz.Kill();
		return true;
	}

	cParser.Init(&sz);
	tResult = cParser.GetFloat(pf);
	if ((tResult != TRITRUE) || !cParser.IsOutside())
	{
		cParser.Kill();
		sz.Kill();
		return false;
	}
	cParser.Kill();
	sz.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void GetText(SCSVRowImmutable* psCSVRow, SCSVRowImmutable* psHeader, const char* szName, CChars* sz)
{
	CChars	szCell;

	GetCell(psCSVRow, FindColumn(psHeader, szName), &szCell);
	sz->Set(&szCell);
	szCell.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every row needs a family and a VCC that isn't zero.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfigurationStore::ReadRow(SCSVRowImmutable* psCSVRow, SCSVRowImmutable* psHeader, SVoltagePropagationTimeRow* psRow)
{
	bool	bResult;

	GetText(psCSVRow, psHeader, "Family", &psRow->szFamily);
	GetText(psCSVRow, psHeader, "Logic Level", &psRow->szLogicLevel);
	GetText(psCSVRow, psHeader, "Propagation", &psRow->szDelayType);
	GetText(psCSVRow, psHeader, "Manufacturer", &psRow->szManufacturer);

	bResult = GetNumber(psCSVRow, psHeader, "VCC", &psRow->fVcc);
	bResult &= GetNumber(psCSVRow, psHeader, "Typical", &psRow->fTypicalDelay);
	bResult &= GetNumber(psCSVRow, psHeader, "Minimum", &psRow->fMinimumDelay);
	bResult &= GetNumber(psCSVRow, psHeader, "Maximum", &psRow->fMaximumDelay);
	bResult &= GetNumber(psCSVRow, psHeader, "VIH", &psRow->fVIH);
	bResult &= GetNumber(psCSVRow, psHeader, "VIL", &psRow->fVIL);
	bResult &= GetNumber(psCSVRow, psHeader, "VOH", &psRow->fVOH);
	bResult &= GetNumber(psCSVRow, psHeader, "VOL", &psRow->fVOL);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Propagation time numbers must be numbers.", NULL);
		return false;
	}

	if (psRow->szFamily.Empty() || !IsVoltage(psRow->fVcc) || (psRow->fVcc == 0))
	{
		gcLogger.Error2(__METHOD__, " Family and VCC must be provided.", NULL);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Blank lines are skipped.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfigurationStore::ReadRows(const char* szFileName, CArrayVoltagePropagationTimeRow* pasRows)
{
	CCSVFileImmutable			cCSVFile;
	CFileUtil					cFileUtil;
	SCSVRowImmutable*			psHeader;
	SCSVRowImmutable*			psCSVRow;
	SVoltagePropagationTimeRow*	psRow;
	size						i;
	CChars						szFirst;
	bool						bEmpty;

	if (!cFileUtil.Exists(szFileName))
	{
		gcLogger.Error2(__METHOD__, " Propagation times file [", szFileName, "] does not exist.", NULL);
		return false;
	}

	cCSVFile.Init(',');
	cCSVFile.Open(DiskFile(szFileName));
	cCSVFile.ReadAllLines();
	cCSVFile.Close();

	if (cCSVFile.NumRows() == 0)
	{
		gcLogger.Error2(__METHOD__, " Propagation times file [", szFileName, "] has no header.", NULL);
		cCSVFile.Kill();
		return false;
	}

	psHeader = cCSVFile.Get(0);
	for (i = 1; i < cCSVFile.NumRows(); i++)
	{
		psCSVRow = cCSVFile.Get(i);
		GetCell(psCSVRow, 0, &szFirst);
		bEmpty = (psCSVRow->NumElements() <= 1) && szFirst.Empty();
		szFirst.Kill();
		if (bEmpty)
		{
			continue;
		}

		psRow = pasRows->Add();
		psRow->Init();
		if (!ReadRow(psCSVRow, psHeader, psRow))
		{
			cCSVFile.Kill();
			return false;
		}
	}
	cCSVFile.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFamilyVoltageConfigurationStore::KillRows(CArrayVoltagePropagationTimeRow* pasRows)
{
	size						i;
	SVoltagePropagationTimeRow*	psRow;

	for (i = 0; i < pasRows->NumElements(); i++)
	{
		psRow = pasRows->Get(i);
		psRow->Kill();
	}
	pasRows->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Copies pcConfiguration into the family, which is added if it is new.
// Fails if the family already has a configuration at its VCC.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfigurationStore::Add(CVoltageConfiguration* pcConfiguration, const char* szLogicLevel, const char* szFamily)
{
	CFamilyVoltageConfiguration*	pcFamily;

	pcFamily = Get(szFamily);
	if (pcFamily == NULL)
	{
		pcFamily = NewMalloc<CFamilyVoltageConfiguration>();
		pcFamily->Init(szFamily);
		mapcFamilies.Add(pcFamily);
	}
	return pcFamily->Add(pcConfiguration, szLogicLevel);
}


//////////////////////////////////////////////////////////////////////////
//
// The family named szFamily, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CFamilyVoltageConfiguration* CFamilyVoltageConfigurationStore::Get(const char* szFamily)
{
	size							i;
	CFamilyVoltageConfiguration*	pcFamily;

	for (i = 0; i < mapcFamilies.NumElements(); i++)
	{
		pcFamily = mapcFamilies.GetPtr(i);
		if (StringCompare(pcFamily->GetFamily(), szFamily) == 0)
		{
			return pcFamily;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFamilyVoltageConfiguration* CFamilyVoltageConfigurationStore::GetDefaultFamily(void) { return Get(DEFAULT_FAMILY); }
size CFamilyVoltageConfigurationStore::NumFamilies(void) { return mapcFamilies.NumElements(); }
CFamilyVoltageConfiguration* CFamilyVoltageConfigurationStore::GetFamily(size uiIndex) { return mapcFamilies.SafeGetPtr(uiIndex); }

