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
#include "MD5.h"
#include "Chars.h"


static unsigned char PADDING[64] =
{
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G, H and I are basic MD5 functions */unsigned long int F(unsigned long int x, unsigned long int y, unsigned long int z){	return (((x) & (y)) | ((~x) & (z)));}unsigned long int G(unsigned long int x, unsigned long int y, unsigned long int z){	return (((x) & (z)) | ((y) & (~z)));}unsigned long int H(unsigned long int x, unsigned long int y, unsigned long int z){	return ((x) ^ (y) ^ (z));}unsigned long int I(unsigned long int x, unsigned long int y, unsigned long int z){	return ((y) ^ ((x) | (~z)));}
/* ROTATE_LEFT rotates x left n bits */unsigned long int ROTATE_LEFT(unsigned long int x, unsigned long int n){	return (((x) << (n)) | ((x) >> (32-(n))));}
/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 *//* Rotation is separate from addition to prevent recomputation */unsigned long int FF(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d, unsigned long int x, unsigned long int s, unsigned long int ac){	(a) += F((b), (c), (d)) + (x) + (unsigned long int)(ac);	(a) = ROTATE_LEFT((a), (s));	(a) += (b);	return a;}
unsigned long int GG(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d, unsigned long int x, unsigned long int s, unsigned long int ac){	(a) += G((b), (c), (d)) + (x) + (unsigned long int)(ac);	(a) = ROTATE_LEFT ((a), (s));	(a) += (b);	return a;}
unsigned long int HH(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d, unsigned long int x, unsigned long int s, unsigned long int ac){	(a) += H((b), (c), (d)) + (x) + (unsigned long int)(ac);	(a) = ROTATE_LEFT ((a), (s));	(a) += (b);	return a;}
unsigned long int II(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d, unsigned long int x, unsigned long int s, unsigned long int ac){	(a) += I((b), (c), (d)) + (x) + (unsigned long int)(ac);	(a) = ROTATE_LEFT ((a), (s));	(a) += (b);	return a;}
/*
The routine MD5Init initializes the message-digest context
mdContext. All fields are set to zero.
*/
void MD5Init (SMD5Context* mdContext)
{
	mdContext->i[0] = mdContext->i[1] = (unsigned long int)0;

	/* Load magic initialization constants. */
	mdContext->buf[0] = (unsigned long int)0x67452301;
	mdContext->buf[1] = (unsigned long int)0xefcdab89;
	mdContext->buf[2] = (unsigned long int)0x98badcfe;
	mdContext->buf[3] = (unsigned long int)0x10325476;
}

/*
The routine MD5Update updates the message-digest context to
account for the presence of each of the characters inBuf[0..inLen-1]
in the message whose digest is being computed.
*/
void MD5Update(SMD5Context *mdContext, unsigned char *inBuf, unsigned int inLen)
{
	unsigned long int in[16];
	int mdi;
	unsigned int i, ii;

	/* compute number of bytes mod 64 */
	mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

	/* update number of bits */
	if ((mdContext->i[0] + ((unsigned long int)inLen << 3)) < mdContext->i[0])
		mdContext->i[1]++;

	mdContext->i[0] += ((unsigned long int)inLen << 3);
	mdContext->i[1] += ((unsigned long int)inLen >> 29);
	while (inLen--)
	{
		/* add new character to buffer, increment mdi */
		mdContext->in[mdi++] = *inBuf++;

		/* transform if necessary */
		if (mdi == 0x40)
		{
			for (i = 0, ii = 0; i < 16; i++, ii += 4)
				in[i] = (((unsigned long int)mdContext->in[ii+3]) << 24) |
				(((unsigned long int)mdContext->in[ii+2]) << 16) |
				(((unsigned long int)mdContext->in[ii+1]) << 8) |
				((unsigned long int)mdContext->in[ii]);
			MD5Transform(mdContext->buf, in);
			mdi = 0;
		}
	}
}

/*
The routine MD5Final terminates the message-digest computation and
ends with the desired message digest in mdContext->digest[0...15].
*/
void MD5Final(SMD5Context *mdContext)
{
	unsigned long int in[16];
	int mdi;
	unsigned int i, ii;
	unsigned int padLen;

	/* save number of bits */
	in[14] = mdContext->i[0];
	in[15] = mdContext->i[1];

	/* compute number of bytes mod 64 */
	mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

	/* pad out to 56 mod 64 */
	padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
	MD5Update (mdContext, PADDING, padLen);

	/* append length in bits and transform */
	for (i = 0, ii = 0; i < 14; i++, ii += 4)
	{
		in[i] = (((unsigned long int)mdContext->in[ii+3]) << 24) |
		(((unsigned long int)mdContext->in[ii+2]) << 16) |
		(((unsigned long int)mdContext->in[ii+1]) << 8) |
		((unsigned long int)mdContext->in[ii]);
	}

	MD5Transform (mdContext->buf, in);

	/* store buffer in digest */
	for (i = 0, ii = 0; i < 4; i++, ii += 4)
	{
		mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
		mdContext->digest[ii+1] =
			(unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
		mdContext->digest[ii+2] =
			(unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
		mdContext->digest[ii+3] =
			(unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
	}
}

void MD5Transform(unsigned long int *buf, unsigned long int *in)
{
	unsigned long int a = buf[0], b = buf[1], c = buf[2], d = buf[3];

	/* Round 1 */
	unsigned long int S11 = 7;
	unsigned long int S12 = 12;
	unsigned long int S13 = 17;
	unsigned long int S14 = 22;
	a = FF(a, b, c, d, in[ 0], S11, 3614090360); /* 1 */
	d = FF(d, a, b, c, in[ 1], S12, 3905402710); /* 2 */
	c = FF(c, d, a, b, in[ 2], S13,  606105819); /* 3 */
	b = FF(b, c, d, a, in[ 3], S14, 3250441966); /* 4 */
	a = FF(a, b, c, d, in[ 4], S11, 4118548399); /* 5 */
	d = FF(d, a, b, c, in[ 5], S12, 1200080426); /* 6 */
	c = FF(c, d, a, b, in[ 6], S13, 2821735955); /* 7 */
	b = FF(b, c, d, a, in[ 7], S14, 4249261313); /* 8 */
	a = FF(a, b, c, d, in[ 8], S11, 1770035416); /* 9 */
	d = FF(d, a, b, c, in[ 9], S12, 2336552879); /* 10 */
	c = FF(c, d, a, b, in[10], S13, 4294925233); /* 11 */
	b = FF(b, c, d, a, in[11], S14, 2304563134); /* 12 */
	a = FF(a, b, c, d, in[12], S11, 1804603682); /* 13 */
	d = FF(d, a, b, c, in[13], S12, 4254626195); /* 14 */
	c = FF(c, d, a, b, in[14], S13, 2792965006); /* 15 */
	b = FF(b, c, d, a, in[15], S14, 1236535329); /* 16 */

	/* Round 2 */
	unsigned long int S21 = 5;
	unsigned long int S22 = 9;
	unsigned long int S23 = 14;
	unsigned long int S24 = 20;
	a = GG(a, b, c, d, in[ 1], S21, 4129170786); /* 17 */
	d = GG(d, a, b, c, in[ 6], S22, 3225465664); /* 18 */
	c = GG(c, d, a, b, in[11], S23,  643717713); /* 19 */
	b = GG(b, c, d, a, in[ 0], S24, 3921069994); /* 20 */
	a = GG(a, b, c, d, in[ 5], S21, 3593408605); /* 21 */
	d = GG(d, a, b, c, in[10], S22,   38016083); /* 22 */
	c = GG(c, d, a, b, in[15], S23, 3634488961); /* 23 */
	b = GG(b, c, d, a, in[ 4], S24, 3889429448); /* 24 */
	a = GG(a, b, c, d, in[ 9], S21,  568446438); /* 25 */
	d = GG(d, a, b, c, in[14], S22, 3275163606); /* 26 */
	c = GG(c, d, a, b, in[ 3], S23, 4107603335); /* 27 */
	b = GG(b, c, d, a, in[ 8], S24, 1163531501); /* 28 */
	a = GG(a, b, c, d, in[13], S21, 2850285829); /* 29 */
	d = GG(d, a, b, c, in[ 2], S22, 4243563512); /* 30 */
	c = GG(c, d, a, b, in[ 7], S23, 1735328473); /* 31 */
	b = GG(b, c, d, a, in[12], S24, 2368359562); /* 32 */

	/* Round 3 */
	unsigned long int S31 = 4;
	unsigned long int S32 = 11;
	unsigned long int S33 = 16;
	unsigned long int S34 = 23;
	a = HH(a, b, c, d, in[ 5], S31, 4294588738); /* 33 */
	d = HH(d, a, b, c, in[ 8], S32, 2272392833); /* 34 */
	c = HH(c, d, a, b, in[11], S33, 1839030562); /* 35 */
	b = HH(b, c, d, a, in[14], S34, 4259657740); /* 36 */
	a = HH(a, b, c, d, in[ 1], S31, 2763975236); /* 37 */
	d = HH(d, a, b, c, in[ 4], S32, 1272893353); /* 38 */
	c = HH(c, d, a, b, in[ 7], S33, 4139469664); /* 39 */
	b = HH(b, c, d, a, in[10], S34, 3200236656); /* 40 */
	a = HH(a, b, c, d, in[13], S31,  681279174); /* 41 */
	d = HH(d, a, b, c, in[ 0], S32, 3936430074); /* 42 */
	c = HH(c, d, a, b, in[ 3], S33, 3572445317); /* 43 */
	b = HH(b, c, d, a, in[ 6], S34,   76029189); /* 44 */
	a = HH(a, b, c, d, in[ 9], S31, 3654602809); /* 45 */
	d = HH(d, a, b, c, in[12], S32, 3873151461); /* 46 */
	c = HH(c, d, a, b, in[15], S33,  530742520); /* 47 */
	b = HH(b, c, d, a, in[ 2], S34, 3299628645); /* 48 */

	/* Round 4 */
	unsigned long int S41 = 6;
	unsigned long int S42 = 10;
	unsigned long int S43 = 15;
	unsigned long int S44 = 21;
	a = II(a, b, c, d, in[ 0], S41, 4096336452); /* 49 */
	d = II(d, a, b, c, in[ 7], S42, 1126891415); /* 50 */
	c = II(c, d, a, b, in[14], S43, 2878612391); /* 51 */
	b = II(b, c, d, a, in[ 5], S44, 4237533241); /* 52 */
	a = II(a, b, c, d, in[12], S41, 1700485571); /* 53 */
	d = II(d, a, b, c, in[ 3], S42, 2399980690); /* 54 */
	c = II(c, d, a, b, in[10], S43, 4293915773); /* 55 */
	b = II(b, c, d, a, in[ 1], S44, 2240044497); /* 56 */
	a = II(a, b, c, d, in[ 8], S41, 1873313359); /* 57 */
	d = II(d, a, b, c, in[15], S42, 4264355552); /* 58 */
	c = II(c, d, a, b, in[ 6], S43, 2734768916); /* 59 */
	b = II(b, c, d, a, in[13], S44, 1309151649); /* 60 */
	a = II(a, b, c, d, in[ 4], S41, 4149444226); /* 61 */
	d = II(d, a, b, c, in[11], S42, 3174756917); /* 62 */
	c = II(c, d, a, b, in[ 2], S43,  718787259); /* 63 */
	b = II(b, c, d, a, in[ 9], S44, 3951481745); /* 64 */

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MD5Dump(SMD5Context* ctx)
{
	CChars	sz;

	sz.Init();
	sz.AppendHexLoHi(ctx->digest, 16);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

