#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QTimer>

#include "NrVolumeChangerMac.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Volume Changer");
    m_pVolc = new NrVolumeChangerMacImpl();
    onTimeoutRead();

    QTimer *tim = new QTimer(this);
    connect(ui->btnNewVol, &QPushButton::clicked, this, &MainWindow::onNewVolumeSet);
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
    qDebug() << "Setting volume to " << v;
    m_pVolc->setVolume(v);
    onTimeoutRead();
}

void MainWindow::onTimeoutRead()
{
    double d = m_pVolc->getVolume();
    qDebug() << "Current volume:" << d;
    ui->txtOldVol->setText(QString::number(d));
}
