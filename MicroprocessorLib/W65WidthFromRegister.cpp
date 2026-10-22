#include "W65WidthFromRegister.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CW65WidthFromRegister::GetName(EWidthFromRegister e)
{
	if (e == WFR_M)
	{
		return "M";
	}
	else if (e == WFR_XY)
	{
		return "XY";
	}
	else
	{
		return "";
	}
}

