/**
   @author Benjamin Chrétien
*/

#ifndef CNOID_SIM_TIME_CONTROLLER_PLUGIN_SIM_TIME_CONTROLLER_ITEM_H_INCLUDED
#define CNOID_SIM_TIME_CONTROLLER_PLUGIN_SIM_TIME_CONTROLLER_ITEM_H_INCLUDED

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>

#include <cnoid/MessageView>

class SimTimeControllerRTC : public RTC::DataFlowComponentBase
{
public:
    SimTimeControllerRTC(RTC::Manager* manager);
    virtual ~SimTimeControllerRTC();

    // RTC methods
    virtual RTC::ReturnCode_t onInitialize();
    virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onStateUpdate(RTC::UniqueId ec_id);

protected:

    void pauseSimulation ();
    void startSimulation ();

    // DataInPort declaration
    RTC::TimedBoolean m_pause;
    RTC::InPort<RTC::TimedBoolean> m_pauseIn;

    RTC::TimedDoubleSeq m_angle;
    RTC::InPort<RTC::TimedDoubleSeq> m_angleIn;
    RTC::TimedDoubleSeq m_torque_in;
    RTC::InPort<RTC::TimedDoubleSeq> m_torqueIn;

    // DataOutPort declaration
    RTC::TimedBoolean m_isPaused;
    RTC::OutPort<RTC::TimedBoolean> m_isPausedOut;

    RTC::TimedDoubleSeq m_torque_out;
    RTC::OutPort<RTC::TimedDoubleSeq> m_torqueOut;
  
private:
    cnoid::MessageView* mv_;
};

extern "C"
{
    DLL_EXPORT void SimTimeControllerRTCInit(RTC::Manager* manager);
};

#endif
