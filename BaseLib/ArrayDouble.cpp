#include "ArrayDouble.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayDouble::AddList(double iStop, ...)
{
	va_list		vaMarker;
	double		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, double);
	while (iValue != iStop)
	{
		Add(iValue);
		iValue = va_arg(vaMarker, double);
	}
	va_end(vaMarker);
}

