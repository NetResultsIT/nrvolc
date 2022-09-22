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
    std::map<std::string, std::string> m_InputDeviceMap;
    std::map<std::string, std::string> m_OutputDeviceMap;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void onTimeoutRead();
    void onNewVolumeSet();
    void onNewVolumeSet_In();
    void onCustomInputDeviceSelected(bool);
    void onCustomOutputDeviceSelected(bool);
    void onCustomInputDeviceChanged();
    void onCustomOutputDeviceChanged();
};
#endif // MAINWINDOW_H
