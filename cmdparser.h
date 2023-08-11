#ifndef CMDPARSER_H
#define CMDPARSER_H

#include <QCommandLineParser>
#include <QTextStream>

#include "canbusutil.h"
#include "fuzzerutil.h"

enum ProgramExecutionMode
{
    EXECUTION_MODE_NONE = 0,
    EXECUTION_MODE_DISPLAY = 1,  // Display and exit
    EXECUTION_MODE_LISTENING = 2,  // Only Listening Frames
    EXECUTION_MODE_FUZZING = 3,
    EXECUTION_MODE_REPLAYING = 4,

    EXECUTION_MODE_END
};

#define SET_EXECUTION_MODE(m, v) { if(m == EXECUTION_MODE_NONE) m = v; }

void CmdParser_setOptions(QCommandLineParser &parser, QTextStream &output);
void CmdParser_helpOptionAnalyze(QCommandLineParser &parser, CanBusUtil &cutil, QTextStream &output);
bool CmdParser_argsNumberCheck(QCommandLineParser &parser, CanBusUtil &cutil, QTextStream &output);
bool CmdParser_canOptionAnalyze(QCommandLineParser &parser, CanBusUtil &cutil, ProgramExecutionMode &mode, QTextStream &output);
bool CmdParser_fuzzerOptionAnalyze(QCommandLineParser &parser, FuzzerUtil &futil, ProgramExecutionMode &mode, QTextStream &output);

void CmdParser_showHelp(QCommandLineParser &parser, CanBusUtil &cutil, QTextStream &output);


#endif // CMDPARSER_H