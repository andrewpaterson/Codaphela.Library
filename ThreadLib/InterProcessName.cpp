#include <stdio.h>
#include <string.h>
#include "InterProcessName.h"


//The '#' and the hash.
#define INTER_PROCESS_NAME_HASH_LENGTH	17


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterProcessName(CChars* psz, const char* szPrefix, char* szName)
{
	psz->Init(szPrefix);
	psz->Append(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ShortenInterProcessName(CChars* psz, char* szName, int iMaxLength)
{
	size			uiLength;
	uint64			ulHash;
	size			i;
	char			szHash[INTER_PROCESS_NAME_HASH_LENGTH + 1];

	uiLength = strlen(szName);
	if (uiLength <= (size)iMaxLength)
	{
		psz->Init(szName);
		return;
	}

	ulHash = 0xcbf29ce484222325ULL;
	for (i = 0; i < uiLength; i++)
	{
		ulHash ^= (uint8)szName[i];
		ulHash *= 0x100000001b3ULL;
	}
	snprintf(szHash, sizeof(szHash), "#%016llx", (unsigned long long)ulHash);

	psz->Init(szName, 0, iMaxLength - INTER_PROCESS_NAME_HASH_LENGTH);
	psz->Append(szHash);
}

