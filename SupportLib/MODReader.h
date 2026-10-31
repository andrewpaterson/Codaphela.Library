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
#ifndef __MOD_READER_H__
#define __MOD_READER_H__
#include "ModSong.h"


//Reads ProTracker style modules: 31 sample modules with an "M.K." (or "M!K!", "M&K!", "N.T.", "FLT4", "4CHN", "6CHN", "8CHN",
//"CD81", "OKTA", "xCHN", "xxCH" or "TDZx") signature and 15 sample Soundtracker modules with none.  FLT8 (which splits patterns
//in two) isn't read.  Sample data cut off by the end of the file is silence.
Ptr<CModSong> LoadMOD(char* szFilename);
Ptr<CModSong> LoadMOD(void* pvFile, lsize uiFileSize);


#endif // __MOD_READER_H__

