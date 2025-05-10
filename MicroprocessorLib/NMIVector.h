#ifndef __NMI_VECTOR_H__
#define __NMI_VECTOR_H__
#include "InterruptVector.h"


class CNMIVector : public CInterruptVector
{
public:
    void    Init(void);
    uint16  GetAddress(CW65C816* pcCPU) override;
};


#endif // __NMI_VECTOR_H__

