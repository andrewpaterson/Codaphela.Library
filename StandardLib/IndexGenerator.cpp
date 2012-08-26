#include "IndexGenerator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexGenerator::Init(void)
{
	moiNext = FIRST_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexGenerator::Kill(void)
{
	moiNext = INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexGenerator::PopIndex(void)
{
	OIndex	oi;

	oi = moiNext;
	StepNext();
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexGenerator::PeekIndex(void)
{
	return moiNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexGenerator::StepNext(void)
{
	moiNext++;
}