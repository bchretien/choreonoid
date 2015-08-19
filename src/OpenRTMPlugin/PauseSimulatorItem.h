/*!
  @file
  @author Benjamin Chrétien
*/

#ifndef CNOID_BODYPLUGIN_PAUSE_SIMULATOR_ITEM_H
#define CNOID_BODYPLUGIN_PAUSE_SIMULATOR_ITEM_H

#include <cnoid/SubSimulatorItem>
#include "exportdecl.h"

namespace cnoid {

class PauseSimulatorItemImpl;

class CNOID_EXPORT PauseSimulatorItem : public SubSimulatorItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    PauseSimulatorItem();
    PauseSimulatorItem(const PauseSimulatorItem& org);
    ~PauseSimulatorItem();

    virtual bool isEnabled();
    virtual bool initializeSimulation(SimulatorItem* simulatorItem);
    virtual void finalizeSimulation();

protected:
    virtual ItemPtr doDuplicate() const;
    virtual void doPutProperties(PutPropertyFunction& putProperty);
    virtual bool store(Archive& archive);
    virtual bool restore(const Archive& archive);

private:
    PauseSimulatorItemImpl* impl;
};

}

#endif
