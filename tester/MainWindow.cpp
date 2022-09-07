#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QTimer>

//TODO allow a factory to return the correct pointer
#ifdef Q_OS_MACOS
#include "NrVolumeChangerMac.h"
#endif

#ifdef Q_OS_WIN
#include "NrVolumeChangerWin.h"
#endif

#ifdef Q_OS_LINUX
#include "NrVolumeChangerLinux.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Volume Changer");
#ifdef Q_OS_MAC
    m_pVolc = new NrVolumeChangerMacImpl();
#elif defined( Q_OS_WIN )
    m_pVolc = new NrVolumeChangerWinImpl();
#include "NrVolumeChangerWin.h"
#elif defined( Q_OS_LINUX )
    m_pVolc = new NrVolumeChangerLinuxImpl();
#endif
    onTimeoutRead();

    QTimer *tim = new QTimer(this);
    connect(ui->btnNewVol, &QPushButton::clicked, this, &MainWindow::onNewVolumeSet);
    connect(ui->btnNewVol_In, &QPushButton::clicked, this, &MainWindow::onNewVolumeSet_In);
    connect(tim, &QTimer::timeout, this, &MainWindow::onTimeoutRead);
    tim->start(4000);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onNewVolumeSet()
{
    double v = ui->spinNewVol->value();
    qDebug() << "Setting output volume to " << v;
    m_pVolc->setDefaultOutputVolume(v);
    onTimeoutRead();
}

void MainWindow::onNewVolumeSet_In()
{
    double v = ui->spinNewVol_In->value();
    qDebug() << "Setting input volume to " << v;
    m_pVolc->setDefaultInputVolume(v);
    onTimeoutRead();
}

void MainWindow::onTimeoutRead()
{
    double d = m_pVolc->getDefaultOutputVolume();
    qDebug() << "Current out volume:" << d;
    ui->txtOldVol->setText(QString::number(d));
    double d1 = m_pVolc->getDefaultInputVolume();
    qDebug() << "Current in volume:" << d1;
    ui->txtOldVol_In->setText(QString::number(d1));
}
