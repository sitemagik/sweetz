/*
 * W.J. van der Laan 2011-2012
 * The PPCoin Developers 2013
 * The Paycoin Developers 2014-2015
 */
#include "sweetcoingui.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include "init.h"
#include "ui_interface.h"
#include "qtipcserver.h"
#include "splashscreen.h"

#include <QApplication>
#include <QMessageBox>
#include <QTextCodec>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/algorithm/string/predicate.hpp>

#if defined(sweetcoin_NEED_QT_PLUGINS) && !defined(_sweetcoin_QT_PLUGINS_INCLUDED)
#define _sweetcoin_QT_PLUGINS_INCLUDED
#define __INSURE__
#include <QtPlugin>
Q_IMPORT_PLUGIN(qcncodecs)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qtwcodecs)
Q_IMPORT_PLUGIN(qkrcodecs)
Q_IMPORT_PLUGIN(qtaccessiblewidgets)
#endif

// Need a global reference for the notifications to find the GUI
static sweetcoinGUI *guiref;
static SplashScreen *splashref;
static WalletModel *walletmodel;
static ClientModel *clientmodel;

int ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style)
{
    // Message from network thread
    if(guiref)
    {
        bool modal = (style & wxMODAL);
        // in case of modal message, use blocking connection to wait for user to click OK
        QMetaObject::invokeMethod(guiref, "error",
                                   modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(message)),
                                   Q_ARG(bool, modal));
    }
    else
    {
        printf("%s: %s\n", caption.c_str(), message.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
    }
    return 4;
}

bool ThreadSafeAskFee(int64 nFeeRequired, const std::string& strCaption)
{
    if(!guiref)
        return false;
    if(nFeeRequired < MIN_TX_FEE || nFeeRequired <= nTransactionFee || fDaemon)
        return true;
    bool payFee = false;

    QMetaObject::invokeMethod(guiref, "askFee", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(qint64, nFeeRequired),
                               Q_ARG(bool*, &payFee));

    return payFee;
}

void ThreadSafeHandleURI(const std::string& strURI)
{
    if(!guiref)
        return;

    QMetaObject::invokeMethod(guiref, "handleURI", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(QString, QString::fromStdString(strURI)));
}

void MainFrameRepaint()
{
    if(clientmodel)
        QMetaObject::invokeMethod(clientmodel, "update", Qt::QueuedConnection);
    if(walletmodel)
        QMetaObject::invokeMethod(walletmodel, "update", Qt::QueuedConnection);
}

void AddressBookRepaint()
{
    if(walletmodel)
        QMetaObject::invokeMethod(walletmodel, "updateAddressList", Qt::QueuedConnection);
}

void InitMessage(const std::string &message)
{
    if(splashref)
    {
        splashref->showMessage(QString::fromStdString(message), Qt::AlignBottom|Qt::AlignHCenter, QColor(55,55,55));
        QApplication::instance()->processEvents();
    }
}

void QueueShutdown()
{
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
}

/*
   Translate string to current locale using Qt.
 */
std::string _(const char* psz)
{
    return QCoreApplication::translate("sweetcoin-core", psz).toStdString();
}

/* Handle runaway exceptions. Shows a message box with the problem and quits the program.
 */
static void handleRunawayException(std::exception *e)
{
    PrintExceptionContinue(e, "Runaway exception");
    QMessageBox::critical(0, "Runaway exception", sweetcoinGUI::tr("A fatal error occurred. Paycoin can no longer continue safely and will quit.") + QString("\n\n") + QString::fromStdString(strMiscWarning));
    exit(1);
}

