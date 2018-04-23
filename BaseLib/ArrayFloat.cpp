#include "ArrayFloat.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayFloat::AddList(float iStop, ...)
{
	va_list		vaMarker;
	float		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, float);
	while (iValue != iStop)
	{
		Add(iValue);
		iValue = va_arg(vaMarker, float);
	}
	va_end(vaMarker);
}

