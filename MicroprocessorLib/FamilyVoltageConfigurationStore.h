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
#ifndef __FAMILY_VOLTAGE_CONFIGURATION_STORE_H__
#define __FAMILY_VOLTAGE_CONFIGURATION_STORE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/CSVFileImutable.h"
#include "FamilyVoltageConfiguration.h"
#include "VoltagePropagationTimeRow.h"


//Logicim's DefaultFamily.
#define DEFAULT_FAMILY		"LVC"


typedef CArrayTemplatePtr<CFamilyVoltageConfiguration>	CArrayFamilyVoltageConfigurationPtr;


//Logicim's FamilyVoltageConfigurationStore and FamilyStore together: the logic families, by name in the order they
//were added, with their voltage configurations.  Logicim keeps one of each in a static instance and loads it from
//Documents/Propagation Times (541).csv; here the caller owns the store and passes Load the file.  Logicim's Family is
//only a name, so the name is kept in the CFamilyVoltageConfiguration.
//
//The file has a header row naming the columns, Family, VCC, Logic Level, Propagation, Minimum, Typical, Maximum, VIH,
//VIL, VOH, VOL and Manufacturer, then a row for each delay of a family at a VCC.  Every family at every VCC needs all
//six delays (tPD is tPHL and tPLH, tEN is tPZH and tPZL, tDIS is tPHZ and tPLZ) and the four voltages; the hold times
//are half the typical delays and the slew times all of them.  A family at a VCC with a Logic Level of TTL or CMOS is
//that default configuration.  Logicim carries a logic level on to the family's later VCCs; here it applies to the
//rows at its VCC only.
class CFamilyVoltageConfigurationStore
{
protected:
	CArrayFamilyVoltageConfigurationPtr	mapcFamilies;

public:
	void							Init(void);
	void							Kill(void);

	bool							Load(const char* szFileName);
	bool							Add(CVoltageConfiguration* pcConfiguration, const char* szLogicLevel, const char* szFamily);

	CFamilyVoltageConfiguration*	Get(const char* szFamily);
	CFamilyVoltageConfiguration*	GetDefaultFamily(void);
	size							NumFamilies(void);
	CFamilyVoltageConfiguration*	GetFamily(size uiIndex);

	static bool						CreateVoltageConfiguration(CVoltageConfiguration* pcConfiguration, float32 fVcc, CArrayVoltagePropagationTimeRowPtr* papsRows);

protected:
	bool							ReadRows(const char* szFileName, CArrayVoltagePropagationTimeRow* pasRows);
	bool							ReadRow(SCSVRowImmutable* psCSVRow, SCSVRowImmutable* psHeader, SVoltagePropagationTimeRow* psRow);
	bool							LoadRows(CArrayVoltagePropagationTimeRow* pasRows);
	void							KillRows(CArrayVoltagePropagationTimeRow* pasRows);
};


#endif // __FAMILY_VOLTAGE_CONFIGURATION_STORE_H__

