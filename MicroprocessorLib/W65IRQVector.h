#ifndef __IRQ_VECTOR_H__
#define __IRQ_VECTOR_H__
#include "W65InterruptVector.h"


class CW65IRQVector : public CW65InterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __IRQ_VECTOR_H__

