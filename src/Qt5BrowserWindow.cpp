#include "Qt5BrowserWindow.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QWebSocket>
#include <QWebSocketServer>


namespace
{
class PersistentCookieJar : public QNetworkCookieJar
{
  public:
    explicit PersistentCookieJar(const QString &filePath, QObject *parent = nullptr)
        : QNetworkCookieJar(parent), m_filePath(filePath)
    {
        load();
    }

    void load()
    {
        QFile f(m_filePath);
        if (!f.open(QIODevice::ReadOnly))
            return;

        QList<QNetworkCookie> list;
        while (!f.atEnd())
        {
            const QByteArray line = f.readLine().trimmed();
            if (line.isEmpty())
                continue;
            const auto parsed = QNetworkCookie::parseCookies(line);
            for (const auto &c : parsed)
                list.push_back(c);
        }
        setAllCookies(list);
    }

    void save() const
    {
        QSaveFile f(m_filePath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return;

        const auto list = allCookies();
        for (const auto &c : list)
        {
            f.write(c.toRawForm());
            f.write("\n");
        }
        f.commit();
    }

  private:
    QString m_filePath;
};
} // namespace

BrowserWindow::BrowserWindow(const QUrl &url) : QMainWindow(nullptr)
{
    view = new QWebView(this);

    // Enable persistent cookies via a custom cookie jar
    const QString baseDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(baseDataDir);
    const QString cookiesFile = baseDataDir + "/cookies.txt";

    qDebug("Using cookie file: %s\n", cookiesFile.toUtf8().constData());

    auto *nam = view->page()->networkAccessManager();
    auto *jar = new PersistentCookieJar(cookiesFile, nam);
    nam->setCookieJar(jar);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, jar, [jar]() { jar->save(); });

    view->setRenderHint(QPainter::SmoothPixmapTransform, false);
    view->setRenderHint(QPainter::TextAntialiasing, false);
    view->setRenderHint(QPainter::Antialiasing, false);

    view->setUrl(url);

    setCentralWidget(view);
    setWindowState(Qt::WindowFullScreen | Qt::WindowActive);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

#ifndef DEBUG
    QApplication::setOverrideCursor(Qt::BlankCursor);
#else
    view->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif

    connect(view, &QWebView::loadFinished, this,
            [this](bool ok)
            {
                if (!ok)
                {
                    view->setHtml("<h1>Failed to load page</h1>");
                }
            });
}
