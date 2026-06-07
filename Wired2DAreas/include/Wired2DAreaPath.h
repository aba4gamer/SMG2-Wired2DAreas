#pragma once

#include "syati.h"

class Wired2DAreaPath : public LiveActor {  // No, it's not an Area
public:
    Wired2DAreaPath(const char *pName);

    virtual ~Wired2DAreaPath();
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();

    TVec3f calcNearestRailPosGravity();
};