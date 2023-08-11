#ifndef CANSAMPLE_HEADER
#define CANSAMPLE_HEADER

#include <QObject>
#include <QTextStream>
#include <QCanBusFrame>

#include "common.h"

class CanFrameSample : public QObject
{
    Q_OBJECT

public :
    explicit CanFrameSample(QObject *parent = nullptr);
    QCanBusFrame *getCanSampleFrame();
    uint getCanSampleNos();
    void printSampleToFile(QString &filename);
    void printSampleToOutput(QTextStream &output);

private :
    void SampleToFormat(QString &formatdata);

private :
    uint m_sampleNos = 0;
    QCanBusFrame m_frame;

};

#endif // CANSAMPLE_HEADER