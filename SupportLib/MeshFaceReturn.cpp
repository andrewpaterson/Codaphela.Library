/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "MeshFaceReturn.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::PackEmpty(void)
{
	manCorners[1] = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::PackOverflow(void)
{
	manCorners[1] = 3 << 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Pack(int iFirstFace, int iCorner1, int iCorner2, int iCorner3, int iNewEdges)
{
	miFirstFace = iFirstFace;
	manCorners[0] = (unsigned char)(iCorner1 | iCorner2 << 4);
	manCorners[1] = (unsigned char)(iCorner3 | 1 << 4);
	manCorners[3] = (unsigned char)(iNewEdges << 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Pack(int iFirstFace, int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iCorner5, int iCorner6, int iNewEdges)
{
	miFirstFace = iFirstFace;
	manCorners[0] = (unsigned char)(iCorner1 | iCorner2 << 4);
	manCorners[1] = (unsigned char)(iCorner3 | 2 << 4);
	manCorners[2] = (unsigned char)(iCorner4 | iCorner5 << 4);
	manCorners[3] = (unsigned char)(iCorner6 | iNewEdges << 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Pack2(CMFRet r2)
{
	manCorners[1] |= 2 << 4;
	manCorners[1] &= 0xef;
	manCorners[2] = r2.manCorners[0];
	manCorners[3] = ((manCorners[3] & 0xf0) + (r2.manCorners[3] & 0xf0)) | (r2.manCorners[1] & 0xf);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Repack(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iCorner5, int iCorner6)
{
	manCorners[0] = (unsigned char)(iCorner1 | iCorner2 << 4);
	manCorners[1] = (manCorners[1] & 0xf0) | (unsigned char)(iCorner3);
	manCorners[2] = (unsigned char)(iCorner4 | iCorner5 << 4);
	manCorners[3] = (manCorners[3] & 0xf0) | (unsigned char)(iCorner6);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Repack(int iCorner1, int iCorner2, int iCorner3)
{
	manCorners[0] = (unsigned char)(iCorner1 | iCorner2 << 4);
	manCorners[1] = (manCorners[1] & 0xf0) | (unsigned char)(iCorner3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMFRet::NumFaces(void)
{
	return manCorners[1] >> 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMFRet::NewEdges(void)
{
	return manCorners[3] >> 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Unpack1(int* piCorner1, int* piCorner2, int* piCorner3)
{
	*piCorner1 = manCorners[0] & 0xf;
	*piCorner2 = manCorners[0] >> 4;
	*piCorner3 = manCorners[1] & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMFRet::Unpack2(int* piCorner4, int* piCorner5, int* piCorner6)
{
	*piCorner4 = manCorners[2] & 0xf;
	*piCorner5 = manCorners[2] >> 4;
	*piCorner6 = manCorners[3] & 0xf;
}

