/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __RECTANGLE_BEST_PACKER_WITH_ROTATION_H__
#define __RECTANGLE_BEST_PACKER_WITH_ROTATION_H__
#include "RectangleBestPacker.h"


//Packs like CRectangleBestPacker but may turn rectangles 90 degrees clockwise.  A turned rectangle's CPackedRectangle is rotated and has the added height as its width.
//The default attempts try every width with the rectangles as added, all turned wide and all turned tall, each with and without turning a rectangle that doesn't fit a space.
//The attempts with the rectangles as added and no turning are the CRectangleBestPacker attempts so the result is never worse.
class CRectangleBestPackerWithRotation : public CRectangleBestPacker
{
CONSTRUCTABLE(CRectangleBestPackerWithRotation)
public:
	void	AddDefaultAttempts(void) override;
};


#endif // __RECTANGLE_BEST_PACKER_WITH_ROTATION_H__

