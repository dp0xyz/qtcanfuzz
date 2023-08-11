#include "fuzzerutil.h"
#include "fileparser.h"
#include "cansample.h"
#include "monitorutil.h"

#include <QDebug>

ByteArray defaultSample(8,0);

FuzzerUtil::FuzzerUtil(QTextStream &output, QObject *parent) :
    QObject(parent),
    m_output(output)
{
}

void FuzzerUtil::setFuzzingOptions(FuzzingOptions &fopt) {
    m_options = fopt;
    return;
}
void FuzzerUtil::setMutatorSeed() {

    if(!IS_MUTATION_OPEN(m_options.mutateFlag))
        return;

    if(m_options.seed == 0) {
        m_mutatorSeed = std::chrono::system_clock::now().time_since_epoch().count();
        //m_mutatorSeed = std::random_device{}();
    } else {
        m_mutatorSeed = m_options.seed;
    }
    return;
}

bool FuzzerUtil::startFuzzing(CanBusUtil &cutil) {

    cutil.connectCanDevice();

    // if specify a single sample to execute, use replay mode
    QFileInfo fi(m_options.inputCorpusPath);
    if (fi.isFile()) {
        return startReplaying(cutil);
    }

    ByteArrayList corpusData;
    getInitialCorpus(corpusData); // TODO 需要根据 isCanFd 来筛选语料库中的Can/CanFd报文

    // create mutator
    uint isCanFd = cutil.getConfigurationParameter(QCanBusDevice::CanFdKey);
    setMutatorSeed();
    ByteArrayMutator mutator(m_mutatorSeed);
    if (m_options.maxPduLen == 0) {
        mutator.set_max_len(isCanFd ? 64 : 8);
    } else {
        mutator.set_max_len(m_options.maxPduLen);
    }

    // create monitor
    MonitorUtil mutil(m_output);
    mutil.initMonitor(cutil, m_options.idDiag, m_options.interval);

    // config canid mutation strategy
    uint frameId = m_options.frameId;
    bool isIdMutate = (frameId == 0);
    uint idMin = m_options.idMin;
    uint idMax = m_options.idMax;
    Rng randCanId(std::random_device{}());
    RngDistrib distribCanId(idMin, idMax);

    QString outputpath;
    getCorpusOutputPath(outputpath);
    uint iterations = m_options.iterations;
    uint round = 0;
    ByteArrayList mutants;
    while(iterations > 0) 
    {
        uint cyclecnt = qMin(iterations, MUTATOR_CYCLE_COUNTS);
        mutants.resize(cyclecnt);
        for (auto &mutant : mutants)
        {
            mutator.MutateOne(corpusData, MUTATOR_CROSSOVER_LEVEL, mutant);
        
            CanFrameSample canSample;
            canSample.getCanSampleFrame()->setFlexibleDataRateFormat(isCanFd);
            if (isIdMutate) {
                frameId = distribCanId(randCanId);
            }
            canSample.getCanSampleFrame()->setFrameId(frameId);
            QByteArray framePayload((const char*)mutant.data(), mutant.size());
            canSample.getCanSampleFrame()->setPayload(framePayload);

            /*if (isCanFd) {
                if (framePayload.size() % 2 != 0) {
                   const int flags = framePayload.data().left(1);
                   frame.setBitrateSwitch(flags & BitrateSwitchFlag);
                    frame.setErrorStateIndicator(flags & ErrorStateIndicatorFlag);
               }
            }*/

            if(cutil.sendFrame(*canSample.getCanSampleFrame())) {
                canSample.printSampleToOutput(m_output);
                QString filename= QStringLiteral("%1/%2_%3").arg(outputpath).arg(m_options.samplePrefix).arg(round);
                canSample.printSampleToFile(filename);

                mutil.startEveryTimeMonitoring(cutil);
	        }

            // TODO add mutant to corpus;
        }
        corpusData = mutants;
        mutants.clear();

        round++;
        iterations = iterations > MUTATOR_CYCLE_COUNTS ? iterations - MUTATOR_CYCLE_COUNTS: 0;
    }

    return true;
}

bool FuzzerUtil::startReplaying(CanBusUtil &cutil) {

    cutil.connectCanDevice();
    ByteArrayList corpusData;
    getInitialCorpus(corpusData); // TODO 需要根据 isCanFd 来筛选语料库中的Can/CanFd报文

    uint frameId = 0;
    // TODO: get frameid
    uint isCanFd = cutil.getConfigurationParameter(QCanBusDevice::CanFdKey);
    for (qsizetype i = 0; i < (qsizetype)corpusData.size(); ++i) {
        CanFrameSample canSample;
        canSample.getCanSampleFrame()->setFlexibleDataRateFormat(isCanFd);
        canSample.getCanSampleFrame()->setFrameId(frameId);
        QByteArray framePayload((const char*)corpusData[i].data(), corpusData[i].size());
        canSample.getCanSampleFrame()->setPayload(framePayload);
        cutil.sendFrame(*canSample.getCanSampleFrame());
        canSample.printSampleToOutput(m_output);
    }

    return true;
}

void FuzzerUtil::getInitialCorpus(ByteArrayList &corpus) {

    if(!m_options.inputCorpusPath.isEmpty()) {
        QFileInfoList flist = GetFileList(m_options.inputCorpusPath);
        if(flist.isEmpty()) {
            m_output << QStringLiteral("Path %1 is invalid.").arg(m_options.inputCorpusPath) << Qt::endl;
        } else {
            ByteArrayList initialCorpus = GetMultiFileData(flist);
            corpus.insert(corpus.begin(), initialCorpus.begin(), initialCorpus.end());
        }
    }
    if(corpus.size() == 0) {
        corpus.push_back(defaultSample);
    }

    return;
}

void FuzzerUtil::getCorpusOutputPath(QString &outputpath) {

    if (m_options.inputCorpusPath.isEmpty()) {
        outputpath = QDir::currentPath();
        return;
    }
    QDir dir(m_options.inputCorpusPath);
    outputpath = m_options.inputCorpusPath;
    if (!dir.exists()) {
        dir.mkpath(m_options.inputCorpusPath);
    }
    return;
}

//bool FuzzerUtil::ConvertSample2CanBusFrame(QString &sample, QCanBusFrame &QCanBusFrame) {

//    return true;
//}

// bool FuzzerUtil::ConvertSample2CanBusFrame(QByteArray &sample, QCanBusFrame &QCanBusFrame) {

//     return true;
// }