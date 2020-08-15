#ifndef __LIFE_CYCLE_H__
#define __LIFE_CYCLE_H__
#include "IntegerHelper.h"


#define LIFE_CYCLE_FLAG_KILL 0x01
#define LIFE_CYCLE_FLAG_FREE 0x02


template <class M>
class CLife
{
protected:
	M**				mppcLifeCycleObject;
	unsigned char	mcFlags;

public:

	void	Init(M** ppcLifeCycleObject, BOOL bMustFree, BOOL bMustKill);
	void	Kill(void);

	BOOL	MustKill(void);
	BOOL	MustFree(void);
};


template <class M>
class CLifeInit
{
protected:
	M*				mpcLifeCycleObject;
	unsigned char	mcFlags;
public:
	void	Init(M* pcLifeCycleObject, BOOL bMustFree, BOOL bMustKill);
	void	ConfigureLife(CLife<M>* pcLife, M** ppcLifeCycleObject);

	M*		GetLife(void);
	BOOL	MustKill(void);
	BOOL	MustFree(void);
};


template <class M>
void CLife<M>::Init(M** ppcLifeCycleObject, BOOL bMustFree, BOOL bMustKill)
{
	mppcLifeCycleObject = ppcLifeCycleObject;
	mcFlags = 0;
	SetFlag(&mcFlags, LIFE_CYCLE_FLAG_FREE, bMustFree);
	SetFlag(&mcFlags, LIFE_CYCLE_FLAG_KILL, bMustKill);
};


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


#endif // __LIFE_CYCLE_H__

