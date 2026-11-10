#include <math.h>
#include "BaseLib/Float4x4.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/AnimationPlayer.h"
#include "SupportLib/ConnectionAnimation.h"
#include "SupportLib/Scene.h"
#include "TestLib/Assert.h"


#define TEST_ANIMATION_PI		3.14159265358979f


//////////////////////////////////////////////////////////////////////////
//
// A turn about z of fAngle, moved to x and scaled by fScale along each
// axis.
//
//////////////////////////////////////////////////////////////////////////
static SFloat4x4 TestAnimationMatrix(float32 fAngle, float32 x, float32 fScale)
{
	SFloat4x4	sMatrix;

	Float4x4RotationZ(&sMatrix, fAngle);
	sMatrix.x.x *= fScale;	sMatrix.x.y *= fScale;	sMatrix.x.z *= fScale;
	sMatrix.y.x *= fScale;	sMatrix.y.y *= fScale;	sMatrix.y.z *= fScale;
	sMatrix.z.x *= fScale;	sMatrix.z.y *= fScale;	sMatrix.z.z *= fScale;
	sMatrix.pos.Init(x, 0.0f, 0.0f, 1.0f);
	return sMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestAnimationMatrix(float32 fAngle, float32 x, float32 fScale, SFloat4x4* psActual)
{
	AssertFloat(cosf(fAngle) * fScale, psActual->x.x, 3);
	AssertFloat(sinf(fAngle) * fScale, psActual->x.y, 3);
	AssertFloat(-sinf(fAngle) * fScale, psActual->y.x, 3);
	AssertFloat(cosf(fAngle) * fScale, psActual->y.y, 3);
	AssertFloat(fScale, psActual->z.z, 3);
	AssertFloat(x, psActual->pos.x, 3);
	AssertFloat(0.0f, psActual->pos.y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Between two keys the turn is slerped and the position and scale
// blended straight, so halfway from no turn to a quarter turn is an eighth
// of a turn.  Before the first key it's at the first, and after the last
// at the last.  Keys go in time order whatever order they're added in.
//
//////////////////////////////////////////////////////////////////////////
void TestConnectionAnimationBlend(void)
{
	Ptr<CConnectionAnimation>	pTrack;
	SFloat4x4					sMatrix;
	SFloat4x4					sKey;

	pTrack = OMalloc<CConnectionAnimation>();
	AssertFalse(pTrack->GetMatrix(&sMatrix, 0.0f));
	AssertFloat(0.0f, pTrack->GetDuration(), 3);

	sKey = TestAnimationMatrix(TEST_ANIMATION_PI / 2.0f, 4.0f, 3.0f);
	pTrack->AddKey(&sKey, 2.0f);
	sKey = TestAnimationMatrix(0.0f, 0.0f, 1.0f);
	pTrack->AddKey(&sKey, 0.0f);
	AssertSize(2, pTrack->NumKeys());
	AssertFloat(0.0f, pTrack->GetKey(0)->fTime, 3);
	AssertFloat(2.0f, pTrack->GetKey(1)->fTime, 3);
	AssertFloat(2.0f, pTrack->GetDuration(), 3);

	AssertTrue(pTrack->GetMatrix(&sMatrix, 1.0f));
	AssertTestAnimationMatrix(TEST_ANIMATION_PI / 4.0f, 2.0f, 2.0f, &sMatrix);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 0.5f));
	AssertTestAnimationMatrix(TEST_ANIMATION_PI / 8.0f, 1.0f, 1.5f, &sMatrix);

	AssertTrue(pTrack->GetMatrix(&sMatrix, -1.0f));
	AssertTestAnimationMatrix(0.0f, 0.0f, 1.0f, &sMatrix);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 0.0f));
	AssertTestAnimationMatrix(0.0f, 0.0f, 1.0f, &sMatrix);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 2.0f));
	AssertTestAnimationMatrix(TEST_ANIMATION_PI / 2.0f, 4.0f, 3.0f, &sMatrix);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 5.0f));
	AssertTestAnimationMatrix(TEST_ANIMATION_PI / 2.0f, 4.0f, 3.0f, &sMatrix);

	//A shear, which splitting a key loses, is blended straight, so it's there at its key and nothing jumps as the key goes by.
	pTrack = OMalloc<CConnectionAnimation>();
	sKey.Identity();
	pTrack->AddKey(&sKey, 0.0f);
	sKey.y.x = 0.3f;
	pTrack->AddKey(&sKey, 1.0f);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 0.999f));
	AssertFloat(0.3f, sMatrix.y.x, 2);
	AssertFloat(1.0f, sMatrix.y.y, 2);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 0.5f));
	AssertFloat(0.15f, sMatrix.y.x, 2);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 0.001f));
	AssertFloat(0.0f, sMatrix.y.x, 2);

	//One key is where it always is.
	pTrack = OMalloc<CConnectionAnimation>();
	sKey = TestAnimationMatrix(0.3f, 7.0f, 1.0f);
	pTrack->AddKey(&sKey, 1.0f);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 0.0f));
	AssertTestAnimationMatrix(0.3f, 7.0f, 1.0f, &sMatrix);
	AssertTrue(pTrack->GetMatrix(&sMatrix, 9.0f));
	AssertTestAnimationMatrix(0.3f, 7.0f, 1.0f, &sMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
// A track moves its connection's local matrix, so a parent carries what's
// animated: the world matrix is made from it by UpdateConnections.  An
// animation lasts as long as its longest track.
//
//////////////////////////////////////////////////////////////////////////
void TestConnectionAnimationApply(void)
{
	Ptr<CScene>					pScene;
	Ptr<CInstance>				pInstance;
	Ptr<CConnection>			pParent;
	Ptr<CConnection>			pChild;
	Ptr<CSceneAnimation>		pAnimation;
	Ptr<CConnectionAnimation>	pTrack;
	SFloat4x4					sKey;

	pScene = OMalloc<CScene>();
	pInstance = OMalloc<CInstance>();
	pParent = OMalloc<CConnection>();
	pParent->msLocalMatrix.pos.Init(0.0f, 10.0f, 0.0f, 1.0f);
	pChild = OMalloc<CConnection>();
	pChild->mpParent = pParent;
	pInstance->mapConnections.Add(pParent);
	pInstance->mapConnections.Add(pChild);
	pScene->mapInstances.Add(pInstance);

	pAnimation = OMalloc<CSceneAnimation>();
	pTrack = OMalloc<CConnectionAnimation>();
	pTrack->mpConnection = pChild;
	sKey = TestAnimationMatrix(0.0f, 0.0f, 1.0f);
	pTrack->AddKey(&sKey, 0.0f);
	sKey = TestAnimationMatrix(0.0f, 4.0f, 1.0f);
	pTrack->AddKey(&sKey, 1.0f);
	pAnimation->mapTracks.Add(pTrack);
	pTrack = OMalloc<CConnectionAnimation>();
	sKey = TestAnimationMatrix(0.0f, 0.0f, 1.0f);
	pTrack->AddKey(&sKey, 3.0f);
	pAnimation->mapTracks.Add(pTrack);
	pScene->mapAnimations.Add(pAnimation);
	AssertFloat(3.0f, pAnimation->GetDuration(), 3);

	//The track with no connection moves nothing.
	pAnimation->Apply(0.5f);
	pScene->UpdateConnections();
	AssertFloat(2.0f, pChild->msLocalMatrix.pos.x, 3);
	AssertFloat(2.0f, pChild->msWorldMatrix.pos.x, 3);
	AssertFloat(10.0f, pChild->msWorldMatrix.pos.y, 3);
	AssertFloat(10.0f, pParent->msWorldMatrix.pos.y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A player that loops goes round from the end to the start; one that
// doesn't stops at the end and has finished.  Its speed scales the time
// it's advanced by, backwards when negative.
//
//////////////////////////////////////////////////////////////////////////
void TestConnectionAnimationPlayer(void)
{
	Ptr<CConnection>			pConnection;
	Ptr<CSceneAnimation>		pAnimation;
	Ptr<CConnectionAnimation>	pTrack;
	Ptr<CAnimationPlayer>		pPlayer;
	SFloat4x4					sKey;

	pConnection = OMalloc<CConnection>();
	pAnimation = OMalloc<CSceneAnimation>();
	pTrack = OMalloc<CConnectionAnimation>();
	pTrack->mpConnection = pConnection;
	sKey = TestAnimationMatrix(0.0f, 0.0f, 1.0f);
	pTrack->AddKey(&sKey, 0.0f);
	sKey = TestAnimationMatrix(0.0f, 8.0f, 1.0f);
	pTrack->AddKey(&sKey, 2.0f);
	pAnimation->mapTracks.Add(pTrack);

	pPlayer = OMalloc<CAnimationPlayer>(pAnimation, true);
	AssertFloat(0.0f, pPlayer->GetTime(), 3);
	pPlayer->Advance(0.5f);
	pPlayer->Apply();
	AssertFloat(2.0f, pConnection->msLocalMatrix.pos.x, 3);
	pPlayer->Advance(2.0f);
	AssertFloat(0.5f, pPlayer->GetTime(), 3);
	AssertFalse(pPlayer->IsFinished());
	pPlayer->SetSpeed(-1.0f);
	pPlayer->Advance(1.0f);
	AssertFloat(1.5f, pPlayer->GetTime(), 3);
	pPlayer->Apply();
	AssertFloat(6.0f, pConnection->msLocalMatrix.pos.x, 3);

	pPlayer = OMalloc<CAnimationPlayer>(pAnimation, false);
	pPlayer->SetSpeed(2.0f);
	pPlayer->Advance(0.5f);
	AssertFloat(1.0f, pPlayer->GetTime(), 3);
	AssertFalse(pPlayer->IsFinished());
	pPlayer->Advance(5.0f);
	AssertFloat(2.0f, pPlayer->GetTime(), 3);
	AssertTrue(pPlayer->IsFinished());
	pPlayer->Apply();
	AssertFloat(8.0f, pConnection->msLocalMatrix.pos.x, 3);
	pPlayer->SetTime(-3.0f);
	AssertFloat(0.0f, pPlayer->GetTime(), 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConnectionAnimation(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestConnectionAnimationBlend();
	ObjectsFlush();
	TestConnectionAnimationApply();
	ObjectsFlush();
	TestConnectionAnimationPlayer();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
