/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "Define.h"
#include "PrimitiveTypes.h"


typedef void (*memset_fast_func)(void*, uint8);
extern memset_fast_func ga_memset_fast[65];


void memset_fast(void* pvDest, uint8 cVal, size_t iByteSize);
void InitMemsetFast(void);


void memset_fast_0bytes(void* pvDest, uint8 cVal);  //For completeness...
void memset_fast_1byte(void* pvDest, uint8 cVal);
void memset_fast_2bytes(void* pvDest, uint8 cVal);
void memset_fast_3bytes(void* pvDest, uint8 cVal);
void memset_fast_4bytes(void* pvDest, uint8 cVal);
void memset_fast_5bytes(void* pvDest, uint8 cVal);
void memset_fast_6bytes(void* pvDest, uint8 cVal);
void memset_fast_7bytes(void* pvDest, uint8 cVal);
void memset_fast_8bytes(void* pvDest, uint8 cVal);
void memset_fast_9bytes(void* pvDest, uint8 cVal);
void memset_fast_10bytes(void* pvDest, uint8 cVal);
void memset_fast_11bytes(void* pvDest, uint8 cVal);
void memset_fast_12bytes(void* pvDest, uint8 cVal);
void memset_fast_13bytes(void* pvDest, uint8 cVal);
void memset_fast_14bytes(void* pvDest, uint8 cVal);
void memset_fast_15bytes(void* pvDest, uint8 cVal);
void memset_fast_16bytes(void* pvDest, uint8 cVal);
void memset_fast_17bytes(void* pvDest, uint8 cVal);
void memset_fast_18bytes(void* pvDest, uint8 cVal);
void memset_fast_19bytes(void* pvDest, uint8 cVal);
void memset_fast_20bytes(void* pvDest, uint8 cVal);
void memset_fast_21bytes(void* pvDest, uint8 cVal);
void memset_fast_22bytes(void* pvDest, uint8 cVal);
void memset_fast_23bytes(void* pvDest, uint8 cVal);
void memset_fast_24bytes(void* pvDest, uint8 cVal);
void memset_fast_25bytes(void* pvDest, uint8 cVal);
void memset_fast_26bytes(void* pvDest, uint8 cVal);
void memset_fast_27bytes(void* pvDest, uint8 cVal);
void memset_fast_28bytes(void* pvDest, uint8 cVal);
void memset_fast_29bytes(void* pvDest, uint8 cVal);
void memset_fast_30bytes(void* pvDest, uint8 cVal);
void memset_fast_31bytes(void* pvDest, uint8 cVal);
void memset_fast_32bytes(void* pvDest, uint8 cVal);
void memset_fast_33bytes(void* pvDest, uint8 cVal);
void memset_fast_34bytes(void* pvDest, uint8 cVal);
void memset_fast_35bytes(void* pvDest, uint8 cVal);
void memset_fast_36bytes(void* pvDest, uint8 cVal);
void memset_fast_37bytes(void* pvDest, uint8 cVal);
void memset_fast_38bytes(void* pvDest, uint8 cVal);
void memset_fast_39bytes(void* pvDest, uint8 cVal);
void memset_fast_40bytes(void* pvDest, uint8 cVal);
void memset_fast_41bytes(void* pvDest, uint8 cVal);
void memset_fast_42bytes(void* pvDest, uint8 cVal);
void memset_fast_43bytes(void* pvDest, uint8 cVal);
void memset_fast_44bytes(void* pvDest, uint8 cVal);
void memset_fast_45bytes(void* pvDest, uint8 cVal);
void memset_fast_46bytes(void* pvDest, uint8 cVal);
void memset_fast_47bytes(void* pvDest, uint8 cVal);
void memset_fast_48bytes(void* pvDest, uint8 cVal);
void memset_fast_49bytes(void* pvDest, uint8 cVal);
void memset_fast_50bytes(void* pvDest, uint8 cVal);
void memset_fast_51bytes(void* pvDest, uint8 cVal);
void memset_fast_52bytes(void* pvDest, uint8 cVal);
void memset_fast_53bytes(void* pvDest, uint8 cVal);
void memset_fast_54bytes(void* pvDest, uint8 cVal);
void memset_fast_55bytes(void* pvDest, uint8 cVal);
void memset_fast_56bytes(void* pvDest, uint8 cVal);
void memset_fast_57bytes(void* pvDest, uint8 cVal);
void memset_fast_58bytes(void* pvDest, uint8 cVal);
void memset_fast_59bytes(void* pvDest, uint8 cVal);
void memset_fast_60bytes(void* pvDest, uint8 cVal);
void memset_fast_61bytes(void* pvDest, uint8 cVal);
void memset_fast_62bytes(void* pvDest, uint8 cVal);
void memset_fast_63bytes(void* pvDest, uint8 cVal);
void memset_fast_64bytes(void* pvDest, uint8 cVal);
void memset_fast_65bytesUp(void* pvDest, uint8 cVal, unsigned int iByteSize);


#endif // !__FAST_MEMSET_H__

