#pragma once

#include <QMainWindow>
#include <QMenu>
#include <QWebSocketServer>
#include <QWidget>
#include <QtWebKit/QWebSettings>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebInspector>
#include <QtWebKitWidgets/QWebView>

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
  public:
    BrowserWindow(const QUrl &url, bool ignoreCertErrors = false);

  private:
    QWebView *view;
    QWebInspector *inspector;

  private slots:
#ifdef DEBUG
    void showContextMenu(const QPoint &pos);
    void toggleDeveloperTools();
#endif
};
