#ifndef CANMONITOR_H
#define CANMONITOR_H

#include "canbusutil.h"
#include "monitorutil.h"

void Monitor_startDetectKeepAlive(CanBusUtil &cutil, uint idDiag);
bool Monitor_checkDetectKeepAlive(QCanBusFrame &frame, MonitroDetectInfo &detectInfo);
void Monitor_startDetectDtcFault(CanBusUtil &cutil, uint idDiag);
bool Monitor_checkDetectDtcFault(QCanBusFrame &frame, MonitroDetectInfo &detectInfo);

#endif // CANMONITOR_H