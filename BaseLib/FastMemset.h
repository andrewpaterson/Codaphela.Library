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
#ifndef __FAST_MEMSET_H__
#define __FAST_MEMSET_H__


typedef void (*memset_fast_func)(void*, unsigned char);
extern memset_fast_func ga_memset_fast[65];


void memset_fast(void* pvDest, unsigned char cVal, size_t iByteSize);
void memset_fast(void* pvDest, unsigned char cVal, int iByteSize);
void InitMemsetFast(void);


void memset_fast_0bytes(void* pvDest, unsigned char cVal);  //For completeness...
void memset_fast_1byte(void* pvDest, unsigned char cVal);
void memset_fast_2bytes(void* pvDest, unsigned char cVal);
void memset_fast_3bytes(void* pvDest, unsigned char cVal);
void memset_fast_4bytes(void* pvDest, unsigned char cVal);
void memset_fast_5bytes(void* pvDest, unsigned char cVal);
void memset_fast_6bytes(void* pvDest, unsigned char cVal);
void memset_fast_7bytes(void* pvDest, unsigned char cVal);
void memset_fast_8bytes(void* pvDest, unsigned char cVal);
void memset_fast_9bytes(void* pvDest, unsigned char cVal);
void memset_fast_10bytes(void* pvDest, unsigned char cVal);
void memset_fast_11bytes(void* pvDest, unsigned char cVal);
void memset_fast_12bytes(void* pvDest, unsigned char cVal);
void memset_fast_13bytes(void* pvDest, unsigned char cVal);
void memset_fast_14bytes(void* pvDest, unsigned char cVal);
void memset_fast_15bytes(void* pvDest, unsigned char cVal);
void memset_fast_16bytes(void* pvDest, unsigned char cVal);
void memset_fast_17bytes(void* pvDest, unsigned char cVal);
void memset_fast_18bytes(void* pvDest, unsigned char cVal);
void memset_fast_19bytes(void* pvDest, unsigned char cVal);
void memset_fast_20bytes(void* pvDest, unsigned char cVal);
void memset_fast_21bytes(void* pvDest, unsigned char cVal);
void memset_fast_22bytes(void* pvDest, unsigned char cVal);
void memset_fast_23bytes(void* pvDest, unsigned char cVal);
void memset_fast_24bytes(void* pvDest, unsigned char cVal);
void memset_fast_25bytes(void* pvDest, unsigned char cVal);
void memset_fast_26bytes(void* pvDest, unsigned char cVal);
void memset_fast_27bytes(void* pvDest, unsigned char cVal);
void memset_fast_28bytes(void* pvDest, unsigned char cVal);
void memset_fast_29bytes(void* pvDest, unsigned char cVal);
void memset_fast_30bytes(void* pvDest, unsigned char cVal);
void memset_fast_31bytes(void* pvDest, unsigned char cVal);
void memset_fast_32bytes(void* pvDest, unsigned char cVal);
void memset_fast_33bytes(void* pvDest, unsigned char cVal);
void memset_fast_34bytes(void* pvDest, unsigned char cVal);
void memset_fast_35bytes(void* pvDest, unsigned char cVal);
void memset_fast_36bytes(void* pvDest, unsigned char cVal);
void memset_fast_37bytes(void* pvDest, unsigned char cVal);
void memset_fast_38bytes(void* pvDest, unsigned char cVal);
void memset_fast_39bytes(void* pvDest, unsigned char cVal);
void memset_fast_40bytes(void* pvDest, unsigned char cVal);
void memset_fast_41bytes(void* pvDest, unsigned char cVal);
void memset_fast_42bytes(void* pvDest, unsigned char cVal);
void memset_fast_43bytes(void* pvDest, unsigned char cVal);
void memset_fast_44bytes(void* pvDest, unsigned char cVal);
void memset_fast_45bytes(void* pvDest, unsigned char cVal);
void memset_fast_46bytes(void* pvDest, unsigned char cVal);
void memset_fast_47bytes(void* pvDest, unsigned char cVal);
void memset_fast_48bytes(void* pvDest, unsigned char cVal);
void memset_fast_49bytes(void* pvDest, unsigned char cVal);
void memset_fast_50bytes(void* pvDest, unsigned char cVal);
void memset_fast_51bytes(void* pvDest, unsigned char cVal);
void memset_fast_52bytes(void* pvDest, unsigned char cVal);
void memset_fast_53bytes(void* pvDest, unsigned char cVal);
void memset_fast_54bytes(void* pvDest, unsigned char cVal);
void memset_fast_55bytes(void* pvDest, unsigned char cVal);
void memset_fast_56bytes(void* pvDest, unsigned char cVal);
void memset_fast_57bytes(void* pvDest, unsigned char cVal);
void memset_fast_58bytes(void* pvDest, unsigned char cVal);
void memset_fast_59bytes(void* pvDest, unsigned char cVal);
void memset_fast_60bytes(void* pvDest, unsigned char cVal);
void memset_fast_61bytes(void* pvDest, unsigned char cVal);
void memset_fast_62bytes(void* pvDest, unsigned char cVal);
void memset_fast_63bytes(void* pvDest, unsigned char cVal);
void memset_fast_64bytes(void* pvDest, unsigned char cVal);
void memset_fast_65bytesUp(void* pvDest, unsigned char cVal, unsigned int iByteSize);



#endif // __FAST_MEMSET_H__

