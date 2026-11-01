#include "StandardLib/Pointer.h"
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"


class CMissile;
class CGameWorld;


class CGraphicPicture
{
public:
	char	mszPretenedImAPicture[16];

	void Init(void);
	void Kill(void);
};


struct SPhysicsPoint
{
	int		x;
	int		y;
	int		z;
};


struct SStateOnKill
{
	SPhysicsPoint		sPoint;
	CGraphicPicture		cPicture;
};


class CPlayerVehicle : public CObject
{
CONSTRUCTABLE(CPlayerVehicle)
DESTRUCTABLE(CPlayerVehicle)
protected:
	CGraphicPicture		mcPicture;
	SPhysicsPoint*		mpsPoint;

	SStateOnKill*		mpsBeforeDeath;
	SStateOnKill*		mpsAfterDeath;

public:
			void	Init(void);
			void	Class(void) override;
			void	Free(void) override;

			void	SetKillHook(SStateOnKill* psBeforeDeath, SStateOnKill* psAfterDeath);

	virtual void	PlayerInput(void) =0;
};


class CHarrier : public CPlayerVehicle
{
CONSTRUCTABLE(CHarrier)
DESTRUCTABLE(CHarrier)
protected:
	Ptr<CArray<CMissile>>	maMissiles;
	Ptr<CGameWorld>			mpWorld;
	int32					miSpeed;

public:
	void					Init(Ptr<CGameWorld> pWorld);
	void					Class(void) override;
	void					Free(void) override;

	void					PlayerInput(void) {};
	Ptr<CMissile>			FireMissile(CPointer& pTarget);
	Ptr<CArray<CMissile>>	GetMissiles(void);
};


class CJeep : public CPlayerVehicle
{
CONSTRUCTABLE(CJeep)
DESTRUCTABLE(CJeep)
protected:
	Ptr<CGameWorld>		mpWorld;
	float				mfFrontWheel;
	float				mfBackWheel;

public:
	void	Init(Ptr<CGameWorld> pWorld);
	void	Class(void) override;
	void	Free(void) override;

	void	PlayerInput(void) {};
};


class CRedJet : public CObject
{
CONSTRUCTABLE(CRedJet)
DESTRUCTABLE(CRedJet)
protected:
	CGraphicPicture		mcPicture;
	Ptr<CGameWorld>		mpWorld;
	SPhysicsPoint*		mpsPoint;

	SStateOnKill*		mpsBeforeDeath;
	SStateOnKill*		mpsAfterDeath;

public:
	void	Init(Ptr<CGameWorld> pWorld);
	void	Class(void) override;
	void	Free(void) override;

	void	SetKillHook(SStateOnKill* psBeforeDeath, SStateOnKill* psAfterDeath);
};


class CMissile : public CObject
{
CONSTRUCTABLE(CMissile)
DESTRUCTABLE(CMissile)
protected:
	Ptr<CGameWorld>		mpWorld;
	CPointer			mpTarget;
	char*				mszKillState;

public:
	void		Init(Ptr<CGameWorld> pWorld);
	void		Class(void) override;
	void		Free(void) override;

	void		SetTarget(CPointer& pTarget);
	CPointer	GetTarget(void);

	void		SetKillString(char* szKillString);
};


class CClusterMissile : public CObject
{
CONSTRUCTABLE(CClusterMissile)
DESTRUCTABLE(CClusterMissile)
public:
	CMissile			mcMissile1;
	CMissile			mcMissile2;

protected:
	Ptr<CGameWorld>		mpWorld;
	char*				mszKillState;

public:
	void	Init(Ptr<CGameWorld> pWorld);
	void	Class(void) override;
	void	Free(void) override;

	void	SetKillString(char* szKillString);
};


class CClusterLauncher : public CObject
{
CONSTRUCTABLE(CClusterLauncher)
DESTRUCTABLE(CClusterLauncher)
public:
	Ptr<CClusterMissile>	mpMissile;

	void 	Init(void);
	void	Class(void) override;
	void	Free(void) override;
};


class CGameWorld : public CObject
{
CONSTRUCTABLE(CGameWorld)
DESTRUCTABLE(CGameWorld)
protected:
	Ptr<CArray<>>		maTickables;
	Ptr<CPlayerVehicle>	mpPlayer1;
	Ptr<CPlayerVehicle>	mpPlayer2;

public:
	void			Init(void);
	void			Class(void) override;
	void			Free(void) override;

	void			Tick(void);
	void			AddPlayer(Ptr<CPlayerVehicle> pPlayer);
	void			AddTickable(const CPointer& pTickable);
	void			RemoveTickable(CPointer& pTickable);
	Ptr<CArray<>>	GetTickables(void);
};

