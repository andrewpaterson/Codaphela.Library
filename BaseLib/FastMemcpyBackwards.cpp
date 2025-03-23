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
#include "PrimitiveTypes.h"
#include "FastMemcpyBackwards.h"


memcpy_fast_backwards_func ga_memcpy_fast_backwards[65];


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitMemcpyFastBackwards(void)
{
	memset(ga_memcpy_fast_backwards, 0, sizeof(ga_memcpy_fast_backwards));

	ga_memcpy_fast_backwards[0] = memcpy_fast_backwards_0bytes;
	ga_memcpy_fast_backwards[1] = memcpy_fast_backwards_1byte;
	ga_memcpy_fast_backwards[2] = memcpy_fast_backwards_2bytes;
	ga_memcpy_fast_backwards[3] = memcpy_fast_backwards_3bytes;
	ga_memcpy_fast_backwards[4] = memcpy_fast_backwards_4bytes;
	ga_memcpy_fast_backwards[5] = memcpy_fast_backwards_5bytes;
	ga_memcpy_fast_backwards[6] = memcpy_fast_backwards_6bytes;
	ga_memcpy_fast_backwards[7] = memcpy_fast_backwards_7bytes;
	ga_memcpy_fast_backwards[8] = memcpy_fast_backwards_8bytes;
	ga_memcpy_fast_backwards[9] = memcpy_fast_backwards_9bytes;

	ga_memcpy_fast_backwards[10] = memcpy_fast_backwards_10bytes;
	ga_memcpy_fast_backwards[11] = memcpy_fast_backwards_11bytes;
	ga_memcpy_fast_backwards[12] = memcpy_fast_backwards_12bytes;
	ga_memcpy_fast_backwards[13] = memcpy_fast_backwards_13bytes;
	ga_memcpy_fast_backwards[14] = memcpy_fast_backwards_14bytes;
	ga_memcpy_fast_backwards[15] = memcpy_fast_backwards_15bytes;
	ga_memcpy_fast_backwards[16] = memcpy_fast_backwards_16bytes;
	ga_memcpy_fast_backwards[17] = memcpy_fast_backwards_17bytes;
	ga_memcpy_fast_backwards[18] = memcpy_fast_backwards_18bytes;
	ga_memcpy_fast_backwards[19] = memcpy_fast_backwards_19bytes;

	ga_memcpy_fast_backwards[20] = memcpy_fast_backwards_20bytes;
	ga_memcpy_fast_backwards[21] = memcpy_fast_backwards_21bytes;
	ga_memcpy_fast_backwards[22] = memcpy_fast_backwards_22bytes;
	ga_memcpy_fast_backwards[23] = memcpy_fast_backwards_23bytes;
	ga_memcpy_fast_backwards[24] = memcpy_fast_backwards_24bytes;
	ga_memcpy_fast_backwards[25] = memcpy_fast_backwards_25bytes;
	ga_memcpy_fast_backwards[26] = memcpy_fast_backwards_26bytes;
	ga_memcpy_fast_backwards[27] = memcpy_fast_backwards_27bytes;
	ga_memcpy_fast_backwards[28] = memcpy_fast_backwards_28bytes;
	ga_memcpy_fast_backwards[29] = memcpy_fast_backwards_29bytes;

	ga_memcpy_fast_backwards[30] = memcpy_fast_backwards_30bytes;
	ga_memcpy_fast_backwards[31] = memcpy_fast_backwards_31bytes;
	ga_memcpy_fast_backwards[32] = memcpy_fast_backwards_32bytes;
	ga_memcpy_fast_backwards[33] = memcpy_fast_backwards_33bytes;
	ga_memcpy_fast_backwards[34] = memcpy_fast_backwards_34bytes;
	ga_memcpy_fast_backwards[35] = memcpy_fast_backwards_35bytes;
	ga_memcpy_fast_backwards[36] = memcpy_fast_backwards_36bytes;
	ga_memcpy_fast_backwards[37] = memcpy_fast_backwards_37bytes;
	ga_memcpy_fast_backwards[38] = memcpy_fast_backwards_38bytes;
	ga_memcpy_fast_backwards[39] = memcpy_fast_backwards_39bytes;

	ga_memcpy_fast_backwards[40] = memcpy_fast_backwards_40bytes;
	ga_memcpy_fast_backwards[41] = memcpy_fast_backwards_41bytes;
	ga_memcpy_fast_backwards[42] = memcpy_fast_backwards_42bytes;
	ga_memcpy_fast_backwards[43] = memcpy_fast_backwards_43bytes;
	ga_memcpy_fast_backwards[44] = memcpy_fast_backwards_44bytes;
	ga_memcpy_fast_backwards[45] = memcpy_fast_backwards_45bytes;
	ga_memcpy_fast_backwards[46] = memcpy_fast_backwards_46bytes;
	ga_memcpy_fast_backwards[47] = memcpy_fast_backwards_47bytes;
	ga_memcpy_fast_backwards[48] = memcpy_fast_backwards_48bytes;
	ga_memcpy_fast_backwards[49] = memcpy_fast_backwards_49bytes;

	ga_memcpy_fast_backwards[50] = memcpy_fast_backwards_50bytes;
	ga_memcpy_fast_backwards[51] = memcpy_fast_backwards_51bytes;
	ga_memcpy_fast_backwards[52] = memcpy_fast_backwards_52bytes;
	ga_memcpy_fast_backwards[53] = memcpy_fast_backwards_53bytes;
	ga_memcpy_fast_backwards[54] = memcpy_fast_backwards_54bytes;
	ga_memcpy_fast_backwards[55] = memcpy_fast_backwards_55bytes;
	ga_memcpy_fast_backwards[56] = memcpy_fast_backwards_56bytes;
	ga_memcpy_fast_backwards[57] = memcpy_fast_backwards_57bytes;
	ga_memcpy_fast_backwards[58] = memcpy_fast_backwards_58bytes;
	ga_memcpy_fast_backwards[59] = memcpy_fast_backwards_59bytes;

	ga_memcpy_fast_backwards[60] = memcpy_fast_backwards_60bytes;
	ga_memcpy_fast_backwards[61] = memcpy_fast_backwards_61bytes;
	ga_memcpy_fast_backwards[62] = memcpy_fast_backwards_62bytes;
	ga_memcpy_fast_backwards[63] = memcpy_fast_backwards_63bytes;
	ga_memcpy_fast_backwards[64] = memcpy_fast_backwards_64bytes;
}


