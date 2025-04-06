#include "GerberCommandFileAttribute.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandFileAttribute::Init(size iNameLength)
{
	CGerberCommand::Init(GC_TF);
	szName.InitLength(iNameLength);
	szValues.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandFileAttribute::Kill(void)
{
	szValues.Kill();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberCommandFileAttribute::NameText(void)
{
	return szName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberCommandFileAttribute::AddValue(size iValueLength)
{
	CChars* psz;

	psz = szValues.AddLength(iValueLength);
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberCommandFileAttribute::IsFileAttribute(void)
{
	return true;
}


