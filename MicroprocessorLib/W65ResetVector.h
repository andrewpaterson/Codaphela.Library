#ifndef __RESET_VECTOR_H__
#define __RESET_VECTOR_H__
#include "W65InterruptVector.h"


class CW65ResetVector : public CW65InterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __RESET_VECTOR_H__

