/**
   \file
   \author Shizuko Hattori
*/

#ifndef CNOID_BODYPLUGIN_COLLISION_SEQ_ENGINE_H
#define CNOID_BODYPLUGIN_COLLISION_SEQ_ENGINE_H

#include <cnoid/TimeSyncItemEngine>
#include <cnoid/WorldItem>
#include <cnoid/ExtensionManager>
#include "exportdecl.h"

namespace cnoid {

class CollisionSeqEngineImpl;
class CollisionSeqItem;

class CNOID_EXPORT CollisionSeqEngine : public TimeSyncItemEngine
{
public:
    static void initialize(ExtensionManager* ext);

    CollisionSeqEngine(WorldItem* worldItem, CollisionSeqItem* collisionSeqItem);
    ~CollisionSeqEngine();

    virtual bool onTimeChanged(double time);

private:
    CollisionSeqEngineImpl* impl;
};

typedef ref_ptr<CollisionSeqEngine> CollisionSeqEnginePtr;

}
#endif
