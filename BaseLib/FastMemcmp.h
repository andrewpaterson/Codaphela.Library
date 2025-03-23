/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __FAST_MEMCMP_H__
#define __FAST_MEMCMP_H__


typedef int  (*memcmp_fast_func)(void*, void*);
extern memcmp_fast_func ga_memcmp_fast[17];


int	memcmp_fast(void* pv1, void* pv2, int iByteSize);
void InitMemcmpFast(void);

int memcmp_fast_0bytes(void* pvDest, void* pvSource);  //For completeness...
int memcmp_fast_1byte(void* pvDest, void* pvSource);
int memcmp_fast_2bytes(void* pvDest, void* pvSource);
int memcmp_fast_3bytes(void* pvDest, void* pvSource);
int memcmp_fast_4bytes(void* pvDest, void* pvSource);
int memcmp_fast_5bytes(void* pvDest, void* pvSource);
int memcmp_fast_6bytes(void* pvDest, void* pvSource);
int memcmp_fast_7bytes(void* pvDest, void* pvSource);
int memcmp_fast_8bytes(void* pvDest, void* pvSource);
int memcmp_fast_9bytes(void* pvDest, void* pvSource);
int memcmp_fast_10bytes(void* pvDest, void* pvSource);
int memcmp_fast_11bytes(void* pvDest, void* pvSource);
int memcmp_fast_12bytes(void* pvDest, void* pvSource);
int memcmp_fast_13bytes(void* pvDest, void* pvSource);
int memcmp_fast_14bytes(void* pvDest, void* pvSource);
int memcmp_fast_15bytes(void* pvDest, void* pvSource);
int memcmp_fast_16bytes(void* pvDest, void* pvSource);


#endif //  __FAST_MEMCMP_H__

