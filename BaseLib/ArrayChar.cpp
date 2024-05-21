#include "ArrayChar.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChar::AddList(char iStop, ...)
{
	va_list		vaMarker;
	char		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, char);
	while (iValue != iStop)
	{
		Add((char)iValue);
		iValue = va_arg(vaMarker, char);
	}
	va_end(vaMarker);
}

