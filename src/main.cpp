#include <QApplication>
#include <cstring>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "Qt6BrowserWindow.h"
#else
#include "Qt5BrowserWindow.h"
#endif

int main(int argc, char *argv[])
{
    // Parse our app-specific flags BEFORE creating QApplication because Qt/QtWebEngine
    // may consume/remove Chromium-style "--" switches from the argument list.
    bool noCheckCert = false;
    QVector<char *> filtered;
    filtered.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        if (i == 0)
        {
            filtered.push_back(argv[i]);
        }
        else if (std::strcmp(argv[i], "--ignore-certificate-errors") == 0)
        {
            noCheckCert = true;
        }
        else if (std::strncmp(argv[i], "--help", 6) == 0 || std::strcmp(argv[i], "-h") == 0)
        {
            qInfo("Usage: %s [--ignore-certificate-errors] <url>\n"
                  "  --ignore-certificate-errors   Ignore SSL certificate errors (not recommended)\n"
                  "  -h, --help                    Show this help message\n",
                  argv[0]);
            return 0;
        }
        else
        {
            filtered.push_back(argv[i]);
        }
    }

    int newArgc = filtered.size();
    QApplication app(newArgc, filtered.data());

    // Ensure stable AppDataLocation for Qt6 persistence
    QCoreApplication::setOrganizationName("rustypig91");
    QCoreApplication::setApplicationName("kiosk-browser");

    // Find the first http(s) URL among remaining args
    QString url;
    for (int i = 1; i < newArgc; ++i)
    {
        const QString arg = QString::fromLocal8Bit(filtered[i]);
        if (arg.startsWith("http"))
        {
            qInfo("Using URL: %s", arg.toUtf8().constData());
            url = arg;
            break;
        }
    }

    if (url.isEmpty())
    {
        qCritical("Error: Please provide a valid URL as an argument.");
        return 1;
    }

    if (noCheckCert)
    {
        qWarning("Warning: SSL certificate errors will be ignored.");
    }

    BrowserWindow window(QUrl{url}, noCheckCert);
    window.show();

    return app.exec();
}
