#ifndef MONITORUTIL_H
#define MONITORUTIL_H

#include <vector>

#include <QElapsedTimer>

#include "canbusutil.h"

enum MonitorModelType
{
    MONITOR_MODEL_KEEPALIVE = 0,
    MONITOR_MODEL_BUSSTATUS = 1,  // Display and exit
    MONITOR_MODEL_BUSERROR = 2,  // Only Listening Frames
    MONITOR_MODEL_DTCFAULT = 3,

    MONITOR_MODEL_END,
    MONITOR_MODEL_INVALID
};

struct MonitroDetectInfo
{
    qint64 dtcNum = 0;
};

typedef void (*MonitorStartFunc) (CanBusUtil &cutil, uint idDiag);
typedef bool (*MonitorCheckFunc) (QCanBusFrame &frame, MonitroDetectInfo &detectInfo);

struct MonitorDef
{
    MonitorModelType monitorType;
    bool monitorStatus;
    QString monitorDescription;
    MonitorStartFunc fpMonitorStart;
    MonitorCheckFunc fpMonitorCheck;
};

/* MonitorUtil must be initialized after candevice connection */
class MonitorUtil : public QObject
{
    Q_OBJECT

public:
    explicit MonitorUtil(QTextStream &output, QObject *parent = nullptr);

    void initMonitor(CanBusUtil &cutil, uint idDiag, uint interval);
    void startEveryTimeMonitoring(CanBusUtil &cutil);

private:
    void setMonitorDetectStatus(enum MonitorModelType type, bool status);
    void clearAllMonitorDetectStatus();
    void setMonitorDiagId(uint idDiag);
    void setMonitorInverval(uint interval);
    void setMonitorFramesReceived(CanBusUtil &cutil);

public slots:
    void checkEveryTimeMonitoring();

private:
    QTextStream &m_output;

    std::vector<MonitorDef> m_Monitors;

    uint m_idDiag = 0x7DF;
    uint m_interval = 50;
    QElapsedTimer m_et;

    MonitroDetectInfo m_detectInfo {};

};

#endif // MONITORUTIL_H

