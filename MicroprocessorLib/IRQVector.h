#ifndef __IRQ_VECTOR_H__
#define __IRQ_VECTOR_H__
#include "InterruptVector.h"


class CIRQVector : public CInterruptVector
{
public:
    void    Init(void);
    uint16  GetAddress(CW65C816* pcCPU) override;
};


#endif // __IRQ_VECTOR_H__

