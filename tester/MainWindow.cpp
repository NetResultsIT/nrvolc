#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QTimer>

#include "VolumeChanger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Volume Changer");

    m_pVolc = NrVolumeChanger::getInstance();
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