#ifndef sweetcoin_QT_TEST
int main(int argc, char *argv[])
{

    // Do this early as we don't want to bother initializing if we are just calling IPC
    for (int i = 1; i < argc; i++)
    {
        if (boost::algorithm::istarts_with(argv[i], "paycoin:"))
        {
            const char *strURI = argv[i];
            try {
                boost::interprocess::message_queue mq(boost::interprocess::open_only, sweetcoinURI_QUEUE_NAME);
                if(mq.try_send(strURI, strlen(strURI), 0))
                    exit(0);
                else
                    break;
            }
            catch (boost::interprocess::interprocess_exception &ex) {
                break;
            }
        }
    }

    // Internal string conversion is all UTF-8
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForTr());

    Q_INIT_RESOURCE(sweetcoin);
    QApplication app(argc, argv);

    // Install global event filter that makes sure that long tooltips can be word-wrapped
    app.installEventFilter(new GUIUtil::ToolTipToRichTextFilter(TOOLTIP_WRAP_THRESHOLD, &app));

    // Command-line options take precedence:
    ParseParameters(argc, argv);

    // ... then sweetcoin.conf:
    if (!boost::filesystem::is_directory(GetDataDir(false)))
    {
        fprintf(stderr, "Error: Specified directory does not exist\n");
        return 1;
    }
    ReadConfigFile(mapArgs, mapMultiArgs);

    // Application identification (must be set before OptionsModel is initialized,
    // as it is used to locate QSettings)
    app.setOrganizationName("Paycoin");
    app.setOrganizationDomain("paycoin.org");
    if(GetBoolArg("-testnet")) // Separate UI settings for testnet
        app.setApplicationName("Paycoin-Qt-testnet");
    else
        app.setApplicationName("Paycoin-Qt");

    // ... then GUI settings:
    OptionsModel optionsModel;

    // Get desired locale (e.g. "de_DE") from command line or use system locale
    QString lang_territory = QString::fromStdString(GetArg("-lang", QLocale::system().name().toStdString()));
    QString lang = lang_territory;
    // Convert to "de" only by truncating "_DE"
    lang.truncate(lang_territory.lastIndexOf('_'));

    QTranslator qtTranslatorBase, qtTranslator, translatorBase, translator;
    // Load language files for configured locale:
    // - First load the translator for the base language, without territory
    // - Then load the more specific locale translator

    // Load e.g. qt_de.qm
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslatorBase);

    // Load e.g. qt_de_DE.qm
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);

    // Load e.g. sweetcoin_de.qm (shortcut "de" needs to be defined in sweetcoin.qrc)
    if (translatorBase.load(lang, ":/translations/"))
        app.installTranslator(&translatorBase);

    // Load e.g. sweetcoin_de_DE.qm (shortcut "de_DE" needs to be defined in sweetcoin.qrc)
    if (translator.load(lang_territory, ":/translations/"))
        app.installTranslator(&translator);

    // Show help message immediately after parsing command-line options (for "-lang") and setting locale,
    // but before showing splash screen.
    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        GUIUtil::HelpMessageBox help;
        help.exec();
        return 1;
    }

    SplashScreen splash(QPixmap(), 0);
    if (GetBoolArg("-splash", true) && !GetBoolArg("-min"))
    {
        splash.show();
        splash.setAutoFillBackground(true);
        splashref = &splash;
    }

    app.processEvents();

    app.setQuitOnLastWindowClosed(false);

    try
    {
        // Regenerate startup link, to fix links to old versions
        if (GUIUtil::GetStartOnSystemStartup())
            GUIUtil::SetStartOnSystemStartup(true);

        sweetcoinGUI window;
        guiref = &window;
        // Set in AppInit for daemon so that RPC works properly
        fTestNet = GetBoolArg("-testnet");
        if(AppInit2())
        {
            {
                // Put this in a block, so that the Model objects are cleaned up before
                // calling Shutdown().

                optionsModel.Upgrade(); // Must be done after AppInit2

                if (splashref)
                    splash.finish(&window);

                ClientModel clientModel(&optionsModel);
                clientmodel = &clientModel;
                WalletModel walletModel(pwalletMain, &optionsModel);
                walletmodel = &walletModel;

                window.setClientModel(&clientModel);
                window.setWalletModel(&walletModel);

                // If -min option passed, start window minimized.
                if(GetBoolArg("-min"))
                {
                    window.showMinimized();
                }
                else
                {
                    window.show();
                }

                // Place this here as guiref has to be defined if we don't want to lose URIs
                ipcInit();

                // Check for URI in argv
                for (int i = 1; i < argc; i++)
                {
                    if (boost::algorithm::istarts_with(argv[i], "paycoin:"))
                    {
                        const char *strURI = argv[i];
                        try {
                            boost::interprocess::message_queue mq(boost::interprocess::open_only, sweetcoinURI_QUEUE_NAME);
                            mq.try_send(strURI, strlen(strURI), 0);
                        }
                        catch (boost::interprocess::interprocess_exception &ex) {
                        }
                    }
                }

                app.exec();

                window.hide();
                window.setClientModel(0);
                window.setWalletModel(0);
                guiref = 0;
                clientmodel = 0;
                walletmodel = 0;
            }
            // Shutdown the core and it's threads, but don't exit sweetcoin-Qt here
            Shutdown(NULL);
        }
        else
        {
            return 1;
        }
    } catch (std::exception& e) {
        handleRunawayException(&e);
    } catch (...) {
        handleRunawayException(NULL);
    }
    return 0;
}
#endif // sweetcoin_QT_TEST