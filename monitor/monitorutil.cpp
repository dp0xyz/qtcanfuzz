
#include "monitorutil.h"
#include "canmonitor.h"

MonitorDef  monitorModelDefinitions[MONITOR_MODEL_END] = {
    {MONITOR_MODEL_KEEPALIVE, false, "KEEP ALIVE DETECT", Monitor_startDetectKeepAlive, Monitor_checkDetectKeepAlive},
    {MONITOR_MODEL_BUSSTATUS, false, "BUS STATUS DETECT", nullptr, nullptr},
    {MONITOR_MODEL_BUSERROR, false, "BUS ERROR DETECT", nullptr, nullptr},
    {MONITOR_MODEL_DTCFAULT, false, "DTC FAULT DETECT", Monitor_startDetectDtcFault, Monitor_checkDetectDtcFault},
};

MonitorUtil::MonitorUtil(QTextStream &output, QObject *parent) :
    QObject(parent),
    m_output(output)
{
    m_Monitors.assign(std::begin(monitorModelDefinitions), std::end(monitorModelDefinitions));
}

void MonitorUtil::setMonitorDetectStatus (enum MonitorModelType type, bool status) {
    m_Monitors[type].monitorStatus = status;
    return;
}

void MonitorUtil::clearAllMonitorDetectStatus() {
    for (int i = 0; i < MONITOR_MODEL_END; i++) {
        m_Monitors[i].monitorStatus = false;
    }
    return;
}

void MonitorUtil::setMonitorDiagId(uint idDiag) {
    if(idDiag != 0) {
        m_idDiag = idDiag;
    }
    return;
}

void MonitorUtil::setMonitorInverval(uint interval) {
    if(interval != 0) {
        m_interval = interval;
    }
    return;
}

void MonitorUtil::setMonitorFramesReceived (CanBusUtil &cutil) {
    QCanBusDevice* canDevice = cutil.getCanDevice();
    connect(canDevice, &QCanBusDevice::framesReceived, this, &MonitorUtil::checkEveryTimeMonitoring);
    return;
}

/* MonitorUtil must be initialized after candevice connection */
void MonitorUtil::initMonitor (CanBusUtil &cutil, uint idDiag, uint interval)
{
    setMonitorDiagId(idDiag);
    setMonitorInverval(interval);

    setMonitorFramesReceived(cutil);

    // 初始化monitor时执行一次初始状态检测
    startEveryTimeMonitoring(cutil);

    return;
}

void MonitorUtil::startEveryTimeMonitoring (CanBusUtil &cutil) 
{
    QCanBusDevice* canDevice = cutil.getCanDevice();

    canDevice->clear();
    clearAllMonitorDetectStatus();

    for (int i = 0; i < MONITOR_MODEL_END; i++) {
        if(m_Monitors[i].fpMonitorStart) {
            m_Monitors[i].fpMonitorStart(cutil, m_idDiag);
        }
    }

    m_et.restart();
    while (m_et.elapsed() <= m_interval) {
        QCoreApplication::processEvents();
    }
    
    for (int i = 0; i < MONITOR_MODEL_END; i++) {
            QString strStatus = (m_Monitors[i].monitorStatus) ? "TRUE" : "FALSE";
            m_output << m_Monitors[i].monitorDescription << QStringLiteral(" : %1").arg(strStatus) << Qt::endl;
    }

    return;
}

void MonitorUtil::checkEveryTimeMonitoring() 
{
    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning("CanBusUtil ReceiveFrames: Unknown sender.");
        return;
    }
    
    while (canDevice->framesAvailable()) {
        QCanBusFrame frame = canDevice->readFrame();

        for (int i = 0; i < MONITOR_MODEL_END; i++) {
            if(m_Monitors[i].fpMonitorCheck) {
                bool status = m_Monitors[i].fpMonitorCheck(frame, m_detectInfo);
                // 检测通过，则置相应状态为 TRUE
                if (status) {
                    setMonitorDetectStatus((MonitorModelType)i, status);
                }
            }
        }
        
        // 读取并检测所有报文，若时间已经到设定的间隔等待时间，则结束本次检测
        if (m_et.elapsed() > m_interval) {
            break;
        }
    }
}