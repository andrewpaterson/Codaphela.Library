#ifndef __COP_VECTOR_H__
#define __COP_VECTOR_H__
#include "InterruptVector.h"


class CCOPVector : public CInterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65C816* pcCPU) override;
};


#endif // __COP_VECTOR_H__

