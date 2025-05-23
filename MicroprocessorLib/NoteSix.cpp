#include "W65C816.h"
#include "W65C816State.h"
#include "NoteSix.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNoteSix::Init(void)
{
	COperation::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNoteSix::Execute(CW65C816* pcCPU)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNoteSix::MustExecute(CW65C816* pcCPU)
{
	return pcCPU->GetState()->NoteSix();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNoteSix::Print(CChars* psz)
{
	psz->Append("Note(6)");
}

