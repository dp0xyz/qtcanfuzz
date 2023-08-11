#include "cmdparser.h"

QStringList progExecuteModeDesc = {
    "EXECUTION_MODE_NONE         ",
    "EXECUTION_MODE_QUERY        display information and exit.",
    "EXECUTION_MODE_LISTENING    only listening frames. when --listen is set.",
    "EXECUTION_MODE_FUZZING      fuzz testing.",
    "EXECUTION_MODE_REPLAYING      replay testing."
};

void CmdParser_setOptions(QCommandLineParser &parser, QTextStream &output)
{
    // parser.addHelpOption(); //使用自定义的helpoption
    parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("plugin"), 
            QStringLiteral("Plugin type to use."));
    parser.addPositionalArgument(QStringLiteral("device"), 
            QStringLiteral("Interface to use. Query available can interfaces through [pluginName --list-device]"));
    parser.addPositionalArgument(QStringLiteral("corpus"), 
            QStringLiteral("Effective in fuzz mode. If it is a file, as the initial sample of fuzz testing. If it is a directory, read all the files in the directory as the initial samples, and store the sample files generated during fuzz testing in this directory."),
            QStringLiteral("[corpus]"));

    parser.addOptions({

        // can base options
        {{"h", "help"},
            QStringLiteral("Displays help on commandline options.")},
        {"list-device",
            QStringLiteral("Displays available CAN bus interfaces for the given plugin.")},
        {{"l", "listen"},
            QStringLiteral("Only listening CAN data on interface.")},
        {"canfd",
            QStringLiteral("Enable CAN FD functionality when listening.")},
        {"bitrate",
            QStringLiteral("Set the CAN bus bitrate to the given value. Unit Bit/s. --bitrate=500000"),
            QStringLiteral("bitrate")},

        // fuzzer options
        {{"f", "fuzz"},
            QStringLiteral("Start fuzzing.")},
        {"iters",
            QStringLiteral("Fuzzing Options. Number of individual test runs (-1 for infinite runs)."),
            QStringLiteral("iters")},
        {"maxlen",
            QStringLiteral("Fuzzing Options. Maximum length of the test input. If 0, try to guess a good value based on the corpus and reports it."),
            QStringLiteral("maxlen")},
        {"lenctrl",
            QStringLiteral("Fuzzing Options. Try generating small inputs first, then try larger inputs over time.  Specifies the rate at which the length limit is increased (smaller == faster).  If 0, immediately try inputs with size up to max_len."),
            QStringLiteral("lenctrl")},
        {"timeout",
            QStringLiteral("Fuzzing Options. If one unit runs more than this number of seconds, regard the unit as crash."),
            QStringLiteral("timeout")},
        {"totaltime",
            QStringLiteral("Fuzzing Options. The maximal total time in seconds to run the fuzzer."),
            QStringLiteral("iterations")},
        {"slowunit",
            QStringLiteral("Fuzzing Options. Report slow units if they run for more than this number of seconds."),
            QStringLiteral("slowunit")},
        {"stats",
            QStringLiteral("Fuzzing Options. If 1, print statistics at exit."),
            QStringLiteral("stats")},
        {"sample_prefix",
            QStringLiteral("Fuzzing Options. Name prefix of the output sample file"),
            QStringLiteral("sample_prefix")},
        {"mutate",
            QStringLiteral("Fuzzing Options. Open mutate mode. Default is all, if none, close mutate. --mutate=all/fast/none"),
            QStringLiteral("mutate")},
        {"seed",
            QStringLiteral("Fuzzing Options. Mutate seed, with the same seed value will produce the same number sequence."),
            QStringLiteral("seed")},
        {"mutate_depth",
            QStringLiteral("Fuzzing Options. Apply this number of consecutive mutations to each input"),
            QStringLiteral("mutate_depth")},
        {"canid",
            QStringLiteral("Fuzzing with a fixed ID"),
            QStringLiteral("canid")},
        {"canidmin",
            QStringLiteral("Set the minimum value of ID range"),
            QStringLiteral("canidmin")},
        {"canidmax",
            QStringLiteral("Set the maximum value of ID range"),
            QStringLiteral("canidmax")},
        {"diagid",
            QStringLiteral("Set the diagnostic ID to monitor exceptions. Default 0x7DF"),
            QStringLiteral("diagid")},
        {"interval",
            QStringLiteral("Interval time of each fuzzy message test, and the total waiting time for exceptions detection. Unit milliseconds, default 50ms."),
            QStringLiteral("interval")},
        
        {{"r", "replay"},
            QStringLiteral("Replay known corpus or single sample. When replaying, [corpus] argutment is required.")},
        
    });

    return;
}

void CmdParser_helpOptionAnalyze(QCommandLineParser &parser, CanBusUtil &cutil, QTextStream &output)
{
    if (parser.isSet("help")) {
        CmdParser_showHelp(parser, cutil, output); // display and exit
    }
    return;
}

bool CmdParser_argsNumberCheck(QCommandLineParser &parser, CanBusUtil &cutil, QTextStream &output)
{
    const QStringList posArgs = parser.positionalArguments();
    if ((posArgs.size() < 1)
        || (posArgs.size() > 3)
        || (posArgs.size() == 1 && (!parser.isSet("list-device")))
        || (parser.isSet("replay") && posArgs.size() != 3)
        ) 
    {
        output << Qt::endl << QStringLiteral("Invalid number of arguments (%1 given).").arg(posArgs.size());
        CmdParser_showHelp(parser, cutil, output);  // display and exit
        return false;
    }

    return true;
}

