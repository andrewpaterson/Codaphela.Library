#ifndef __ABORT_VECTOR_H__
#define __ABORT_VECTOR_H__
#include "InterruptVector.h"


class CAbortVector : public CInterruptVector
{
public: 
    uint16 GetAddress(CW65C816* pcCPU) override;
};


#endif // __ABORT_VECTOR_H__

