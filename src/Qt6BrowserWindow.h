#pragma once

#include <QMainWindow>
#include <QWebEngineView>

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
  public:
    BrowserWindow(const QUrl &url, bool ignoreCertErrors = false);

  private:
    QWebEngineView *view;
};
