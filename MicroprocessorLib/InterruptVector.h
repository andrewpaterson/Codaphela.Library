#ifndef __INTERRUPT_VECTOR_H__
#define __INTERRUPT_VECTOR_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "ReflectiveData.h"


class CW65C816;
class CInterruptVector
{
public:
	virtual uint16 GetAddress(CW65C816* pcCPU) =0;
};


#endif // __INTERRUPT_VECTOR_H__

