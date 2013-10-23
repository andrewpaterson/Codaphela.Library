#include "DistDetachedFroms.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistDetachedFroms::Init(void)
{
	apcDetached.Init(1024);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistDetachedFroms::Kill(void)
{
	apcDetached.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistDetachedFroms::Add(CBaseObject* pcObject)
{
	apcDetached.Add(&pcObject);
}
