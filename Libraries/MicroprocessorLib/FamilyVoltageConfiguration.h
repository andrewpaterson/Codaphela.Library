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
#ifndef __FAMILY_VOLTAGE_CONFIGURATION_H__
#define __FAMILY_VOLTAGE_CONFIGURATION_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "VoltageConfigurationSource.h"


//Logicim's DefaultLogicLevels.
#define LOGIC_LEVEL_TTL			"TTL"
#define LOGIC_LEVEL_CMOS		"CMOS"
#define DEFAULT_LOGIC_LEVEL		LOGIC_LEVEL_TTL


typedef CArrayTemplate<CVoltageConfiguration>	CArrayVoltageConfiguration;


//Logicim's FamilyVoltageConfiguration: a logic family, like LVC or HCT, at each supply voltage it has data for.
//Between those voltages the thresholds, output voltages, slew rates and hold times are interpolated linearly.  Below
//the lowest the voltages are interpolated from zero and above the highest they are scaled with VCC; the slew rates
//and hold times outside them are infinite (no power), as in Logicim.  With VCC zero or NaN there is no power: no
//voltages, infinite slew rates and the longest hold time.
//
//The configurations are copied in and kept sorted by VCC as they are added, so Logicim's sort is not needed.  A
//configuration returned by Get is only valid until the next one is added.  One of them can be the family's TTL
//configuration and one its CMOS configuration, see GetDefaultVoltageConfiguration.
class CFamilyVoltageConfiguration : public CVoltageConfigurationSource
{
protected:
	CChars						mszFamily;
	CArrayVoltageConfiguration	macConfigurations;
	float32						mfTTLVcc;  //NaN when the family has no TTL configuration.
	float32						mfCMOSVcc;

public:
			void					Init(const char* szFamily);
			void					Kill(void);

			bool					Add(CVoltageConfiguration* pcConfiguration, const char* szLogicLevel);

	virtual CVoltageConfiguration*	Get(float32 fVcc);
			CVoltageConfiguration*	GetEqualOrHigherConfiguration(float32 fVcc);
			CVoltageConfiguration*	GetLowerConfiguration(float32 fVcc);
			CVoltageConfiguration*	GetDefaultVoltageConfiguration(const char* szLogicLevel);
			size					NumConfigurations(void);
			CVoltageConfiguration*	GetConfiguration(size uiIndex);
			char*					GetFamily(void);

	virtual float32					GetLowVoltageIn(float32 fVcc);
	virtual float32					GetHighVoltageIn(float32 fVcc);
	virtual float32					GetVoltageOut(bool bValue, float32 fVcc);
	virtual ETraceValue				GetValue(float32 fVoltage, float32 fVcc);
	virtual float32					CalculateStartVoltage(float32 fPortVoltage, float32 fVcc);
	virtual float32					GetVoltsPerTimeLowToHigh(float32 fVcc);
	virtual float32					GetVoltsPerTimeHighToLow(float32 fVcc);
	virtual int64					CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage, float32 fVcc);

protected:
			bool					HasPower(float32 fVcc);
			float32					InterpolateVoltage(float32 fHigherVoltage, float32 fLowerVoltage, CVoltageConfiguration* pcHigher, CVoltageConfiguration* pcLower, float32 fVcc);
			float32					LinearInterpolateVoltage(float32 fHigherVoltage, float32 fLowerVoltage, float32 fHigherVcc, float32 fLowerVcc, float32 fVcc);
			int64					LinearInterpolateTime(int64 iHigherTime, int64 iLowerTime, float32 fHigherVcc, float32 fLowerVcc, float32 fVcc);
};


#endif // __FAMILY_VOLTAGE_CONFIGURATION_H__

