#include "ArrayFloat.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayFloat::AddList(double iStop, ...)
{
	va_list		vaMarker;
	double		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, double);
	while (iValue != iStop)
	{
		Add((float)iValue);
		iValue = va_arg(vaMarker, double);
	}
	va_end(vaMarker);
}

