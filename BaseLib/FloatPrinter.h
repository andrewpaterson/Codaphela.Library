#ifndef __FLOAT_PRINTER_H__
#define __FLOAT_PRINTER_H__
#include "Bool.h"


char* FloatToString(char* szDest, int iDestLength, float f, int iMaxDecimals = -1, BOOL bAppendF = FALSE);


#endif // !__FLOAT_PRINTER_H__