/**
 * Process CAN Optional Arguments and Positional Arguments. 
 * The Optional Arguments are usually used to query or configure.
 * When used to query, The program should exit after the query is completed.
 **/
bool CmdParser_canOptionAnalyze(QCommandLineParser &parser, CanBusUtil &cutil, ProgramExecutionMode &mode, QTextStream &output)
{
    // Optional Arguments
    if (parser.isSet("listen")) {
        cutil.setListeningOption(true);
        SET_EXECUTION_MODE(mode, EXECUTION_MODE_LISTENING);
    }
    
    if (parser.isSet("canfd"))
        cutil.setConfigurationParameter(QCanBusDevice::CanFdKey, true);

    if (parser.isSet("bitrate"))
        cutil.setConfigurationParameter(QCanBusDevice::BitRateKey, parser.value("bitrate").toInt());

    // Positional Arguments
    const QStringList posArgs = parser.positionalArguments();

    // plugin --list_device. Exit after query.
    if (posArgs.size() == 1 && parser.isSet("list-device")) {
        cutil.printDevices(posArgs.at(0));
        SET_EXECUTION_MODE(mode, EXECUTION_MODE_DISPLAY);
    }

    // plugin device
    if (posArgs.size() >= 2) {
        cutil.setPluginName(posArgs.at(0));
        cutil.setDeviceName(posArgs.at(1));
    }

    return true;
}

bool CmdParser_fuzzerOptionAnalyze(QCommandLineParser &parser, FuzzerUtil &futil, ProgramExecutionMode &mode, QTextStream &output)
{
    FuzzingOptions fopt;
    const QStringList posArgs = parser.positionalArguments();

    if (parser.isSet("fuzz")) {
        SET_EXECUTION_MODE(mode, EXECUTION_MODE_FUZZING);

        if (parser.isSet("iters")) 
            fopt.iterations = parser.value("iters").toUInt();
        if (parser.isSet("maxlen")) 
            fopt.maxPduLen = parser.value("maxlen").toUInt();
        if (parser.isSet("lenctrl")) 
            fopt.lenControl = parser.value("lenctrl").toUInt();
        if (parser.isSet("timeout")) 
            fopt.unitTimeout = parser.value("timeout").toUInt();
        if (parser.isSet("totaltime")) 
            fopt.maxTotalTime = parser.value("totaltime").toUInt();
        if (parser.isSet("slowunit")) 
            fopt.slowUnits = parser.value("slowunit").toUInt();
        if (parser.isSet("stats")) 
            fopt.statsFlag = (parser.value("stats").toUInt() != 0);
        if (parser.isSet("sample_prefix")) 
            fopt.samplePrefix = parser.value("sample_prefix");

        if (parser.isSet("mutate")) {
            if(parser.value("mutate") == "all") 
                fopt.mutateFlag = MUTATION_MODE_ALL;
            else if(parser.value("mutate") == "fast") 
                fopt.mutateFlag = MUTATION_MODE_FAST;
            else if(parser.value("mutate") == "none") 
                fopt.mutateFlag = MUTATION_MODE_NONE;
            else
                fopt.mutateFlag = MUTATION_MODE_ALL;
        }
        if (parser.isSet("seed"))
            fopt.seed = parser.value("seed").toUInt();
        if (parser.isSet("mutate_depth")) 
            fopt.mutateDepth = parser.value("mutate_depth").toUInt();
        if (parser.isSet("canid")) 
            fopt.frameId = parser.value("canid").toUInt(nullptr, 16);
        if (parser.isSet("canidmin")) 
            fopt.idMin = parser.value("canidmin").toUInt(nullptr, 16);
        if (parser.isSet("canidmax")) 
            fopt.idMax = parser.value("canidmax").toUInt(nullptr, 16);
        if (parser.isSet("diagid")) 
            fopt.idDiag = parser.value("diagid").toUInt(nullptr, 16);
        if (parser.isSet("interval")) 
            fopt.interval = parser.value("interval").toUInt();

        // plugin device corpus
        if (posArgs.size() == 3) {
            fopt.inputCorpusPath = posArgs.at(2);
        }
    }

    if (parser.isSet("replay")) {
        SET_EXECUTION_MODE(mode, EXECUTION_MODE_REPLAYING);

        fopt.mutateFlag = MUTATION_MODE_NONE; // Replay mode does not need mutation
        // plugin device corpus
        fopt.inputCorpusPath = posArgs.at(2);
    }

    futil.setFuzzingOptions(fopt);
   
    return true;
}

void CmdParser_showHelp(QCommandLineParser &parser, CanBusUtil &cutil, QTextStream &output)
{
    output << Qt::endl << Qt::endl << parser.helpText();

    cutil.printPlugins();

    output << QStringLiteral("----------------------------------------------------------") << Qt::endl;
    output << QStringLiteral("All supported Program Execution Mode:") << Qt::endl;
    for (auto i = 0; i < EXECUTION_MODE_END; ++i) {
        output << progExecuteModeDesc[i] << Qt::endl;
    }
    
    output << Qt::endl;
    exit(0);
    return;
}