//Calls each function Libc65's string816.asm and string816_16.asm define on the W65C816, as declared, so the compiled
//object uses each register entry by the name the fixed rule gives its declaration (TestCPP816AssemblyStringEntries).
//It is only compiled, never run.  The counts aren't constants so no call is made into inline code.
#include <string.h>
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/StringHelper.h"


volatile int	giCount;
void*			gpv;


void StringCalls(char* sz1, char* sz2)
{
	size_t			uiCount;
	unsigned short	uiCount16;

	uiCount = (size_t)giCount;
	uiCount16 = (unsigned short)giCount;

	gpv = memcpy(sz1, sz2, uiCount);
	gpv = memmove(sz1, sz2, uiCount);
	gpv = memset(sz1, giCount, uiCount);
	giCount += memcmp(sz1, sz2, uiCount);
	gpv = memchr(sz1, giCount, uiCount);
	giCount += (int)strlen(sz1);
	gpv = strcpy(sz1, sz2);
	gpv = strncpy(sz1, sz2, uiCount);
	gpv = strcat(sz1, sz2);
	gpv = strncat(sz1, sz2, uiCount);
	giCount += strcmp(sz1, sz2);
	giCount += strncmp(sz1, sz2, uiCount);
	gpv = strchr(sz1, giCount);
	gpv = strrchr(sz1, giCount);

	gpv = memcpy16(sz1, sz2, uiCount16);
	gpv = memmove16(sz1, sz2, uiCount16);
	gpv = memset16(sz1, giCount, uiCount16);
	giCount += memcmp16(sz1, sz2, uiCount16);
	giCount += (int)strlen16(sz1);

	MemSwp(sz1, sz2, (lsize)giCount);
	MemSwp16(sz1, sz2, (size)giCount);
	giCount += (int)FindFirstSetBitLarge(sz1, (lsize)giCount);
	giCount += (int)FindFirstClearBitLarge(sz1, (lsize)giCount);
	giCount += (int)FindLastSetBitLarge(sz1, (lsize)giCount);
	giCount += (int)FindLastClearBitLarge(sz1, (lsize)giCount);
	giCount += (int)FindFirstSetBit(sz1, (size)giCount);
	giCount += (int)FindFirstClearBit(sz1, (size)giCount);
	giCount += (int)FindLastSetBit(sz1, (size)giCount);
	giCount += (int)FindLastClearBit(sz1, (size)giCount);
	giCount += (int)FindFirstClearBit((uint32)giCount);
}
