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

** ------------------------------------------------------------------------ **/
#ifndef __IMAGE_RESAMPLE_SMART_H__
#define __IMAGE_RESAMPLE_SMART_H__
#include "Image.h"


//The largest differences (as fractions of a channel's range) between two pixels that are still smoothed together.
//The YUV thresholds are those of Maxim Stepin's hq filters (48, 7 and 6 out of 255).
#define SMART_THRESHOLD_Y			(48.0 / 255.0)
#define SMART_THRESHOLD_U			(7.0 / 255.0)
#define SMART_THRESHOLD_V			(6.0 / 255.0)
#define SMART_THRESHOLD_CHANNEL		(48.0 / 255.0)


//Resamples every channel of pcSource into pcDest, which must already have the same channels and its new size.
//Smooths like bilinear inside areas of similar colour but keeps the edges between different colours sharp; after
//Maxim Stepin's Smart Texture Filtering (http://www.hiend3d.com/smartflt.html), whose exact rules aren't published.
//
//Each dest pixel samples the same 2 x 2 source pixels with the same weights as bilinear.  Pixels are similar if their
//red, green and blue differ by less than the YUV thresholds and every other channel by less than SMART_THRESHOLD_CHANNEL;
//similar pixels (and pixels similar to those) form a group.  The dest pixel takes the group with the most bilinear weight
//and blends only that group's pixels.  So a flat or smooth block is bilinear, a block of all different pixels is nearest
//neighbour, a straight edge stays at the midpoint and a staircase corner is cut along a curve joining the edge midpoints.
//Values are float64 and integer channels are rounded.  Returns false if a channel can't be resampled.
bool ResampleSmart(Ptr<CImage> pcSource, Ptr<CImage> pcDest);


#endif // __IMAGE_RESAMPLE_SMART_H__

