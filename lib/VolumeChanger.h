#ifndef NRVOLC_LIB_H
#define NRVOLC_LIB_H

#include "lib_global.h"
#include <QObject>

class NRVOLC_LIB_EXPORT NrVolumeChanger : public QObject
{
    Q_OBJECT
public:
    NrVolumeChanger(QObject *parent=nullptr);
    virtual int setDefaultInputVolume(double percent) = 0;
    virtual double getDefaultInputVolume() const = 0;
    virtual int setDefaultOutputVolume(double percent) = 0;
    virtual double getDefaultOutputVolume() const = 0;
    static NrVolumeChanger* getInstance();
};

#endif // LIB_H
