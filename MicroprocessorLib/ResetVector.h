#ifndef __RESET_VECTOR_H__
#define __RESET_VECTOR_H__
#include "InterruptVector.h"


class CResetVector : public CInterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __RESET_VECTOR_H__

