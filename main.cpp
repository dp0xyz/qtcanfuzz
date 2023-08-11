// Custom header file
#include "sigtermhandler.h"
#include "cmdparser.h"

// System header file
#include <signal.h>

int main(int argc, char *argv[])
{
    // Basic Application information
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("canfuzz"));
    QCoreApplication::setApplicationVersion(QStringLiteral("v0.1"));

    // Process user interrupt，etc. CTRL + C
    std::unique_ptr<SigTermHandler> s(SigTermHandler::instance());
    if (signal(SIGINT, SigTermHandler::handle) == SIG_ERR)
        return -1;
    QObject::connect(s.get(), &SigTermHandler::sigTermSignal, &app, &QCoreApplication::quit);

    // Set CommandLine Options
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Welcome to %1 %2.")
                                                    .arg(app.applicationName())
                                                    .arg(app.applicationVersion()));
    QTextStream output(stdout);
    CmdParser_setOptions(parser, output);

    // Process CommandLine Options
    parser.process(app);
    ProgramExecutionMode mode = EXECUTION_MODE_NONE;
    CanBusUtil cutil(output);
    CmdParser_helpOptionAnalyze(parser, cutil, output);
    if(!CmdParser_argsNumberCheck(parser, cutil, output)) return -1;
    if(!CmdParser_canOptionAnalyze(parser, cutil, mode, output)) return -1;
    FuzzerUtil futil(output);
    if(!CmdParser_fuzzerOptionAnalyze(parser, futil, mode, output)) return -1; 

    // Running
    switch (mode) {
        case EXECUTION_MODE_DISPLAY:
            return 0;
        case EXECUTION_MODE_LISTENING:
            cutil.startListening();
            break;
        case EXECUTION_MODE_FUZZING:
            futil.startFuzzing(cutil);
            break;
        case EXECUTION_MODE_REPLAYING:
            futil.startReplaying(cutil);
            break;
        default:
            output << Qt::endl << QStringLiteral("Invalid Execution Mode (%1 given).").arg(mode);
            CmdParser_showHelp(parser, cutil, output);
            return 0;
    }

    return app.exec();
}
