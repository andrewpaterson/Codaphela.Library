#ifndef __REDIRECT_PRINT_F__
#define __REDIRECT_PRINT_F__


typedef void (*PrintFunction)(const char* szString);


extern PrintFunction	gPrintFunction;


int eprintf(const char* szUnformatted, ...);


void SetPrintFunction(PrintFunction pPrintFunction);


#endif // __REDIRECT_PRINT_F__

