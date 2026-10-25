//Memory and string functions for the 65C64.  All but strstr are in string.asm, written with the 65C64's block and
//vector operations; the C++ versions here are their reference and are not built for the 65C64.  memcpy, memset and
//memcmp work a 32 bit word (uint32_t: an int is 16 bits on the W65C816) at a time when the addresses are 4 byte
//aligned, otherwise a byte at a time.
#include <string.h>
#include <stdint.h>


#ifndef __65C64__
void* memcpy(void* pvDest, const void* pvSource, size_t uiBytes)
{
	char*		pcDest;
	const char*	pcSource;

	pcDest = (char*)pvDest;
	pcSource = (const char*)pvSource;
	if ((((size_t)pcDest | (size_t)pcSource) & 3) == 0)
	{
		while (uiBytes >= 4)
		{
			*(uint32_t*)pcDest = *(const uint32_t*)pcSource;
			pcDest += 4;
			pcSource += 4;
			uiBytes -= 4;
		}
	}
	while (uiBytes > 0)
	{
		*pcDest++ = *pcSource++;
		uiBytes--;
	}
	return pvDest;
}


void* memmove(void* pvDest, const void* pvSource, size_t uiBytes)
{
	char*		pcDest;
	const char*	pcSource;

	pcDest = (char*)pvDest;
	pcSource = (const char*)pvSource;
	if ((pcDest < pcSource) || (pcDest >= pcSource + uiBytes))
	{
		return memcpy(pvDest, pvSource, uiBytes);
	}

	//The areas overlap with the destination above the source: copy from the end, a word at a time when both ends are
	//word aligned.
	pcDest += uiBytes;
	pcSource += uiBytes;
	if ((((size_t)pcDest | (size_t)pcSource) & 3) == 0)
	{
		while (uiBytes >= 4)
		{
			pcDest -= 4;
			pcSource -= 4;
			*(uint32_t*)pcDest = *(const uint32_t*)pcSource;
			uiBytes -= 4;
		}
	}
	while (uiBytes > 0)
	{
		*--pcDest = *--pcSource;
		uiBytes--;
	}
	return pvDest;
}


void* memset(void* pvDest, int iValue, size_t uiBytes)
{
	char*			pcDest;
	uint32_t		uiValue;

	pcDest = (char*)pvDest;
	if (((size_t)pcDest & 3) == 0)
	{
		uiValue = (unsigned char)iValue;
		uiValue |= uiValue << 8;
		uiValue |= uiValue << 16;
		while (uiBytes >= 4)
		{
			*(uint32_t*)pcDest = uiValue;
			pcDest += 4;
			uiBytes -= 4;
		}
	}
	while (uiBytes > 0)
	{
		*pcDest++ = (char)iValue;
		uiBytes--;
	}
	return pvDest;
}


int memcmp(const void* pv1, const void* pv2, size_t uiBytes)
{
	const unsigned char*	puc1;
	const unsigned char*	puc2;

	puc1 = (const unsigned char*)pv1;
	puc2 = (const unsigned char*)pv2;
	if ((((size_t)puc1 | (size_t)puc2) & 3) == 0)
	{
		//Equal words are skipped; the bytes of the first word that differs are compared below.
		while ((uiBytes >= 4) && (*(const uint32_t*)puc1 == *(const uint32_t*)puc2))
		{
			puc1 += 4;
			puc2 += 4;
			uiBytes -= 4;
		}
	}
	while (uiBytes > 0)
	{
		if (*puc1 != *puc2)
		{
			return *puc1 - *puc2;
		}
		puc1++;
		puc2++;
		uiBytes--;
	}
	return 0;
}


void* memchr(const void* pv, int iValue, size_t uiBytes)
{
	const unsigned char*	puc;

	puc = (const unsigned char*)pv;
	while (uiBytes > 0)
	{
		if (*puc == (unsigned char)iValue)
		{
			return (void*)puc;
		}
		puc++;
		uiBytes--;
	}
	return NULL;
}


size_t strlen(const char* sz)
{
	const char*	pc;

	pc = sz;
	while (*pc)
	{
		pc++;
	}
	return pc - sz;
}


char* strcpy(char* szDest, const char* szSource)
{
	char*	pc;

	pc = szDest;
	while ((*pc++ = *szSource++))
	{
	}
	return szDest;
}


char* strncpy(char* szDest, const char* szSource, size_t uiChars)
{
	size_t	i;

	for (i = 0; (i < uiChars) && szSource[i]; i++)
	{
		szDest[i] = szSource[i];
	}
	for (; i < uiChars; i++)
	{
		szDest[i] = '\0';
	}
	return szDest;
}


char* strcat(char* szDest, const char* szSource)
{
	strcpy(szDest + strlen(szDest), szSource);
	return szDest;
}


char* strncat(char* szDest, const char* szSource, size_t uiChars)
{
	char*	pc;

	pc = szDest + strlen(szDest);
	while ((uiChars > 0) && *szSource)
	{
		*pc++ = *szSource++;
		uiChars--;
	}
	*pc = '\0';
	return szDest;
}


int strcmp(const char* sz1, const char* sz2)
{
	while (*sz1 && (*sz1 == *sz2))
	{
		sz1++;
		sz2++;
	}
	return (unsigned char)*sz1 - (unsigned char)*sz2;
}


int strncmp(const char* sz1, const char* sz2, size_t uiChars)
{
	while ((uiChars > 0) && *sz1 && (*sz1 == *sz2))
	{
		sz1++;
		sz2++;
		uiChars--;
	}
	if (uiChars == 0)
	{
		return 0;
	}
	return (unsigned char)*sz1 - (unsigned char)*sz2;
}


char* strchr(const char* sz, int c)
{
	for (;;)
	{
		if (*sz == (char)c)
		{
			return (char*)sz;
		}
		if (!*sz)
		{
			return NULL;
		}
		sz++;
	}
}


char* strrchr(const char* sz, int c)
{
	const char*	pcFound;

	pcFound = NULL;
	for (;;)
	{
		if (*sz == (char)c)
		{
			pcFound = sz;
		}
		if (!*sz)
		{
			return (char*)pcFound;
		}
		sz++;
	}
}


#endif // __65C64__


//The places sz could hold szFind start with szFind's first character: memchr finds them and memcmp checks them.
char* strstr(const char* sz, const char* szFind)
{
	size_t		uiFind;
	size_t		uiLength;
	const char*	pc;

	uiFind = strlen(szFind);
	if (uiFind == 0)
	{
		return (char*)sz;
	}
	uiLength = strlen(sz);
	while (uiLength >= uiFind)
	{
		pc = (const char*)memchr(sz, szFind[0], uiLength - uiFind + 1);
		if (!pc)
		{
			return NULL;
		}
		uiLength -= pc - sz;
		sz = pc;
		if (memcmp(sz, szFind, uiFind) == 0)
		{
			return (char*)sz;
		}
		sz++;
		uiLength--;
	}
	return NULL;
}
