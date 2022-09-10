#ifndef __FLOAT_PRINTER_H__
#define __FLOAT_PRINTER_H__


char* FloatToString(char* szDest, int iDestLength, float f, int iMaxDecimals = -1, bool bAppendF = false);
char* DoubleToString(char* szDest, int iDestLength, double f, int iMaxDecimals = -1, bool bAppendF = false);


#endif // !__FLOAT_PRINTER_H__

