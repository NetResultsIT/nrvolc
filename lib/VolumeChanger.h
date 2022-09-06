#ifndef NRVOLC_LIB_H
#define NRVOLC_LIB_H

#include "lib_global.h"
#include <QObject>

class NRVOLC_LIB_EXPORT NrVolumeChanger : public QObject
{
    Q_OBJECT
public:
    NrVolumeChanger(QObject *parent=nullptr);
    virtual int setVolume(double percent) = 0;
    virtual double getVolume() const = 0;
};

#endif // LIB_H
