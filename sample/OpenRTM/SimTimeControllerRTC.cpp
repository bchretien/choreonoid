/**
   @author Benjamin Chrétien
*/

#include "SimTimeControllerRTC.h"
#include <cnoid/Link>
#include <cnoid/Archive>
#include <cnoid/MessageView>
#include <cnoid/ExecutablePath>
#include <cnoid/FileUtil>
#include <cnoid/ItemTreeView>
#include <cnoid/SimulatorItem>
#include <cnoid/Sleep>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

using namespace std;
using namespace boost;
using namespace cnoid;

namespace {

const char* spec[] =
{
    "implementation_id", "SimTimeControllerRTC",
    "type_name",         "SimTimeControllerRTC",
    "description",       "Simulation Time Controller",
    "version",           "0.1",
    "vendor",            "CNRS-LIRMM",
    "category",          "Choreonoid",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

}


SimTimeControllerRTC::SimTimeControllerRTC(RTC::Manager* manager)
  : RTC::DataFlowComponentBase(manager),
    m_pauseIn("pause", m_pause),
    m_isPausedOut("isPaused", m_isPaused),
    m_angleIn("q", m_angle),
    m_torqueIn("u_in", m_torque_in),
    m_torqueOut("u_out", m_torque_out),
    mv_ (MessageView::instance())
{
}


SimTimeControllerRTC::~SimTimeControllerRTC()
{
}


RTC::ReturnCode_t SimTimeControllerRTC::onInitialize()
{
    // Set InPort buffers
    addInPort("pause", m_pauseIn);

    // Set OutPort buffer
    addOutPort("isPaused", m_isPausedOut);

    // Dummy ports for BodyRC
    addInPort("q", m_angleIn);
    addInPort("u_in", m_torqueIn);
    addOutPort("u_out", m_torqueOut);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t SimTimeControllerRTC::onActivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t SimTimeControllerRTC::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t SimTimeControllerRTC::onExecute(RTC::UniqueId ec_id)
{
    if(m_pauseIn.isNew()){
        m_pauseIn.read();
        if(m_pause.data)
        {
            pauseSimulation();
            m_isPaused.data = true;
            m_isPaused.tm = m_pause.tm;
            m_isPausedOut.write();
            while(m_pause.data)
            {
                if(m_pauseIn.isNew())
                  m_pauseIn.read();
                msleep(50);
            }
            startSimulation();
            m_isPaused.data = false;
            m_isPaused.tm = m_pause.tm;
            m_isPausedOut.write();
        }
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t SimTimeControllerRTC::onStateUpdate(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


void SimTimeControllerRTC::pauseSimulation()
{
    ItemList<SimulatorItem> simulators;
    if (ItemTreeView::mainInstance())
        simulators = ItemTreeView::mainInstance()->selectedItems<SimulatorItem>();

    for(int i=0; i < simulators.size(); ++i){
        SimulatorItem* simulator = simulators.get(i);
        if(simulator->isRunning())
        {
            mv_->putln("Paused simulation");
            simulator->pauseSimulation();
        }
    }
}


void SimTimeControllerRTC::startSimulation()
{
    ItemList<SimulatorItem> simulators;
    if (ItemTreeView::mainInstance())
        simulators = ItemTreeView::mainInstance()->selectedItems<SimulatorItem>();

    for(int i=0; i < simulators.size(); ++i){
        SimulatorItem* simulator = simulators.get(i);
        if(simulator->isRunning())
        {
            mv_->putln("Start simulation");
            simulator->restartSimulation();
        }
    }
}


extern "C"
{
    DLL_EXPORT void SimTimeControllerRTCInit(RTC::Manager* manager)
    {
        coil::Properties profile(spec);
        manager->registerFactory(profile,
                                 RTC::Create<SimTimeControllerRTC>,
                                 RTC::Delete<SimTimeControllerRTC>);
    }
};
