/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __INDEX_OBJECTS_H__
#define __INDEX_OBJECTS_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Chars.h"
#include "BaseObject.h"


#define INDEXED_OBJECTS_PAGE_SIZE		256		//Objects in a page.
#define INDEXED_OBJECTS_DIRECTORY_SIZE	256		//Pages in a directory.
#define INDEXED_OBJECTS_PAGE_SHIFT		8
#define INDEXED_OBJECTS_DIRECTORY_SHIFT	16


//The objects of INDEXED_OBJECTS_PAGE_SIZE indexes in a row from oiFirst, a multiple of the size.  NULL where there is
//none.
struct SIndexedObjectsPage
{
	OIndex			oiFirst;
	size			uiNumObjects;
	CBaseObject*	apcObjects[INDEXED_OBJECTS_PAGE_SIZE];
};


//The pages of INDEXED_OBJECTS_DIRECTORY_SIZE pages' indexes in a row.  NULL where there is none.
struct SIndexedObjectsDirectory
{
	size					uiNumPages;
	SIndexedObjectsPage*	apsPages[INDEXED_OBJECTS_DIRECTORY_SIZE];
};


//Where an iteration is: the index it looks from next.  Objects may be added and taken out as it goes.
struct SIndexesIterator
{
	OIndex	oiNext;
};


typedef CArrayTemplatePtr<SIndexedObjectsDirectory>	CArrayIndexedObjectsDirectories;


// CIndexedObjects maps an OIndex to a CBaseObject*.  An object system hands out indexes in a row from one, so they are
// kept as a table of two levels: an index's high bits pick a directory, its middle eight a page in the directory and
// its low eight the object's place in the page.  An object is found, put or taken out in a few loads, in any order,
// and they are iterated in index order.  The page last used is tried first (an object system mostly asks about
// indexes near the last); a page that empties stays until another does, so making and freeing an object over and
// over at the start of a page doesn't make and free the page each time.
class CIndexedObjects
{
protected:
	CArrayIndexedObjectsDirectories	mapsDirectories;	//By index >> INDEXED_OBJECTS_DIRECTORY_SHIFT, NULL where there is none.
	SIndexedObjectsPage*			mpsLastPage;
	SIndexedObjectsPage*			mpsEmptiedPage;		//The last page that emptied, if it is still where it was.
	SIndexedObjectsPage*			mpsSparePage;		//A page taken out (all NULL), for the next page needed.
	size							muiNumIndexed;

public:
	void					Init(void);
	void					Kill(void);
	CBaseObject*			Get(OIndex oi);
	bool					Has(OIndex oi);
	bool					Add(OIndex oi, CBaseObject* pcObject);
	bool					Remove(OIndex oi);
	size					NumIndexed(void) const;

	void					Print(CChars* pszDest);

	OIndex					StartIteration(SIndexesIterator* psIterator);
	OIndex					Iterate(SIndexesIterator* psIterator);
	CBaseObject*			StartIterationObject(SIndexesIterator* psIterator);
	CBaseObject*			IterateObject(SIndexesIterator* psIterator);

protected:
	SIndexedObjectsDirectory*	GetDirectory(OIndex oi);
	SIndexedObjectsPage*		GetPage(OIndex oi);
	SIndexedObjectsPage*		AddPage(OIndex oi);
	void						RemovePage(SIndexedObjectsPage* psPage);
	void						PageEmptied(SIndexedObjectsPage* psPage);
	bool						Step(SIndexesIterator* psIterator, OIndex* poi, CBaseObject** ppcObject);
};


#endif // __INDEX_OBJECTS_H__
