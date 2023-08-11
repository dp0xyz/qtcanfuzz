#include "canmonitor.h"

QByteArray dataUds10Req = QByteArray::fromHex("0210015555555555");
QByteArray dataUds10Resp = QByteArray::fromHex("065001"); // 06 50 01 00 32 01 F4 AA

QByteArray dataUds19Req = QByteArray::fromHex("031901FF55555555");
QByteArray dataUds19Resp = QByteArray::fromHex("037F1978AAAAAAAA"); // 03 7F 19 78 AA AA AA AA
QByteArray dataUds19RespFault = QByteArray::fromHex("065901"); // 06 59 01 XX 01 00 0C AA // 000C为故障数量

void Monitor_startDetectKeepAlive(CanBusUtil &cutil, uint idDiag) 
{
    QCanBusFrame frameUds10Req(idDiag, dataUds10Req);

    uint isCanFd = cutil.getConfigurationParameter(QCanBusDevice::CanFdKey);
    frameUds10Req.setFlexibleDataRateFormat(isCanFd);

    cutil.sendFrame(frameUds10Req);

    return;
}

bool Monitor_checkDetectKeepAlive(QCanBusFrame &frame, MonitroDetectInfo &detectInfo)
{
    qint64 frameid = frame.frameId();
    if (!(frameid & 0x700)) {
        return false;
    }

    if ((frame.payload().size() == 8) && (frame.payload().startsWith(dataUds10Resp))) {
        return true;
    }

    return false;
}

void Monitor_startDetectDtcFault(CanBusUtil &cutil, uint idDiag) 
{
    QCanBusFrame frameUds19Req(idDiag, dataUds19Req);

    uint isCanFd = cutil.getConfigurationParameter(QCanBusDevice::CanFdKey);
    frameUds19Req.setFlexibleDataRateFormat(isCanFd);

    cutil.sendFrame(frameUds19Req);

    return;
}

bool Monitor_checkDetectDtcFault(QCanBusFrame &frame, MonitroDetectInfo &detectInfo) 
{
    qint64 frameid = frame.frameId();
    if (!(frameid & 0x700)) {
        return false;
    }

    if ((frame.payload().size() == 8) && (frame.payload().startsWith(dataUds19RespFault))) {
        qint64 dtcCountHighByte = frame.payload().data()[5];
        qint64 dtcCountLowByte = frame.payload().data()[6];
        qint64 dtcNum = (dtcCountHighByte << 8) + dtcCountLowByte;

        if (dtcNum == detectInfo.dtcNum) {
            return true;
        } else {
            detectInfo.dtcNum = dtcNum;
        }
    }

    return false;
}