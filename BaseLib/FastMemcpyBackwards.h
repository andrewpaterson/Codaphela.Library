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
#ifndef __FAST_MEMCPY_BACKWARDS_H__
#define __FAST_MEMCPY_BACKWARDS_H__
#include <string.h>
#include "PrimitiveTypes.h"


typedef void (*memcpy_fast_backwards_func)(void*, void*);
extern memcpy_fast_backwards_func ga_memcpy_fast_backwards[65];


void memcpy_fast_backwards(void* pvDest, void* pvSource, uint32 uiByteSize);
void InitMemcpyFastBackwards(void);
void memcpy_backwards_large(void* pvDest, void* pvSource, uint32 uiByteSize);

void memcpy_fast_backwards_0bytes(void* pvDest, void* pvSource);  //For completeness...
void memcpy_fast_backwards_1byte(void* pvDest, void* pvSource);
void memcpy_fast_backwards_2bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_3bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_4bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_5bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_6bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_7bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_8bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_9bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_10bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_11bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_12bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_13bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_14bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_15bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_16bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_17bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_18bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_19bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_20bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_21bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_22bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_23bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_24bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_25bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_26bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_27bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_28bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_29bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_30bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_31bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_32bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_33bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_34bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_35bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_36bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_37bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_38bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_39bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_40bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_41bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_42bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_43bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_44bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_45bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_46bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_47bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_48bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_49bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_50bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_51bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_52bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_53bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_54bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_55bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_56bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_57bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_58bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_59bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_60bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_61bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_62bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_63bytes(void* pvDest, void* pvSource);
void memcpy_fast_backwards_64bytes(void* pvDest, void* pvSource);


#endif // __FAST_MEMCPY_BACKWARDS_H__