void memcpy_fast_backwards_0bytes(void* pvDest, void* pvSource)
{
}


void memcpy_fast_backwards_1byte(void* pvDest, void* pvSource)
{
	((char*)pvDest)[0] = ((char*)pvSource)[0];
}


void memcpy_fast_backwards_2bytes(void* pvDest, void* pvSource)
{
	((int16*)pvDest)[0] = ((int16*)pvSource)[0];
}


void memcpy_fast_backwards_3bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[2] = ((char*)pvSource)[2];
	((int16*)pvDest)[0] = ((int16*)pvSource)[0];
}


void memcpy_fast_backwards_4bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_5bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[4] = ((char*)pvSource)[4];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_6bytes(void* pvDest, void* pvSource)
{
	((int16*)pvDest)[2] = ((int16*)pvSource)[2];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_7bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[6] = ((char*)pvSource)[6];
	((int16*)pvDest)[2] = ((int16*)pvSource)[2];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}



void memcpy_fast_backwards_8bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_9bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[8] = ((char*)pvSource)[8];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_10bytes(void* pvDest, void* pvSource)
{
	((int16*)pvDest)[4] = ((int16*)pvSource)[4];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_11bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[10] = ((char*)pvSource)[10];
	((int16*)pvDest)[4] = ((int16*)pvSource)[4];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_12bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_13bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[12] = ((char*)pvSource)[12];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_14bytes(void* pvDest, void* pvSource)
{
	((int16*)pvDest)[6] = ((int16*)pvSource)[6];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_15bytes(void* pvDest, void* pvSource)
{
	((char*)pvDest)[14] = ((char*)pvSource)[14];
	((int16*)pvDest)[6] = ((int16*)pvSource)[6];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards_16bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[3] = ((int*)pvSource)[3];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_backwards(void* pvDest, void* pvSource, uint32 uiByteSize)
{
	if (uiByteSize <= 64)
	{
		ga_memcpy_fast_backwards[uiByteSize](pvDest, pvSource);
	}
	else
	{
		memcpy_backwards_large(pvDest, pvSource, uiByteSize);
	}
}


void memcpy_backwards_large(void* pvDest, void* pvSource, uint32 uiByteSize)
{
	memmove(pvDest, pvSource, uiByteSize);
}


void memcpy_fast_backwards_17bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_1byte(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_18bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_2bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_19bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_3bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_20bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_4bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_21bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_5bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_22bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_6bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_23bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_7bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_24bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_8bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_25bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_9bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_26bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_10bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_27bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_11bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_28bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_12bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_29bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_13bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_30bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_14bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_31bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_15bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_32bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_33bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_1byte(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_34bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_2bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_35bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_3bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_36bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_4bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_37bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_5bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_38bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_6bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_39bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_7bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_40bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_8bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_41bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_9bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_42bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_10bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}



void memcpy_fast_backwards_43bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_11bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_44bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_12bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_45bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_13bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_46bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_14bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_47bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_15bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_48bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_49bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_1byte(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_50bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_2bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_51bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_3bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_52bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_4bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_53bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_5bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_54bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_6bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_55bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_7bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_56bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_8bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_57bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_9bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_58bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_10bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_59bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_11bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_60bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_12bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_61bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_13bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_62bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_14bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_63bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_15bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}


void memcpy_fast_backwards_64bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_backwards_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_backwards_16bytes(pvDest, pvSource);
}

