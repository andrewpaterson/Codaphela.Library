#ifndef __INSTRUCTION_CYCLES_H__
#define __INSTRUCTION_CYCLES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Malloc.h"
#include "W65BusCycle.h"
#include "AddressingMode.h"
#include "NotMemoryLock.h"


class CW65InstructionCycles
{
protected:
    CW65BusCycleArray      mapcCycles;
    EAddressingMode     meAddressingMode;

public:
    void                Init(EAddressingMode eAddressingMode, CW65BusCycleArray* papcCycles);
    void                Kill(void);

    EAddressingMode     GetAddressingMode(void);
    CW65BusCycle*          GetBusCycle(int16 miIndex);
    size                Size(void);

protected:
    void ValidateDoneOperation(void);
    void ValidateComponent(void);
};


#endif // __INSTRUCTION_CYCLES_H__

