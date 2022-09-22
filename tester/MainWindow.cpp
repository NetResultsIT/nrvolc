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

    //only on windows we have the device list so far
#ifndef WIN32
    ui->rdoUseCustDevice->setEnabled(false);
    ui->rdoUseCustDevice_In->setEnabled(false);
#endif

    QTimer *tim = new QTimer(this);
    connect(ui->btnNewVol, &QPushButton::clicked, this, &MainWindow::onNewVolumeSet);
    connect(ui->btnNewVol_In, &QPushButton::clicked, this, &MainWindow::onNewVolumeSet_In);
    connect(ui->rdoUseCustDevice, &QRadioButton::toggled, this, &MainWindow::onCustomOutputDeviceSelected);
    connect(ui->rdoUseCustDevice_In, &QRadioButton::toggled, this, &MainWindow::onCustomInputDeviceSelected);
    connect(ui->cmdDeviceList, &QComboBox::currentTextChanged, this, &MainWindow::onCustomOutputDeviceChanged);
    connect(ui->cmdDeviceList_In, &QComboBox::currentTextChanged, this, &MainWindow::onCustomInputDeviceChanged);
    connect(tim, &QTimer::timeout, this, &MainWindow::onTimeoutRead);
    tim->start(4000);

    m_OutputDeviceMap = m_pVolc->getDeviceList(NRVOLC::OUTPUT_DEVICE);
    for (auto const& x : m_OutputDeviceMap)
    {
        ui->cmdDeviceList->addItem(QString::fromStdString(x.first));
    }

    m_InputDeviceMap = m_pVolc->getDeviceList(NRVOLC::INPUT_DEVICE);
    for (auto const& x : m_InputDeviceMap)
    {
        ui->cmdDeviceList_In->addItem(QString::fromStdString(x.first));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onNewVolumeSet()
{
    double v = ui->spinNewVol->value();
    if (ui->rdoUseDefDevice->isChecked()) {
        qDebug() << "Setting default output volume to " << v;
        m_pVolc->setDefaultOutputVolume(v);
    } else {
        qDebug() << "Setting output volume to " << v << " for device " << ui->cmdDeviceList->currentIndex();
        std::string devUid = m_OutputDeviceMap.at(ui->cmdDeviceList->currentText().toStdString());
        int err = m_pVolc->setOutputDeviceVolume(devUid, v);
    }
    onTimeoutRead();
}

void MainWindow::onNewVolumeSet_In()
{
    double v = ui->spinNewVol_In->value();
    if (ui->rdoUseDefDevice_In->isChecked()) {
        qDebug() << "Setting default input volume to " << v;
        m_pVolc->setDefaultInputVolume(v);
    } else {
        qDebug() << "Setting input volume to " << v << " for device " << ui->cmdDeviceList_In->currentIndex();
        std::string devUid = m_InputDeviceMap.at(ui->cmdDeviceList_In->currentText().toStdString());
        int err = m_pVolc->setInputDeviceVolume(devUid, v);
    }
    onTimeoutRead();
}

void MainWindow::onTimeoutRead()
{
    double d;
    if (ui->rdoUseDefDevice->isChecked()) {
        qDebug() << "Reading default out device volume";
        d = m_pVolc->getDefaultOutputVolume();
    } else {
        qDebug() << "Reading volume of out device " << ui->cmdDeviceList->currentIndex();
        std::string devUid = m_OutputDeviceMap.at(ui->cmdDeviceList->currentText().toStdString());
        d = m_pVolc->getOutputDeviceVolume(devUid);
    }
    qDebug() << "Current out volume:" << d;
    ui->txtOldVol->setText(QString::number(d));

    double d1;
    if (ui->rdoUseDefDevice_In->isChecked()) {
        qDebug() << "Reading default in device volume";
        d1 = m_pVolc->getDefaultInputVolume();
    } else {
        qDebug() << "Reading volume of in device " << ui->cmdDeviceList_In->currentIndex();
        std::string devUid = m_InputDeviceMap.at(ui->cmdDeviceList_In->currentText().toStdString());
        d1 = m_pVolc->getInputDeviceVolume(devUid);
    }
    qDebug() << "Current in volume:" << d1;
    ui->txtOldVol_In->setText(QString::number(d1));
}

void MainWindow::onCustomInputDeviceSelected(bool selected)
{
    ui->cmdDeviceList_In->setEnabled(selected);
}

void MainWindow::onCustomOutputDeviceSelected(bool selected)
{
    ui->cmdDeviceList->setEnabled(selected);
}


void MainWindow::onCustomInputDeviceChanged()
{
    int idx = ui->cmdDeviceList_In->currentIndex();
    qDebug() << "selected input device " << idx;
    onTimeoutRead();
}

void MainWindow::onCustomOutputDeviceChanged()
{
    int idx = ui->cmdDeviceList->currentIndex();
    qDebug() << "selected output device " << idx;
    onTimeoutRead();
}
