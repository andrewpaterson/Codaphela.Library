#ifndef __COP_VECTOR_H__
#define __COP_VECTOR_H__
#include "W65InterruptVector.h"


class CW65COPVector : public CW65InterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __COP_VECTOR_H__

