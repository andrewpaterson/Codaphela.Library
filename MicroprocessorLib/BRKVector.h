#ifndef __BRK_VECTOR_H__
#define __BRK_VECTOR_H__
#include "InterruptVector.h"


class CBRKVector : public CInterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __BRK_VECTOR_H__

