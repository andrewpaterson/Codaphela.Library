/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INPUT_ACTION_CONFIG_H__
#define __INPUT_ACTION_CONFIG_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Unknown.h"
#include "InputDeviceValue.h"
#include "InputCategoryGeneric.h"


enum EInputChordType
{
	ICT_Unknown,
	ICT_Toggle,  //ActionEventFunc expects only 1 or 0.
	ICT_Range,  //ActionEventFunc can handle 1 to 0.
};


enum EInputEvaluatorType
{
	IET_Unknown,

	IET_SpecificSource,
	IET_GenericSource,
	IET_AnyToggleSource,
};


class CInputVirtualDevice;
struct SInputConfigSpecificSource
{
	CInputDevice*		pcDevice;
	CInputSourceDesc*	pcSource;
};
struct SInputConfigGenericSource
{
	CInputCategoryGeneric*	pcGeneric;
};
struct SInputConfigAnyToggleSource
{
};


union UInputConfig
{
	SInputConfigSpecificSource	sDeviceSource;
	SInputConfigGenericSource	sDeviceGeneric;
	SInputConfigAnyToggleSource	sDeviceAny;
};


class CInputVirtualDevice;
class CInputSourceDesc;
class CInputVirtualDeviceSource;
class CInputSourceEvaluator : public CUnknown
{
CONSTRUCTABLE(CInputSourceEvaluator);
protected:
	EInputEvaluatorType	meEvaluatorType;
	EInputChordType		meChordType;

	UInputConfig		muConfig;

	float				mfStateDeadzone;
	float				mfDeltaSensitivity;

public:
	void 							Init(void);
	void 							Kill(void);
	void							Copy(CInputSourceEvaluator* pcSource, CInputDevice* pcDevice);
	void							ToString(CChars* psz);

	void							SetSpecificSource(CInputVirtualDeviceSource* pcSource, EInputChordType eType);
	void							SetSpecificSource(CInputDevice* pcDevice, CInputSourceDesc* pcSource, EInputChordType eType);
	void							SetGenericSource(CInputCategoryGeneric* pcGeneric, EInputChordType eType);
	void							SetAnyToggleSource(void);
	BOOL							Evaluate(SInputDeviceValue* psEvent, float* pfValue);
	BOOL							EvaluateSpecificSource(float* pfValue, SInputDeviceValue* psEvent, SInputConfigSpecificSource* psConfig);
	BOOL							EvaluateGenericSource(float* pfValue, SInputDeviceValue* psEvent, SInputConfigGenericSource* psConfig);
	BOOL							EvaluateAnyToggleSource(float* pfValue, SInputDeviceValue* psEvent, SInputConfigAnyToggleSource* psConfig);
	BOOL							EvaluateMatching(float* pfValue, SInputDeviceValue* psEvent);

	EInputEvaluatorType				GetEvaluatorType(void);
	EInputChordType					GetChordType(void);
	UInputConfig*					GetConfig(void);
	float							GetStateDeadzone(void);
	float							GetDeltaSensitivity(void);
	SInputConfigSpecificSource*		GetDeviceSource(void);
	SInputConfigGenericSource*		GetDeviceGeneric(void);
	SInputConfigAnyToggleSource*	GetDeviceAny(void);
};


#endif // __INPUT_ACTION_CONFIG_H__

