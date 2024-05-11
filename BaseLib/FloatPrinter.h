#ifndef __FLOAT_PRINTER_H__
#define __FLOAT_PRINTER_H__
#include "PrimitiveTypes.h"


char* FloatToString(char* szDest, size iDestLength, float f, int iMaxDecimals = -1, bool bAppendF = false);
char* DoubleToString(char* szDest, size iDestLength, double f, int iMaxDecimals = -1, bool bAppendF = false);


#endif // __FLOAT_PRINTER_H__

