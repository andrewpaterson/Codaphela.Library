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
	iValue = (char)va_arg(vaMarker, int);  //char is promoted to int when passed through ...
	while (iValue != iStop)
	{
		Add((char)iValue);
		iValue = (char)va_arg(vaMarker, int);  //char is promoted to int when passed through ...
	}
	va_end(vaMarker);
}

