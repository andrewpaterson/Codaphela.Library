#include "W65C816.h"
#include "W65C816State.h"
#include "AddressBank.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddressBank::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddressBank::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CAddressBank::GetOffset(CW65C816* pcCPU)
{
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CAddressBank::GetBank(CW65C816* pcCPU)
{
    return pcCPU->GetState()->GetAddress()->GetBank();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddressBank::Print(CChars* psz)
{
    psz->Append("AAB,");
}

