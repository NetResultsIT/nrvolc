#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class NrVolumeChanger;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    NrVolumeChanger *m_pVolc;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void onTimeoutRead();
    void onNewVolumeSet();
    void onNewVolumeSet_In();
};
#endif // MAINWINDOW_H
