/*!
  @file
  @author Benjamin Chrétien
*/

#include "PauseSimulatorItem.h"
#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>
#include <cnoid/SimulatorItem>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/Archive>
#include <cnoid/ValueTreeUtil>
#include <cnoid/Body>
#include <cnoid/Sleep>
#include <cnoid/OpenRTMUtil>
#include <boost/bind.hpp>
#include "gettext.h"

#include <iostream>

using namespace std;
using namespace cnoid;
using boost::format;

namespace cnoid {

class PauseSimulatorRTC : public RTC::DataFlowComponentBase
{
public:
    PauseSimulatorRTC(RTC::Manager* manager);
    ~PauseSimulatorRTC();

    // DataFlowComponentBase methods
    virtual RTC::ReturnCode_t onInitialize();

    bool pause();

protected:
    // DataInPort declaration
    RTC::TimedBoolean m_pause;
    RTC::InPort<RTC::TimedBoolean> m_pauseIn;

    // DataOutPort declaration
    RTC::TimedBoolean m_isPaused;
    RTC::OutPort<RTC::TimedBoolean> m_isPausedOut;
};

class PauseSimulatorItemImpl
{
public:
    PauseSimulatorItem* self;
    ostream& os;
    PauseSimulatorRTC* rtc;
    SimulatorItem* simulatorItem;
    double worldTimeStep;
    double currentTime;

    bool isEnabled;
    double sleepTime;
    std::string instanceName;

    PauseSimulatorItemImpl(PauseSimulatorItem* self);
    PauseSimulatorItemImpl(PauseSimulatorItem* self, const PauseSimulatorItemImpl& org);
    ~PauseSimulatorItemImpl();

    // SimulatorItem methods
    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPreDynamics();
    void onPostDynamics();
    void finalizeSimulation();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);

    bool createRTC();
    bool deleteRTC();
};

}


PauseSimulatorRTC::PauseSimulatorRTC(RTC::Manager* manager)
    : RTC::DataFlowComponentBase(manager),
      m_pauseIn("pause", m_pause),
      m_isPausedOut("isPaused", m_isPaused)
{
  m_pause.data = false;
}


PauseSimulatorRTC::~PauseSimulatorRTC()
{}


RTC::ReturnCode_t PauseSimulatorRTC::onInitialize()
{
    // Set InPort buffers
    addInPort("pause", m_pauseIn);

    // Set OutPort buffer
    addOutPort("isPaused", m_isPausedOut);

    return RTC::RTC_OK;
}


bool PauseSimulatorRTC::pause()
{
    if(m_pauseIn.isNew()){
      m_pauseIn.read();
    }
    return m_pause.data;
}


void PauseSimulatorItem::initializeClass(ExtensionManager* ext)
{
    const char* spec[] =
    {
        "implementation_id", "PauseSimulatorRTC",
        "type_name",         "PauseSimulatorRTC",
        "description",       "RTC used to pause the simulation",
        "version",           "0.1",
        "vendor",            "AIST",
        "category",          "Choreonoid",
        "activity_type",     "DataFlowComponent",
        "max_instance",      "10",
        "language",          "C++",
        "lang_type",         "compile",
        ""
    };

    RTC::Properties profile(spec);
    RTC::Manager::instance().registerFactory(
        profile, RTC::Create<PauseSimulatorRTC>, RTC::Delete<PauseSimulatorRTC>);

    ext->itemManager().registerClass<PauseSimulatorItem>(N_("PauseSimulatorItem"))
                      .addCreationPanel<PauseSimulatorItem>();
}


PauseSimulatorItem::PauseSimulatorItem()
{
    impl = new PauseSimulatorItemImpl(this);
}


PauseSimulatorItemImpl::PauseSimulatorItemImpl(PauseSimulatorItem* self)
    : self(self),
      os(MessageView::instance()->cout())
{
    simulatorItem = 0;
    rtc = 0;
    isEnabled = true;
    sleepTime = 0.;
    instanceName = "PauseSimulatorRTC";
}


