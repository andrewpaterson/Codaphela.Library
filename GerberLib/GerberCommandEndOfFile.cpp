#include "GerberCommandEndOfFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandEndOfFile::Init(void)
{
	CGerberCommand::Init(GC_M02);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandEndOfFile::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberCommandEndOfFile::IsEndOfFile(void)
{
	return true;
}

