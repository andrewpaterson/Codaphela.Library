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
#ifndef __INPUT_DEVICES_READER_H__
#define __INPUT_DEVICES_READER_H__
#include "BaseLib/Define.h"
#include "BaseLib/Chars.h"
#include "BaseLib/MarkupDoc.h"
#include "CoreLib/TypeNames.h"
#include "InputDeviceDesc.h"


class CInputDevices;
class CInputDataFormat;
class CInputDevicesReader
{
public:
	CChars			mszDirectory;
	CInputDevices*	mpcInputDevices;

	void 				Init(CInputDevices* pcInputDevices, char* szDirectory);
	void 				Kill(void);

	BOOL				ReadInputDevicesXML(void);
	BOOL				ReadCategories(CMarkupTag* pcParentTag);
	BOOL				ReadDataFormats(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames);
	BOOL				ReadDataFormatChannels(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames, CInputDataFormat* pcFormat);
	BOOL				ReadNamedDataFormat(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames);
	CInputDataFormat*	ReadDefinedDataFormat(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames);
	BOOL				ReadDevices(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames);
	BOOL				ReadDevice(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames, BOOL bPhysical);
	BOOL				ReadChanges(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc);
	BOOL				ReadSources(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc, CTypeNames* pcTypeNames, BOOL bDump);
	BOOL				ReadSource(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc, BOOL bDump);
	BOOL				ReadSourceValue(CMarkupTag* pcParentTag, CInputSourceDesc* pcSourceDesc);
	BOOL				ReadValueChannel(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue);
	BOOL				ReadStateDetail(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue);
	BOOL				ReadDeltaDetail(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue);
	BOOL 				ReadVariables1(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc);
	BOOL 				ReadVariables2(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc);
	BOOL 				ReadVariableValue1(CMarkupTag* pcValueTag, CInputDeviceVariableDesc* pcVariable, int* piInitialValue);
	BOOL 				ReadVariableValue2(CMarkupTag* pcValueTag, CInputDeviceVariableDesc* pcVariable, CInputChordDescs* pcChordDescs);
	BOOL				ReadVariableValueVariable(CMarkupTag* pcTag, CInputDeviceVariableValueDesc* pcVariableValue);
	void				CopyVirtualDeviceDescChords(CInputDeviceCopyContext* pcContext);
};


#endif // __INPUT_DEVICES_READER_H__

