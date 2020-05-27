/*!
     Copyright 2020. Coding Studio. All rights reserved.
     BSD License Usage

     [Coding Studio]
     Git     : https://github.com/devghframework/OpenWorld2D
     email   : devlee.freebsd@gmail.com
     twitch  : https://www.twitch.tv/codingstudio
     youtube : https://www.youtube.com/channel/UCMj3LpAxKiBmPeScDkan0sg?view_as=subscriber
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "isometric.h"
#include "maincamera.h"
#include "tilemap.h"
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/*!
 * \class MainWindow
 * \brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void showEvent(QShowEvent *event) override;


private slots:
    void on_cboBackgroundColor_currentIndexChanged(int index);
    void mornitoringTimer();

    void on_btnLoadBackgroundImage_clicked();
    void on_chkShowDefaultTileMap_stateChanged(int arg1);
    void on_chkShowTileLine_stateChanged(int arg1);
    void on_chkShowDataMap_stateChanged(int arg1);
    void on_chkScroll_stateChanged(int arg1);


private:
    Ui::MainWindow *m_ui;

    QTimer *timer;
};
#endif // MAINWINDOW_H
