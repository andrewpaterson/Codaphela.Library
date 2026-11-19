//Calls of the C library's memory and file functions with counts no wider than a size_t on the W65C816: a size, a
//size_t, a sizeof, an int, a byte.  Each must be the C library's own function, though BaseLib/PrimitiveTypes.h has
//overloads of the names (TestCPP816AssemblyLargeCounts).  It is only compiled, never run.  The counts aren't constants
//so no call is made into inline code.
#include <stdio.h>
#include "BaseLib/PrimitiveTypes.h"


struct SThing
{
	long	l;
	int		i;
};


volatile size	guiNatural;
volatile uint8	guiByte;
volatile int	giInt;
void*			gpv;
int				gi;


void NaturalCounts(char* sz1, char* sz2, FILE* pcFile)
{
	gpv = memcpy(sz1, sz2, guiNatural);
	gpv = memmove(sz1, sz2, giInt);
	gpv = memset(sz1, 0, guiByte);
	gi = memcmp(sz1, sz2, sizeof(SThing) * guiNatural);
	gpv = memchr(sz1, 'a', guiNatural);
	gpv = malloc(guiNatural);
	gpv = malloc(sizeof(SThing) + giInt);
	gpv = realloc(gpv, giInt);
	gpv = calloc(guiNatural, sizeof(SThing));
	gpv = calloc(guiByte, giInt);
	gi = (int)fread(sz1, 1, guiNatural, pcFile);
	gi = (int)fwrite(sz1, sizeof(SThing), giInt, pcFile);
}
