#include "Qt6BrowserWindow.h"

BrowserWindow::BrowserWindow(const QUrl &url) : QMainWindow(nullptr)
{
    // Optimize QtWebEngine performance by setting Chromium flags
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--disable-gpu-vsync --disable-smooth-scrolling --disable-accelerated-2d-canvas "
            "--disable-background-timer-throttling --disable-renderer-backgrounding "
            "--disable-features=PaintHolding,SitePerProcess,TouchpadOverscrollHistoryNavigation "
            "--no-sandbox");

    view = new QWebEngineView(this);
    view->setUrl(url);

    setCentralWidget(view);
    setWindowState(Qt::WindowFullScreen | Qt::WindowActive);
    // setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(view, &QWebEngineView::loadFinished, this,
            [this](bool ok)
            {
                if (!ok)
                {
                    view->setHtml("<h1>Failed to load page</h1>");
                }
            });
}
