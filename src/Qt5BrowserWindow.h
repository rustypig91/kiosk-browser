#pragma once

#include <QMainWindow>
#include <QMenu>
#include <QWebSocketServer>
#include <QWidget>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebInspector>
#include <QtWebKit/QWebSettings>

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
  public:
    BrowserWindow(const QUrl &url);

  private:
    QWebView *view;
    QWebInspector *inspector;

  private slots:
    void showContextMenu(const QPoint &pos);
    void toggleDeveloperTools();
};
