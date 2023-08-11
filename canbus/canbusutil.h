#ifndef CANBUSUTIL_H
#define CANBUSUTIL_H

#include <QObject>
#include <QtSerialBus>

QT_BEGIN_NAMESPACE

class QCanBusFrame;
class QCoreApplication;
class QTextStream;

QT_END_NAMESPACE

enum { BitrateSwitchFlag = 1, ErrorStateIndicatorFlag = 2 };

class CanBusUtil : public QObject
{
    Q_OBJECT

public:
    explicit CanBusUtil(QTextStream &output, QObject *parent = nullptr);
    
    void setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    bool getConfigurationParameter(QCanBusDevice::ConfigurationKey key);
    void setListeningOption(bool listeningOption);
    void setPluginName(QString pluginName);
    void setDeviceName(QString deviceName);
    QCanBusDevice* getCanDevice();
    void printPlugins();
    void printDevices(const QString &pluginName);
    bool connectCanDevice();
    void startListening();
    bool sendFrame(const QCanBusFrame &frame);

private:
    bool parseDataField(QCanBusFrame::FrameId &id, QString &payload);
    bool setFrameFromPayload(QString payload, QCanBusFrame *frame);

public slots:
    void receiveFrames();
    void handleError(QCanBusDevice::CanBusError /*error*/);

private:
    QTextStream &m_output;

    QCanBus *m_canBus = nullptr;
    std::unique_ptr<QCanBusDevice> m_canDevice;
    using ConfigurationParameter = QHash<QCanBusDevice::ConfigurationKey, QVariant>;
    ConfigurationParameter m_configurationParameter;
    
    bool m_listening = false;
    QString m_pluginName;
    QString m_deviceName;
    QString m_data;
};



#endif // CANBUSUTIL_H