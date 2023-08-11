#include "cansample.h"
#include "fileparser.h"


CanFrameSample::CanFrameSample(QObject *parent) :
    QObject(parent)
{
    std::mt19937 randSampleNo(std::random_device{}());
    m_sampleNos = randSampleNo();
    return;
}

QCanBusFrame* CanFrameSample::getCanSampleFrame() {
    return &m_frame;
}

uint CanFrameSample::getCanSampleNos() {
    return m_sampleNos;
}

void CanFrameSample::printSampleToFile(QString &filename) {
    QString formatdata;
//    SampleToFormat(formatdata);
    formatdata = m_frame.payload().toHex(' ').toUpper();
    SaveDataToFile(formatdata, filename);
}

void CanFrameSample::printSampleToOutput(QTextStream &output) {
    QString formatdata;
    SampleToFormat(formatdata);
    output << formatdata << Qt::endl;
    return;
}

void CanFrameSample::SampleToFormat(QString &formatdata) {

    qint64 milliSecs = m_frame.timeStamp().seconds() * 1000 + m_frame.timeStamp().microSeconds() / 1000;
    QString timer = QDateTime(QDate(1970,1,1), QTime(0,0,Qt::LocalTime,0))
                        .addMSecs(milliSecs)
                        .currentDateTime()
                        .toString("yyyy-MM-dd HH:mm:ss.zzz");

    QString type = m_frame.frameId() > 0x7FF ? "EXT" : "STD";

    formatdata = QStringLiteral("%1  |  %2  |  Tx  |  1  |  Can  |  %3  |  ")
                .arg(m_sampleNos, 8, 16, QLatin1Char('0'))
                .arg(timer)
                .arg(type);
    formatdata += QStringLiteral("%4  |  DATA  |  [%5]  |  %6")
                .arg(m_frame.frameId(), 8, 16, QLatin1Char(' ')).toUpper()
                .arg(m_frame.payload().size())
                .arg(m_frame.payload().toHex(' ').toUpper());

    return;
}
