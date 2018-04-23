#include "ArrayChar.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChar::AddList(int iStop, ...)
{
	va_list		vaMarker;
	int		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, int);
	while (iValue != iStop)
	{
		Add((char)iValue);
		iValue = va_arg(vaMarker, int);
	}
	va_end(vaMarker);
}

