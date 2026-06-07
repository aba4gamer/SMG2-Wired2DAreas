#include "kamek/hooks.h"
#include "Wired2DAreaPath.h"

// - Author: Aba Gamer
// - Place a Wired2DAreaPath and link a PathID to make it work.
// - SW_A: Enables and disables the 2D mode when it changes
// - You can change the gravity while you're at this mode, but the gravity where this object is will be the "main".
//
//  - This requires a good use of cameras. Also, didn't you hear that salty water isn't good for machines? So you know now.
//  - Global namespace hmmmmmmmmmm


// Stole from PTUtils
void turnToDirectionUpFront(LiveActor *pActor, TVec3f rUp, TVec3f rFront)
{
    TMtx34f mtx;
    MR::makeMtxUpFront((TPos3f *)&mtx, rUp, rFront);
    ((TRot3f *)&mtx)->getEulerXYZ(pActor->mRotation);
    pActor->mRotation *= 57.295776f;
}

void turnToDirectionGravityFront(LiveActor *pActor, TVec3f rFront)
{
    turnToDirectionUpFront(pActor, -pActor->mGravity, rFront);
}

namespace Wired2DAreaPathUtils
{
    TVec3f *mRotation;
    TVec3f *mTranslation;
    bool activate = false;
}

Wired2DAreaPath::Wired2DAreaPath(const char *pName) : LiveActor(pName)
{
    Wired2DAreaPathUtils::mTranslation = &mTranslation;
    Wired2DAreaPathUtils::mRotation = &mRotation;
}

Wired2DAreaPath::~Wired2DAreaPath() {}

void Wired2DAreaPath::init(const JMapInfoIter &rIter)
{
    MR::initDefaultPos(this, rIter);

    initRailRider(rIter);

    MR::useStageSwitchReadA(this, rIter);

    MR::calcGravity(this);  // Just one time. 
    MR::invalidateClipping(this);

    MR::connectToSceneMapObjMovement(this);

    makeActorAppeared();
}

void Wired2DAreaPath::control()
{
    if (MR::isOnSwitchA(this))
    {
        Wired2DAreaPathUtils::activate = true;

        TVec3f railPos;
        TVec3f delta;   // This is the correct way to name a vector between two positions, right? RIGHT?
        TVec3f nearestPos;
        TVec3f nearestDirection;
        TVec3f sideVec;
        TVec3f playerPos = *MR::getPlayerPos();

        delta.normalize(calcNearestRailPosGravity() - playerPos);

        MR::calcNearestRailPosAndDirection(&nearestPos, &nearestDirection, this, playerPos);
        mTranslation = nearestPos;

        PSVECCrossProduct(nearestDirection, mGravity, sideVec);
        turnToDirectionGravityFront(this, sideVec);
        
        // Smooth correction in case Mario goes off the area. This happens on normal 2D areas but by default not here
        TVec3f dirTo2DPlane = delta - nearestDirection * delta.dot(nearestDirection);
        if (PSVECMag(dirTo2DPlane) > 0.01f)
            MR::getPlayerPos()->add(dirTo2DPlane * 0.75f);
    }
    else {
        Wired2DAreaPathUtils::activate = false;
    }
}

TVec3f Wired2DAreaPath::calcNearestRailPosGravity()
{
    TVec3f railPos;
    TVec3f playerPos = *MR::getPlayerPos();
    MR::calcNearestRailPos(&railPos, this, playerPos);

    TVec3f delta = railPos - playerPos;
    TVec3f gravityNorm;
    gravityNorm.normalize(mGravity);
    return playerPos + (delta - gravityNorm * delta.dot(gravityNorm));
}

AreaObj *checkAreaMode(const char *pName, const TVec3f &pPlayerPos)
{
    if (Wired2DAreaPathUtils::activate)
    {
        return (AreaObj *)1; // TODO: Find a better way to handle this. I just don't want it to be null!
    }
    else
    {
        AreaObj *pArea = MR::getAreaObj(pName, pPlayerPos);
        register Mario *pMario;
        __asm {mr pMario, r31}; // Faster than doing MR::getMarioHolder()->getMarioActor().mMario
        pMario->_6A0 = pArea; // Suspicious point, make this be missing is *probably* what removes the smooth centering
        return pArea;
    }
}

kmCall(0x803A4AB8, checkAreaMode);

void voidy() {}

kmCall(0x803A4AC0, voidy); // I can't do 60000000 here for some reason
kmCall(0x803A4B04, voidy);

void saveRotation(const AreaObj *pArea, TVec3f *pMarioAreaRot)
{
    if (Wired2DAreaPathUtils::activate)
    {
        pMarioAreaRot = Wired2DAreaPathUtils::mRotation;
    }
    else if (pArea != nullptr)
    {
        MR::calcCubeRotate(pArea, pMarioAreaRot);
    }
}

kmCall(0x803A4ADC, saveRotation);

void makeMtx(MtxPtr mtx, f32 v1, f32 v2, f32 v3, f32 v4, f32 v5, f32 v6)
{
    if (Wired2DAreaPathUtils::activate)
        MR::makeMtxTR(mtx, v1, v2, v3, Wired2DAreaPathUtils::mRotation->x, Wired2DAreaPathUtils::mRotation->y, Wired2DAreaPathUtils::mRotation->z);
    else
        MR::makeMtxTR(mtx, v1, v2, v3, v4, v5, v6);
}

kmCall(0x803A4AFC, makeMtx);

void calcCubeOrWiredAreaPathPos(const AreaObj *pArea, TVec3f *pMarioAreaPos)
{
    if (Wired2DAreaPathUtils::activate)
    {
        pMarioAreaPos = Wired2DAreaPathUtils::mTranslation;
    }
    else if (pArea != nullptr)
    {
        MR::calcCubePos(pArea, pMarioAreaPos);
    }
}

kmCall(0x803A4B38, calcCubeOrWiredAreaPathPos);