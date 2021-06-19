#ifndef __LIFE_CYCLE_H__
#define __LIFE_CYCLE_H__
#include "IntegerHelper.h"


#define LIFE_CYCLE_FLAG_KILL 0x01
#define LIFE_CYCLE_FLAG_FREE 0x02
#define LIFE_CYCLE_FLAG_MASK 0x000000FF

#define LIFE_CYCLE_INITIALISED 0xB6A45B00
#define LIFE_CYCLE_KILLED      0x5D59B400

template <class M>
class CLife
{
protected:
	M**		mppcLifeCycleObject;
	uint32	mcFlags;

public:

	void	Init(M** ppcLifeCycleObject, BOOL bMustFree, BOOL bMustKill);
	void	Kill(void);

	BOOL	MustKill(void);
	BOOL	MustFree(void);

	BOOL	IsInitialised(void);
	BOOL	HasLifeCycleObject(void);
	BOOL	IsKilled(void);
};


template <class M>
class CLifeInit
{
protected:
	M*				mpcLifeCycleObject;
	unsigned char	mcFlags;

public:
	void	Init(M* pcLifeCycleObject, BOOL bMustFree = TRUE, BOOL bMustKill = TRUE);
	void	Null(void);

	void	ConfigureLife(CLife<M>* pcLife, M** ppcLifeCycleObject);

	M*		GetLife(void);
	BOOL	MustKill(void);
	BOOL	MustFree(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLife<M>::Init(M** ppcLifeCycleObject, BOOL bMustFree, BOOL bMustKill)
{
	mppcLifeCycleObject = ppcLifeCycleObject;
	mcFlags = LIFE_CYCLE_INITIALISED;
	SetFlag(&mcFlags, LIFE_CYCLE_FLAG_FREE, bMustFree);
	SetFlag(&mcFlags, LIFE_CYCLE_FLAG_KILL, bMustKill);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, class SuperClass, typename ... Args>
CLifeInit<SuperClass> LifeAlloc(Args ... args)
{
	CLifeInit<SuperClass>	cLife;
	SpecificClass*			pcLife;

	pcLife = NewMalloc<SpecificClass>();
	pcLife->Init(args...);
	cLife.Init(pcLife, TRUE, TRUE); 
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SuperClass>
CLifeInit<SuperClass> LifeLocal(SuperClass* pcLifeCycleObject)
{
	CLifeInit<SuperClass>	cLife;

	cLife.Init(pcLifeCycleObject, FALSE, FALSE);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SuperClass>
CLifeInit<SuperClass> LifeNull(void)
{
	CLifeInit<SuperClass>	cLife;
	cLife.Null();
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLifeInit<M>::Init(M* pcLifeCycleObject, BOOL bMustFree, BOOL bMustKill)
{
	mpcLifeCycleObject = pcLifeCycleObject;
	mcFlags = 0;
	SetFlag(&mcFlags, LIFE_CYCLE_FLAG_FREE, bMustFree);
	SetFlag(&mcFlags, LIFE_CYCLE_FLAG_KILL, bMustKill);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLifeInit<M>::Null(void)
{
	Init(NULL, FALSE, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLifeInit<M>::ConfigureLife(CLife<M>* pcLife, M** ppcLifeCycleObject)
{
	pcLife->Init(ppcLifeCycleObject, MustFree(), MustKill());
	if (ppcLifeCycleObject)
	{
		*ppcLifeCycleObject = mpcLifeCycleObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
M* CLifeInit<M>::GetLife(void)
{
	return mpcLifeCycleObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLifeInit<M>::MustKill(void)
{
	return FixBool(mcFlags & LIFE_CYCLE_FLAG_KILL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLifeInit<M>::MustFree(void)
{
	return FixBool(mcFlags & LIFE_CYCLE_FLAG_FREE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLife<M>::Kill(void)
{
	CKillable*	mpcKillable;

	if (mppcLifeCycleObject)
	{
		mpcKillable = *mppcLifeCycleObject;
		if (mpcKillable)
		{
			if (MustKill())
			{
				mpcKillable->Kill();
				mcFlags = (mcFlags & LIFE_CYCLE_FLAG_MASK) | LIFE_CYCLE_KILLED;
			}
			if (MustFree())
			{
				free(*mppcLifeCycleObject);
			}
			*mppcLifeCycleObject = NULL;
		}
		mppcLifeCycleObject = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLife<M>::MustKill(void)
{
	return FixBool(mcFlags & LIFE_CYCLE_FLAG_KILL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLife<M>::MustFree(void)
{
	return FixBool(mcFlags & LIFE_CYCLE_FLAG_FREE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLife<M>::IsInitialised(void)
{
	return (mcFlags & ~0x000000FF) == LIFE_CYCLE_INITIALISED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLife<M>::HasLifeCycleObject(void)
{
	if (mppcLifeCycleObject != NULL)
	{
		return *mppcLifeCycleObject != NULL;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CLife<M>::IsKilled(void)
{
	return (mcFlags & ~0x000000FF) == LIFE_CYCLE_KILLED;
}


#endif // !__LIFE_CYCLE_H__