PauseSimulatorItem::PauseSimulatorItem(const PauseSimulatorItem& org)
    : SubSimulatorItem(org)
{
    impl = new PauseSimulatorItemImpl(this, *org.impl);
}


PauseSimulatorItemImpl::PauseSimulatorItemImpl(PauseSimulatorItem* self, const PauseSimulatorItemImpl& org)
    : self(self),
      os(MessageView::instance()->cout())
{
    simulatorItem = 0;
    rtc = 0;
    isEnabled = org.isEnabled;
    sleepTime = org.sleepTime;
    instanceName = org.instanceName;
}


bool PauseSimulatorItem::isEnabled()
{
    return impl->isEnabled;
}


ItemPtr PauseSimulatorItem::doDuplicate() const
{
    return new PauseSimulatorItem(*this);
}


PauseSimulatorItem::~PauseSimulatorItem()
{
    delete impl;
}


PauseSimulatorItemImpl::~PauseSimulatorItemImpl()
{
    deleteRTC();
}


bool PauseSimulatorItemImpl::createRTC()
{
    if(rtc){
        deleteRTC();
    }

    if(rtc || instanceName.empty()){
        return false;
    }

    // Create a component
    format param("PauseSimulatorRTC?instance_name=%1%&logger.enable=NO&"
                 "exec_cxt.periodic_type=PeriodicExecutionContext&"
                 "exec_cxt.periodic.rate=10");

    rtc = dynamic_cast<PauseSimulatorRTC*>
      (createManagedRTC(str(param % instanceName).c_str()));

    if(rtc){
        os << (format(_("RTC \"%1%\" of \"%2%\" has been created."))
               % instanceName % self->name()) << endl;
    } else {
        os << (format(_("RTC \"%1%\" of \"%2%\" cannot be created."))
               % instanceName % self->name()) << endl;
    }

    return (rtc != 0);
}


bool PauseSimulatorItemImpl::deleteRTC()
{
    if(rtc){
        if(cnoid::deleteRTC(rtc, true)){
            os << (format(_("RTC \"%1%\" of \"%2%\" has been deleted."))
                   % instanceName % self->name()) << endl;
            rtc = 0;
        } else {
            os << (format(_("RTC \"%1%\" of \"%2%\" cannot be deleted."))
                   % instanceName % self->name()) << endl;
        }
    }

    return (rtc == 0);
}


bool PauseSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool PauseSimulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    worldTimeStep = simulatorItem->worldTimeStep();
    currentTime = 0;

    simulatorItem->addPreDynamicsFunction
      (boost::bind(&PauseSimulatorItemImpl::onPreDynamics, this));
    simulatorItem->addPostDynamicsFunction
      (boost::bind(&PauseSimulatorItemImpl::onPostDynamics, this));

    bool status = createRTC();

    return true;
}


void PauseSimulatorItemImpl::onPreDynamics()
{
    currentTime = simulatorItem->currentTime();
}


void PauseSimulatorItemImpl::onPostDynamics()
{
    while(rtc->pause())
    {
        msleep(sleepTime);
    }
}


void PauseSimulatorItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void PauseSimulatorItemImpl::finalizeSimulation()
{
}


void PauseSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void PauseSimulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Enabled"), isEnabled, changeProperty(isEnabled));
    putProperty.min(0.0)(_("Sleep time (ms)"), sleepTime, changeProperty(sleepTime));
    putProperty(_("RTC instance name"), instanceName, changeProperty(instanceName));
}


bool PauseSimulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool PauseSimulatorItemImpl::store(Archive& archive)
{
    archive.write("enabled", isEnabled);
    archive.write("sleepTime", sleepTime);
    archive.write("instanceName", instanceName);

    return true;
}


bool PauseSimulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool PauseSimulatorItemImpl::restore(const Archive& archive)
{
    archive.read("enabled", isEnabled);
    archive.read("sleepTime", sleepTime);
    archive.read("instanceName", instanceName);

    return true;
}

