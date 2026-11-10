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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __ANIMATION_PLAYER_H__
#define __ANIMATION_PLAYER_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "ConnectionAnimation.h"


//Plays an animation.  Advance moves its time on by seconds times its speed: one that loops goes round from its end to its
//start, and one that doesn't stops at its end, when it's finished.  Apply puts the animation's connections where its tracks have
//them at that time, and CScene::UpdateConnections then makes their world matrices.  A player is the app's: a scene file holds
//animations, not who is playing them.
class CAnimationPlayer : public CObject
{
CONSTRUCTABLE(CAnimationPlayer);
DESTRUCTABLE(CAnimationPlayer);
protected:
	Ptr<CSceneAnimation>	mpAnimation;
	float32					mfTime;		//Seconds from the animation's start.
	float32					mfSpeed;	//1 plays it as fast as it was made.
	bool					mbLoop;

public:
	void		Init(Ptr<CSceneAnimation> pAnimation, bool bLoop);
	void		Free(void) override;
	void		Class(void) override;

	void		Advance(float32 fSeconds);
	void		Apply(void);

	void		SetTime(float32 fTime);
	float32		GetTime(void);
	void		SetSpeed(float32 fSpeed);
	float32		GetSpeed(void);
	bool		IsFinished(void);
	Ptr<CSceneAnimation>	GetAnimation(void);
};


#endif // __ANIMATION_PLAYER_H__
