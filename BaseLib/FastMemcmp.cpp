/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "FastMemcmp.h"


memcmp_fast_func ga_memcmp_fast[17];


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitMemcmpFast(void)
{
	memset(ga_memcmp_fast, 0, sizeof(ga_memcmp_fast));

	ga_memcmp_fast[0] = memcmp_fast_0bytes;
	ga_memcmp_fast[1] = memcmp_fast_1byte;
	ga_memcmp_fast[2] = memcmp_fast_2bytes;
	ga_memcmp_fast[3] = memcmp_fast_3bytes;
	ga_memcmp_fast[4] = memcmp_fast_4bytes;
	ga_memcmp_fast[5] = memcmp_fast_5bytes;
	ga_memcmp_fast[6] = memcmp_fast_6bytes;
	ga_memcmp_fast[7] = memcmp_fast_7bytes;

	ga_memcmp_fast[8] = memcmp_fast_8bytes;
	ga_memcmp_fast[9] = memcmp_fast_9bytes;
	ga_memcmp_fast[10] = memcmp_fast_10bytes;
	ga_memcmp_fast[11] = memcmp_fast_11bytes;
	ga_memcmp_fast[12] = memcmp_fast_12bytes;
	ga_memcmp_fast[13] = memcmp_fast_13bytes;
	ga_memcmp_fast[14] = memcmp_fast_14bytes;
	ga_memcmp_fast[15] = memcmp_fast_15bytes;
	ga_memcmp_fast[16] = memcmp_fast_16bytes;
}


int memcmp_fast(void* pv1, void* pv2, int iByteSize)
{
	//THIS IS NOT FASTER THAN memcmp.  Use memcmp instead.
	if (iByteSize <= 16)
	{
		return ga_memcmp_fast[iByteSize](pv1, pv2);
	}
	else
	{
		return memcmp(pv1, pv2, iByteSize);
	}
}


int memcmp_fast_0bytes(void* pvDest, void* pvSource)
{
	return 0;
}


int memcmp_fast_1byte(void* pv1, void* pv2)
{
	return (int)(((char*)pv1)[0] - ((char*)pv2)[0]);
}


int memcmp_fast_2bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[1] - ((char*)pv2)[1]);
}


int memcmp_fast_3bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[2] - ((char*)pv2)[2]);
}


int memcmp_fast_4bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[3] - ((char*)pv2)[3]);
}


int memcmp_fast_5bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[4] - ((char*)pv2)[4]);
}


int memcmp_fast_6bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[5] - ((char*)pv2)[5]);
}


int memcmp_fast_7bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[6] - ((char*)pv2)[6]);
}


int memcmp_fast_8bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[7] - ((char*)pv2)[7]);
}


int memcmp_fast_9bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[8] - ((char*)pv2)[8]);
}


int memcmp_fast_10bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[9] - ((char*)pv2)[9]);
}


int memcmp_fast_11bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[9] - ((char*)pv2)[9];		if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[10] - ((char*)pv2)[10]);
}


int memcmp_fast_12bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[9] - ((char*)pv2)[9];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[10] - ((char*)pv2)[10];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[11] - ((char*)pv2)[11]);
}


int memcmp_fast_13bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[9] - ((char*)pv2)[9];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[10] - ((char*)pv2)[10];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[11] - ((char*)pv2)[11];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[12] - ((char*)pv2)[12]);
}


int memcmp_fast_14bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[9] - ((char*)pv2)[9];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[10] - ((char*)pv2)[10];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[11] - ((char*)pv2)[11];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[12] - ((char*)pv2)[12];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[13] - ((char*)pv2)[13]);
}


int memcmp_fast_15bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[9] - ((char*)pv2)[9];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[10] - ((char*)pv2)[10];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[11] - ((char*)pv2)[11];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[12] - ((char*)pv2)[12];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[13] - ((char*)pv2)[13];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[14] - ((char*)pv2)[14]);
}


int memcmp_fast_16bytes(void* pv1, void* pv2)
{
	char c;
	c = ((char*)pv1)[0] - ((char*)pv2)[0];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[1] - ((char*)pv2)[1];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[2] - ((char*)pv2)[2];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[3] - ((char*)pv2)[3];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[4] - ((char*)pv2)[4];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[5] - ((char*)pv2)[5];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[6] - ((char*)pv2)[6];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[7] - ((char*)pv2)[7];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[8] - ((char*)pv2)[8];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[9] - ((char*)pv2)[9];		if (c != 0) return (int)c; 
	c = ((char*)pv1)[10] - ((char*)pv2)[10];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[11] - ((char*)pv2)[11];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[12] - ((char*)pv2)[12];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[13] - ((char*)pv2)[13];	if (c != 0) return (int)c; 
	c = ((char*)pv1)[14] - ((char*)pv2)[14];	if (c != 0) return (int)c; 
	return (int)(((char*)pv1)[15] - ((char*)pv2)[15]);
}

