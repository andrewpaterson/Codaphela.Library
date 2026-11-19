//Calls of the C library's memory and file functions with counts wider than a size_t on the W65C816: an lsize, a long,
//a long long, a constant made with KB.  Each must be the 32 suffixed function's, chosen by BaseLib/PrimitiveTypes.h's
//overloads from the count's type (TestCPP816AssemblyLargeCounts).  It is only compiled, never run.  The counts aren't
//constants so no call is made into inline code.
#include <stdio.h>
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Numbers.h"


volatile lsize	guiLarge;
volatile long	giLong;
volatile int64	giLongLong;
void*			gpv;
int				gi;


void LargeCounts(char* sz1, char* sz2, FILE* pcFile)
{
	gpv = memcpy(sz1, sz2, guiLarge);
	gpv = memmove(sz1, sz2, giLong);
	gpv = memset(sz1, 0, giLongLong);
	gi = memcmp(sz1, sz2, guiLarge);
	gpv = memchr(sz1, 'a', guiLarge);
	gpv = malloc(guiLarge);
	gpv = malloc(70 KB);
	gpv = realloc(gpv, guiLarge);
	gpv = calloc(guiLarge, 4);
	gpv = calloc(4, guiLarge);
	gi = (int)fread(sz1, 1, guiLarge, pcFile);
	gi = (int)fwrite(sz1, guiLarge, 1, pcFile);
}
