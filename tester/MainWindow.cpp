#include "MainWindow.h"
#include "VolumeChanger.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setWindowTitle("Volume Changer");
  m_pVolc = NrVolumeChanger::getInstance();
  connect(ui->btnNewVol, &QPushButton::clicked, this,
          &MainWindow::onNewVolumeSet);
  connect(ui->btnNewVol_In, &QPushButton::clicked, this,
          &MainWindow::onNewVolumeSet_In);
  connect(ui->rdoUseCustDevice, &QRadioButton::toggled, this,
          &MainWindow::onCustomOutputDeviceSelected);
  connect(ui->rdoUseCustDevice_In, &QRadioButton::toggled, this,
          &MainWindow::onCustomInputDeviceSelected);
  connect(ui->cmdDeviceList, &QComboBox::currentTextChanged, this,
          &MainWindow::onCustomOutputDeviceChanged);
  connect(ui->cmdDeviceList_In, &QComboBox::currentTextChanged, this,
          &MainWindow::onCustomInputDeviceChanged);
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MainWindow::onTimeoutRead);
  timer->start(4000);
  onTimeoutRead();
}
MainWindow::~MainWindow() { delete ui; }

void MainWindow::onNewVolumeSet() {
  auto it =
      m_OutputDeviceMap.find(ui->cmdDeviceList->currentText().toStdString());
  int err =
      ui->rdoUseDefDevice->isChecked()
          ? m_pVolc->setDefaultOutputVolume(ui->spinNewVol->value())
      : it != m_OutputDeviceMap.end()
          ? m_pVolc->setOutputDeviceVolume(it->second, ui->spinNewVol->value())
          : NRVOLC_ERROR;
  if (err != NRVOLC_NO_ERROR)
    QMessageBox::warning(this, "Volume", "Failed to set output volume.");
  onTimeoutRead();
}
void MainWindow::onNewVolumeSet_In() {
  auto it =
      m_InputDeviceMap.find(ui->cmdDeviceList_In->currentText().toStdString());
  int err = ui->rdoUseDefDevice_In->isChecked()
                ? m_pVolc->setDefaultInputVolume(ui->spinNewVol_In->value())
            : it != m_InputDeviceMap.end()
                ? m_pVolc->setInputDeviceVolume(it->second,
                                                ui->spinNewVol_In->value())
                : NRVOLC_ERROR;
  if (err != NRVOLC_NO_ERROR)
    QMessageBox::warning(this, "Volume", "Failed to set input volume.");
  onTimeoutRead();
}
void MainWindow::onTimeoutRead() {
  auto refresh = [this](QComboBox *box, std::map<std::string, std::string> &map,
                        NRVOLC::DeviceType type) {
    std::map<std::string, std::string> fresh;
    if (m_pVolc->getDeviceList(fresh, type) != NRVOLC_NO_ERROR)
      return;
    if (fresh == map)
      return;
    auto old = map.find(box->currentText().toStdString());
    std::string selected = old == map.end() ? std::string() : old->second;
    bool defaultSelected = type == NRVOLC::INPUT_DEVICE
                               ? ui->rdoUseDefDevice_In->isChecked()
                               : ui->rdoUseDefDevice->isChecked();
    QSignalBlocker blocker(box);
    box->clear();
    map.swap(fresh);
    int selectedIndex = -1;
    for (const auto &entry : map) {
      box->addItem(QString::fromStdString(entry.first));
      if (!selected.empty() && entry.second == selected)
        selectedIndex = box->count() - 1;
    }
    box->setCurrentIndex(defaultSelected && selected.empty() && !map.empty()
                             ? 0
                             : selectedIndex);
  };
  refresh(ui->cmdDeviceList, m_OutputDeviceMap, NRVOLC::OUTPUT_DEVICE);
  refresh(ui->cmdDeviceList_In, m_InputDeviceMap, NRVOLC::INPUT_DEVICE);
  double value = 0;
  auto out =
      m_OutputDeviceMap.find(ui->cmdDeviceList->currentText().toStdString());
  int err = ui->rdoUseDefDevice->isChecked()
                ? m_pVolc->getDefaultOutputVolume(value)
            : out != m_OutputDeviceMap.end()
                ? m_pVolc->getOutputDeviceVolume(out->second, value)
                : NRVOLC_ERROR;
  ui->txtOldVol->setText(err == NRVOLC_NO_ERROR ? QString::number(value)
                                                : "Error");
  auto in =
      m_InputDeviceMap.find(ui->cmdDeviceList_In->currentText().toStdString());
  err = ui->rdoUseDefDevice_In->isChecked()
            ? m_pVolc->getDefaultInputVolume(value)
        : in != m_InputDeviceMap.end()
            ? m_pVolc->getInputDeviceVolume(in->second, value)
            : NRVOLC_ERROR;
  ui->txtOldVol_In->setText(err == NRVOLC_NO_ERROR ? QString::number(value)
                                                   : "Error");
}
void MainWindow::onCustomInputDeviceSelected(bool selected) {
  ui->cmdDeviceList_In->setEnabled(selected);
}
void MainWindow::onCustomOutputDeviceSelected(bool selected) {
  ui->cmdDeviceList->setEnabled(selected);
}
void MainWindow::onCustomInputDeviceChanged() { onTimeoutRead(); }
void MainWindow::onCustomOutputDeviceChanged() { onTimeoutRead(); }
