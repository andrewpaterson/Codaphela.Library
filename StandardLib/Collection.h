/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __COLLECTION_H__
#define __COLLECTION_H__
#include "BaseObject.h"
#include "StackPointers.h"
#include "StandardLib/SetIterator.h"

//Note: This class has no fields.
class CObjects;
class CCollection : public CBaseObject
{
CONSTRUCTABLE(CCollection)
protected:
	bool	mbSubRoot;

public:
							~CCollection();
			void			Init(void);
			void			Kill(void) final;
			bool			IsCollection(void) override;
			bool			IsObject(void) override;
			CBaseObject*	Dehollow(void);
	virtual void			TouchAll(void) =0;
	virtual void			KillAll(void) =0;

	virtual CPointer		StartIterationPointer(SSetIterator* psIter) =0;
	virtual CPointer		IteratePointer(SSetIterator* psIter) =0;

	virtual size			NumElements(void) =0;
			size			Length(void);
			size			Size(void);
	virtual size			NonNullElements(void) =0;
	virtual bool			IsEmpty(void) =0;

	virtual bool			RemoveAll(void) =0;
			bool			Clear(void);

			void			MakeSubRoot(void);
			bool			IsSubRoot(void);

	virtual void			UnsafePointTo(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld) =0;

protected:
			void			Class(void) override;
			void			EmbedFields(void);

			bool			AddObjectFrom(CEmbeddedObject* pcObject, bool bResult);
			bool			RemoveObjectTryFree(CEmbeddedObject* pcObject, bool bResult);
};


#endif // __COLLECTION_H__

