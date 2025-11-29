#include "BrowserWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Enforce URL is provided as a command line argument
    if (argc < 2 || !QUrl(argv[1]).isValid())
    {
        qCritical("Error: Please provide a valid URL as the first argument.");
        return 1;
    }

    QStringList arguments = app.arguments();
    QString url = nullptr;

    for (const QString &arg : arguments)
    {
        if (arg.startsWith("http"))
        {
            url = arg;
            break;
        }
    }

    if (url == nullptr)
    {
        qCritical("Error: No valid URL found in arguments.");
        return 1;
    }

    BrowserWindow window(QUrl{url});
    window.show();

    return app.exec();
}
