/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __SUB_IMAGE_H__
#define __SUB_IMAGE_H__
#include "BaseLib/Int2.h"
#include "Rectangle.h"


// This class is used for drawing a source image onto a destination image.

//S0,0                   .
// Source origin (ignored)
//                       .
//                   0---------+
//                 Right Offset 
//                       .
//                       .
//      *-----------------------*   0                                    0
//      |                .      |   |                                    |
//      |    |       |   .      |   | Top Offset                         |
//      |                .      |   |                                    |
//      | -  *-------*  -. -  - |   +   -                                |
//      |    | ImRct |   .      |       | Rectangle Height               |
//      |    |       |   .      |       |                                |
//......|....|.......|.A0,0.....|.......|... Horizontal Alignment line...-
//      |    |       |   .      |       |
//      | -  *-------*  -. -  - |  0    -
//      |                .      |  |
//      |    |       |   .      |  | Bottom Offset
//      |                .      |  |
//      *-----------------------*  +
//                       .
//      0----+           .
//     Left Offset       .
//                  Vertical Alignment line
//                       .
//           |-------|   .
//          Rectangle Width
//                       .
//     0---------------- - 

// The Image Rectangle is a rectangle over the source image or a texture.  It's left, right and top, bottom are only used to find it's height and width.
// The Image Rectangle is where the pixels come from.  

// The best way to visualise this is a large image sized to include the offsets but the outside is transparent.
// This large image is then centered (0,0) on the alignment lines.

// If the image is left aligned then the Vertical Alignment line (msAlignment.x) is 0.
// If the image is right aligned then the Vertical Alignment line is -(Left Offset + Rectangle Width + Right Offset).  Note the negative.
// If the image is center aligned then it's as above divide by 2 (still negative).

// All offsets are positive.  It is wrong if any of the Image Rectangle lies outside of the larger 'image' formed by the offsets.

#define SUB_IMAGE_ALIGNMENT_LEFT				0x01
#define SUB_IMAGE_ALIGNMENT_CENTER_HORIZONTAL	0x02
#define SUB_IMAGE_ALIGNMENT_RIGHT				0x04
#define SUB_IMAGE_ALIGNMENT_TOP					0x08
#define SUB_IMAGE_ALIGNMENT_CENTER_VERTICAL		0x10
#define SUB_IMAGE_ALIGNMENT_BOTTOM				0x20


class CSubImage
{
public:
	CRectangle	mcImageRect;  //For width and height.
	SInt2		msOffsetTopLeft;  //+ve
	SInt2		msOffsetBottomRight;  //+ve
	SInt2		msAlignment;  //-ve

	void 	Init(void);
	void 	Init(int iLeft, int iTop, int iRight, int iBottom, int iLeftOffset, int iTopOffset, int iRightOffset, int iBottomOffset, int iAlignment = SUB_IMAGE_ALIGNMENT_LEFT | SUB_IMAGE_ALIGNMENT_TOP);
	void 	Kill(void);
	void 	Copy(CSubImage* pcSubImage);
	void 	SetAlignment(int iAlignment);
	void 	SetHorizontalAlignment(int iAlignment);  //Left/Right alignment.
	void 	SetVerticalAlignment(int iAlignment);  //Top/Bottom alignment.
	bool	IsImageEmpty(void);

	//All the methods below are relative.  Add X,Y to the return to get a meaningful destination position.
	int		GetFullWidth(void);
	int		GetFullHeight(void);
	int		GetImageWidth(void);
	int		GetImageHeight(void);
	SInt2	GetFullSize(void);
	SInt2	GetImageSize(void);

	int		GetImageLeft(void);
	int		GetImageTop(void);
	int		GetImageRight(void);
	int		GetImageBottom(void);

	int		GetFullLeft(void);
	int		GetFullTop(void);
	int		GetFullRight(void);
	int		GetFullBottom(void);

	void	GetFullDestBounds(int x, int y, CRectangle* pcDest);
	SInt2	GetImageDestPos(int x, int y);
	void	GetFullSourceBounds(CRectangle* pcReturn);
	void	GetImageSourceBounds(CRectangle* pcReturn);
	void	GetImageDestBounds(int x, int y, CRectangle* pcReturn);
	void	AdjustImageRect(CSubImage* pcSubImage);
	void	AdjustImageRect(int iLeft, int iTop, int iRight, int iBottom);
	void	AdjustToEmpty(void);
};


typedef CArrayTemplate<CSubImage> CArraySubImage;


#endif // !__SUB_IMAGE_H__

