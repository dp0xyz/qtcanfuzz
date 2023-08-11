#include "canbusutil.h"

CanBusUtil::CanBusUtil(QTextStream &output, QObject *parent) : 
    QObject(parent),
    m_output(output),
    m_canBus(QCanBus::instance()) {}

void CanBusUtil::setConfigurationParameter(QCanBusDevice::ConfigurationKey key,
                                           const QVariant &value) {
    m_configurationParameter[key] = value;
}
bool CanBusUtil::getConfigurationParameter(QCanBusDevice::ConfigurationKey key) {
    return m_configurationParameter[key].toBool();
}

void CanBusUtil::setListeningOption(bool listeningOption) {
    m_listening = listeningOption;
}

void CanBusUtil::setPluginName(QString pluginName) {
    m_pluginName = pluginName;
}

void CanBusUtil::setDeviceName(QString deviceName) {
    m_deviceName = deviceName;
}

QCanBusDevice* CanBusUtil::getCanDevice() {
    return m_canDevice.get();
}

void CanBusUtil::printPlugins()
{
    if (!m_canBus) {
        m_output << QStringLiteral("Error: Cannot create QCanBus.") << Qt::endl;
        return;
    }
    
    m_output << QStringLiteral("----------------------------------------------------------") << Qt::endl;
    m_output << QStringLiteral("All supported can hardware plugin types list:") << Qt::endl;
    const QStringList plugins = m_canBus->plugins();
    for (const QString &plugin : plugins)
        m_output << plugin << Qt::endl;

    m_output << Qt::endl;
    return;
}

void CanBusUtil::printDevices(const QString &pluginName)
{
    if (!m_canBus) {
        m_output << QStringLiteral("Error: Cannot create QCanBus.") << Qt::endl;
        return;
    }

    QString errorMessage;
    const QList<QCanBusDeviceInfo> devices = m_canBus->availableDevices(pluginName, &errorMessage);
    if (!errorMessage.isEmpty()) {
        m_output << QStringLiteral("Error gathering available devices: '%1'").arg(errorMessage) << Qt::endl;
        return;
    }

    for (const QCanBusDeviceInfo &info : devices) {
        m_output << info.name() << Qt::endl;
    }

    return;
}

bool CanBusUtil::connectCanDevice()
{
    if (!m_canBus) {
        m_output << QStringLiteral("Error: Cannot create QCanBus.") << Qt::endl;
        return false;
    }

    if (!m_canBus->plugins().contains(m_pluginName)) {
        m_output << QStringLiteral("Cannot find CAN bus plugin '%1'.").arg(m_pluginName) << Qt::endl;
        return false;
    }

    m_canDevice.reset(m_canBus->createDevice(m_pluginName, m_deviceName));
    if (!m_canDevice) {
        m_output << QStringLiteral("Cannot create CAN bus device: '%1'").arg(m_deviceName) << Qt::endl;
        return false;
    }

    const auto constEnd = m_configurationParameter.constEnd();
    for (auto i = m_configurationParameter.constBegin(); i != constEnd; ++i) {
        m_canDevice->setConfigurationParameter(i.key(), i.value());
    }

    connect(m_canDevice.get(), &QCanBusDevice::errorOccurred, this, &CanBusUtil::handleError);
    if (!m_canDevice->connectDevice()) {
        m_output << QStringLiteral("Cannot connect CAN bus device: '%1'").arg(m_deviceName) << Qt::endl;
        return false;
    }

    return true;
}

void CanBusUtil::startListening()
{
    connectCanDevice();
    if(m_listening) {
        connect(m_canDevice.get(), &QCanBusDevice::framesReceived, this, &CanBusUtil::receiveFrames);
    }
    return;
}

void CanBusUtil::receiveFrames() 
{

    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning("CanBusUtil ReceiveFrames: Unknown sender.");
        return;
    }

    while (canDevice->framesAvailable()) {
        const QCanBusFrame frame = canDevice->readFrame();

        QString view = "";

        qint64 milliSecs = frame.timeStamp().seconds() * 1000 + frame.timeStamp().microSeconds() / 1000;
        QString timer = QDateTime(QDate(1970,1,1), QTime(0,0,Qt::LocalTime,0))
                            .addMSecs(milliSecs)
                            .currentDateTime()
                            .toString("yyyy-MM-dd HH:mm:ss.zzz");

        view += timer;

        QLatin1String flags(" - - -  ");
        // With CAN FD, the payload can be transmitted at a higher data bitrate, if QCanBusFrame::hasBitrateSwitch() is set.
        if (frame.hasBitrateSwitch())
            flags[0] = QLatin1Char('B');
        // CAN FD with Error State Indicator set. This flag is set by the transmitter's CAN FD hardware to indicate the transmitter's error state.
        if (frame.hasErrorStateIndicator())
            flags[2] = QLatin1Char('E');
        // A frame that is received as echo when the frame with the same content was successfully sent to the CAN bus. 
        // This flag is set for frames sent by the application itself as well as for frames sent by other applications running on the same system.
        if (frame.hasLocalEcho())
            flags[4] = QLatin1Char('L');

        view += flags;
        
        if (frame.frameType() == QCanBusFrame::ErrorFrame)
            // Interprets frame as error frame and returns a human readable description of the error.
            view += canDevice->interpretErrorFrame(frame);
        else
            view += frame.toString();

        m_output << view << Qt::endl;
    }
}

