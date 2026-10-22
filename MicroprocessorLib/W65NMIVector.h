#ifndef __NMI_VECTOR_H__
#define __NMI_VECTOR_H__
#include "W65InterruptVector.h"


class CW65NMIVector : public CW65InterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __NMI_VECTOR_H__

