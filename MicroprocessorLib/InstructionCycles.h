#ifndef __INSTRUCTION_CYCLES_H__
#define __INSTRUCTION_CYCLES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Malloc.h"
#include "BusCycleArray.h"
#include "AddressingMode.h"
#include "NotMemoryLock.h"


class CInstructionCycles
{
protected:
    CBusCycleArray      mapcCycles;
    EAddressingMode     meAddressingMode;

public:
    void                Init(EAddressingMode eAddressingMode, CBusCycleArray* papcCycles);
    void                Kill(void);

    EAddressingMode     GetAddressingMode(void);
    CBusCycle*          GetBusCycle(int16 miIndex);
    size                Size(void);

protected:
    void ValidateDoneOperation(void);
    void Validate(void);
};


#endif // __INSTRUCTION_CYCLES_H__

