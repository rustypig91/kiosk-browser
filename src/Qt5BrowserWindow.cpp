#include "Qt5BrowserWindow.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkProxy>
#include <QProcess>
#include <QTimer>
#include <QWebSocket>
#include <QWebSocketServer>

BrowserWindow::BrowserWindow(const QUrl &url) : QMainWindow(nullptr)
{
    view = new QWebView(this);

    view->setRenderHint(QPainter::SmoothPixmapTransform, false);
    view->setRenderHint(QPainter::TextAntialiasing, false);
    view->setRenderHint(QPainter::Antialiasing, false);

    // Show Web Inspector
    view->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

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
}
