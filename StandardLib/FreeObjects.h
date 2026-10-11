#ifndef __FREE_OBJECTS_H__
#define __FREE_OBJECTS_H__
#include "Object.h"
#include "SetObject.h"


class CFreeObjects : public CObject
{
protected:
	Ptr<CSetObject>		mpFreeObjects;

public:
						void			Init(void);
						void			Free(void) override;
						void			Class(void) override;

						bool			Save(CObjectWriter* pcFile) override;
						bool			Load(CObjectReader* pcFile) override;

	template<class M>	M*				Add(Ptr<M>& p);
						CBaseObject*	Add(CPointer &p);
						bool			Remove(CBaseObject* pcObject);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CFreeObjects::Add(Ptr<M>& p)
{
	return (M*)Add((CPointer&)p);
}


#endif // __FREE_OBJECTS_H__

