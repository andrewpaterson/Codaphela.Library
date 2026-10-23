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
#include "BaseLib/StringHelper.h"
#include "Voltage.h"
#include "FamilyVoltageConfiguration.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFamilyVoltageConfiguration::Init(const char* szFamily)
{
	mszFamily.Init(szFamily);
	macConfigurations.Init();
	mfTTLVcc = NoVoltage();
	mfCMOSVcc = NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFamilyVoltageConfiguration::Kill(void)
{
	size					i;
	CVoltageConfiguration*	pcConfiguration;

	for (i = 0; i < macConfigurations.NumElements(); i++)
	{
		pcConfiguration = macConfigurations.Get(i);
		pcConfiguration->Kill();
	}
	macConfigurations.Kill();
	mszFamily.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Copies pcConfiguration in.  A logic level of TTL or CMOS, in any case,
// makes it that default configuration; any other is ignored.  Fails if
// the family already has a configuration at its VCC.
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfiguration::Add(CVoltageConfiguration* pcConfiguration, const char* szLogicLevel)
{
	CChars	sz;
	CChars	szLevel;

	if (Get(pcConfiguration->GetVcc()) != NULL)
	{
		sz.Init(" Voltage configuration for Family [");
		sz.Append(mszFamily);
		sz.Append("], VCC [");
		AppendVoltage(&sz, pcConfiguration->GetVcc());
		sz.Append("] already added.");
		gcLogger.Error2(__METHOD__, sz.Text(), NULL);
		sz.Kill();
		return false;
	}

	macConfigurations.InsertIntoSorted(CompareVoltageConfigurationVcc, pcConfiguration, false);

	if (szLogicLevel != NULL)
	{
		szLevel.Init(szLogicLevel);
		if (szLevel.EqualsIgnoreCase(LOGIC_LEVEL_TTL))
		{
			mfTTLVcc = pcConfiguration->GetVcc();
		}
		else if (szLevel.EqualsIgnoreCase(LOGIC_LEVEL_CMOS))
		{
			mfCMOSVcc = pcConfiguration->GetVcc();
		}
		szLevel.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The configuration at exactly fVcc, or NULL.  No voltage (NaN) is checked
// first because under /fp:fast a NaN can compare equal to anything.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfiguration* CFamilyVoltageConfiguration::Get(float32 fVcc)
{
	size					i;
	CVoltageConfiguration*	pcConfiguration;

	if (!IsVoltage(fVcc))
	{
		return NULL;
	}

	for (i = 0; i < macConfigurations.NumElements(); i++)
	{
		pcConfiguration = macConfigurations.Get(i);
		if (pcConfiguration->GetVcc() == fVcc)
		{
			return pcConfiguration;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The configuration with the lowest VCC at or above fVcc, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfiguration* CFamilyVoltageConfiguration::GetEqualOrHigherConfiguration(float32 fVcc)
{
	size					i;
	CVoltageConfiguration*	pcConfiguration;

	for (i = 0; i < macConfigurations.NumElements(); i++)
	{
		pcConfiguration = macConfigurations.Get(i);
		if (pcConfiguration->GetVcc() >= fVcc)
		{
			return pcConfiguration;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The configuration with the highest VCC below fVcc, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfiguration* CFamilyVoltageConfiguration::GetLowerConfiguration(float32 fVcc)
{
	size					i;
	CVoltageConfiguration*	pcConfiguration;
	CVoltageConfiguration*	pcPrevious;

	pcPrevious = NULL;
	for (i = 0; i < macConfigurations.NumElements(); i++)
	{
		pcConfiguration = macConfigurations.Get(i);
		if (pcConfiguration->GetVcc() >= fVcc)
		{
			return pcPrevious;
		}
		pcPrevious = pcConfiguration;
	}
	return pcPrevious;
}


//////////////////////////////////////////////////////////////////////////
//
// The TTL or CMOS configuration, matched exactly as in Logicim, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfiguration* CFamilyVoltageConfiguration::GetDefaultVoltageConfiguration(const char* szLogicLevel)
{
	if (StringCompare(szLogicLevel, LOGIC_LEVEL_TTL) == 0)
	{
		return Get(mfTTLVcc);
	}
	else if (StringCompare(szLogicLevel, LOGIC_LEVEL_CMOS) == 0)
	{
		return Get(mfCMOSVcc);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFamilyVoltageConfiguration::HasPower(float32 fVcc)
{
	return (fVcc != 0) && IsVoltage(fVcc);
}


//////////////////////////////////////////////////////////////////////////
//
// A voltage at fVcc from the configurations either side of it, see the
// class comment.  fHigherVoltage and fLowerVoltage are the voltage in
// pcHigher and pcLower; either can be NULL but not both.
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::InterpolateVoltage(float32 fHigherVoltage, float32 fLowerVoltage, CVoltageConfiguration* pcHigher, CVoltageConfiguration* pcLower, float32 fVcc)
{
	float32	fFraction;

	if (pcHigher != NULL)
	{
		if (pcHigher->GetVcc() == fVcc)
		{
			return fHigherVoltage;
		}
		else if (pcLower != NULL)
		{
			return LinearInterpolateVoltage(fHigherVoltage, fLowerVoltage, pcHigher->GetVcc(), pcLower->GetVcc(), fVcc);
		}
		else
		{
			return LinearInterpolateVoltage(fHigherVoltage, 0, pcHigher->GetVcc(), 0, fVcc);
		}
	}
	else
	{
		fFraction = fVcc / pcLower->GetVcc();
		return fLowerVoltage * fFraction;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::GetHighVoltageIn(float32 fVcc)
{
	CVoltageConfiguration*	pcHigher;
	CVoltageConfiguration*	pcLower;

	if (!HasPower(fVcc))
	{
		return NoVoltage();
	}

	pcHigher = GetEqualOrHigherConfiguration(fVcc);
	pcLower = GetLowerConfiguration(fVcc);
	if ((pcHigher == NULL) && (pcLower == NULL))
	{
		return NoVoltage();
	}
	return InterpolateVoltage(pcHigher ? pcHigher->GetHighVoltageIn() : 0, pcLower ? pcLower->GetHighVoltageIn() : 0, pcHigher, pcLower, fVcc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::GetLowVoltageIn(float32 fVcc)
{
	CVoltageConfiguration*	pcHigher;
	CVoltageConfiguration*	pcLower;

	if (!HasPower(fVcc))
	{
		return NoVoltage();
	}

	pcHigher = GetEqualOrHigherConfiguration(fVcc);
	pcLower = GetLowerConfiguration(fVcc);
	if ((pcHigher == NULL) && (pcLower == NULL))
	{
		return NoVoltage();
	}
	return InterpolateVoltage(pcHigher ? pcHigher->GetLowVoltageIn() : 0, pcLower ? pcLower->GetLowVoltageIn() : 0, pcHigher, pcLower, fVcc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::GetVoltageOut(bool bValue, float32 fVcc)
{
	CVoltageConfiguration*	pcHigher;
	CVoltageConfiguration*	pcLower;

	if (!HasPower(fVcc))
	{
		return NoVoltage();
	}

	pcHigher = GetEqualOrHigherConfiguration(fVcc);
	pcLower = GetLowerConfiguration(fVcc);
	if ((pcHigher == NULL) && (pcLower == NULL))
	{
		return NoVoltage();
	}
	return InterpolateVoltage(pcHigher ? pcHigher->GetVoltageOut(bValue) : 0, pcLower ? pcLower->GetVoltageOut(bValue) : 0, pcHigher, pcLower, fVcc);
}


//////////////////////////////////////////////////////////////////////////
//
// The longest time when there is no power, or fVcc is outside the
// family's configurations.
//
//////////////////////////////////////////////////////////////////////////
int64 CFamilyVoltageConfiguration::CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage, float32 fVcc)
{
	CVoltageConfiguration*	pcHigher;
	CVoltageConfiguration*	pcLower;

	if (!HasPower(fVcc))
	{
		return MAX_INT64;
	}

	pcHigher = GetEqualOrHigherConfiguration(fVcc);
	if (pcHigher != NULL)
	{
		if (pcHigher->GetVcc() == fVcc)
		{
			return pcHigher->CalculateHoldTime(fOutVoltage, fPortVoltage);
		}

		pcLower = GetLowerConfiguration(fVcc);
		if (pcLower != NULL)
		{
			return LinearInterpolateTime(pcHigher->CalculateHoldTime(fOutVoltage, fPortVoltage), pcLower->CalculateHoldTime(fOutVoltage, fPortVoltage), pcHigher->GetVcc(), pcLower->GetVcc(), fVcc);
		}
	}
	return MAX_INT64;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ETraceValue CFamilyVoltageConfiguration::GetValue(float32 fVoltage, float32 fVcc)
{
	if (!HasPower(fVcc) || !IsVoltage(fVoltage))
	{
		return TV_Undriven;
	}

	if (fVoltage >= GetHighVoltageIn(fVcc))
	{
		return TV_High;
	}

	if (fVoltage <= GetLowVoltageIn(fVcc))
	{
		return TV_Low;
	}

	return TV_Unsettled;
}


//////////////////////////////////////////////////////////////////////////
//
// A port with no voltage starts from the low voltage out.
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::CalculateStartVoltage(float32 fPortVoltage, float32 fVcc)
{
	if (IsVoltage(fPortVoltage))
	{
		return fPortVoltage;
	}
	else
	{
		return GetVoltageOut(false, fVcc);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Infinite when there is no power, or fVcc is outside the family's
// configurations.
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::GetVoltsPerTimeLowToHigh(float32 fVcc)
{
	CVoltageConfiguration*	pcHigher;
	CVoltageConfiguration*	pcLower;

	if (!HasPower(fVcc))
	{
		return InfiniteVoltsPerTime();
	}

	pcHigher = GetEqualOrHigherConfiguration(fVcc);
	if (pcHigher != NULL)
	{
		if (pcHigher->GetVcc() == fVcc)
		{
			return pcHigher->GetVoltsPerTimeLowToHigh();
		}

		pcLower = GetLowerConfiguration(fVcc);
		if (pcLower != NULL)
		{
			return LinearInterpolateVoltage(pcHigher->GetVoltsPerTimeLowToHigh(), pcLower->GetVoltsPerTimeLowToHigh(), pcHigher->GetVcc(), pcLower->GetVcc(), fVcc);
		}
	}
	return InfiniteVoltsPerTime();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::GetVoltsPerTimeHighToLow(float32 fVcc)
{
	CVoltageConfiguration*	pcHigher;
	CVoltageConfiguration*	pcLower;

	if (!HasPower(fVcc))
	{
		return InfiniteVoltsPerTime();
	}

	pcHigher = GetEqualOrHigherConfiguration(fVcc);
	if (pcHigher != NULL)
	{
		if (pcHigher->GetVcc() == fVcc)
		{
			return pcHigher->GetVoltsPerTimeHighToLow();
		}

		pcLower = GetLowerConfiguration(fVcc);
		if (pcLower != NULL)
		{
			return LinearInterpolateVoltage(pcHigher->GetVoltsPerTimeHighToLow(), pcLower->GetVoltsPerTimeHighToLow(), pcHigher->GetVcc(), pcLower->GetVcc(), fVcc);
		}
	}
	return InfiniteVoltsPerTime();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CFamilyVoltageConfiguration::LinearInterpolateVoltage(float32 fHigherVoltage, float32 fLowerVoltage, float32 fHigherVcc, float32 fLowerVcc, float32 fVcc)
{
	float32	fFraction;

	fFraction = (fVcc - fLowerVcc) / (fHigherVcc - fLowerVcc);
	return fLowerVoltage + (fHigherVoltage - fLowerVoltage) * fFraction;
}


//////////////////////////////////////////////////////////////////////////
//
// The difference is scaled in float32 and truncated, as in Logicim.
//
//////////////////////////////////////////////////////////////////////////
int64 CFamilyVoltageConfiguration::LinearInterpolateTime(int64 iHigherTime, int64 iLowerTime, float32 fHigherVcc, float32 fLowerVcc, float32 fVcc)
{
	float32	fFraction;

	fFraction = (fVcc - fLowerVcc) / (fHigherVcc - fLowerVcc);
	return iLowerTime + (int64)((iHigherTime - iLowerTime) * fFraction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFamilyVoltageConfiguration::NumConfigurations(void) { return macConfigurations.NumElements(); }
CVoltageConfiguration* CFamilyVoltageConfiguration::GetConfiguration(size uiIndex) { return macConfigurations.SafeGet(uiIndex); }
char* CFamilyVoltageConfiguration::GetFamily(void) { return mszFamily.Text(); }

