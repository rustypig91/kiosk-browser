#include "BrowserWindow.h"

BrowserWindow::BrowserWindow(const QUrl &url) : QMainWindow(nullptr)
{
    view = new QWebEngineView(this);
    view->setUrl(url);

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
