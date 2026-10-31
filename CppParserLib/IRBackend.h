/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_BACKEND_H__
#define __I_R_BACKEND_H__
#include "BaseLib/ArrayTemplate.h"
#include "CPPErrors.h"
#include "IRModule.h"
#include "IRSourceMarkers.h"


//What the 65C64 (CIR65Backend) and W65C816 (CIR816Backend) back ends share: the module and the label counter, the
//function being written and the text it goes to, its source markers, errors and block labels, and writing a module
//as sections (each function and global a section of its own), its globals' data, labels and errors.
class CIRBackend
{
protected:
	CIRModule*				mpcModule;
	int*					mpiLabels;
	CIRFunction*			mpcFunction;
	CChars*					mpszOut;
	uint32					muiSource;			//The source token + 1 of the last marker written (Source).
	bool					mbSourceMarkers;	//Source writes markers (SetSourceMarkers), for a listing.
	CIRSourceMarkers		mcSourceMarkers;	//The markers while the text passes run.
	CCPPErrors*				mpcErrors;
	bool					mbResult;
	CArrayTemplate<int32>	maiBlockLabels;		//Per block index.
	int32					miFrameBytes;
	const char*				mszSectionOptions;	//After each function's section name: ", comdat" for the W65C816's runtime.

public:
	virtual bool	GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors) = 0;

	bool			GenerateModule(CChars* pszDest, CCPPErrors* pcErrors);
	void			GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest);
	void			SetSourceMarkers(bool bSourceMarkers);

protected:
	void			Init(CIRModule* pcModule, int* piLabels);
	void			Kill(void);

	void			Binding(const char* szName, uint32 uiAttributes, CChars* pszDest);
	bool			Error(const char* szMessage, const char* szDetail = NULL);
	void			Source(uint32 uiSource);
	int32			NewLabel(void);
	void			Label(int32 iLabel);
};


#endif // __I_R_BACKEND_H__

