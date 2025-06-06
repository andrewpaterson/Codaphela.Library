#include "W65C816.h"
#include "ReadOpcode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadOpcode::Init(void)
{
	CDataOperation::Init(true, true, true, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadOpcode::Execute(CW65C816* pcCPU)
{
	pcCPU->ReadOpcode();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadOpcode::IsInitialSide(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadOpcode::Print(CChars* psz)
{
	psz->Append("Read(Opcode)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CReadOpcode::GetDone8(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CReadOpcode::GetDone16(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadOpcode::IsFetchOpcode(void)
{
	return true;
}

