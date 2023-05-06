/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __XML_FILE_H__
#define __XML_FILE_H__
#include "Markup.h"
#include "XMLParser.h"


class CXMLFile
{
public:
	CMarkup		mcMarkup;
			
	void 	Init(void);
	void 	Kill(void);
	bool 	Read(char* szFilename, char* szDirectory);
	bool 	Write(char* szFilename, char* szDirectory);

private:
	bool	Entities(CXMLParser* pcXMLParser, CChars* pszDirectory);
	bool	Read(char* szFilename, CMarkupDoc* pcDoc, CChars* pszDirectory);
};


#endif // !__XML_FILE_H__

