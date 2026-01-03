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

        if (!QFile::exists(m_filePath))
        {
            QFile create(m_filePath);
            if (create.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                create.close();
                qInfo("Created new cookie file: %s", m_filePath.toUtf8().constData());
            }
            else
            {
                qWarning("Cookie load failed: cannot create %s", m_filePath.toUtf8().constData());
                return;
            }
        }

        QFile f(m_filePath);
        if (!f.open(QIODevice::ReadOnly))
        {
            qWarning("Cookie load failed: cannot open %s", m_filePath.toUtf8().constData());
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
        qDebug("Saving %d cookies to %s", list.size(), m_filePath.toUtf8().constData());
        if (!f.commit())
        {
            qWarning("Cookie save failed: commit error for %s", m_filePath.toUtf8().constData());
        }
    }

  private:
    QString m_filePath;
};
} // namespace

BrowserWindow::BrowserWindow(const QUrl &url) : QMainWindow(nullptr)
{
    view = new QWebView(this);

    // Enable persistent cookies via a custom cookie jar
    QString baseDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDataDir.isEmpty())
    {
        // Fallback to generic data location when AppDataLocation cannot be resolved
        QString generic = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        if (!generic.isEmpty())
        {
            baseDataDir = QDir(generic).filePath(QCoreApplication::organizationName() + "/" +
                                                 QCoreApplication::applicationName());
        }
    }

    bool madePath = !baseDataDir.isEmpty() && QDir().mkpath(baseDataDir);
    const QString cookiesFile = QDir(baseDataDir).filePath("cookies.txt");

    qInfo("Using cookie file: %s", cookiesFile.toUtf8().constData());
    if (!madePath)
    {
        qWarning("Failed to create cookie directory: %s", baseDataDir.toUtf8().constData());
    }

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
    // QApplication::setOverrideCursor(Qt::BlankCursor);

    connect(view, &QWebView::loadFinished, this,
            [this](bool ok)
            {
                if (!ok)
                {
                    view->setHtml("<h1>Failed to load page</h1>");
                }
            });
#ifdef DEBUG
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