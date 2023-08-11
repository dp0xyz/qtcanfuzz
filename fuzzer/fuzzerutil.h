#ifndef FUZZERUTIL_H
#define FUZZERUTIL_H

#include <QObject>
#include <QTextStream>
#include <QCanBusFrame>

#include "canbusutil.h"
#include "bamutator.h"

enum FuzzingMutationMode {
    MUTATION_MODE_NONE = 0,
    MUTATION_MODE_ALL = 1,
    MUTATION_MODE_FAST = 2,

    MUTATION_MODE_END
};

struct FuzzingOptions {

    uint iterations = -1L;
    uint maxPduLen = 0; //bytes
    uint lenControl = 100;
    uint unitTimeout = 1200; // --timeout If one unit runs more than this number of seconds, regard the unit as crash
    uint maxTotalTime = 0; //--totaltime The maximal total time in seconds to run the fuzzer
    uint slowUnits = 10; // --slowunit Report slow units if they run for more than this number of seconds.
    bool statsFlag = 0; // If 1, print statistics at exit.
    QString samplePrefix; // Name prefix of the output sample file
    QString inputCorpusPath;

    //mutate options
    FuzzingMutationMode mutateFlag = MUTATION_MODE_ALL; 
    quint64 seed = 0; //with the same seed value will produce the same number sequence
    uint mutateDepth = 5; //Apply this number of consecutive mutations to each input

    //id options
    uint frameId = 0;
    uint idMin = 0;
    uint idMax = 0x1FFFFFFF;
    uint idDiag = 0;
    uint interval = 0;
};

#define IS_MUTATION_OPEN(mflag) (mflag == MUTATION_MODE_NONE ? false : true)
#define MUTATOR_CROSSOVER_LEVEL (50)
#define MUTATOR_CYCLE_COUNTS ((uint)1000)


class FuzzerUtil : public QObject
{
    Q_OBJECT

public:
    explicit FuzzerUtil(QTextStream &output, QObject *parent = nullptr);
    void setFuzzingOptions(FuzzingOptions &fopt);
    bool startFuzzing(CanBusUtil &cutil);
    bool startReplaying(CanBusUtil &cutil);
    //bool convertSample2CanBusFrame(QString &sample, QCanBusFrame &QCanBusFrame);
    //bool runOneSample();

private:
    void setMutatorSeed();
    bool readOneSampleToCanFrame();
    bool sendOneCanFrame();
    bool sendAllCorpusCanFrame();
    void getInitialCorpus(ByteArrayList &corpus);
    void getCorpusOutputPath(QString &outputpath);

private:
    QTextStream &m_output;

    FuzzingOptions m_options;

    quint64 m_mutatorSeed; //Mutate Random number generator

};


#endif //FUZZERUTIL_H