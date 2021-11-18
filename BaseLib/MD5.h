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
#ifndef __MD5_H__
#define __MD5_H__

/*
***********************************************************************
** md5.h -- header file for implementation of MD5                    **
** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
** Created: 2/17/90 RLR                                              **
** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version               **
** Revised (for MD5): RLR 4/27/91                                    **
**   -- G modified to have y&~z instead of y&z                       **
**   -- FF, GG, HH modified to add in last register done             **
**   -- Access pattern: round 2 works mod 5, round 3 works mod 3     **
**   -- distinct additive constant for each step                     **
**   -- round 4 added, working mod 7                                 **
***********************************************************************
*/

/*
***********************************************************************
** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
**                                                                   **
** License to copy and use this software is granted provided that    **
** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
** Digest Algorithm" in all material mentioning or referencing this  **
** software or this function.                                        **
**                                                                   **
** License is also granted to make and use derivative works          **
** provided that such works are identified as "derived from the RSA  **
** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
** material mentioning or referencing the derived work.              **
**                                                                   **
** RSA Data Security, Inc. makes no representations concerning       **
** either the merchantability of this software or the suitability    **
** of this software for any particular purpose.  It is provided "as  **
** is" without express or implied warranty of any kind.              **
**                                                                   **
** These notices must be retained in any copies of any part of this  **
** documentation and/or software.                                    **
***********************************************************************
*/


/* Data structure for MD5 (Message-Digest) computation */
struct SMD5Context
{
	unsigned long int i[2];	 /* number of _bits_ handled mod 2^64 */
	unsigned long int buf[4];	/* scratch buffer */
	unsigned char in[64];	/* input buffer */
	unsigned char digest[16];	/* actual digest after MD5Final call */
};


void MD5Init(SMD5Context* mdContext);
void MD5Update(SMD5Context* mdContext, unsigned char*, unsigned int);
void MD5Final(SMD5Context* mdContext);
void MD5Transform(unsigned long int*, unsigned long int*);
void MD5Dump(SMD5Context* ctx);


#endif // !__MD5_H__

