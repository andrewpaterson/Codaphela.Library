#include "BaseLib/Logger.h"
#include "W65C816.h"
#include "W65C816State.h"
#include "ExecuteIf8Bit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExecuteIf16Bit::Init(CW65C1616Func fFunction, EWidthFromRegister eWidth)
{
    mfFunction = fFunction;
    meWidth = eWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExecuteIf16Bit::Execute(CW65C1616* pcCPU)
{
    if (meWidth == WFR_M)
    {
        if (pcCPU->GetState()->IsMemory16Bit())
        {
            (pcCPU->*mfFunction)();
        }
    }
    else if (meWidth == WFR_XY)
    {
        if (pcCPU->GetState()->IsIndex16Bit())
        {
            (pcCPU->*mfFunction)();
        }
    }
    else
    {
        LOG_ERROR("Unknown Width.");
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExecuteIf16Bit::Print(CChars* psz)
{
    //Work out some way of getting the function name out.
    psz->Append("Execute_If_16Bit_");
    psz->Append(CWidthFromRegister::GetName(meWidth));
    psz->Append("(HELP!)");
}