void CanBusUtil::handleError(QCanBusDevice::CanBusError)
{
    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning("ReadTask::handleError: Unknown sender.");
        return;
    }

    m_output << QStringLiteral("Read error: '%1'").arg(canDevice->errorString()) << Qt::endl;
}

bool CanBusUtil::sendFrame(const QCanBusFrame &frame)
{
    /*quint32 id;
    QString payload;
    QCanBusFrame frame;

    if (!parseDataField(id, payload))
        return false;

    if (!setFrameFromPayload(payload, &frame))
        return false;

    if (id > 0x1FFFFFFF) { // 29 bits
        m_output << QStringLiteral("Cannot send invalid frame ID: '%1'").arg(id, 0, 16) << Qt::endl;
        return false;
    }

    frame.setFrameId(id);

    if (frame.hasFlexibleDataRateFormat())
        m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);*/

    return m_canDevice->writeFrame(frame);
}

bool CanBusUtil::parseDataField(QCanBusFrame::FrameId &id, QString &payload)
{
    int hashMarkPos = m_data.indexOf('#');
    if (hashMarkPos < 0) {
        m_output << QStringLiteral("Data field invalid: No separation character '#' found !") << Qt::endl;
        return false;
    }

    bool ok = false;
    id = QStringView{m_data}.left(hashMarkPos).toUInt(&ok, 16);
    if(!ok) {
        m_output << QStringLiteral("Frame id is invalid : '%1'").arg(QStringView{m_data}.left(hashMarkPos)) << Qt::endl;
        return false;
    }
    payload = m_data.right(m_data.size() - hashMarkPos - 1);

    return true;
}

bool CanBusUtil::setFrameFromPayload(QString payload, QCanBusFrame *frame)
{
    // Rremote Frame
    if (!payload.isEmpty() && payload.at(0).toUpper() == 'R') {
        frame->setFrameType(QCanBusFrame::RemoteRequestFrame);

        if (payload.size() == 1) // payload = "R"
            return true;

        bool ok = false;
        quint32 rtrFrameLength = QStringView{payload}.mid(1).toInt(&ok, 16); // RTR请求数据帧长度也用16进制标识，与ID、数据帧保持一致
        if(!ok) {
            m_output << QStringLiteral("RTR FrameLength is invalid : '%1'").arg(QStringView{payload}.mid(1)) << Qt::endl;
            return false;
        }

        if (rtrFrameLength >= 0 && rtrFrameLength <= 8) { // payload = "R8"
            frame->setPayload(QByteArray(rtrFrameLength, 0));
            return true;
        }

        // m_output << QStringLiteral("Error: RTR frame length must be between 0 and 8 (including).") << Qt::endl;
        return false;
    }

    // FD Frame
    if (!payload.isEmpty() && payload.at(0) == '#') {
        frame->setFlexibleDataRateFormat(true);
        payload.remove(0, 1);
    }

    // Data Frame
    const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f]*$"));
    if (!re.match(payload).hasMatch()) {
        m_output << QStringLiteral("Data field invalid: Only hex numbers allowed.") << Qt::endl;
        return false;
    }

    if (payload.size() % 2 != 0) {
        if (frame->hasFlexibleDataRateFormat()) {
            enum { BitrateSwitchFlag = 1, ErrorStateIndicatorFlag = 2 };
            const int flags = QStringView{payload}.left(1).toInt(nullptr, 16);
            frame->setBitrateSwitch(flags & BitrateSwitchFlag);
            frame->setErrorStateIndicator(flags & ErrorStateIndicatorFlag);
            payload.remove(0, 1);
        } else {
            m_output << QStringLiteral("Data field invalid: Size is not multiple of two.") << Qt::endl;
            return false;
        }
    }

    QByteArray bytes = QByteArray::fromHex(payload.toLatin1());

    const int maxSize = frame->hasFlexibleDataRateFormat() ? 64 : 8;
    if (bytes.size() > maxSize) {
        m_output << QStringLiteral("Data field invalid: Size is longer than %1 bytes.").arg(maxSize) << Qt::endl;
        return false;
    }

    frame->setPayload(bytes);

    return true;
}