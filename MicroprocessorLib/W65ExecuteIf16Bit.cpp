#include "BaseLib/Logger.h"
#include "W65CCore.h"
#include "W65ExecuteIf16Bit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65ExecuteIf16Bit::Init(CW65CFunc fFunction, EWidthFromRegister eWidth, bool bInitialSide)
{
    CW65Operation::Init();

    mfFunction = fFunction;
    meWidth = eWidth;
    mbInitialSide = bInitialSide;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65ExecuteIf16Bit::Execute(CW65CCore* pcCPU)
{
    if (meWidth == WFR_M)
    {
        if (pcCPU->IsMemory16Bit())
        {
            (pcCPU->*mfFunction)();
        }
    }
    else if (meWidth == WFR_XY)
    {
        if (pcCPU->IsIndex16Bit())
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
bool CW65ExecuteIf16Bit::IsInitialSide(void)
{
    return mbInitialSide;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65ExecuteIf16Bit::Print(CChars* psz)
{
    //Work out some way of getting the function name out.
    psz->Append("Execute_If_16Bit_");
    psz->Append(CW65WidthFromRegister::GetName(meWidth));
    psz->Append("(HELP!)");
}

