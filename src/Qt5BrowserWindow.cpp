#include "Qt5BrowserWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QProcess>
#include <QSaveFile>
#include <QSslError>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>
#include <QWebSocketServer>

namespace
{
class PersistentCookieJar : public QNetworkCookieJar
{
  public:
    explicit PersistentCookieJar(QObject *parent = nullptr)
        : QNetworkCookieJar(parent), m_saveTimer(new QTimer(this)), m_saveDebounceMs(1000)
    {
        QString baseDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(baseDataDir);
        m_filePath = QDir(baseDataDir).filePath("cookies.txt");
        qInfo("Using cookie file: %s", m_filePath.toUtf8().constData());

        m_saveTimer->setSingleShot(true);
        QObject::connect(m_saveTimer, &QTimer::timeout, this, [this]() { save(); });

        load();
    }

    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url) override
    {
        const bool accepted = QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
        if (accepted)
            scheduleSave();
        return accepted;
    }

    void load()
    {
        if (!QFile::exists(m_filePath))
        {
            qInfo("Cookie file does not exist yet: %s", m_filePath.toUtf8().constData());
            return;
        }

        QFile f(m_filePath);
        if (!f.open(QIODevice::ReadOnly))
        {
            qWarning("Cookie load failed: cannot open %s", m_filePath.toUtf8().constData());
            return;
        }

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

        qDebug("Loaded %d cookies from %s", list.size(), m_filePath.toUtf8().constData());
    }

    void save() const
    {
        QSaveFile f(m_filePath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qWarning("Cookie save failed: cannot open %s", m_filePath.toUtf8().constData());
            return;
        }

        const auto list = allCookies();
        for (const auto &c : list)
        {
            f.write(c.toRawForm());
            f.write("\n");
        }

        if (!f.commit())
        {
            qWarning("Cookie save failed: commit error for %s", m_filePath.toUtf8().constData());
            return;
        }

        qDebug("Saved %d cookies to %s", list.size(), m_filePath.toUtf8().constData());
    }

  private:
    QString m_filePath;
    QTimer *m_saveTimer;
    int m_saveDebounceMs;

    void scheduleSave() { m_saveTimer->start(m_saveDebounceMs); }
};
} // namespace

BrowserWindow::BrowserWindow(const QUrl &url, bool ignoreCertErrors) : QMainWindow(nullptr)
{
    view = new QWebView(this);

    // Enable persistent cookies via a custom cookie jar
    auto *nam = view->page()->networkAccessManager();
    auto *jar = new PersistentCookieJar(nam);
    nam->setCookieJar(jar);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, jar, [jar]() { jar->save(); });

    // Optionally ignore SSL certificate errors for all network requests
    if (ignoreCertErrors)
    {
        QObject::connect(
            nam,
            static_cast<void (QNetworkAccessManager::*)(QNetworkReply *, const QList<QSslError> &)>(&QNetworkAccessManager::sslErrors),
            this,
            [](QNetworkReply *reply, const QList<QSslError> &errors) {
                Q_UNUSED(errors);
                if (reply)
                    reply->ignoreSslErrors();
            });
    }

    view->setRenderHint(QPainter::SmoothPixmapTransform, false);
    view->setRenderHint(QPainter::TextAntialiasing, false);
    view->setRenderHint(QPainter::Antialiasing, false);

    view->setUrl(url);

    setCentralWidget(view);
    setWindowState(Qt::WindowFullScreen | Qt::WindowActive);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(view, &QWebView::loadFinished, this,
            [this](bool ok)
            {
                if (!ok)
                {
                    view->setHtml("<h1>Failed to load page</h1>");
                }
            });

#ifndef DEBUG
    QApplication::setOverrideCursor(Qt::BlankCursor);
#else
    QWebSocketServer *consoleServer = new QWebSocketServer(QStringLiteral("Debug Console Server"),
                                                           QWebSocketServer::NonSecureMode, this);

    // Enable developer extras and context menu handling
    view->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    inspector = new QWebInspector();
    inspector->setPage(view->page());
    inspector->setWindowFlags(Qt::Window);
    inspector->resize(1024, 700);
    inspector->hide();

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, &BrowserWindow::showContextMenu);
#endif
}

#ifdef DEBUG
void BrowserWindow::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    QAction *reloadAction = menu.addAction("Reload");
    QAction *devToolsAction = menu.addAction("Developer Tools");

    QAction *chosen = menu.exec(view->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == reloadAction)
    {
        view->reload();
    }
    else if (chosen == devToolsAction)
    {
        toggleDeveloperTools();
    }
}

void BrowserWindow::toggleDeveloperTools()
{
    if (!inspector)
        return;

    if (inspector->isVisible())
        inspector->hide();
    else
        inspector->show();
}
#endif
