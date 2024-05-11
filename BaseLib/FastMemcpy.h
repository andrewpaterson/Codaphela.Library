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
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licensessize_t

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FAST_MEMCPY_H__
#define __FAST_MEMCPY_H__
#include "Define.h"
#include "PrimitiveTypes.h"


typedef void (*memcpy_fast_func)(void*, void*);
extern memcpy_fast_func ga_memcpy_fast[65];


void memcpy_fast(void* pvDest, void* pvSource, size uiByteSize);
void memcpy_large(void* pvDest, void* pvSource, size uiByteSize);
void InitMemcpyFast(void);

void memcpy_fast_0bytes(void* pvDest, void* pvSource);  //For completeness...
void memcpy_fast_1byte(void* pvDest, void* pvSource);
void memcpy_fast_2bytes(void* pvDest, void* pvSource);
void memcpy_fast_3bytes(void* pvDest, void* pvSource);
void memcpy_fast_4bytes(void* pvDest, void* pvSource);
void memcpy_fast_5bytes(void* pvDest, void* pvSource);
void memcpy_fast_6bytes(void* pvDest, void* pvSource);
void memcpy_fast_7bytes(void* pvDest, void* pvSource);
void memcpy_fast_8bytes(void* pvDest, void* pvSource);
void memcpy_fast_9bytes(void* pvDest, void* pvSource);
void memcpy_fast_10bytes(void* pvDest, void* pvSource);
void memcpy_fast_11bytes(void* pvDest, void* pvSource);
void memcpy_fast_12bytes(void* pvDest, void* pvSource);
void memcpy_fast_13bytes(void* pvDest, void* pvSource);
void memcpy_fast_14bytes(void* pvDest, void* pvSource);
void memcpy_fast_15bytes(void* pvDest, void* pvSource);
void memcpy_fast_16bytes(void* pvDest, void* pvSource);
void memcpy_fast_17bytes(void* pvDest, void* pvSource);
void memcpy_fast_18bytes(void* pvDest, void* pvSource);
void memcpy_fast_19bytes(void* pvDest, void* pvSource);
void memcpy_fast_20bytes(void* pvDest, void* pvSource);
void memcpy_fast_21bytes(void* pvDest, void* pvSource);
void memcpy_fast_22bytes(void* pvDest, void* pvSource);
void memcpy_fast_23bytes(void* pvDest, void* pvSource);
void memcpy_fast_24bytes(void* pvDest, void* pvSource);
void memcpy_fast_25bytes(void* pvDest, void* pvSource);
void memcpy_fast_26bytes(void* pvDest, void* pvSource);
void memcpy_fast_27bytes(void* pvDest, void* pvSource);
void memcpy_fast_28bytes(void* pvDest, void* pvSource);
void memcpy_fast_29bytes(void* pvDest, void* pvSource);
void memcpy_fast_30bytes(void* pvDest, void* pvSource);
void memcpy_fast_31bytes(void* pvDest, void* pvSource);
void memcpy_fast_32bytes(void* pvDest, void* pvSource);
void memcpy_fast_33bytes(void* pvDest, void* pvSource);
void memcpy_fast_34bytes(void* pvDest, void* pvSource);
void memcpy_fast_35bytes(void* pvDest, void* pvSource);
void memcpy_fast_36bytes(void* pvDest, void* pvSource);
void memcpy_fast_37bytes(void* pvDest, void* pvSource);
void memcpy_fast_38bytes(void* pvDest, void* pvSource);
void memcpy_fast_39bytes(void* pvDest, void* pvSource);
void memcpy_fast_40bytes(void* pvDest, void* pvSource);
void memcpy_fast_41bytes(void* pvDest, void* pvSource);
void memcpy_fast_42bytes(void* pvDest, void* pvSource);
void memcpy_fast_43bytes(void* pvDest, void* pvSource);
void memcpy_fast_44bytes(void* pvDest, void* pvSource);
void memcpy_fast_45bytes(void* pvDest, void* pvSource);
void memcpy_fast_46bytes(void* pvDest, void* pvSource);
void memcpy_fast_47bytes(void* pvDest, void* pvSource);
void memcpy_fast_48bytes(void* pvDest, void* pvSource);
void memcpy_fast_49bytes(void* pvDest, void* pvSource);
void memcpy_fast_50bytes(void* pvDest, void* pvSource);
void memcpy_fast_51bytes(void* pvDest, void* pvSource);
void memcpy_fast_52bytes(void* pvDest, void* pvSource);
void memcpy_fast_53bytes(void* pvDest, void* pvSource);
void memcpy_fast_54bytes(void* pvDest, void* pvSource);
void memcpy_fast_55bytes(void* pvDest, void* pvSource);
void memcpy_fast_56bytes(void* pvDest, void* pvSource);
void memcpy_fast_57bytes(void* pvDest, void* pvSource);
void memcpy_fast_58bytes(void* pvDest, void* pvSource);
void memcpy_fast_59bytes(void* pvDest, void* pvSource);
void memcpy_fast_60bytes(void* pvDest, void* pvSource);
void memcpy_fast_61bytes(void* pvDest, void* pvSource);
void memcpy_fast_62bytes(void* pvDest, void* pvSource);
void memcpy_fast_63bytes(void* pvDest, void* pvSource);
void memcpy_fast_64bytes(void* pvDest, void* pvSource);


#endif // __FAST_MEMCPY_H__

