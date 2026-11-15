#include "ArrayChar.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class PRIMITIVE>
void CArrayCharBase<PRIMITIVE>::AddList(char iStop, ...)
{
	va_list		vaMarker;
	char		iValue;

	va_start(vaMarker, iStop);
	iValue = (char)va_arg(vaMarker, int);  //char is promoted to int when passed through ...
	while (iValue != iStop)
	{
		this->Add((char)iValue);
		iValue = (char)va_arg(vaMarker, int);  //char is promoted to int when passed through ...
	}
	va_end(vaMarker);
}


//The only instances: ArrayChar.h declares them extern.
template class CArrayCharBase<CArrayTemplatePrimitive<char> >;
template class CArrayCharBase<CArrayLargeTemplatePrimitive<char> >;

