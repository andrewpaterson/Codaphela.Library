#include "BaseLib/Float4x4.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/AnimationPlayer.h"
#include "SupportLib/Scene.h"
#include "SupportLib/Skeleton.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SFloat4x4 TestSkeletonMove(float32 x, float32 y)
{
	SFloat4x4	sMatrix;

	sMatrix.Identity();
	sMatrix.pos.Init(x, y, 0.0f, 1.0f);
	return sMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
// A root, an arm two up from it and a hand one up from that.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CSkeleton> TestSkeletonArm(void)
{
	Ptr<CSkeleton>	pSkeleton;
	Ptr<CBone>		pRoot;
	Ptr<CBone>		pArm;
	SFloat4x4		sMatrix;

	pSkeleton = OMalloc<CSkeleton>();
	pSkeleton->mszName.Append("Arm");
	sMatrix = TestSkeletonMove(0.0f, 0.0f);
	pRoot = pSkeleton->AddBone("Root", NULL, &sMatrix);
	sMatrix = TestSkeletonMove(0.0f, 2.0f);
	pArm = pSkeleton->AddBone("Arm", pRoot, &sMatrix);
	sMatrix = TestSkeletonMove(0.0f, 1.0f);
	pSkeleton->AddBone("Hand", pArm, &sMatrix);
	return pSkeleton;
}


//////////////////////////////////////////////////////////////////////////
//
// A bone knows its place, and one whose parent is in another skeleton
// isn't added.
//
//////////////////////////////////////////////////////////////////////////
void TestSkeletonBones(void)
{
	Ptr<CSkeleton>	pSkeleton;
	Ptr<CSkeleton>	pOther;
	Ptr<CBone>		pBone;
	SFloat4x4		sMatrix;
	SLogConfig		sLogConfig;

	pSkeleton = TestSkeletonArm();
	pOther = TestSkeletonArm();
	AssertSize(3, pSkeleton->NumBones());
	pBone = pSkeleton->GetBone(2);
	AssertString("Hand", pBone->mszName.Text());
	AssertSize(2, pBone->muiIndex);
	AssertPointer(&pSkeleton->GetBone(1), &pBone->mpParent);
	AssertFloat(1.0f, pBone->msLocalMatrix.pos.y, 3);
	AssertTrue(pSkeleton->Contains(pBone));
	AssertFalse(pOther->Contains(pBone));
	AssertFalse(pSkeleton->Contains(NULL));

	sMatrix = TestSkeletonMove(0.0f, 0.0f);
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(pSkeleton->AddBone("Finger", pOther->GetBone(2), &sMatrix).IsNull());
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(3, pSkeleton->NumBones());
}


//////////////////////////////////////////////////////////////////////////
//
// Each instance has its own connections for the bones, which start at the
// bones' rest matrices and hang from what the instance gives.  A skinned
// mesh's matrices are bound to them in the skin's order, whatever order the
// skeleton has them in.
//
//////////////////////////////////////////////////////////////////////////
void TestSkeletonInstances(void)
{
	Ptr<CScene>			pScene;
	Ptr<CSkeleton>		pSkeleton;
	Ptr<CInstance>		pFirst;
	Ptr<CInstance>		pSecond;
	Ptr<CConnection>	pRoot;
	Ptr<CConnection>	pHand;
	Ptr<CMeshEditor>	pEditor;
	Ptr<CInstanceMesh>	pFirstMesh;
	Ptr<CInstanceMesh>	pSecondMesh;
	CMesh*				pcMesh;

	pScene = OMalloc<CScene>();
	pSkeleton = TestSkeletonArm();

	pFirst = OMalloc<CInstance>();
	pFirst->MakeBones(pSkeleton, NULL);
	pSecond = OMalloc<CInstance>();
	pRoot = OMalloc<CConnection>();
	pRoot->msLocalMatrix = TestSkeletonMove(100.0f, 0.0f);
	pSecond->mapConnections.Add(pRoot);
	pSecond->MakeBones(pSkeleton, pRoot);
	pScene->mapInstances.Add(pFirst);
	pScene->mapInstances.Add(pSecond);

	AssertPointer(&pSkeleton, &pFirst->mpSkeleton);
	AssertSize(3, pFirst->mapConnections.NumElements());
	AssertSize(4, pSecond->mapConnections.NumElements());
	pHand = pSecond->mapConnections.Get(3);
	AssertString("Hand", pHand->mszName.Text());
	AssertPointer(&pSkeleton->GetBone(2), &pHand->mpBone);
	AssertPointer(&pSecond->mapConnections.Get(2), &pHand->mpParent);
	AssertTrue(pFirst->mapConnections.Get(0)->mpParent.IsNull());
	AssertPointer(&pRoot, &pSecond->mapConnections.Get(1)->mpParent);
	AssertPointer(&pHand, &pSecond->GetBoneConnection(pSkeleton->GetBone(2)));
	AssertPointer(&pFirst->mapConnections.Get(2), &pFirst->GetBoneConnection(pSkeleton->GetBone(2)));
	AssertTrue(pFirst->GetBoneConnection(NULL).IsNull());
	AssertTrue(pRoot->mpBone.IsNull());

	pScene->UpdateConnections();
	AssertFloat(3.0f, pFirst->mapConnections.Get(2)->msWorldMatrix.pos.y, 3);
	AssertFloat(0.0f, pFirst->mapConnections.Get(2)->msWorldMatrix.pos.x, 3);
	AssertFloat(3.0f, pHand->msWorldMatrix.pos.y, 3);
	AssertFloat(100.0f, pHand->msWorldMatrix.pos.x, 3);

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcSkin.mbInUse = true;
	pcMesh->mcSkin.mapBones.Add(pSkeleton->GetBone(2));
	pcMesh->mcSkin.mapBones.Add(pSkeleton->GetBone(0));
	pFirstMesh = pFirst->AddMesh(pEditor);
	pSecondMesh = pSecond->AddMesh(pEditor);
	AssertTrue(pFirstMesh->IsSkinned());
	AssertSize(2, pSecondMesh->mapConnections.NumElements());
	AssertPointer(&pHand, &pSecondMesh->mapConnections.Get(0));
	AssertPointer(&pSecond->mapConnections.Get(1), &pSecondMesh->mapConnections.Get(1));
	AssertPointer(&pFirst->mapConnections.Get(2), &pFirstMesh->mapConnections.Get(0));
	AssertSize(1, pFirst->mapMeshes.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestSkeletonPosition(float32 x, float32 y, float32 z, SFloat32Vec3* psActual)
{
	AssertFloat(x, psActual->x, 3);
	AssertFloat(y, psActual->y, 3);
	AssertFloat(z, psActual->z, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh bound to the arm at rest: a corner on the root, one on the arm,
// one between the arm and the hand whose weights add up to a half, and
// one with no weights.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> TestSkeletonArmMesh(Ptr<CSkeleton> pSkeleton)
{
	Ptr<CMeshEditor>	pEditor;
	CMeshSkin*			pcSkin;
	SSkinWeight*		psWeight;
	SFloat4x4			sInverse;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->mszName.Append("Sleeve");
	pcSkin = &pEditor->mpMesh->mcSkin;
	pcSkin->mbInUse = true;
	pEditor->AddCorner(1.0f, 0.0f, 0.0f);
	pEditor->AddCorner(1.0f, 2.0f, 0.0f);
	pEditor->AddCorner(1.0f, 3.0f, 0.0f);
	pEditor->AddCorner(5.0f, 5.0f, 5.0f);
	psWeight = pcSkin->mcSkinVerts.Get(0)->mcWeights.Add();
	psWeight->iMatrix = 0;
	psWeight->fWeight = 1.0f;
	psWeight = pcSkin->mcSkinVerts.Get(1)->mcWeights.Add();
	psWeight->iMatrix = 1;
	psWeight->fWeight = 1.0f;
	psWeight = pcSkin->mcSkinVerts.Get(2)->mcWeights.Add();
	psWeight->iMatrix = 1;
	psWeight->fWeight = 0.25f;
	psWeight = pcSkin->mcSkinVerts.Get(2)->mcWeights.Add();
	psWeight->iMatrix = 2;
	psWeight->fWeight = 0.25f;

	sInverse = TestSkeletonMove(0.0f, 0.0f);
	pcSkin->mcInverseSkinMatricies.Add(&sInverse);
	sInverse = TestSkeletonMove(0.0f, -2.0f);
	pcSkin->mcInverseSkinMatricies.Add(&sInverse);
	sInverse = TestSkeletonMove(0.0f, -3.0f);
	pcSkin->mcInverseSkinMatricies.Add(&sInverse);
	pcSkin->mapBones.Add(pSkeleton->GetBone(0));
	pcSkin->mapBones.Add(pSkeleton->GetBone(1));
	pcSkin->mapBones.Add(pSkeleton->GetBone(2));
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// Posed in floats: the root moved 10 along x, and the arm and the hand
// each turned a quarter about z, taking x to y.  The corner between the
// arm and the hand is half way between where each puts it, and the one
// with no weights stays where it was bound.  A rigid mesh is where its
// connection puts it, and a skinned mesh with a bone its instance hasn't
// got can't be posed.
//
//////////////////////////////////////////////////////////////////////////
void TestSkeletonPose(void)
{
	Ptr<CScene>			pScene;
	Ptr<CSkeleton>		pSkeleton;
	Ptr<CSkeleton>		pOther;
	Ptr<CInstance>		pInstance;
	Ptr<CInstance>		pElsewhere;
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshEditor>	pRigid;
	Ptr<CConnection>	pConnection;
	CArrayFloat32Vec3	asPositions;
	SFloat4x4			sMatrix;
	SLogConfig			sLogConfig;

	pScene = OMalloc<CScene>();
	pSkeleton = TestSkeletonArm();
	pEditor = TestSkeletonArmMesh(pSkeleton);
	pInstance = OMalloc<CInstance>();
	pInstance->MakeBones(pSkeleton, NULL);
	pInstanceMesh = pInstance->AddMesh(pEditor);
	pScene->mapInstances.Add(pInstance);
	pInstance->mapConnections.Get(0)->msLocalMatrix = TestSkeletonMove(10.0f, 0.0f);
	Float4x4RotationZ(&sMatrix, 3.14159265f / 2.0f);
	sMatrix.pos.Init(0.0f, 2.0f, 0.0f, 1.0f);
	pInstance->mapConnections.Get(1)->msLocalMatrix = sMatrix;
	sMatrix.pos.Init(0.0f, 1.0f, 0.0f, 1.0f);
	pInstance->mapConnections.Get(2)->msLocalMatrix = sMatrix;
	pScene->UpdateConnections();

	asPositions.Init();
	AssertTrue(pInstanceMesh->Pose(&asPositions));
	AssertSize(4, asPositions.NumElements());
	AssertTestSkeletonPosition(11.0f, 0.0f, 0.0f, asPositions.Get(0));
	AssertTestSkeletonPosition(10.0f, 3.0f, 0.0f, asPositions.Get(1));
	AssertTestSkeletonPosition(8.5f, 2.5f, 0.0f, asPositions.Get(2));
	AssertTestSkeletonPosition(5.0f, 5.0f, 5.0f, asPositions.Get(3));

	pRigid = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pRigid->AddCorner(1.0f, 2.0f, 3.0f);
	pConnection = OMalloc<CConnection>();
	Float4x4Translation(&pConnection->msLocalMatrix, 0.0f, 0.0f, 7.0f);
	pInstance->mapConnections.Add(pConnection);
	pInstanceMesh = pInstance->AddMesh(pRigid);
	pInstanceMesh->mapConnections.Add(pConnection);
	pScene->UpdateConnections();
	AssertTrue(pInstanceMesh->Pose(&asPositions));
	AssertSize(1, asPositions.NumElements());
	AssertTestSkeletonPosition(1.0f, 2.0f, 10.0f, asPositions.Get(0));

	pOther = OMalloc<CSkeleton>();
	sMatrix = TestSkeletonMove(0.0f, 0.0f);
	pOther->AddBone("Elsewhere", NULL, &sMatrix);
	pElsewhere = OMalloc<CInstance>();
	pElsewhere->MakeBones(pOther, NULL);
	sLogConfig = gcLogger.SetSilent();
	pInstanceMesh = pElsewhere->AddMesh(pEditor);
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(pInstanceMesh->Pose(&asPositions));
	AssertSize(0, asPositions.NumElements());
	asPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The arm waves: its root moves along x and its arm up and down.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CSceneAnimation> TestSkeletonWave(Ptr<CSkeleton> pSkeleton)
{
	Ptr<CSceneAnimation>		pAnimation;
	Ptr<CConnectionAnimation>	pTrack;
	SFloat4x4					sKey;

	pAnimation = OMalloc<CSceneAnimation>();
	pAnimation->mpSkeleton = pSkeleton;

	pTrack = OMalloc<CConnectionAnimation>();
	pTrack->mpBone = pSkeleton->GetBone(0);
	sKey = TestSkeletonMove(0.0f, 0.0f);
	pTrack->AddKey(&sKey, 0.0f);
	sKey = TestSkeletonMove(10.0f, 0.0f);
	pTrack->AddKey(&sKey, 1.0f);
	pAnimation->mapTracks.Add(pTrack);

	pTrack = OMalloc<CConnectionAnimation>();
	pTrack->mpBone = pSkeleton->GetBone(1);
	sKey = TestSkeletonMove(0.0f, 2.0f);
	pTrack->AddKey(&sKey, 0.0f);
	sKey = TestSkeletonMove(0.0f, 6.0f);
	pTrack->AddKey(&sKey, 1.0f);
	pAnimation->mapTracks.Add(pTrack);
	return pAnimation;
}


//////////////////////////////////////////////////////////////////////////
//
// One animation of a skeleton plays on two instances at their own times,
// and moves nothing on its own.  An instance with another skeleton binds
// none of it.  The scene's own animation still moves its connections.
//
//////////////////////////////////////////////////////////////////////////
void TestSkeletonAnimation(void)
{
	Ptr<CScene>					pScene;
	Ptr<CSkeleton>				pSkeleton;
	Ptr<CInstance>				pFirst;
	Ptr<CInstance>				pSecond;
	Ptr<CInstance>				pStranger;
	Ptr<CConnection>			pRoot;
	Ptr<CConnection>			pLamp;
	Ptr<CSceneAnimation>		pWave;
	Ptr<CSceneAnimation>		pSwing;
	Ptr<CConnectionAnimation>	pTrack;
	Ptr<CAnimationPlayer>		pFirstPlayer;
	Ptr<CAnimationPlayer>		pSecondPlayer;
	Ptr<CAnimationPlayer>		pStrangerPlayer;
	Ptr<CAnimationPlayer>		pSwingPlayer;
	SFloat4x4					sKey;

	pScene = OMalloc<CScene>();
	pSkeleton = TestSkeletonArm();
	pFirst = OMalloc<CInstance>();
	pFirst->MakeBones(pSkeleton, NULL);
	pSecond = OMalloc<CInstance>();
	pRoot = OMalloc<CConnection>();
	pRoot->msLocalMatrix = TestSkeletonMove(0.0f, 100.0f);
	pSecond->mapConnections.Add(pRoot);
	pSecond->MakeBones(pSkeleton, pRoot);
	pStranger = OMalloc<CInstance>();
	pStranger->MakeBones(TestSkeletonArm(), NULL);
	pScene->mapInstances.Add(pFirst);
	pScene->mapInstances.Add(pSecond);
	pScene->mapInstances.Add(pStranger);

	pWave = TestSkeletonWave(pSkeleton);
	pScene->mapAnimations.Add(pWave);
	AssertTrue(pWave->IsSkeletal());
	pWave->Apply(1.0f);
	AssertFloat(0.0f, pFirst->mapConnections.Get(0)->msLocalMatrix.pos.x, 3);

	pFirstPlayer = OMalloc<CAnimationPlayer>(pWave, pFirst, false);
	pSecondPlayer = OMalloc<CAnimationPlayer>(pWave, pSecond, false);
	pStrangerPlayer = OMalloc<CAnimationPlayer>(pWave, pStranger, false);
	AssertSize(2, pFirstPlayer->NumBound());
	AssertSize(0, pStrangerPlayer->NumBound());
	AssertPointer(&pSecond, &pSecondPlayer->GetInstance());

	pFirstPlayer->SetTime(0.25f);
	pSecondPlayer->SetTime(0.75f);
	pFirstPlayer->Apply();
	pSecondPlayer->Apply();
	pStrangerPlayer->Apply();
	pScene->UpdateConnections();
	AssertFloat(2.5f, pFirst->mapConnections.Get(2)->msWorldMatrix.pos.x, 3);
	AssertFloat(4.0f, pFirst->mapConnections.Get(2)->msWorldMatrix.pos.y, 3);
	AssertFloat(7.5f, pSecond->mapConnections.Get(3)->msWorldMatrix.pos.x, 3);
	AssertFloat(106.0f, pSecond->mapConnections.Get(3)->msWorldMatrix.pos.y, 3);
	AssertFloat(0.0f, pStranger->mapConnections.Get(2)->msWorldMatrix.pos.x, 3);
	AssertFloat(3.0f, pStranger->mapConnections.Get(2)->msWorldMatrix.pos.y, 3);

	pLamp = OMalloc<CConnection>();
	pLamp->mpParent = pFirst->GetBoneConnection(pSkeleton->GetBone(2));
	pFirst->mapConnections.Add(pLamp);
	pSwing = OMalloc<CSceneAnimation>();
	pTrack = OMalloc<CConnectionAnimation>();
	pTrack->mpConnection = pLamp;
	sKey = TestSkeletonMove(0.0f, 0.0f);
	pTrack->AddKey(&sKey, 0.0f);
	sKey = TestSkeletonMove(0.0f, -4.0f);
	pTrack->AddKey(&sKey, 2.0f);
	pSwing->mapTracks.Add(pTrack);
	pScene->mapAnimations.Add(pSwing);
	AssertFalse(pSwing->IsSkeletal());
	pSwingPlayer = OMalloc<CAnimationPlayer>(pSwing, false);
	pSwingPlayer->SetTime(1.0f);
	pSwingPlayer->Apply();
	pScene->UpdateConnections();
	AssertFloat(2.5f, pLamp->msWorldMatrix.pos.x, 3);
	AssertFloat(2.0f, pLamp->msWorldMatrix.pos.y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A bone added to a skeleton after its instances, skins and animations were
// made moves none of the others, so all of them still work.  Only an
// instance made afterwards has a connection for it.
//
//////////////////////////////////////////////////////////////////////////
void TestSkeletonAddBoneLater(void)
{
	Ptr<CScene>				pScene;
	Ptr<CSkeleton>			pSkeleton;
	Ptr<CInstance>			pBefore;
	Ptr<CInstance>			pAfter;
	Ptr<CMeshEditor>		pEditor;
	Ptr<CMeshEditor>		pHooked;
	Ptr<CInstanceMesh>		pInstanceMesh;
	Ptr<CSceneAnimation>	pWave;
	Ptr<CAnimationPlayer>	pPlayer;
	Ptr<CBone>				pHook;
	SFloat4x4				sMatrix;
	SLogConfig				sLogConfig;

	pScene = OMalloc<CScene>();
	pSkeleton = TestSkeletonArm();
	pBefore = OMalloc<CInstance>();
	pBefore->MakeBones(pSkeleton, NULL);
	pScene->mapInstances.Add(pBefore);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->mpMesh->mcSkin.mbInUse = true;
	pEditor->mpMesh->mcSkin.mapBones.Add(pSkeleton->GetBone(2));
	pWave = TestSkeletonWave(pSkeleton);

	sMatrix = TestSkeletonMove(0.5f, 0.0f);
	pHook = pSkeleton->AddBone("Hook", pSkeleton->GetBone(2), &sMatrix);
	AssertSize(3, pHook->muiIndex);
	AssertSize(2, pSkeleton->GetBone(2)->muiIndex);

	pInstanceMesh = pBefore->AddMesh(pEditor);
	AssertPointer(&pBefore->mapConnections.Get(2), &pInstanceMesh->mapConnections.Get(0));
	AssertTrue(pBefore->GetBoneConnection(pHook).IsNull());
	pPlayer = OMalloc<CAnimationPlayer>(pWave, pBefore, false);
	AssertSize(2, pPlayer->NumBound());

	//A mesh posed by the new bone, which the instance made before it hasn't got, can't be posed by that instance.
	pHooked = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pHooked->mpMesh->mcSkin.mbInUse = true;
	pHooked->mpMesh->mcSkin.mapBones.Add(pSkeleton->GetBone(0));
	pHooked->mpMesh->mcSkin.mapBones.Add(pHook);
	sLogConfig = gcLogger.SetSilent();
	pInstanceMesh = pBefore->AddMesh(pHooked);
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(2, pInstanceMesh->mapConnections.NumElements());
	AssertPointer(&pBefore->mapConnections.Get(0), &pInstanceMesh->mapConnections.Get(0));
	AssertTrue(pInstanceMesh->mapConnections.Get(1).IsNull());

	pAfter = OMalloc<CInstance>();
	pAfter->MakeBones(pSkeleton, NULL);
	pScene->mapInstances.Add(pAfter);
	AssertSize(4, pAfter->mapConnections.NumElements());
	pScene->UpdateConnections();
	AssertFloat(0.5f, pAfter->GetBoneConnection(pHook)->msWorldMatrix.pos.x, 3);
	AssertFloat(3.0f, pAfter->GetBoneConnection(pHook)->msWorldMatrix.pos.y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSkeleton(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestSkeletonBones();
	ObjectsFlush();
	TestSkeletonInstances();
	ObjectsFlush();
	TestSkeletonAnimation();
	ObjectsFlush();
	TestSkeletonPose();
	ObjectsFlush();
	TestSkeletonAddBoneLater();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

