#ifndef __NATIVE_INPUT_H__
#define __NATIVE_INPUT_H__
#include "InputLib/InputDevices.h"


class CNativeInput
{
public:
	virtual void Kill(void) =0;
	virtual void AddDevicesTo(CInputDevices* pcDevices) =0;
	virtual void Update(CInputDevices* pcDevices, uint32 uiSequence) =0;
};


#endif // __NATIVE_INPUT_H__

