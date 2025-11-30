#pragma once

#include <QMainWindow>
#include <QWebEngineView>

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
  public:
    BrowserWindow(const QUrl &url);

  private:
    QWebEngineView *view;
};
