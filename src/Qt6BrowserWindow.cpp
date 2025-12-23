#include "Qt6BrowserWindow.h"
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <qapplication.h>

#ifndef REMOTE_DEBUGGING_PORT
#define REMOTE_DEBUGGING_PORT "5001"
#endif

BrowserWindow::BrowserWindow(const QUrl &url) : QMainWindow(nullptr)
{
    // Optimize QtWebEngine performance by setting Chromium flags
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--disable-gpu-vsync --disable-smooth-scrolling --disable-accelerated-2d-canvas "
            "--disable-background-timer-throttling --disable-renderer-backgrounding "
            "--disable-features=PaintHolding,SitePerProcess,TouchpadOverscrollHistoryNavigation "
            "--no-sandbox");
#ifdef DEBUG
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "0.0.0.0:" REMOTE_DEBUGGING_PORT);
#else
    QApplication::setOverrideCursor(Qt::BlankCursor);
    QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
    setCursor(Qt::BlankCursor);
#endif
    QWebEngineProfile *profile = new QWebEngineProfile("kiosk-browser-profile");
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    view = new QWebEngineView(profile);
    view->setUrl(url);

    qDebug("PersistentStoragePath: %s", profile->persistentStoragePath().toUtf8().constData());
    qDebug("CachePath: %s", profile->cachePath().toUtf8().constData());

    setCentralWidget(view);
    setWindowState(Qt::WindowFullScreen | Qt::WindowActive);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(view, &QWebEngineView::loadFinished, this,
            [this](bool ok)
            {
                if (!ok)
                {
                    view->setHtml("<h1>Failed to load page</h1>");
                }
            });
}
