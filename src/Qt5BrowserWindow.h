#pragma once

#include <QMainWindow>
#include <QMenu>
#include <QWebSocketServer>
#include <QWidget>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebView>

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
  public:
    BrowserWindow(const QUrl &url);

  private:
    QWebView *view;
};
