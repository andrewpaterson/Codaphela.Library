#include "W65C816.h"
#include "NoteOne.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNoteOne::Init(EWidthFromRegister eWidth)
{
    COperation::Init();

    meWidth = eWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNoteOne::Execute(CW65C816* pcCPU)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNoteOne::IsInitialSide(void)
{
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNoteOne::MustExecute(CW65C816* pcCPU)
{
    if (meWidth == WFR_M)
    {
        return pcCPU->IsMemory16Bit();
    }
    else if (meWidth == WFR_XY)
    {
        return pcCPU->IsIndex16Bit();
    }
    else
    {
        LOG_ERROR("Unknown Width.");
        return false;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNoteOne::Print(CChars* psz)
{
	psz->Append("Note(1)");
}

