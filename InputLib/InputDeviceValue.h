/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __INPUT_DEVICE_VALUE_H__
#define __INPUT_DEVICE_VALUE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LinkedListTemplateFreeList.h"
#include "BaseLib/Chars.h"

#define INPUT_VALUES_CHUNK_SIZE		4


class CInputDevice;
class CInputSourceDesc;
struct SInputDeviceValueSource
{
	CInputDevice*		pcDevice;
	CInputSourceDesc*	pcSource;  //This isn't a CInputVirtualSourceDesc because the device is known.
};


struct SInputDeviceValue : SInputDeviceValueSource
{
	float				fValue;

	void Init(CInputDevice* pcDevice, CInputSourceDesc* pcSource, float fValue);
	void ToString(CChars* psz);
};


class CInputDeviceValue : public SInputDeviceValue
{
public:
	unsigned int	uiSequence;
	int				iOrder;
	bool			bRestEmitted;

	void Init(CInputDevice* pcDevice, CInputSourceDesc* pcSource, float fValue, unsigned int uiSequence, bool bRestEmitted, int iOrder);
};


typedef CLinkedListTemplateFreeList<CInputDeviceValue>	CListInputDeviceValue;
typedef CArrayTemplate<CInputDeviceValue*>	CArrayInputDeviceValuePtr;


int SortInputEventFunc(const void* pv1, const void* pv2);
int SortInputEventPtrFunc(const void* pv1, const void* pv2);


class CInputDeviceValues
{
public:
	CListInputDeviceValue	mlcValues;

	void				Init(void);
	void				Kill(void);
	void				Add(CInputDevice* pcDevice, CInputSourceDesc* pcSource, float fValue, unsigned int uiSequence, bool bRestEmitted, int iOrder);
	void				Clear(void);
	void				Sort(void);
	CInputDeviceValue*	StartIteration(void);
	CInputDeviceValue*	Iterate(CInputDeviceValue* pcInputDeviceValue);
	void				ToString(CChars* psz);
	void				Dump(void);
};


#endif // !__INPUT_DEVICE_VALUE_H__

